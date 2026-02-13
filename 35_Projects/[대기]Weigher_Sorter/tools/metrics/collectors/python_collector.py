"""Python collector.

Quick mode:
    - Count python source files only (fast, no subprocess heavy tools)

Full mode:
    - Reuse existing logic from python_coverage.py by spawning it and then
        loading the latest produced JSON report (python_metrics_*.json)
    - Extract coverage, quality issue count, code line stats (for scoring)

NOTE: We shell out instead of importing to avoid side-effects & keep isolation.
"""

from __future__ import annotations

import ast
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

from .base_collector import BaseCollector, CollectorResult


class PythonCollector(BaseCollector):
    name = "python"

    def __init__(
        self,
        src_root: Path,
        metrics_dir: Path,
        quick: bool = False,
        strict: bool = False,
    ) -> None:
        super().__init__(strict=strict)
        self.src_root = src_root
        self.quick = quick
        self.metrics_dir = metrics_dir

    def collect(self) -> CollectorResult:  # type: ignore[override]
        py_files = list(self.src_root.rglob("*.py"))

        # Quick mode: only count files
        if self.quick:
            return CollectorResult(
                file_count=len(py_files),
                quick_mode=True,
            )

        # Full mode: run python_coverage.py script to refresh metrics
        analyzer_script = self.metrics_dir / "python_coverage.py"
        if not analyzer_script.exists():
            return CollectorResult(
                error="python_coverage.py not found", file_count=len(py_files)
            )

        try:
            proc = subprocess.run(
                [sys.executable, str(analyzer_script)],
                cwd=self.metrics_dir.parent.parent,  # project root (assuming tools/metrics/...)
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",  # avoid UnicodeDecodeError noise
            )
        except Exception as e:  # pragma: no cover - defensive
            return CollectorResult(
                error=f"execution failed: {e}", file_count=len(py_files)
            )

        tests_passed = proc.returncode == 0

        # Locate latest python_metrics_*.json in reports dir
        reports_dir = self.metrics_dir / "reports"
        reports = list(reports_dir.glob("python_metrics_*.json"))
        if not reports:
            return CollectorResult(
                error="no python metrics report", file_count=len(py_files)
            )

        latest = max(reports, key=lambda p: p.stat().st_mtime)
        try:
            with latest.open(encoding="utf-8") as f:
                data = json.load(f)
        except Exception as e:  # pragma: no cover
            return CollectorResult(error=f"report read error: {e}")

        summary = data.get("summary", {})

        # If coverage looks suspiciously zero, attempt a direct fallback read of coverage.json
        coverage_percent = float(summary.get("total_coverage", 0.0) or 0.0)
        coverage_fallback_used = False
        if coverage_percent == 0.0:
            # 1차: reports_dir (metrics_dir/reports) 내 coverage.json (테스트 시나리오)
            coverage_json = self.metrics_dir / "reports" / "coverage.json"
            if not coverage_json.exists():
                # 2차: 기존 경로 (프로덕션 기본)
                coverage_json = (
                    self.metrics_dir.parent.parent
                    / "tools"
                    / "reports"
                    / "coverage.json"
                )
            try:
                if coverage_json.exists():
                    with coverage_json.open(encoding="utf-8") as cf:
                        cov_raw = json.load(cf)
                    fallback_pct = cov_raw.get("totals", {}).get("percent_covered", 0.0)
                    if fallback_pct:  # non-zero
                        coverage_percent = float(fallback_pct)
                        coverage_fallback_used = True
            except Exception:
                # Silent - we'll stay at 0 if fallback fails
                pass

        # Derive top offender files (by issue count) if quality section present
        top_offenders: list[dict[str, Any]] = []
        try:
            quality_files = data.get("quality", {}).get("files", {})
            offenders: list[tuple[str, int]] = []
            for path_str, info in quality_files.items():
                count = 0
                if isinstance(info, dict):
                    count = int(
                        info.get("issue_count") or len(info.get("issues", [])) or 0
                    )
                offenders.append((path_str, count))
            offenders.sort(key=lambda t: t[1], reverse=True)
            for p, c in offenders[:5]:
                top_offenders.append({"path": p, "issues": c})
        except Exception:
            # Non-critical
            pass

        # Docstring coverage analysis (simple AST walk)
        total_defs = 0
        documented_defs = 0
        for py in py_files:
            try:
                tree = ast.parse(py.read_text(encoding="utf-8"), filename=str(py))
                for node in ast.walk(tree):
                    if isinstance(
                        node, ast.FunctionDef | ast.AsyncFunctionDef | ast.ClassDef
                    ):
                        total_defs += 1
                        if ast.get_docstring(node):
                            documented_defs += 1
            except Exception:
                # Skip files with parse errors (they will be caught by other tooling)
                continue
        doc_coverage = (documented_defs / total_defs) if total_defs else 0.0

        # Test file count
        tests_root = self.metrics_dir.parent.parent / "tests"
        test_files = list(tests_root.glob("test_*.py")) if tests_root.exists() else []

        result = CollectorResult(
            file_count=len(py_files),
            coverage_percent=coverage_percent,
            quality_issues=summary.get("quality_issues", 0),
            code_lines=summary.get("code_lines", 0),
            total_files=summary.get("total_files", 0),
            test_file_count=len(test_files),
            tests_passed=tests_passed,
            doc_total=total_defs,
            doc_documented=documented_defs,
            doc_coverage=round(doc_coverage, 3),
            top_offenders=top_offenders,
            quick_mode=False,
        )
        if coverage_fallback_used:
            result["coverage_fallback"] = True
        # Provide stderr in case of silent failures to aid debugging (truncated)
        if not tests_passed and proc.stderr:
            result["test_stderr_tail"] = proc.stderr[-500:]
        return result

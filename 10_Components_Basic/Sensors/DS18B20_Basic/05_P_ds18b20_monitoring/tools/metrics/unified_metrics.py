#!/usr/bin/env python3
"""Unified Metrics Engine (Phase 5B Step 1 Skeleton)

Implements initial CLI + quick/full/domain-only mode wiring.
Current implementation: minimal collectors + JSON export skeleton.

Planned expansions (Phase 5B/5C):
  - Integrate existing python_coverage.py logic (coverage, complexity, quality)
  - Add Arduino metrics adapter
  - Domain runtime probes (EEPROM reload tests, protocol command diff, latency)
  - Scoring framework & fail-under gating
  - HTML/Markdown exporters
"""

from __future__ import annotations

import argparse
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

from collectors.base_collector import BaseCollector
from collectors.domain_collector import DomainCollector

# Local imports (relative paths)
from collectors.python_collector import PythonCollector
from collectors.runtime_collector import RuntimeCollector
from collectors.tooling_collector import ToolingCollector
from collectors.ui_collector import UIStabilityCollector
from exporters.html_exporter import export_html
from exporters.json_exporter import export_json
from exporters.markdown_exporter import export_markdown

SCHEMA_VERSION = 1
ENGINE_VERSION = "0.1.0"


@dataclass
class EngineConfig:
    mode: str
    strict: bool
    output_dir: Path
    fail_under: float | None
    formats: list[str]
    trend_enabled: bool
    profile_latency: bool


class UnifiedMetricsEngine:
    def __init__(self, config: EngineConfig) -> None:
        self.config = config
        self.project_root = Path.cwd()
        self.src_python = self.project_root / "src" / "python"
        self.reports_dir = config.output_dir
        # Directory where metrics_result_*.json are stored
        self.metrics_reports_dir = self.project_root / "tools" / "metrics" / "reports"

    # Added stub for type checkers; actual implementation is bound dynamically below
    def _compute_trend(
        self, current_total: float, raw: dict[str, Any]
    ) -> dict[str, Any] | None:  # pragma: no cover
        return None

    # Collector selection based on mode
    def _build_collectors(self) -> list[BaseCollector]:
        collectors: list[BaseCollector] = []
        quick = self.config.mode == "quick"

        # Python collector always in quick/full; skip in domain-only
        if self.config.mode in {"quick", "full"}:
            collectors.append(
                PythonCollector(
                    self.src_python,
                    metrics_dir=self.project_root / "tools" / "metrics",
                    quick=quick,
                    strict=self.config.strict,
                )
            )

        # Domain collector always included except export-schema
        if self.config.mode in {"quick", "full", "domain-only"}:
            collectors.append(
                DomainCollector(self.project_root, strict=self.config.strict)
            )

        # Runtime collector (latency) only in full mode or when explicitly requested
        if self.config.mode == "full" or self.config.profile_latency:
            collectors.append(RuntimeCollector(strict=self.config.strict))
        # Tooling & automation only meaningful in full mode
        if self.config.mode == "full":
            collectors.append(
                ToolingCollector(self.project_root, strict=self.config.strict)
            )
            collectors.append(
                UIStabilityCollector(
                    self.src_python / "dashboard", strict=self.config.strict
                )
            )
        # Placeholder: future Arduino collector insertion point
        return collectors

    def run(self) -> dict[str, Any]:
        collectors = self._build_collectors()
        raw: dict[str, Any] = {}
        errors: list[str] = []
        warnings: list[str] = []

        for collector in collectors:
            result = collector.safe_collect()
            raw[collector.name] = result
            if "error" in result:
                warnings.append(f"collector:{collector.name} error={result['error']}")

        # Placeholder scoring (will integrate real weights)
        total_score = 0.0
        breakdown: dict[str, dict[str, float]] = {}

        # Core Code Quality (25): coverage (max 12), quality issues inverse (max 13)
        # Extended decay window: 0 issues => full 13, 200+ issues => 0.
        py_raw = raw.get("python", {})
        if py_raw and not py_raw.get("error") and not py_raw.get("quick_mode"):
            coverage = float(py_raw.get("coverage_percent", 0.0))
            coverage_component = min(coverage / 50.0, 1.0) * 12  # 50% == target 12
            issues = int(py_raw.get("quality_issues", 0))
            issue_window = (
                200  # broadened so large refactors don't instantly drop score to zero
            )
            issue_component = (
                max(0.0, (issue_window - min(issues, issue_window)) / issue_window) * 13
            )
            quality_total = coverage_component + issue_component
            breakdown["core_quality"] = {
                "score": round(quality_total, 2),
                "max": 25.0,
                "coverage_component": round(coverage_component, 2),
                "issues_component": round(issue_component, 2),
            }
            total_score += quality_total
        else:
            breakdown["core_quality"] = {"score": 0.0, "max": 25.0}

        # Surface any top offender files (high issue concentration) as warnings (informational only)
        if py_raw.get("top_offenders"):
            for offender in py_raw["top_offenders"]:
                if offender.get("issues", 0) > 0:
                    warnings.append(
                        f"lint concentration: {offender['path']} -> {offender['issues']} issues"
                    )

        # Mark when coverage had to fall back to stale report
        if py_raw.get("coverage_fallback"):
            warnings.append(
                "coverage fallback used (primary coverage report missing or empty)"
            )

        # Domain Simulation & Persistence (15): persistence_rate (7), protocol_coverage (8)
        domain_raw = raw.get("domain", {})
        if domain_raw and not domain_raw.get("error"):
            persistence = float(domain_raw.get("persistence_rate", 0.0))
            protocol_cov = float(domain_raw.get("protocol_coverage", 0.0))
            persistence_component = max(0.0, min(persistence, 1.0)) * 7
            protocol_component = max(0.0, min(protocol_cov, 1.0)) * 8
            domain_total = persistence_component + protocol_component
            breakdown["domain_simulation"] = {
                "score": round(domain_total, 2),
                "max": 15.0,
            }
            total_score += domain_total
        else:
            breakdown["domain_simulation"] = {"score": 0.0, "max": 15.0}

        # Real-time & Protocol reliability (15) — latency p95 scaling
        rt_raw = raw.get("runtime", {})
        if rt_raw and not rt_raw.get("error"):
            latency_p95 = float(rt_raw.get("latency_p95_ms", 0.0))
            if latency_p95 <= 120:
                rt_score = 15.0
            elif latency_p95 >= 300:
                rt_score = 0.0
            else:
                rt_score = 15.0 * (1 - (latency_p95 - 120) / (300 - 120))
            breakdown["realtime_protocol"] = {"score": round(rt_score, 2), "max": 15.0}
            total_score += rt_score
        else:
            breakdown["realtime_protocol"] = {"score": 0.0, "max": 15.0}

        # Testing (20): coverage (12), test file ratio (5), pass status (3)
        # Rationale: We separate raw coverage contribution used in core_quality (first 12 points there) from broader testing maturity
        # to avoid double-counting we scale differently: here coverage scaled against 80% target.
        if py_raw and not py_raw.get("error") and not py_raw.get("quick_mode"):
            coverage = float(py_raw.get("coverage_percent", 0.0))
            test_files = int(py_raw.get("test_file_count", 0))
            total_files = int(py_raw.get("total_files", 0)) or 1
            tests_passed = bool(py_raw.get("tests_passed", False))
            # Coverage component (cap at 12 when reaching 80%)
            test_cov_component = min(coverage / 80.0, 1.0) * 12.0
            # Test file ratio relative to prod modules (exclude test files) simplistic heuristic
            prod_files = max(total_files - test_files, 1)
            test_file_ratio = min(test_files / prod_files, 1.0)
            test_file_component = test_file_ratio * 5.0
            pass_component = 3.0 if tests_passed else 0.0
            testing_total = test_cov_component + test_file_component + pass_component
            breakdown["testing"] = {
                "score": round(testing_total, 2),
                "max": 20.0,
                "coverage_component": round(test_cov_component, 2),
                "file_ratio_component": round(test_file_component, 2),
                "pass_component": pass_component,
            }
            total_score += testing_total
        else:
            breakdown["testing"] = {"score": 0.0, "max": 20.0}

        # Structure & Docs (10): doc coverage (8), module organization heuristic (2)
        # Heuristic: If doc_coverage >= 0.85 award full 8; else linear. Organization: if avg definitions per file within bounds.
        if py_raw and not py_raw.get("error") and not py_raw.get("quick_mode"):
            doc_cov = float(py_raw.get("doc_coverage", 0.0))  # 0..1
            doc_component = min(doc_cov / 0.85, 1.0) * 8.0
            doc_total_defs = int(py_raw.get("doc_total", 0))
            file_count = int(py_raw.get("total_files", 0)) or 1
            avg_defs = doc_total_defs / file_count
            # Ideal avg defs range 5..20 (avoid god-modules & trivial files). Score 2 if within range else scaled.
            if 5 <= avg_defs <= 20:
                org_component = 2.0
            elif avg_defs < 5:
                org_component = max(0.0, (avg_defs / 5.0) * 2.0)
            else:  # avg_defs > 20
                # linear decay: 20 ->2, 40->0
                org_component = max(0.0, (40 - min(avg_defs, 40)) / 20.0 * 2.0)
            structure_total = doc_component + org_component
            breakdown["structure_docs"] = {
                "score": round(structure_total, 2),
                "max": 10.0,
                "doc_component": round(doc_component, 2),
                "organization_component": round(org_component, 2),
            }
            total_score += structure_total
        else:
            breakdown["structure_docs"] = {"score": 0.0, "max": 10.0}

        # Placeholder blocks for remaining categories so schema is stable
        # Tooling & Automation (10): presence of scripts (4), history depth (4), security/quality config (2)
        tooling_raw = raw.get("tooling", {})
        if tooling_raw and not tooling_raw.get("error"):
            scripts_found = tooling_raw.get("scripts_found", [])
            scripts_missing = tooling_raw.get("scripts_missing", [])
            history = int(tooling_raw.get("report_history_count", 0))
            security_cfg = bool(tooling_raw.get("has_security_config", False))
            quality_dir = bool(tooling_raw.get("has_quality_dir", False))
            # Scripts component: all expected present -> 4, partial linear
            expected_total = len(scripts_found) + len(scripts_missing)
            scripts_component = 0.0
            if expected_total:
                scripts_component = (len(scripts_found) / expected_total) * 4.0
            # History component: 5+ history files -> full 4, else linear
            history_component = min(history / 5.0, 1.0) * 4.0
            config_component = (1.0 if security_cfg else 0.0) + (
                1.0 if quality_dir else 0.0
            )
            tooling_total = scripts_component + history_component + config_component
            breakdown["tooling_automation"] = {
                "score": round(tooling_total, 2),
                "max": 10.0,
                "scripts_component": round(scripts_component, 2),
                "history_component": round(history_component, 2),
                "config_component": round(config_component, 2),
            }
            total_score += tooling_total
        else:
            breakdown["tooling_automation"] = {"score": 0.0, "max": 10.0}

        # UI Stability (5): callback coverage (3), error handling in callbacks (1), logging density (1)
        ui_raw = raw.get("ui", {})
        if ui_raw and not ui_raw.get("error"):
            cb_total = int(ui_raw.get("callback_functions", 0))
            cb_errors = int(ui_raw.get("callbacks_with_error_handling", 0))
            logging_calls = int(ui_raw.get("logging_calls", 0))
            try_blocks = int(ui_raw.get("try_except_blocks", 0))
            # Callback coverage relative to modest target (>=6 callbacks -> full 3)
            cb_component = min(cb_total / 6.0, 1.0) * 3.0
            # Error handling: if >=50% callbacks have try/except -> full 1 else proportional
            error_component = 0.0
            if cb_total:
                error_component = min(cb_errors / cb_total, 1.0) * 1.0
            # Logging density: (logging_calls + try blocks) vs target 25 -> full 1
            log_metric = logging_calls + try_blocks
            logging_component = min(log_metric / 25.0, 1.0) * 1.0
            ui_total = cb_component + error_component + logging_component
            breakdown["ui_stability"] = {
                "score": round(ui_total, 2),
                "max": 5.0,
                "callbacks_component": round(cb_component, 2),
                "error_handling_component": round(error_component, 2),
                "logging_component": round(logging_component, 2),
            }
            total_score += ui_total
        else:
            breakdown["ui_stability"] = {"score": 0.0, "max": 5.0}

        # Fail-hard adjustments (partial initial implementation)
        if domain_raw and not domain_raw.get("error"):
            persistence_rate = float(domain_raw.get("persistence_rate", 0.0))
            drift_rate = float(domain_raw.get("drift_rate", 0.0))
            if persistence_rate < 1.0:
                warnings.append(f"persistence below 1.0 ({persistence_rate})")
            if drift_rate > 0.0:
                warnings.append(
                    f"configuration drift detected ({drift_rate}) -> total capped at 65"
                )
                total_score = min(total_score, 65.0)

        total_score = round(total_score, 2)

        # Trend (compare against previous metrics_result_*.json)
        trend: dict[str, Any] | None = None
        if self.config.trend_enabled:
            try:
                trend = self._compute_trend(total_score, raw)
            except Exception as e:  # pragma: no cover - defensive
                warnings.append(f"trend computation failed: {e}")
                trend = None

        data: dict[str, Any] = {
            "schema_version": SCHEMA_VERSION,
            "generated_at": datetime.now(timezone.utc).isoformat(),
            "engine": {
                "version": ENGINE_VERSION,
                "python": sys.version.split()[0],
                "mode": self.config.mode,
            },
            "scores": {
                "total": total_score,
                "breakdown": breakdown,
            },
            "raw": raw,
            "trend": trend,
            "warnings": warnings,
            "errors": errors,
        }

        return data


def parse_args(argv: list[str]) -> EngineConfig:
    parser = argparse.ArgumentParser(description="Unified Metrics Engine")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--quick", action="store_true", help="Run quick subset")
    group.add_argument(
        "--full", action="store_true", help="Run full analysis (future expansion)"
    )
    group.add_argument(
        "--domain-only", action="store_true", help="Run only domain metrics"
    )

    parser.add_argument(
        "--export-schema", action="store_true", help="Print schema (JSON) and exit"
    )
    parser.add_argument(
        "--fail-under", type=float, help="Fail if total score below threshold"
    )
    parser.add_argument(
        "--output-dir",
        default="tools/metrics/reports",
        help="Output directory for artifacts",
    )
    parser.add_argument(
        "--format", default="json", help="Comma separated export formats (json,html,md)"
    )
    parser.add_argument(
        "--no-trend", action="store_true", help="Disable trend comparison"
    )
    parser.add_argument(
        "--strict", action="store_true", help="Raise on collector errors"
    )
    parser.add_argument(
        "--profile-latency",
        action="store_true",
        help="Enable latency profiling (future)",
    )

    args = parser.parse_args(argv)

    if args.export_schema:
        # Minimal schema preview
        import json

        schema_preview = {
            "schema_version": SCHEMA_VERSION,
            "required_fields": [
                "schema_version",
                "generated_at",
                "engine",
                "scores",
                "raw",
            ],
            "engine": {"version": "string", "python": "str", "mode": "str"},
            "scores": {"total": "float", "breakdown": "object"},
            "raw": {"python": "object?", "domain": "object"},
        }
        print(json.dumps(schema_preview, indent=2, ensure_ascii=False))
        sys.exit(0)

    mode = "quick"
    if args.full:
        mode = "full"
    elif args.domain_only:
        mode = "domain-only"

    config = EngineConfig(
        mode=mode,
        strict=args.strict,
        output_dir=Path(args.output_dir),
        fail_under=args.fail_under,
        formats=[f.strip() for f in args.format.split(",") if f.strip()],
        trend_enabled=not args.no_trend,
        profile_latency=args.profile_latency,
    )
    return config


def main(argv: list[str] | None = None) -> int:
    argv = argv or sys.argv[1:]
    config = parse_args(argv)
    engine = UnifiedMetricsEngine(config)
    data = engine.run()

    # Export formats
    if "json" in config.formats:
        out_json = export_json(data, config.output_dir)
        print(f"JSON metrics written: {out_json}")
    if "md" in config.formats or "markdown" in config.formats:
        out_md = export_markdown(data, config.output_dir)
        print(f"Markdown metrics written: {out_md}")
    if "html" in config.formats:
        out_html = export_html(data, config.output_dir)
        print(f"HTML metrics written: {out_html}")

    total = data["scores"]["total"]
    if config.fail_under is not None and total < config.fail_under:
        print(f"Fail-under threshold not met: {total:.2f} < {config.fail_under}")
        return 1
    return 0


# ----- Trend utilities -----
def _list_previous_results(reports_dir: Path) -> list[Path]:
    if not reports_dir.exists():
        return []
    return sorted(
        reports_dir.glob("metrics_result_*.json"), key=lambda p: p.stat().st_mtime
    )


def _extract_core_fields(report_path: Path) -> dict[str, Any]:
    import json as _json

    with report_path.open(encoding="utf-8") as f:
        data = _json.load(f)
    return {
        "total": data.get("scores", {}).get("total"),
        "coverage": data.get("raw", {}).get("python", {}).get("coverage_percent"),
        "issues": data.get("raw", {}).get("python", {}).get("quality_issues"),
        "timestamp": data.get("generated_at"),
        "path": str(report_path.name),
    }


def _delta(curr: float | None, prev: float | None) -> float | None:
    if curr is None or prev is None:
        return None
    return round(curr - prev, 3)


def _percent_delta(curr: float | None, prev: float | None) -> float | None:
    if curr is None or prev is None or prev == 0:
        return None
    return round(((curr - prev) / prev) * 100.0, 2)


def _classify_trend(delta: float | None, positive_is_good: bool = True) -> str | None:
    if delta is None:
        return None
    if delta == 0:
        return "flat"
    improving = delta > 0 if positive_is_good else delta < 0
    return "improving" if improving else "regressing"


def _safe_ratio(num: float | int | None, denom: float | int | None) -> float | None:
    try:
        if num is None or denom in (None, 0):
            return None
        return round(float(num) / float(denom), 3)
    except Exception:
        return None


def unified_metrics_engine__compute_trend(
    self: UnifiedMetricsEngine, current_total: float, raw: dict[str, Any]
) -> dict[str, Any] | None:  # type: ignore
    """Compute trend information based on the immediately previous metrics_result file.

    Returns None if no previous report exists.
    """
    reports = _list_previous_results(self.metrics_reports_dir)
    if len(reports) < 1:
        return None
    prev_path = reports[-1]
    # Avoid using the just-created file (caller computes before export). If last is same mtime range, pick second last.
    # This caller executes before writing the new export, so last is truly previous.
    prev = _extract_core_fields(prev_path)

    curr_cov = raw.get("python", {}).get("coverage_percent")
    curr_issues = raw.get("python", {}).get("quality_issues")

    delta_total = _delta(current_total, prev.get("total"))
    delta_cov = _delta(curr_cov, prev.get("coverage"))
    delta_issues = _delta(
        prev.get("issues"), curr_issues
    )  # positive means fewer issues

    return {
        "previous_report": prev,
        "deltas": {
            "total_score": delta_total,
            "coverage_percent": delta_cov,
            "issues_reduced": delta_issues,
        },
        "classifications": {
            "score": _classify_trend(delta_total, positive_is_good=True),
            "coverage": _classify_trend(delta_cov, positive_is_good=True),
            "issues": _classify_trend(delta_issues, positive_is_good=True),
        },
    }


# Bind method dynamically (keeps top of file cleaner without changing earlier class definition order)
UnifiedMetricsEngine._compute_trend = unified_metrics_engine__compute_trend


if __name__ == "__main__":  # pragma: no cover
    sys.exit(main())

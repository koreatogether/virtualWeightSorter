"""Tooling & Automation collector

Captures signals about automation scripts, report history, documentation templates,
legacy tool presence, etc., to feed the tooling_automation scoring category.
"""

from __future__ import annotations

from pathlib import Path

from .base_collector import BaseCollector, CollectorResult


class ToolingCollector(BaseCollector):
    name = "tooling"

    def __init__(self, project_root: Path, strict: bool = False) -> None:
        super().__init__(strict=strict)
        self.project_root = project_root
        self.metrics_reports = project_root / "tools" / "metrics" / "reports"

    def collect(self) -> CollectorResult:  # type: ignore[override]
        scripts_expected = [
            self.project_root / "tools" / "run_all_checks.py",
            self.project_root / "tools" / "metrics" / "python_coverage.py",
            self.project_root / "tools" / "metrics" / "unified_metrics.py",
        ]
        legacy_candidates = [
            self.project_root / "tools" / "metrics" / "integrated_metrics.py",
            self.project_root / "tools" / "metrics" / "run_metrics_simple.py",
        ]

        scripts_found: list[str] = []
        scripts_missing: list[str] = []
        for p in scripts_expected:
            if p.exists():
                scripts_found.append(p.name)
            else:
                scripts_missing.append(p.name)

        legacy_present = [p.name for p in legacy_candidates if p.exists()]

        # Count history files for potential trend capability
        report_count = 0
        python_report_count = 0
        if self.metrics_reports.exists():
            report_count = len(list(self.metrics_reports.glob("metrics_result_*.json")))
            python_report_count = len(
                list(self.metrics_reports.glob("python_metrics_*.json"))
            )

        # Documentation templates (under docs/template)
        docs_template_dir = self.project_root / "docs" / "template"
        template_files = []
        if docs_template_dir.exists():
            template_files = [f.name for f in docs_template_dir.glob("*.md")]

        # Presence of security / quality automation markers
        security_config = (self.project_root / "tools" / "security").exists() or (
            self.project_root / "tools" / "security" / "temp_security_check.json"
        ).exists()
        quality_dir = (self.project_root / "tools" / "quality").exists()

        return CollectorResult(
            scripts_found=scripts_found,
            scripts_missing=scripts_missing,
            legacy_present=legacy_present,
            report_history_count=report_count,
            python_report_history_count=python_report_count,
            template_file_count=len(template_files),
            template_files=template_files[:10],
            has_security_config=security_config,
            has_quality_dir=quality_dir,
        )

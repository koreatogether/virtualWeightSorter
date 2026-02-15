import json
import subprocess
import sys
from pathlib import Path

# Ensure project root on path for `tools.metrics` imports
PROJECT_ROOT = Path(__file__).resolve().parents[1]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from tools.metrics.collectors.python_collector import PythonCollector


class DummyCompleted:
    def __init__(self):
        self.returncode = 0
        self.stdout = ""
        self.stderr = ""


def test_python_collector_coverage_fallback(monkeypatch, tmp_path: Path):
    """Simulate a python_metrics report with zero coverage + coverage.json with non-zero to trigger fallback."""
    metrics_dir = tmp_path / "tools" / "metrics"
    reports_dir = metrics_dir / "reports"
    reports_dir.mkdir(parents=True)

    (metrics_dir / "python_coverage.py").write_text("print('dummy analyzer')", encoding="utf-8")

    metrics_report = reports_dir / "python_metrics_000000.json"
    metrics_report.write_text(
        json.dumps({
            "summary": {
                "total_coverage": 0,
                "quality_issues": 3,
                "total_files": 10,
                "code_lines": 100
            },
            "quality": {"files": {}}
        }),
        encoding="utf-8"
    )

    coverage_json = reports_dir / "coverage.json"
    coverage_json.write_text(
        json.dumps({
            "totals": {"percent_covered": 57.3},
            "files": {"a.py": {"executed_lines": [1], "missing_lines": []}}
        }),
        encoding="utf-8"
    )

    def fake_run(*args, **kwargs):
        return DummyCompleted()

    monkeypatch.setattr(subprocess, "run", fake_run)

    collector = PythonCollector(src_root=tmp_path, metrics_dir=metrics_dir, quick=False)
    result = collector.collect()

    assert result["coverage_percent"] == 57.3
    assert result.get("coverage_fallback") is True
    assert result.get("tests_passed") is True

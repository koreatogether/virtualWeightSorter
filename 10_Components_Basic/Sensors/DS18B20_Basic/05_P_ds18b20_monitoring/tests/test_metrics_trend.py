import json
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
METRICS_DIR = PROJECT_ROOT / 'tools' / 'metrics'
for p in [PROJECT_ROOT, METRICS_DIR]:
    if str(p) not in sys.path:
        sys.path.insert(0, str(p))

from tools.metrics.unified_metrics import (
    EngineConfig,
    UnifiedMetricsEngine,
)


def test_trend_computation(tmp_path: Path):
    """Verify trend delta and classification logic against a synthetic previous report."""
    reports_dir = tmp_path / "reports"
    reports_dir.mkdir(parents=True)

    previous = {
        "schema_version": 1,
        "generated_at": "2025-08-18T00:00:00Z",
        "engine": {"version": "0.1.0", "python": "3.x", "mode": "quick"},
        "scores": {"total": 50.0, "breakdown": {}},
        "raw": {"python": {"coverage_percent": 40.0, "quality_issues": 120}},
    }
    prev_file = reports_dir / "metrics_result_prev.json"
    prev_file.write_text(json.dumps(previous), encoding="utf-8")

    config = EngineConfig(
        mode="quick",
        strict=False,
        output_dir=tmp_path,
        fail_under=None,
        formats=["json"],
        trend_enabled=True,
        profile_latency=False,
    )
    engine = UnifiedMetricsEngine(config)

    engine.metrics_reports_dir = reports_dir

    raw = {"python": {"coverage_percent": 60.0, "quality_issues": 100}}
    trend = engine._compute_trend(current_total=55.0, raw=raw)  # type: ignore

    assert trend is not None
    deltas = trend["deltas"]
    assert deltas["total_score"] == 5.0
    assert deltas["coverage_percent"] == 20.0
    assert deltas["issues_reduced"] == 20.0
    classes = trend["classifications"]
    assert classes["score"] == "improving"
    assert classes["coverage"] == "improving"
    assert classes["issues"] == "improving"

import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
METRICS_DIR = PROJECT_ROOT / 'tools' / 'metrics'
for p in [PROJECT_ROOT, METRICS_DIR]:
    if str(p) not in sys.path:
        sys.path.insert(0, str(p))

from tools.metrics.exporters.json_exporter import _validate


def test_validate_rejects_missing_total():
    data = {
        "schema_version": 1,
        "generated_at": "2025-08-18T00:00:00Z",
        "engine": {"version": "0.1.0"},
        "scores": {"breakdown": {}},
        "raw": {},
    }

    try:
        _validate(data)
        assert False, "Expected ValueError for missing scores.total"
    except ValueError as e:
        assert "scores.total is required" in str(e)


def test_validate_accepts_numeric_total():
    data = {
        "schema_version": 1,
        "generated_at": "2025-08-18T00:00:00Z",
        "engine": {"version": "0.1.0"},
        "scores": {"total": 42.5, "breakdown": {"python": 42.5}},
        "raw": {},
    }

    # should not raise
    _validate(data)

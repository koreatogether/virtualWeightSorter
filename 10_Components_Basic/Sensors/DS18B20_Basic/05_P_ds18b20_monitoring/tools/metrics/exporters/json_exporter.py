"""JSON exporter for unified metrics with lightweight schema validation.

Avoids external jsonschema dependency; performs structural checks based on
`tools/metrics/schema/metrics_schema_v1.json` (only required keys & simple types).
If validation fails it raises ValueError (caller can catch and convert to warning).
"""

from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

_SCHEMA_CACHE: dict[str, Any] | None = None


def _load_schema() -> dict[str, Any]:
    global _SCHEMA_CACHE
    if _SCHEMA_CACHE is not None:
        return _SCHEMA_CACHE
    schema_path = Path("tools/metrics/schema/metrics_schema_v1.json")
    try:
        with schema_path.open(encoding="utf-8") as f:
            _SCHEMA_CACHE = json.load(f)
    except FileNotFoundError:
        # Fallback minimal schema definition
        _SCHEMA_CACHE = {
            "required": ["schema_version", "generated_at", "engine", "scores", "raw"]
        }
    return _SCHEMA_CACHE  # type: ignore


def _validate(data: dict[str, Any]) -> None:
    schema = _load_schema()
    required = schema.get("required", [])
    missing = [k for k in required if k not in data]
    if missing:
        raise ValueError(f"Missing required top-level keys: {missing}")
    # Simple type checks (defensive, non-exhaustive)
    if not isinstance(data.get("schema_version"), int):
        raise ValueError("schema_version must be int")
    if not isinstance(data.get("engine"), dict):
        raise ValueError("engine must be object")
    if not isinstance(data.get("scores"), dict):
        raise ValueError("scores must be object")
    if not isinstance(data.get("raw"), dict):
        raise ValueError("raw must be object")

    # Enforce that scores.total exists and is numeric, and that breakdown is present
    scores = data.get("scores", {})
    if "total" not in scores:
        raise ValueError("scores.total is required")
    if not isinstance(scores.get("total"), int | float):
        raise ValueError("scores.total must be numeric (int or float)")
    if "breakdown" not in scores or not isinstance(scores.get("breakdown"), dict):
        raise ValueError("scores.breakdown must be an object/dict and is required")


def export_json(data: dict[str, Any], output_dir: Path) -> Path:
    # Validate before writing so invalid data does not pollute history.
    _validate(data)
    output_dir.mkdir(parents=True, exist_ok=True)
    ts = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")
    out_path = output_dir / f"metrics_result_{ts}.json"
    with out_path.open("w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    return out_path

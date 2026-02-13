"""Markdown exporter for unified metrics (initial)."""

from __future__ import annotations

from datetime import datetime, timezone
from pathlib import Path
from typing import Any


def export_markdown(data: dict[str, Any], output_dir: Path) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    ts = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")
    out_path = output_dir / f"metrics_result_{ts}.md"
    scores = data.get("scores", {})
    breakdown = scores.get("breakdown", {})

    lines: list[str] = []
    lines.append("# Unified Metrics Report")
    lines.append("")
    lines.append(f"Generated: {data.get('generated_at', '')}")
    lines.append(f"Schema Version: {data.get('schema_version')}")
    lines.append(f"Engine Mode: {data.get('engine', {}).get('mode')}")
    lines.append("")
    lines.append(f"## Total Score\n**{scores.get('total', 0):.2f}**")
    lines.append("\n## Breakdown")
    lines.append("| Category | Score | Max | Ratio |")
    lines.append("|----------|-------|-----|-------|")
    for cat, info in breakdown.items():
        score = info.get("score", 0.0)
        max_v = info.get("max", 0.0)
        ratio = (score / max_v) if max_v else 0.0
        lines.append(f"| {cat} | {score:.2f} | {max_v:.1f} | {ratio:.2%} |")

    if data.get("warnings"):
        lines.append("\n## Warnings")
        for w in data["warnings"]:
            lines.append(f"- {w}")

    if data.get("errors"):
        lines.append("\n## Errors")
        for e in data["errors"]:
            lines.append(f"- {e}")

    with out_path.open("w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    return out_path

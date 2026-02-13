"""HTML exporter (minimal) for unified metrics."""

from __future__ import annotations

import html
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

STYLE = """
body { font-family: Arial, sans-serif; margin: 1.5rem; }
h1 { font-size: 1.4rem; }
.table { border-collapse: collapse; }
.table th, .table td { border: 1px solid #ccc; padding: 4px 8px; font-size: 0.9rem; }
.badge { padding: 2px 6px; border-radius: 4px; }
.badge-ok { background:#2e7d32; color:#fff; }
.badge-warn { background:#f9a825; color:#000; }
.badge-err { background:#c62828; color:#fff; }
"""


def export_html(data: dict[str, Any], output_dir: Path) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    ts = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")
    out_path = output_dir / f"metrics_result_{ts}.html"

    scores = data.get("scores", {})
    breakdown = scores.get("breakdown", {})

    rows = []
    for cat, info in breakdown.items():
        score = info.get("score", 0.0)
        max_v = info.get("max", 0.0)
        ratio = (score / max_v) if max_v else 0.0
        cls = (
            "badge-ok"
            if ratio >= 0.8
            else ("badge-warn" if ratio >= 0.5 else "badge-err")
        )
        rows.append(
            f"<tr><td>{html.escape(cat)}</td><td>{score:.2f}</td><td>{max_v:.1f}</td><td><span class='badge {cls}'>{ratio:.0%}</span></td></tr>"
        )

    html_doc = f"""<!DOCTYPE html>
<html lang='en'>
<head>
<meta charset='utf-8' />
<title>Unified Metrics Report</title>
<style>{STYLE}</style>
</head>
<body>
<h1>Unified Metrics Report</h1>
<p>Generated: {html.escape(str(data.get("generated_at", "")))} | Schema v{data.get("schema_version")}</p>
<h2>Total Score: {scores.get("total", 0):.2f}</h2>
<table class='table'>
<thead><tr><th>Category</th><th>Score</th><th>Max</th><th>Ratio</th></tr></thead>
<tbody>
{"".join(rows)}
</tbody>
</table>
</body>
</html>"""

    out_path.write_text(html_doc, encoding="utf-8")
    return out_path

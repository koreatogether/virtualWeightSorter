"""UI Stability collector

Heuristically inspects the dashboard (Dash app) code to measure callback density,
error handling presence, and logging usage to feed ui_stability scoring.
"""

from __future__ import annotations

import ast
from pathlib import Path

from .base_collector import BaseCollector, CollectorResult


class UIStabilityCollector(BaseCollector):
    name = "ui"

    def __init__(self, dashboard_dir: Path, strict: bool = False) -> None:
        super().__init__(strict=strict)
        self.dashboard_dir = dashboard_dir

    def collect(self) -> CollectorResult:  # type: ignore[override]
        app_file = self.dashboard_dir / "app.py"
        if not app_file.exists():
            return CollectorResult(error="dashboard app.py not found")

        try:
            source = app_file.read_text(encoding="utf-8")
            tree = ast.parse(source, filename=str(app_file))
        except Exception as e:  # pragma: no cover
            return CollectorResult(error=f"parse error: {e}")

        callback_functions = 0
        callbacks_with_error_handling = 0
        logging_calls = 0
        try_except_blocks = 0

        for node in ast.walk(tree):
            # Count functions decorated with @app.callback
            if isinstance(node, ast.FunctionDef):
                has_callback = False
                has_error_handling = False
                for deco in node.decorator_list:
                    if (
                        isinstance(deco, ast.Call)
                        and getattr(deco.func, "attr", "") == "callback"
                    ):
                        has_callback = True
                        break
                    if isinstance(deco, ast.Attribute) and deco.attr == "callback":
                        has_callback = True
                        break
                if has_callback:
                    callback_functions += 1
                    # inspect body for try/except usage
                    for inner in ast.walk(node):
                        if isinstance(inner, ast.Try):
                            has_error_handling = True
                            break
                    if has_error_handling:
                        callbacks_with_error_handling += 1

            # Count logging.* calls
            if isinstance(node, ast.Call):
                func = node.func
                if (
                    isinstance(func, ast.Attribute)
                    and func.attr
                    in {
                        "debug",
                        "info",
                        "warning",
                        "error",
                        "exception",
                        "critical",
                    }
                    and isinstance(func.value, ast.Name)
                    and func.value.id
                    in {
                        "logger",
                        "logging",
                    }
                ):
                    logging_calls += 1

            if isinstance(node, ast.Try):
                try_except_blocks += 1

        # Basic layout complexity: count html.* component usage occurrences
        html_usages = source.count("html.")

        return CollectorResult(
            callback_functions=callback_functions,
            callbacks_with_error_handling=callbacks_with_error_handling,
            logging_calls=logging_calls,
            try_except_blocks=try_except_blocks,
            html_usages=html_usages,
        )

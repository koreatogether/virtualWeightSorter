"""Run script for the Dash app.

This script intentionally contains only runtime code and should be the
single place used to start the development server locally.
"""

try:
    # Prefer absolute package import when running from project root
    from src.python.app import app
except Exception:  # pragma: no cover - fallback for different execution contexts
    try:
        # If run as a module inside the package
        from .app import app
    except Exception:
        # Last resort: plain import (works if PYTHONPATH includes src/python)
        from app import app  # type: ignore[no-redef]


if __name__ == "__main__":
    # Bind to localhost for local development
    app.run(host="127.0.0.1", port=8050, debug=False)

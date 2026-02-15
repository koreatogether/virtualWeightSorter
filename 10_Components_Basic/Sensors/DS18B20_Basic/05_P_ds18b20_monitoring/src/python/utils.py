"""Utility helpers used across the dashboard project.

Add small, well-tested pure functions here. Avoid runtime side-effects.
"""

from typing import Any


def ensure_list(x: Any) -> list:
    """Return a list for the given input (wrap scalars)."""
    if x is None:
        return []
    if isinstance(x, list):
        return x
    return [x]

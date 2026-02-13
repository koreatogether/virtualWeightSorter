"""Base collector interface for unified metrics engine."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Any


class CollectorResult(dict[str, Any]):
    """Typed alias for collector result structure."""


class BaseCollector(ABC):
    """Abstract base class for all collectors.

    Contract:
      - collect() returns a serializable dict (no complex objects)
      - must never raise uncaught exceptions; on failure returns {'error': str}
    """

    name: str = "base"
    enabled: bool = True

    def __init__(self, strict: bool = False) -> None:
        self.strict = strict

    def safe_collect(self) -> CollectorResult:
        """Execute collection with error isolation."""
        if not self.enabled:
            return CollectorResult(disabled=True)
        try:
            data = self.collect()
            if not isinstance(data, dict):  # type: ignore[unreachable]
                return CollectorResult(error="Collector returned non-dict")
            return CollectorResult(**data)
        except Exception as e:  # pragma: no cover - defensive
            if self.strict:
                raise
            return CollectorResult(error=repr(e))

    @abstractmethod
    def collect(self) -> CollectorResult:  # pragma: no cover - interface
        """Perform collection; override in subclass."""
        raise NotImplementedError

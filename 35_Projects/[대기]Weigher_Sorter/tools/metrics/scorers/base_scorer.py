"""Base scorer utilities."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Any


class ScoreBlock(dict[str, Any]):
    pass


class BaseScorer(ABC):
    name: str = "base"

    def __init__(self, weight: float) -> None:
        self.weight = weight

    @abstractmethod
    def score(self, raw: dict) -> ScoreBlock:  # pragma: no cover - interface
        raise NotImplementedError

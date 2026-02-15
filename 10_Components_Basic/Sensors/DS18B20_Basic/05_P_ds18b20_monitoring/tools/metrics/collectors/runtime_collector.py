"""Runtime collector (latency + placeholder for future real-time metrics)."""

from __future__ import annotations

import time
from statistics import quantiles

from .base_collector import BaseCollector, CollectorResult


class RuntimeCollector(BaseCollector):
    name = "runtime"

    def __init__(
        self, iterations: int = 30, strict: bool = False, enabled: bool = True
    ) -> None:
        super().__init__(strict=strict)
        self.iterations = iterations
        self.enabled = enabled

    def collect(self) -> CollectorResult:  # type: ignore[override]
        if not self.enabled:
            return CollectorResult(disabled=True)

        # Simple synthetic "loop latency" simulation (later replaced by real harness)
        latencies: list[float] = []
        for _ in range(self.iterations):
            start = time.perf_counter()
            # Placeholder workload
            sum(i * i for i in range(500))
            latencies.append((time.perf_counter() - start) * 1000.0)  # ms

        latencies.sort()
        p95 = (
            quantiles(latencies, n=100)[94] if len(latencies) >= 20 else max(latencies)
        )

        return CollectorResult(
            iterations=self.iterations,
            latency_avg_ms=sum(latencies) / len(latencies),
            latency_p95_ms=p95,
        )

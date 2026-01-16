"""
Arduino Simulation Runner
대량 시뮬레이션 실행 및 결과 수집을 담당하는 메인 엔진

주요 기능:
- 10,000회 시뮬레이션 실행
- 실시간 진행률 모니터링
- 결과 수집 및 분석
- 성능 벤치마킹
"""

import json
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional

import pandas as pd
import plotly.express as px
from random_generator_sim import create_simulation


@dataclass
class SimulationConfig:
    """시뮬레이션 설정"""

    iterations: int = 10000
    seed: Optional[int] = None
    show_progress: bool = True
    save_results: bool = True
    output_dir: str = "src/results"
    parallel_workers: int = 1
    progress_callback: Optional[Callable] = None


@dataclass
class SimulationProgress:
    """시뮬레이션 진행 상태"""

    current_iteration: int = 0
    total_iterations: int = 0
    start_time: float = 0.0
    elapsed_time: float = 0.0
    generation_rate: float = 0.0
    estimated_remaining: float = 0.0
    percentage: float = 0.0


class SimulationRunner:
    """Arduino 시뮬레이션 실행 엔진"""

    def __init__(self, config: "Optional[SimulationConfig]" = None):
        self.config = config or SimulationConfig()
        self.progress = SimulationProgress()
        self.is_running = False
        self.should_stop = False
        self._lock = threading.Lock()

        # 결과 저장 디렉토리 생성
        Path(self.config.output_dir).mkdir(parents=True, exist_ok=True)

        print("Simulation Runner initialized")
        print(f"Config: {self.config.iterations:,} iterations, seed={self.config.seed}")

    def run_single_simulation(
        self, config: "Optional[SimulationConfig]" = None
    ) -> Dict[str, Any]:
        """단일 시뮬레이션 실행"""
        if config is None:
            config = self.config

        print("\n=== Single Simulation ===")
        print(f"Iterations: {config.iterations:,}")
        print(f"Seed: {config.seed}")

        # 시뮬레이션 환경 생성
        arduino, simulator = create_simulation(seed=config.seed)

        # Arduino setup 시뮬레이션
        simulator.simulate_arduino_setup()

        # 진행 상태 초기화
        self.progress = SimulationProgress(
            total_iterations=config.iterations, start_time=time.time()
        )
        self.is_running = True
        self.should_stop = False

        try:
            # 대량 시뮬레이션 실행
            results = simulator.run_batch_simulation(
                iterations=config.iterations, show_progress=config.show_progress
            )

            # 추가 메타데이터
            results["simulation_config"] = asdict(config)
            results["runner_info"] = {
                "runner_version": "1.0.0",
                "execution_timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
                "total_execution_time": time.time() - self.progress.start_time,
            }

            # 결과 저장
            if config.save_results:
                filename = self._save_results(results)
                results["saved_filename"] = filename

            return results

        finally:
            self.is_running = False

    def run_multiple_simulations(
        self, seeds: List[int], config: "Optional[SimulationConfig]" = None
    ) -> List[Dict[str, Any]]:
        """여러 시드로 다중 시뮬레이션 실행"""
        if config is None:
            config = self.config

        print("\n=== Multiple Simulations ===")
        print(f"Seeds: {seeds}")
        print(f"Iterations per seed: {config.iterations:,}")
        print(f"Total simulations: {len(seeds) * config.iterations:,}")

        all_results = []

        for i, seed in enumerate(seeds):
            print(f"\n--- Simulation {i+1}/{len(seeds)} (seed={seed}) ---")

            # 개별 시뮬레이션 설정
            sim_config = SimulationConfig(
                iterations=config.iterations,
                seed=seed,
                show_progress=config.show_progress,
                save_results=config.save_results,
                output_dir=config.output_dir,
            )

            # 시뮬레이션 실행
            result = self.run_single_simulation(sim_config)
            result["simulation_index"] = i
            result["seed_used"] = seed

            all_results.append(result)

        # 통합 분석 결과 생성
        combined_results = self._combine_multiple_results(all_results)

        if config.save_results:
            filename = self._save_combined_results(combined_results)
            print(f"Combined results saved to: {filename}")

        return all_results

    def run_parallel_simulation(
        self, seeds: List[int], config: "Optional[SimulationConfig]" = None
    ) -> List[Dict[str, Any]]:
        """병렬 시뮬레이션 실행 (실험적 기능)"""
        if config is None:
            config = self.config

        print("\n=== Parallel Simulations ===")
        print(f"Seeds: {seeds}")
        print(f"Workers: {config.parallel_workers}")
        print(f"Iterations per seed: {config.iterations:,}")

        all_results = []

        def run_single_seed(seed: int) -> Dict[str, Any]:
            """단일 시드 시뮬레이션 (병렬 실행용)"""
            sim_config = SimulationConfig(
                iterations=config.iterations,
                seed=seed,
                show_progress=False,  # 병렬 실행 시 진행률 표시 비활성화
                save_results=False,  # 개별 저장 비활성화
                output_dir=config.output_dir,
            )

            arduino, simulator = create_simulation(seed=seed)
            simulator.simulate_arduino_setup()

            result = simulator.run_batch_simulation(
                iterations=sim_config.iterations, show_progress=False
            )

            result["simulation_config"] = asdict(sim_config)
            result["seed_used"] = seed

            return result

        # 병렬 실행
        with ThreadPoolExecutor(max_workers=config.parallel_workers) as executor:
            # 작업 제출
            future_to_seed = {
                executor.submit(run_single_seed, seed): seed for seed in seeds
            }

            # 결과 수집
            for future in as_completed(future_to_seed):
                seed = future_to_seed[future]
                try:
                    result = future.result()
                    all_results.append(result)
                    print(f"Completed simulation for seed {seed}")
                except Exception as exc:
                    print(f"Simulation for seed {seed} generated an exception: {exc}")

        # 결과 정렬 (시드 순서대로)
        all_results.sort(key=lambda x: x["seed_used"])

        # 통합 분석
        combined_results = self._combine_multiple_results(all_results)

        if config.save_results:
            filename = self._save_combined_results(combined_results)
            print(f"Parallel simulation results saved to: {filename}")

        return all_results

    def _combine_multiple_results(
        self, results_list: List[Dict[str, Any]]
    ) -> Dict[str, Any]:
        """다중 시뮬레이션 결과 통합 분석"""
        if not results_list:
            return {}

        # 기본 통계
        total_iterations = sum(
            r["simulation_info"]["total_iterations"] for r in results_list
        )
        total_time = sum(
            r["simulation_info"]["total_time_seconds"] for r in results_list
        )
        avg_generation_rate = sum(
            r["simulation_info"]["generation_rate_per_second"] for r in results_list
        ) / len(results_list)

        # 분포 통합
        combined_distribution = {0: 0, 1: 0, 2: 0}
        for result in results_list:
            for num, count in result["distribution_analysis"]["counts"].items():
                combined_distribution[num] += count

        combined_percentages = {
            num: (count / total_iterations) * 100 if total_iterations > 0 else 0
            for num, count in combined_distribution.items()
        }

        # 제약 조건 위반 통합
        total_violations = sum(
            r["constraint_verification"]["consecutive_violations"] for r in results_list
        )

        # 전이 분석 통합
        combined_transitions = {}
        for result in results_list:
            for transition, count in result["transition_analysis"][
                "transitions"
            ].items():
                combined_transitions[transition] = (
                    combined_transitions.get(transition, 0) + count
                )

        # 성능 메트릭 평균
        performance_metrics = {}
        metric_keys = results_list[0]["performance_metrics"].keys()
        for key in metric_keys:
            values = [
                r["performance_metrics"][key]
                for r in results_list
                if key in r["performance_metrics"]
            ]
            if values:
                performance_metrics[key] = sum(values) / len(values)

        return {
            "combined_analysis": {
                "total_simulations": len(results_list),
                "total_iterations": total_iterations,
                "total_time_seconds": total_time,
                "average_generation_rate": avg_generation_rate,
                "seeds_used": [r.get("seed_used", "unknown") for r in results_list],
            },
            "combined_distribution": {
                "counts": combined_distribution,
                "percentages": combined_percentages,
                "distribution_variance": self._calculate_distribution_variance(
                    results_list
                ),
            },
            "combined_constraints": {
                "total_violations": total_violations,
                "violation_rate_percent": (
                    (total_violations / total_iterations) * 100
                    if total_iterations > 0
                    else 0
                ),
                "all_constraints_satisfied": total_violations == 0,
            },
            "combined_transitions": {
                "transitions": combined_transitions,
                "unique_transitions": len(combined_transitions),
            },
            "average_performance": performance_metrics,
            "individual_results": results_list,
        }

    def _calculate_distribution_variance(
        self, results_list: List[Dict[str, Any]]
    ) -> Dict[str, float]:
        """분포의 분산 계산 (시드별 일관성 측정)"""
        if len(results_list) < 2:
            return {"0": 0.0, "1": 0.0, "2": 0.0}

        variance = {}
        for num in [0, 1, 2]:
            percentages = [
                r["distribution_analysis"]["percentages"][num] for r in results_list
            ]
            mean = sum(percentages) / len(percentages)
            variance[num] = sum((p - mean) ** 2 for p in percentages) / len(percentages)

        return variance

    def _save_results(self, results: Dict[str, Any]) -> str:
        """단일 시뮬레이션 결과 저장"""
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        seed = results.get("hardware_simulation", {}).get("random_seed", "unknown")
        filename = f"{self.config.output_dir}/simulation_single_{seed}_{timestamp}.json"

        with open(filename, "w", encoding="utf-8") as f:
            json.dump(results, f, indent=2, ensure_ascii=False)

        print(f"Results saved to: {filename}")
        return filename

    def _save_combined_results(self, results: Dict[str, Any]) -> str:
        """통합 시뮬레이션 결과 저장"""
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        num_sims = results.get("combined_analysis", {}).get("total_simulations", 0)
        filename = f"{self.config.output_dir}/simulation_combined_{num_sims}sims_{timestamp}.json"

        with open(filename, "w", encoding="utf-8") as f:
            json.dump(results, f, indent=2, ensure_ascii=False)

        return filename

    def stop_simulation(self):
        """실행 중인 시뮬레이션 중단"""
        self.should_stop = True
        print("Simulation stop requested...")

    def get_progress(self) -> SimulationProgress:
        """현재 진행 상태 반환"""
        with self._lock:
            return self.progress


# ==================== 편의 함수들 ====================


def run_quick_simulation(
    iterations: int = 1000, seed: "Optional[int]" = None
) -> Dict[str, Any]:
    """빠른 시뮬레이션 실행"""
    config = SimulationConfig(
        iterations=iterations, seed=seed, show_progress=True, save_results=True
    )

    runner = SimulationRunner(config)
    return runner.run_single_simulation()


def run_multiple_seeds_simulation(
    seeds: List[int], iterations: int = 10000
) -> List[Dict[str, Any]]:
    """다중 시드 시뮬레이션 실행"""
    config = SimulationConfig(
        iterations=iterations, show_progress=True, save_results=True
    )

    runner = SimulationRunner(config)
    return runner.run_multiple_simulations(seeds)


def run_benchmark_simulation(iterations: int = 10000) -> Dict[str, Any]:
    """벤치마크 시뮬레이션 실행"""
    print("Arduino Uno R4 WiFi Random Number Generator Benchmark")
    print("=" * 60)

    # 다양한 시드로 테스트
    test_seeds = [12345, 54321, 98765, 11111, 99999]

    config = SimulationConfig(
        iterations=iterations, show_progress=True, save_results=True, parallel_workers=2
    )

    runner = SimulationRunner(config)

    print(f"Running benchmark with {len(test_seeds)} different seeds...")
    print(f"Total iterations: {len(test_seeds) * iterations:,}")

    start_time = time.time()
    results = runner.run_parallel_simulation(test_seeds, config)
    end_time = time.time()

    print(f"\nBenchmark completed in {end_time - start_time:.2f} seconds")
    print(f"Total generations: {len(test_seeds) * iterations:,}")
    print(
        f"Overall rate: {(len(test_seeds) * iterations) / (end_time - start_time):,.0f} gen/sec"
    )

    return results if isinstance(results, dict) else {}


# ==================== 테스트 코드 ====================

if __name__ == "__main__":
    print("Arduino Simulation Runner Test")
    print("=" * 50)

    # 1. 빠른 테스트
    print("\n1. Quick Test (1,000 iterations)")
    quick_result = run_quick_simulation(iterations=1000, seed=12345)
    print(
        f"Generation rate: {quick_result['simulation_info']['generation_rate_per_second']:,.0f} gen/sec"
    )
    print(
        f"Constraint satisfied: {quick_result['constraint_verification']['constraint_satisfied']}"
    )

    # 2. 다중 시드 테스트
    print("\n2. Multiple Seeds Test")
    multi_results = run_multiple_seeds_simulation(
        [11111, 22222, 33333], iterations=2000
    )
    print(f"Completed {len(multi_results)} simulations")

    # 3. 벤치마크 테스트 (선택적)
    run_full_benchmark = input("\nRun full benchmark? (y/N): ").lower().strip() == "y"
    if run_full_benchmark:
        print("\n3. Full Benchmark Test")
        benchmark_results = run_benchmark_simulation(iterations=10000)
        print("Benchmark completed!")


def update_distribution_chart(data):
    if not data or "distribution_analysis" not in data:
        return px.bar(title="No data available")
    dist_data = data["distribution_analysis"]
    if not dist_data or "counts" not in dist_data or "percentages" not in dist_data:
        return px.bar(title="No data available")
    df = pd.DataFrame(
        {
            "Number": list(dist_data["counts"].keys()),
            "Count": list(dist_data["counts"].values()),
            "Percentage": list(dist_data["percentages"].values()),
        }
    )
    fig = px.bar(
        df,
        x="Number",
        y="Count",
        title="Number Distribution",
        text="Percentage",
        color="Number",
    )
    fig.update_traces(texttemplate="%{text:.1f}%", textposition="outside")
    fig.update_layout(showlegend=False)
    return fig

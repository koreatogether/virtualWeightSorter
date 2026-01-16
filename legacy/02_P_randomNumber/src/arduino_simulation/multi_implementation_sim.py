"""
Multi-Implementation Arduino Simulator
여러 Arduino 구현 방식을 동시에 테스트하고 비교하는 시뮬레이터

주요 기능:
- YAML 파일에서 구현 정의 읽기
- 다중 구현 동시 실행
- 성능 비교 및 분석
- 최적 구현 추천
"""

import time
from dataclasses import dataclass
from typing import Any, Dict, List, Optional

import yaml
from arduino_mock import ArduinoUnoR4WiFiMock


@dataclass
class ImplementationResult:
    """개별 구현 결과"""

    id: str
    name: str
    description: str
    execution_time: float
    generation_rate: float
    memory_usage: int
    distribution: Dict[int, int]
    constraint_violations: int
    generated_sequence: List[int]
    error_message: Optional[str] = None
    success: bool = True


@dataclass
class ComparisonReport:
    """구현 비교 보고서"""

    total_implementations: int
    successful_implementations: int
    failed_implementations: int
    best_performance: str
    best_memory_efficiency: str
    best_distribution: str
    recommended_implementation: str
    detailed_results: List[ImplementationResult]
    benchmark_timestamp: str


class MultiImplementationSimulator:
    """다중 구현 시뮬레이터"""

    def __init__(self, config_file: str = "arduino_implementations.yaml"):
        self.config_file = config_file
        self.implementations = {}
        self.test_config = {}
        self.comparison_metrics = []
        self.recommendation_weights = {}

        self._load_configuration()
        print(f"Loaded {len(self.implementations)} implementations")

    def _load_configuration(self):
        """YAML 설정 파일 로드"""
        try:
            with open(self.config_file, encoding="utf-8") as f:
                config = yaml.safe_load(f)

            # 구현 정의 로드
            for impl in config.get("implementations", []):
                if impl.get("enabled", True):
                    self.implementations[impl["id"]] = impl

            # 테스트 설정 로드
            self.test_config = config.get("test_config", {})
            self.comparison_metrics = config.get("comparison_metrics", [])
            self.recommendation_weights = config.get("recommendation_weights", {})

            print(f"Configuration loaded from {self.config_file}")

        except Exception as e:
            print(f"Error loading configuration: {e}")
            self._create_default_config()

    def _create_default_config(self):
        """기본 설정 생성"""
        self.implementations = {
            "lookup_table_v1": {
                "id": "lookup_table_v1",
                "name": "Default Lookup Table",
                "description": "Basic lookup table implementation",
                "type": "lookup_table",
                "lookup_table": [[1, 1, 2], [0, 0, 2], [0, 1, 0]],
            }
        }
        self.test_config = {"default_iterations": 10000, "default_seed": 12345}

    def run_all_implementations(
        self, iterations: int = None, seed: int = None
    ) -> ComparisonReport:
        """모든 활성화된 구현 실행"""
        iterations = iterations or self.test_config.get("default_iterations", 10000)
        seed = seed or self.test_config.get("default_seed", 12345)

        print("\n=== Multi-Implementation Benchmark ===")
        print(f"Implementations: {len(self.implementations)}")
        print(f"Iterations per implementation: {iterations:,}")
        print(f"Seed: {seed}")
        print("-" * 50)

        results = []
        successful = 0
        failed = 0

        for impl_id, impl_config in self.implementations.items():
            print(f"\nTesting: {impl_config['name']}")

            try:
                result = self._run_single_implementation(impl_config, iterations, seed)
                results.append(result)
                successful += 1

                print(
                    f"✅ Success: {result.generation_rate:,.0f} gen/sec, "
                    f"{result.constraint_violations} violations"
                )

            except Exception as e:
                failed_result = ImplementationResult(
                    id=impl_id,
                    name=impl_config["name"],
                    description=impl_config["description"],
                    execution_time=0,
                    generation_rate=0,
                    memory_usage=0,
                    distribution={},
                    constraint_violations=0,
                    generated_sequence=[],
                    error_message=str(e),
                    success=False,
                )
                results.append(failed_result)
                failed += 1

                print(f"❌ Failed: {e}")

        # 비교 분석
        report = self._generate_comparison_report(results, successful, failed)

        return report

    def _run_single_implementation(
        self, impl_config: Dict[str, Any], iterations: int, seed: int
    ) -> ImplementationResult:
        """단일 구현 실행"""
        arduino = ArduinoUnoR4WiFiMock(seed=seed)
        generator = ImplementationGenerator(impl_config, arduino)

        # 성능 측정 시작
        start_time = time.time()
        arduino.reset_performance_counters()

        # 시뮬레이션 실행
        generated_numbers = []
        previous_number = -1

        for i in range(iterations):
            number = generator.generate_number(previous_number)
            generated_numbers.append(number)
            previous_number = number

        end_time = time.time()
        execution_time = end_time - start_time

        # 결과 분석
        distribution = {i: generated_numbers.count(i) for i in range(3)}
        constraint_violations = sum(
            1
            for i in range(1, len(generated_numbers))
            if generated_numbers[i] == generated_numbers[i - 1]
        )

        generation_rate = iterations / execution_time if execution_time > 0 else 0
        memory_usage = arduino.get_performance_stats()["sram_usage_bytes"]

        return ImplementationResult(
            id=impl_config["id"],
            name=impl_config["name"],
            description=impl_config["description"],
            execution_time=execution_time,
            generation_rate=generation_rate,
            memory_usage=memory_usage,
            distribution=distribution,
            constraint_violations=constraint_violations,
            generated_sequence=generated_numbers[:100],  # 처음 100개만 저장
        )

    def _generate_comparison_report(
        self, results: List[ImplementationResult], successful: int, failed: int
    ) -> ComparisonReport:
        """비교 보고서 생성"""
        successful_results = [r for r in results if r.success]

        if not successful_results:
            return ComparisonReport(
                total_implementations=len(results),
                successful_implementations=0,
                failed_implementations=failed,
                best_performance="None",
                best_memory_efficiency="None",
                best_distribution="None",
                recommended_implementation="None",
                detailed_results=results,
                benchmark_timestamp=time.strftime("%Y-%m-%d %H:%M:%S"),
            )

        # 최고 성능 찾기
        best_performance = max(successful_results, key=lambda x: x.generation_rate)
        best_memory = min(successful_results, key=lambda x: x.memory_usage)

        # 최고 분포 균등성 찾기
        def distribution_score(result):
            dist = result.distribution
            total = sum(dist.values())
            if total == 0:
                return float("inf")
            percentages = [count / total for count in dist.values()]
            # 33.33%에서 얼마나 벗어났는지 계산
            return sum(abs(p - 0.3333) for p in percentages)

        best_distribution = min(successful_results, key=distribution_score)

        # 추천 구현 계산
        recommended = self._calculate_recommendation(successful_results)

        return ComparisonReport(
            total_implementations=len(results),
            successful_implementations=successful,
            failed_implementations=failed,
            best_performance=best_performance.name,
            best_memory_efficiency=best_memory.name,
            best_distribution=best_distribution.name,
            recommended_implementation=recommended.name,
            detailed_results=results,
            benchmark_timestamp=time.strftime("%Y-%m-%d %H:%M:%S"),
        )

    def _calculate_recommendation(
        self, results: List[ImplementationResult]
    ) -> ImplementationResult:
        """최적 구현 추천 계산"""
        if not results:
            return results[0]

        # 정규화된 점수 계산
        max_rate = max(r.generation_rate for r in results)
        min_memory = min(r.memory_usage for r in results) or 1
        min_violations = min(r.constraint_violations for r in results)

        best_score = -1
        best_impl = results[0]

        for result in results:
            # 각 메트릭을 0-1로 정규화
            performance_score = result.generation_rate / max_rate if max_rate > 0 else 0
            memory_score = min_memory / (result.memory_usage or 1)
            reliability_score = (
                1.0 if result.constraint_violations == min_violations else 0.5
            )

            # 가중 평균 계산
            total_score = (
                performance_score * self.recommendation_weights.get("performance", 0.4)
                + memory_score
                * self.recommendation_weights.get("memory_efficiency", 0.3)
                + reliability_score
                * self.recommendation_weights.get("reliability", 0.3)
            )

            if total_score > best_score:
                best_score = total_score
                best_impl = result

        return best_impl

    def get_implementation_list(self) -> List[Dict[str, Any]]:
        """사용 가능한 구현 목록 반환"""
        return [
            {
                "id": impl["id"],
                "name": impl["name"],
                "description": impl["description"],
                "type": impl.get("type", "unknown"),
                "enabled": True,
            }
            for impl in self.implementations.values()
        ]


class ImplementationGenerator:
    """개별 구현의 숫자 생성기"""

    def __init__(self, impl_config: Dict[str, Any], arduino: ArduinoUnoR4WiFiMock):
        self.config = impl_config
        self.arduino = arduino
        self.type = impl_config.get("type", "unknown")

        # 타입별 초기화
        if self.type == "lookup_table":
            self.lookup_table = impl_config["lookup_table"]
        elif self.type == "dictionary":
            self.mapping = impl_config["mapping"]
        elif self.type == "pattern":
            self.pattern = impl_config["pattern"]
            self.pattern_index = 0
        elif self.type == "weighted":
            self.weights = impl_config["weights"]

    def generate_number(self, previous: int) -> int:
        """구현 타입에 따른 숫자 생성"""
        candidate = self.arduino.random_range(0, 3)

        if self.type == "lookup_table":
            return self._lookup_table_method(previous, candidate)
        elif self.type == "conditional":
            return self._conditional_method(previous, candidate)
        elif self.type == "dictionary":
            return self._dictionary_method(previous, candidate)
        elif self.type == "formula":
            return self._formula_method(previous, candidate)
        elif self.type == "bitwise":
            return self._bitwise_method(previous, candidate)
        elif self.type == "retry":
            return self._retry_method(previous)
        elif self.type == "weighted":
            return self._weighted_method(previous)
        elif self.type == "pattern":
            return self._pattern_method()
        elif self.type == "hybrid":
            return self._hybrid_method(previous, candidate)
        else:
            return self._lookup_table_method(previous, candidate)  # 기본값

    def _lookup_table_method(self, previous: int, candidate: int) -> int:
        """룩업 테이블 방식"""
        if previous == -1:
            return candidate
        return self.lookup_table[previous][candidate]

    def _conditional_method(self, previous: int, candidate: int) -> int:
        """조건문 방식"""
        if previous == -1:
            return candidate

        if previous == 0:
            if candidate == 0:
                return 1
            elif candidate == 1:
                return 1
            else:
                return 2
        elif previous == 1:
            if candidate == 0:
                return 0
            elif candidate == 1:
                return 0
            else:
                return 2
        else:  # previous == 2
            if candidate == 0:
                return 0
            elif candidate == 1:
                return 1
            else:
                return 0

    def _dictionary_method(self, previous: int, candidate: int) -> int:
        """딕셔너리 방식"""
        if previous == -1:
            return candidate
        key = f"{previous},{candidate}"
        return self.mapping.get(key, candidate)

    def _formula_method(self, previous: int, candidate: int) -> int:
        """수학 공식 방식"""
        if previous == -1:
            return candidate
        return (previous + candidate * 2) % 3

    def _bitwise_method(self, previous: int, candidate: int) -> int:
        """비트 연산 방식"""
        if previous == -1:
            return candidate
        result = (previous ^ candidate) & 0x3
        if result == previous:
            result = (result + 1) % 3
        return result

    def _retry_method(self, previous: int) -> int:
        """재시도 방식"""
        max_retries = self.config.get("max_retries", 10)
        for _ in range(max_retries):
            candidate = self.arduino.random_range(0, 3)
            if candidate != previous:
                return candidate
        return (previous + 1) % 3  # fallback

    def _weighted_method(self, previous: int) -> int:
        """가중치 방식"""
        if previous == -1:
            return self.arduino.random_range(0, 3)

        weights = self.weights.get(previous, [33, 33, 34])
        rand_val = self.arduino.random_range(0, 100)

        cumulative = 0
        for i, weight in enumerate(weights):
            cumulative += weight
            if rand_val < cumulative:
                return i
        return 2  # fallback

    def _pattern_method(self) -> int:
        """패턴 방식"""
        result = self.pattern[self.pattern_index]
        self.pattern_index = (self.pattern_index + 1) % len(self.pattern)
        return result

    def _hybrid_method(self, previous: int, candidate: int) -> int:
        """하이브리드 방식"""
        switch_prob = self.config.get("switch_probability", 0.1)
        if self.arduino.random_range(0, 100) < switch_prob * 100:
            return self._formula_method(previous, candidate)
        else:
            # 기본 룩업 테이블 사용
            if not hasattr(self, "lookup_table"):
                self.lookup_table = [[1, 1, 2], [0, 0, 2], [0, 1, 0]]
            return self._lookup_table_method(previous, candidate)


# ==================== 편의 함수들 ====================


def run_multi_implementation_test(
    config_file: str = "arduino_implementations.yaml",
    iterations: int = 10000,
    seed: int = 12345,
) -> ComparisonReport:
    """다중 구현 테스트 실행"""
    simulator = MultiImplementationSimulator(config_file)
    return simulator.run_all_implementations(iterations, seed)


# ==================== 테스트 코드 ====================

if __name__ == "__main__":
    print("Multi-Implementation Arduino Simulator Test")
    print("=" * 60)

    # 테스트 실행
    report = run_multi_implementation_test(iterations=5000)

    print("\n=== Benchmark Results ===")
    print(f"Total implementations: {report.total_implementations}")
    print(f"Successful: {report.successful_implementations}")
    print(f"Failed: {report.failed_implementations}")
    print(f"Best performance: {report.best_performance}")
    print(f"Best memory efficiency: {report.best_memory_efficiency}")
    print(f"Best distribution: {report.best_distribution}")
    print(f"Recommended: {report.recommended_implementation}")

    print("\n=== Detailed Results ===")
    for result in report.detailed_results:
        if result.success:
            print(
                f"{result.name}: {result.generation_rate:,.0f} gen/sec, "
                f"{result.constraint_violations} violations"
            )
        else:
            print(f"{result.name}: FAILED - {result.error_message}")

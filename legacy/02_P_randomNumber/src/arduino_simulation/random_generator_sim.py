"""
Random Number Generator Arduino Simulation
Arduino Uno R4 WiFi에서 실행되는 랜덤 숫자 생성기의 정확한 시뮬레이션

원본 Arduino 코드의 핵심 로직:
- 0, 1, 2 중에서 랜덤 숫자 생성
- 이전 숫자와 동일하지 않도록 보장 (룩업 테이블 사용)
- 반복문과 논리연산자 사용 금지 제약 조건 준수
"""

import json
import time
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple

from arduino_mock import ArduinoUnoR4WiFiMock


@dataclass
class GenerationStats:
    """랜덤 숫자 생성 통계"""

    total_count: int = 0
    start_time: float = 0.0
    number_frequency: Dict[int, int] = None
    transition_matrix: Dict[str, int] = None
    generation_times: List[float] = None
    constraint_violations: int = 0

    def __post_init__(self):
        if self.number_frequency is None:
            self.number_frequency = {0: 0, 1: 0, 2: 0}
        if self.transition_matrix is None:
            self.transition_matrix = {}
        if self.generation_times is None:
            self.generation_times = []


class RandomNumberGeneratorSim:
    """
    Arduino Random Number Generator 시뮬레이션
    원본 Arduino 코드의 정확한 동작을 Python으로 재현
    """

    def __init__(self, arduino_mock: ArduinoUnoR4WiFiMock):
        self.arduino = arduino_mock
        self.previous_number = -1  # Arduino 코드와 동일한 초기값
        self.generation_count = 0
        self.stats = GenerationStats(start_time=time.time())

        # 룩업 테이블 (Arduino 코드와 동일)
        # [이전숫자][후보숫자] = 결과숫자
        self.lookup_table = [
            [1, 1, 2],  # 이전이 0일 때: 0->1, 1->1, 2->2
            [0, 0, 2],  # 이전이 1일 때: 0->0, 1->0, 2->2
            [0, 1, 0],  # 이전이 2일 때: 0->0, 1->1, 2->0
        ]

        print("Random Number Generator Simulation initialized")
        print("Lookup Table:")
        for i, row in enumerate(self.lookup_table):
            print(f"  Previous {i}: {row}")

    def generate_random_number(self) -> int:
        """
        Arduino 코드의 generateRandomNumber() 함수 시뮬레이션
        정확한 타이밍과 메모리 사용량 반영
        """
        start_time = time.perf_counter()

        # Arduino random(0, 3) 함수 호출 시뮬레이션
        candidate = self.arduino.random_range(0, 3)

        # 룩업 테이블을 사용한 결과 계산 (Arduino 코드와 동일한 로직)
        if self.previous_number == -1:
            # 첫 번째 생성: 후보값을 그대로 사용
            result = candidate
        else:
            # 룩업 테이블 조회
            result = self.lookup_table[self.previous_number][candidate]

        # 통계 업데이트
        self._update_stats(result, candidate, start_time)

        # 상태 업데이트
        self.previous_number = result
        self.generation_count += 1

        return result

    def _update_stats(self, result: int, candidate: int, start_time: float):
        """통계 정보 업데이트"""
        generation_time = time.perf_counter() - start_time

        self.stats.total_count += 1
        self.stats.number_frequency[result] += 1
        self.stats.generation_times.append(generation_time)

        # 전이 행렬 업데이트
        if self.previous_number != -1:
            transition = f"{self.previous_number}->{result}"
            self.stats.transition_matrix[transition] = (
                self.stats.transition_matrix.get(transition, 0) + 1
            )

            # 제약 조건 위반 검사 (연속된 동일한 숫자)
            if self.previous_number == result:
                self.stats.constraint_violations += 1

    def simulate_arduino_setup(self):
        """Arduino setup() 함수 시뮬레이션"""
        print("\n=== Arduino Setup Simulation ===")

        # Serial 초기화
        self.arduino.Serial_begin(9600)
        self.arduino.Serial_println("Random Number Generator - Arduino Uno R4 WiFi")
        self.arduino.Serial_println("Numbers: 0, 1, 2")
        self.arduino.Serial_println("Constraint: No consecutive identical numbers")
        self.arduino.Serial_println("=" * 50)

        # 랜덤 시드 설정 (Arduino의 analogRead(0) 시뮬레이션)
        seed_value = self.arduino.analogRead(0)  # 노이즈를 시드로 사용
        self.arduino.randomSeed(seed_value)

        print(f"Setup completed. Random seed: {seed_value}")

    def simulate_arduino_loop(self, iterations: int = 20) -> List[int]:
        """
        Arduino loop() 함수 시뮬레이션
        지정된 횟수만큼 랜덤 숫자 생성
        """
        print(f"\n=== Arduino Loop Simulation ({iterations} iterations) ===")

        generated_numbers = []

        for i in range(iterations):
            # 랜덤 숫자 생성
            number = self.generate_random_number()
            generated_numbers.append(number)

            # Serial 출력 (Arduino 코드와 동일)
            prev_display = (
                "none" if len(generated_numbers) == 1 else str(generated_numbers[-2])
            )
            self.arduino.Serial_println(
                f"Generated: {number} (Previous: {prev_display})"
            )

            # Arduino의 delay() 시뮬레이션 (선택적)
            self.arduino.delay(100)  # 100ms 지연

        return generated_numbers

    def run_batch_simulation(
        self, iterations: int = 10000, show_progress: bool = True
    ) -> Dict[str, Any]:
        """
        대량 시뮬레이션 실행 (10,000회)
        Arduino 하드웨어 특성을 반영한 정확한 시뮬레이션
        """
        print(f"\n=== Batch Simulation ({iterations:,} iterations) ===")

        # 성능 카운터 리셋
        self.arduino.reset_performance_counters()
        batch_start_time = time.time()

        generated_numbers = []

        for i in range(iterations):
            number = self.generate_random_number()
            generated_numbers.append(number)

            # 진행률 표시
            if show_progress and (i + 1) % (iterations // 10) == 0:
                progress = ((i + 1) / iterations) * 100
                elapsed = time.time() - batch_start_time
                rate = (i + 1) / elapsed if elapsed > 0 else 0
                print(
                    f"Progress: {progress:5.1f}% ({i+1:,}/{iterations:,}) - {rate:,.0f} gen/sec"
                )

        batch_end_time = time.time()

        # 결과 분석
        analysis_results = self._analyze_results(
            generated_numbers, batch_start_time, batch_end_time
        )

        return analysis_results

    def _analyze_results(
        self, generated_numbers: List[int], start_time: float, end_time: float
    ) -> Dict[str, Any]:
        """시뮬레이션 결과 분석"""
        total_time = end_time - start_time
        arduino_stats = self.arduino.get_performance_stats()

        # 기본 통계
        total_count = len(generated_numbers)
        generation_rate = total_count / total_time if total_time > 0 else 0

        # 분포 분석
        distribution = {i: generated_numbers.count(i) for i in range(3)}
        distribution_percentages = {
            i: (count / total_count) * 100 if total_count > 0 else 0
            for i, count in distribution.items()
        }

        # 제약 조건 검증
        consecutive_violations = 0
        for i in range(1, len(generated_numbers)):
            if generated_numbers[i] == generated_numbers[i - 1]:
                consecutive_violations += 1

        # 전이 분석
        transitions = {}
        for i in range(1, len(generated_numbers)):
            prev_num = generated_numbers[i - 1]
            curr_num = generated_numbers[i]
            transition = f"{prev_num}->{curr_num}"
            transitions[transition] = transitions.get(transition, 0) + 1

        # 통계적 분석
        if self.stats.generation_times:
            avg_generation_time = sum(self.stats.generation_times) / len(
                self.stats.generation_times
            )
            min_generation_time = min(self.stats.generation_times)
            max_generation_time = max(self.stats.generation_times)
        else:
            avg_generation_time = min_generation_time = max_generation_time = 0

        return {
            "simulation_info": {
                "total_iterations": total_count,
                "total_time_seconds": total_time,
                "generation_rate_per_second": generation_rate,
                "arduino_board": "Uno R4 WiFi",
                "simulation_timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            },
            "distribution_analysis": {
                "counts": distribution,
                "percentages": distribution_percentages,
                "expected_percentage": 33.33,  # 이상적인 균등 분포
            },
            "constraint_verification": {
                "consecutive_violations": consecutive_violations,
                "violation_rate_percent": (
                    (consecutive_violations / total_count) * 100
                    if total_count > 0
                    else 0
                ),
                "constraint_satisfied": consecutive_violations == 0,
            },
            "transition_analysis": {
                "transitions": transitions,
                "unique_transitions": len(transitions),
                "expected_transitions": 6,  # 3x3 - 3 (대각선 제외)
            },
            "performance_metrics": {
                "avg_generation_time_microseconds": avg_generation_time * 1_000_000,
                "min_generation_time_microseconds": min_generation_time * 1_000_000,
                "max_generation_time_microseconds": max_generation_time * 1_000_000,
                "arduino_instruction_count": arduino_stats["instruction_count"],
                "arduino_function_calls": arduino_stats["function_calls"],
                "sram_usage_percent": arduino_stats["sram_usage_percent"],
            },
            "hardware_simulation": {
                "clock_speed_mhz": arduino_stats["clock_speed_hz"] / 1_000_000,
                "free_memory_bytes": arduino_stats["free_memory_bytes"],
                "random_seed": arduino_stats["random_seed"],
            },
            "sample_sequence": (
                generated_numbers[:50]
                if len(generated_numbers) >= 50
                else generated_numbers
            ),
        }

    def save_results(self, results: Dict[str, Any], filename: str = None):
        """결과를 JSON 파일로 저장"""
        if filename is None:
            timestamp = time.strftime("%Y%m%d_%H%M%S")
            filename = f"src/results/simulation_results_{timestamp}.json"

        with open(filename, "w", encoding="utf-8") as f:
            json.dump(results, f, indent=2, ensure_ascii=False)

        print(f"Results saved to: {filename}")
        return filename

    def reset(self):
        """시뮬레이터 상태 리셋"""
        self.previous_number = -1
        self.generation_count = 0
        self.stats = GenerationStats(start_time=time.time())
        self.arduino.reset_performance_counters()
        print("Simulator reset completed")


# ==================== 편의 함수들 ====================


def create_simulation(
    seed: Optional[int] = None,
) -> Tuple[ArduinoUnoR4WiFiMock, RandomNumberGeneratorSim]:
    """시뮬레이션 환경 생성 편의 함수"""
    arduino_mock = ArduinoUnoR4WiFiMock(seed=seed)
    simulator = RandomNumberGeneratorSim(arduino_mock)
    return arduino_mock, simulator


def run_quick_test(seed: int = 12345, iterations: int = 20) -> Dict[str, Any]:
    """빠른 테스트 실행"""
    arduino, simulator = create_simulation(seed=seed)

    # Arduino setup 시뮬레이션
    simulator.simulate_arduino_setup()

    # Arduino loop 시뮬레이션
    generated_numbers = simulator.simulate_arduino_loop(iterations)

    # 간단한 분석
    results = simulator._analyze_results(generated_numbers, time.time(), time.time())

    return results


# ==================== 테스트 코드 ====================

if __name__ == "__main__":
    print("Arduino Uno R4 WiFi Random Number Generator Simulation")
    print("=" * 60)

    # 빠른 테스트
    print("\n1. Quick Test (20 generations)")
    quick_results = run_quick_test(seed=12345, iterations=20)

    print(f"\nDistribution: {quick_results['distribution_analysis']['counts']}")
    print(
        f"Constraint violations: {quick_results['constraint_verification']['consecutive_violations']}"
    )
    print(
        f"Unique transitions: {quick_results['transition_analysis']['unique_transitions']}"
    )

    # 대량 시뮬레이션 테스트
    print("\n2. Batch Simulation Test (1,000 generations)")
    arduino, simulator = create_simulation(seed=54321)
    batch_results = simulator.run_batch_simulation(iterations=1000, show_progress=True)

    print("\nBatch Results:")
    print(
        f"Generation rate: {batch_results['simulation_info']['generation_rate_per_second']:,.0f} gen/sec"
    )
    print(f"Distribution: {batch_results['distribution_analysis']['percentages']}")
    print(
        f"Constraint satisfied: {batch_results['constraint_verification']['constraint_satisfied']}"
    )
    print(
        f"SRAM usage: {batch_results['performance_metrics']['sram_usage_percent']:.2f}%"
    )

    # 결과 저장
    filename = simulator.save_results(batch_results)
    print(f"\nResults saved to: {filename}")

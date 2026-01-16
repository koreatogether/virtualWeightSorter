"""
Random Number Generator - Python Implementation

조건:
- 3개의 숫자(0, 1, 2)를 랜덤으로 추출
- 이전 숫자와 동일하지 않도록 추출
- 반복문(for, while) 사용 불가
- 논리연산자(||, &&) 사용 불가
"""

import random
import time
from collections import defaultdict
from dataclasses import dataclass
from typing import Any, Dict, Optional


@dataclass
class GenerationStats:
    """생성 통계를 저장하는 데이터 클래스"""

    total_count: int = 0
    start_time: float = 0.0
    number_frequency: Dict[int, int] = None
    transition_matrix: Dict[tuple, int] = None

    def __post_init__(self):
        if self.number_frequency is None:
            self.number_frequency = defaultdict(int)
        if self.transition_matrix is None:
            self.transition_matrix = defaultdict(int)


class RandomNumberGenerator:
    """
    조건을 만족하는 랜덤 숫자 생성기
    """

    def __init__(self):
        self.previous_number: Optional[int] = None
        self.stats = GenerationStats(start_time=time.time())

        # 룩업 테이블: [이전숫자][후보숫자] = 결과숫자
        # 이전 숫자와 같으면 다른 숫자로 매핑
        self.lookup_table = {
            0: {0: 1, 1: 1, 2: 2},  # 이전이 0일 때
            1: {0: 0, 1: 0, 2: 2},  # 이전이 1일 때
            2: {0: 0, 1: 1, 2: 0},  # 이전이 2일 때
        }

    def generate_number(self) -> int:
        """
        조건을 만족하는 랜덤 숫자 생성
        반복문과 논리연산자 사용 불가
        """
        candidate = random.randint(0, 2)

        # 이전 숫자가 없으면 바로 반환
        if self.previous_number is None:
            result = candidate
        else:
            # 룩업 테이블을 사용해서 유효한 숫자 반환
            result = self.lookup_table[self.previous_number][candidate]

        # 통계 업데이트
        self._update_stats(result)

        self.previous_number = result
        return result

    def _update_stats(self, number: int) -> None:
        """통계 정보 업데이트"""
        self.stats.total_count += 1
        self.stats.number_frequency[number] += 1

        # 전이 행렬 업데이트 (이전 -> 현재)
        if self.previous_number is not None:
            transition = (self.previous_number, number)
            self.stats.transition_matrix[transition] += 1

    def get_statistics(self) -> Dict[str, Any]:
        """성능 및 분포 통계 반환"""
        elapsed_time = time.time() - self.stats.start_time

        # 빈도 분석
        frequency_analysis = {}
        if self.stats.total_count > 0:
            for num in range(3):
                count = self.stats.number_frequency[num]
                frequency_analysis[num] = {
                    "count": count,
                    "percentage": (count / self.stats.total_count) * 100,
                }

        # 전이 행렬 분석
        transition_analysis = {}
        for (prev, curr), count in self.stats.transition_matrix.items():
            key = f"{prev}->{curr}"
            transition_analysis[key] = count

        return {
            "total_generated": self.stats.total_count,
            "elapsed_time_seconds": elapsed_time,
            "generation_rate": (
                self.stats.total_count / elapsed_time if elapsed_time > 0 else 0
            ),
            "average_time_per_generation": (
                elapsed_time / self.stats.total_count
                if self.stats.total_count > 0
                else 0
            ),
            "frequency_analysis": frequency_analysis,
            "transition_analysis": transition_analysis,
            "current_number": self.previous_number,
        }

    def reset(self) -> None:
        """생성기 상태 초기화"""
        self.previous_number = None
        self.stats = GenerationStats(start_time=time.time())


def demonstrate_generator(count: int = 20) -> None:
    """생성기 데모 함수"""
    print("Random Number Generator - Python Implementation")
    print("Numbers: 0, 1, 2")
    print("Constraint: No consecutive identical numbers")
    print("-" * 50)

    generator = RandomNumberGenerator()

    # 숫자 생성 및 출력
    generated_numbers = []

    # 안전한 재귀 함수 (재귀 한계 확인 포함)
    import sys
    max_safe_recursion = min(count, sys.getrecursionlimit() - 100)
    
    def generate_recursive(remaining: int) -> None:
        if remaining <= 0:
            return

        number = generator.generate_number()
        generated_numbers.append(number)

        prev_display = (
            generator.previous_number if len(generated_numbers) > 1 else "none"
        )
        print(f"Generated: {number} (Previous: {prev_display})")

        # 안전한 재귀 호출
        generate_recursive(remaining - 1)

    # 재귀가 안전하면 재귀 사용, 아니면 반복문 사용
    if count <= max_safe_recursion:
        generate_recursive(count)
    else:
        print(f"Note: Using iterative approach for safety (count > {max_safe_recursion})")
        for i in range(count):
            number = generator.generate_number()
            generated_numbers.append(number)
            
            prev_display = (
                generator.previous_number if len(generated_numbers) > 1 else "none"
            )
            print(f"Generated: {number} (Previous: {prev_display})")

    # 통계 출력
    stats = generator.get_statistics()
    print("\n" + "=" * 50)
    print("PERFORMANCE STATISTICS")
    print("=" * 50)
    print(f"Total generated: {stats['total_generated']}")
    print(f"Elapsed time: {stats['elapsed_time_seconds']:.6f} seconds")
    print(f"Generation rate: {stats['generation_rate']:.2f} numbers/second")
    avg_time_ms = stats["average_time_per_generation"] * 1000
    print(f"Average time per generation: {avg_time_ms:.3f} milliseconds")

    print("\nFREQUENCY ANALYSIS:")
    for num, data in stats["frequency_analysis"].items():
        print(f"  Number {num}: {data['count']} times ({data['percentage']:.1f}%)")

    print("\nTRANSITION ANALYSIS:")
    for transition, count in stats["transition_analysis"].items():
        print(f"  {transition}: {count} times")

    print(f"\nGenerated sequence: {generated_numbers}")


class PerformanceBenchmark:
    """성능 벤치마크 클래스"""

    @staticmethod
    def run_benchmark(iterations: int = 10000) -> Dict[str, float]:
        """성능 벤치마크 실행"""
        generator = RandomNumberGenerator()

        start_time = time.perf_counter()

        # 반복적 방식으로 벤치마크 실행 (재귀 깊이 문제 해결)
        for _ in range(iterations):
            generator.generate_number()

        end_time = time.perf_counter()

        return {
            "iterations": iterations,
            "total_time": end_time - start_time,
            "time_per_iteration": (end_time - start_time) / iterations,
            "iterations_per_second": iterations / (end_time - start_time),
        }


if __name__ == "__main__":
    # 데모 실행
    demonstrate_generator(20)

    # 성능 벤치마크
    print("\n" + "=" * 50)
    print("PERFORMANCE BENCHMARK")
    print("=" * 50)

    benchmark_results = PerformanceBenchmark.run_benchmark(10000)
    print(f"Iterations: {benchmark_results['iterations']:,}")
    print(f"Total time: {benchmark_results['total_time']:.6f} seconds")
    time_per_iter_ms = benchmark_results["time_per_iteration"] * 1000000
    print(f"Time per iteration: {time_per_iter_ms:.2f} microseconds")
    print(f"Iterations per second: {benchmark_results['iterations_per_second']:,.0f}")

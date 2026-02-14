"""
Arduino R4 Minima Weigher-Sorter 통합 테스트 러너
- 6가지 난수 생성기 × 3가지 조합 알고리즘 = 18가지 조합 테스트
- Phase 1-5 문서 기반 완전 시뮬레이션
- 10,000세트부터 1,000,000세트까지 스케일러블 테스트
- Arduino R4 제약 조건 실시간 검증
"""

import time
import random
import json
import csv
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
from datetime import datetime

from arduino_r4_simulator import arduino, ArduinoR4Minima
from random_generators import create_all_generators, RandomGenerator
from combination_algorithms import create_all_algorithms, CombinationAlgorithm
from performance_monitor import performance_monitor, PerformanceMonitor


@dataclass
class TestConfiguration:
    """테스트 설정"""
    test_sets: int = 10000          # 테스트 세트 수 (10,000 ~ 1,000,000)
    target_weight: int = 2000       # 목표 무게
    tolerance: int = 70             # 허용 오차 (±70g)
    timeout_ms: int = 5000          # 타임아웃 (5초)
    
    # Arduino R4 환경 시뮬레이션 설정
    simulate_fatigue: bool = True    # 피로도 시뮬레이션
    simulate_noise: bool = True      # analogRead 노이즈
    simulate_memory_pressure: bool = True  # 메모리 압박 시뮬레이션
    
    # 결과 출력 설정
    detailed_logging: bool = False   # 상세 로깅
    csv_output: bool = True         # CSV 출력
    json_output: bool = True        # JSON 출력
    realtime_monitoring: bool = True # 실시간 모니터링


class TestResult:
    """전체 테스트 결과"""
    
    def __init__(self, config: TestConfiguration):
        self.config = config
        self.start_time = time.time()
        self.end_time = None
        
        # 18가지 조합 결과 저장
        self.combination_results = {}
        
        # 전체 통계
        self.total_tests_run = 0
        self.total_success = 0
        self.arduino_resets = 0
        self.constraint_violations = 0
    
    def add_combination_result(self, generator_name: str, algorithm_name: str, 
                             metrics: Dict):
        """조합 결과 추가"""
        combination_key = f"{generator_name}_{algorithm_name}"
        self.combination_results[combination_key] = metrics
        
        self.total_tests_run += metrics.get('total_tests', 0)
        self.total_success += metrics.get('success_count', 0)
        
        if not metrics.get('arduino_compatible', True):
            self.constraint_violations += 1
    
    def finalize(self):
        """테스트 완료 처리"""
        self.end_time = time.time()
        
    def get_duration(self) -> float:
        """테스트 소요 시간 (초)"""
        end = self.end_time if self.end_time else time.time()
        return end - self.start_time
    
    def get_overall_success_rate(self) -> float:
        """전체 성공률"""
        if self.total_tests_run == 0:
            return 0.0
        return (self.total_success / self.total_tests_run) * 100
    
    def export_results(self, base_filename: str):
        """결과 내보내기"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        import os

        # If base_filename has no directory, put outputs under src/python/result/test_runner/
        if not os.path.isabs(base_filename) and not os.path.dirname(base_filename):
            module_dir = os.path.dirname(__file__)
            output_dir = os.path.join(module_dir, 'result', 'test_runner')
            os.makedirs(output_dir, exist_ok=True)
            base_filename = os.path.join(output_dir, base_filename)

        if self.config.csv_output:
            csv_filename = f"{base_filename}_{timestamp}.csv"
            self._export_csv(csv_filename)

        if self.config.json_output:
            json_filename = f"{base_filename}_{timestamp}.json"
            self._export_json(json_filename)
    
    def _export_csv(self, filename: str):
        """CSV 내보내기"""
        with open(filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            
            # 헤더
            writer.writerow([
                'combination', 'generator', 'algorithm',
                'success_rate', 'avg_error', 'max_error',
                'avg_time_ms', 'max_time_ms', 'p95_time_ms',
                'memory_peak_kb', 'timeout_count', 'stack_overflow_count',
                'arduino_compatible', 'total_tests'
            ])
            
            # 데이터
            for combination_key, metrics in self.combination_results.items():
                generator, algorithm = combination_key.split('_', 1)
                writer.writerow([
                    combination_key, generator, algorithm,
                    metrics.get('success_rate', 0),
                    metrics.get('avg_error', 0),
                    metrics.get('max_error', 0),
                    metrics.get('avg_time_ms', 0),
                    metrics.get('max_time_ms', 0),
                    metrics.get('p95_time_ms', 0),
                    metrics.get('memory_peak_kb', 0),
                    metrics.get('timeout_count', 0),
                    metrics.get('stack_overflow_count', 0),
                    metrics.get('arduino_compatible', False),
                    metrics.get('total_tests', 0)
                ])
    
    def _export_json(self, filename: str):
        """JSON 내보내기"""
        data = {
            'test_config': {
                'test_sets': self.config.test_sets,
                'target_weight': self.config.target_weight,
                'tolerance': self.config.tolerance,
                'timeout_ms': self.config.timeout_ms
            },
            'test_summary': {
                'duration_seconds': self.get_duration(),
                'total_tests_run': self.total_tests_run,
                'overall_success_rate': self.get_overall_success_rate(),
                'arduino_resets': self.arduino_resets,
                'constraint_violations': self.constraint_violations
            },
            'combination_results': self.combination_results,
            'arduino_specs': {
                'sram_size': arduino.specs.SRAM_SIZE,
                'flash_size': arduino.specs.FLASH_SIZE,
                'clock_speed': arduino.specs.CLOCK_SPEED
            }
        }
        
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)


class WeigherSorterTestRunner:
    """Weigher-Sorter 통합 테스트 러너"""
    
    def __init__(self, config: TestConfiguration):
        self.config = config
        self.generators = create_all_generators()
        self.algorithms = create_all_algorithms()
        
        # 성능 모니터링 초기화
        self.monitor = PerformanceMonitor()
        
        print(f"Initialized WeigherSorter Test Runner")
        print(f"  Generators: {len(self.generators)}")
        print(f"  Algorithms: {len(self.algorithms)}")
        print(f"  Total combinations: {len(self.generators) * len(self.algorithms)}")
        print(f"  Test sets per combination: {config.test_sets}")
        print(f"  Total tests: {len(self.generators) * len(self.algorithms) * config.test_sets}")
    
    def run_all_tests(self) -> TestResult:
        """모든 조합 테스트 실행"""
        result = TestResult(self.config)
        
        print("\n" + "="*80)
        print("STARTING ARDUINO R4 MINIMA WEIGHER-SORTER TESTS")
        print("="*80)
        
        combination_count = 0
        total_combinations = len(self.generators) * len(self.algorithms)
        
        for generator in self.generators:
            for algorithm in self.algorithms:
                combination_count += 1
                
                print(f"\n[{combination_count}/{total_combinations}] Testing: "
                      f"{generator.name} + {algorithm.name}")
                
                # 단일 조합 테스트 실행
                metrics = self._run_single_combination(generator, algorithm)
                result.add_combination_result(generator.name, algorithm.name, metrics)
                
                # 실시간 모니터링
                if self.config.realtime_monitoring:
                    self._print_combination_summary(generator.name, algorithm.name, metrics)
                
                # Arduino 메모리 상태 확인 및 필요시 리셋
                if self._should_reset_arduino():
                    arduino.reset()
                    result.arduino_resets += 1
                    print("    Arduino reset due to memory pressure")
        
        result.finalize()
        return result
    
    def _run_single_combination(self, generator: RandomGenerator, 
                               algorithm: CombinationAlgorithm) -> Dict:
        """단일 조합 테스트 실행"""
        combination_key = self.monitor.start_test(generator.name, algorithm.name)
        
        successful_tests = 0
        failed_tests = 0
        
        for test_set in range(self.config.test_sets):
            # 진행률 표시 (1000세트마다)
            if (test_set + 1) % 1000 == 0:
                progress = ((test_set + 1) / self.config.test_sets) * 100
                print(f"    Progress: {progress:.1f}% ({test_set + 1}/{self.config.test_sets})")
            
            try:
                # 무게 생성
                weights = generator.generate_weights(12)
                
                # 조합 알고리즘 실행
                combination_result = algorithm.find_combination(weights)
                
                # 결과 기록
                self.monitor.end_test(combination_key, combination_result, weights)
                
                if combination_result.is_success:
                    successful_tests += 1
                else:
                    failed_tests += 1
                
                # 메모리 압박 시뮬레이션
                if self.config.simulate_memory_pressure and test_set % 100 == 0:
                    self._simulate_memory_pressure()
                
            except Exception as e:
                failed_tests += 1
                if self.config.detailed_logging:
                    print(f"    Error in test {test_set}: {e}")
        
        # 조합 결과 반환
        return self.monitor.get_combination_report(combination_key)
    
    def _print_combination_summary(self, generator_name: str, algorithm_name: str, 
                                  metrics: Dict):
        """조합별 요약 정보 출력"""
        print(f"    Success Rate: {metrics['success_rate']:.1f}%")
        print(f"    Avg Error: {metrics['avg_error']:.1f}g")
        print(f"    Avg Time: {metrics['avg_time_ms']:.1f}ms")
        print(f"    Peak Memory: {metrics['memory_peak_kb']:.1f}KB")
        print(f"    Arduino Compatible: {metrics['arduino_compatible']}")
        
        if metrics['violations']:
            print(f"    Violations: {', '.join(metrics['violations'])}")
    
    def _should_reset_arduino(self) -> bool:
        """Arduino 리셋 필요 여부 확인"""
        stats = arduino.get_stats()
        
        # 메모리 사용률이 90% 이상이거나 스택이 깊어진 경우
        return (stats['memory_usage_percent'] > 90 or 
                stats['max_stack_depth'] > 80)
    
    def _simulate_memory_pressure(self):
        """메모리 압박 시뮬레이션"""
        if self.config.simulate_memory_pressure:
            # 임시 메모리 할당으로 압박 상황 시뮬레이션
            pressure_size = random.randint(100, 500)
            arduino.allocate_memory(pressure_size)
            
            # 일정 확률로 메모리 해제
            if random.random() < 0.3:
                arduino.free_memory(pressure_size)
    
    def run_quick_test(self) -> TestResult:
        """빠른 테스트 (각 조합당 100세트)"""
        original_test_sets = self.config.test_sets
        self.config.test_sets = 100
        
        print("Running Quick Test (100 sets per combination)...")
        result = self.run_all_tests()
        
        self.config.test_sets = original_test_sets
        return result
    
    def run_benchmark_test(self) -> TestResult:
        """벤치마크 테스트 (특정 조합들만 테스트)"""
        # 대표적인 3가지 조합만 테스트
        benchmark_combinations = [
            ("Arduino_Random", "Random_Sampling"),
            ("XorShift32", "Greedy_LocalSearch"),
            ("Expert", "Dynamic_Programming")
        ]
        
        result = TestResult(self.config)
        
        print(f"Running Benchmark Test ({len(benchmark_combinations)} combinations)...")
        
        for gen_name, algo_name in benchmark_combinations:
            generator = next((g for g in self.generators if g.name == gen_name), None)
            algorithm = next((a for a in self.algorithms if a.name == algo_name), None)
            
            if generator and algorithm:
                print(f"\nBenchmarking: {gen_name} + {algo_name}")
                metrics = self._run_single_combination(generator, algorithm)
                result.add_combination_result(gen_name, algo_name, metrics)
        
        result.finalize()
        return result


def main():
    """메인 함수"""
    print("Arduino R4 Minima Weigher-Sorter Simulator")
    print("=" * 50)
    
    # 테스트 설정
    config = TestConfiguration(
        test_sets=1000,  # 기본 1000세트 (빠른 테스트용)
        detailed_logging=False,
        realtime_monitoring=True
    )
    
    # 테스트 러너 생성
    runner = WeigherSorterTestRunner(config)
    
    # 사용자 선택
    print("\nTest Options:")
    print("1. Quick Test (100 sets per combination)")
    print("2. Standard Test (1,000 sets per combination)")  
    print("3. Full Test (10,000 sets per combination)")
    print("4. Benchmark Test (3 combinations only)")
    
    choice = input("Select test option (1-4): ").strip()
    
    if choice == '1':
        result = runner.run_quick_test()
    elif choice == '2':
        config.test_sets = 1000
        result = runner.run_all_tests()
    elif choice == '3':
        config.test_sets = 10000
        result = runner.run_all_tests()
    elif choice == '4':
        result = runner.run_benchmark_test()
    else:
        print("Invalid choice. Running quick test...")
        result = runner.run_quick_test()
    
    # 결과 출력
    print("\n" + "="*80)
    print("TEST RESULTS SUMMARY")
    print("="*80)
    
    print(f"Total Duration: {result.get_duration():.1f} seconds")
    print(f"Total Tests: {result.total_tests_run}")
    print(f"Overall Success Rate: {result.get_overall_success_rate():.1f}%")
    print(f"Arduino Resets: {result.arduino_resets}")
    print(f"Constraint Violations: {result.constraint_violations}")
    
    # 상위 5개 조합 출력
    sorted_combinations = sorted(
        result.combination_results.items(),
        key=lambda x: x[1]['success_rate'],
        reverse=True
    )
    
    print(f"\nTop 5 Combinations:")
    print("-" * 60)
    for i, (combination, metrics) in enumerate(sorted_combinations[:5]):
        print(f"{i+1}. {combination}: {metrics['success_rate']:.1f}% success, "
              f"{metrics['avg_time_ms']:.1f}ms avg, {metrics['memory_peak_kb']:.1f}KB peak")
    
    # 결과 파일 내보내기
    result.export_results("weigher_sorter_test_results")
    
    print(f"\nResults exported to files with timestamp.")
    print("Test completed successfully!")


if __name__ == "__main__":
    main()
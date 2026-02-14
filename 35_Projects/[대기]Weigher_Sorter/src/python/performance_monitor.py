"""
Arduino R4 성능 모니터링 시스템
- 메모리 사용량 프로파일링
- 실행시간 측정 및 통계
- Arduino R4 제약 조건 검증
- Phase 3 데이터 집계 시스템
"""

import time
import statistics
import os
from typing import Dict, List, Any, Optional, Tuple
from dataclasses import dataclass, field
from arduino_r4_simulator import arduino, ArduinoR4Minima


@dataclass
class PerformanceMetrics:
    """성능 측정 지표"""
    success_count: int = 0          # 성공 횟수 (1930-2070g 달성)
    total_error: int = 0            # 누적 오차 (평균 계산용)
    max_error: int = 0              # 최대 오차
    avg_time_ms: float = 0.0        # 평균 실행시간
    max_time_ms: int = 0            # 최대 실행시간
    memory_peak_kb: float = 0.0     # 최대 메모리 사용량 (KB)
    timeout_count: int = 0          # 5초 초과 횟수
    stack_overflow_count: int = 0   # 스택 오버플로우 횟수
    
    # Arduino R4 특화 메트릭
    total_tests: int = 0
    execution_times: List[int] = field(default_factory=list)
    memory_usage_history: List[int] = field(default_factory=list)
    
    def add_result(self, is_success: bool, error: int, time_ms: int, 
                   memory_bytes: int, timeout: bool = False, stack_overflow: bool = False):
        """테스트 결과 추가"""
        self.total_tests += 1
        
        if is_success:
            self.success_count += 1
        
        self.total_error += error
        self.max_error = max(self.max_error, error)
        
        self.execution_times.append(time_ms)
        self.avg_time_ms = statistics.mean(self.execution_times)
        self.max_time_ms = max(self.max_time_ms, time_ms)
        
        memory_kb = memory_bytes / 1024
        self.memory_usage_history.append(memory_bytes)
        self.memory_peak_kb = max(self.memory_peak_kb, memory_kb)
        
        if timeout:
            self.timeout_count += 1
        
        if stack_overflow:
            self.stack_overflow_count += 1
    
    def get_success_rate(self) -> float:
        """성공률 반환 (%"""
        if self.total_tests == 0:
            return 0.0
        return (self.success_count / self.total_tests) * 100
    
    def get_avg_error(self) -> float:
        """평균 오차 반환"""
        if self.total_tests == 0:
            return 0.0
        return self.total_error / self.total_tests
    
    def get_percentiles(self) -> Dict[str, float]:
        """실행시간 백분위수 계산"""
        if not self.execution_times:
            return {'p50': 0, 'p95': 0, 'p99': 0}
        
        sorted_times = sorted(self.execution_times)
        n = len(sorted_times)
        
        return {
            'p50': sorted_times[int(n * 0.5)],
            'p95': sorted_times[int(n * 0.95)],
            'p99': sorted_times[int(n * 0.99)]
        }
    
    def to_dict(self) -> Dict[str, Any]:
        """딕셔너리로 변환"""
        percentiles = self.get_percentiles()
        
        return {
            'success_rate': self.get_success_rate(),
            'avg_error': self.get_avg_error(),
            'max_error': self.max_error,
            'avg_time_ms': self.avg_time_ms,
            'max_time_ms': self.max_time_ms,
            'p50_time_ms': percentiles['p50'],
            'p95_time_ms': percentiles['p95'],
            'p99_time_ms': percentiles['p99'],
            'memory_peak_kb': self.memory_peak_kb,
            'timeout_count': self.timeout_count,
            'stack_overflow_count': self.stack_overflow_count,
            'total_tests': self.total_tests
        }


class ArduinoConstraintValidator:
    """Arduino R4 제약 조건 검증기"""
    
    def __init__(self):
        self.max_memory_kb = 28  # 실제 사용 가능한 메모리 (32KB - 4KB 시스템)
        self.max_time_ms = 5000  # 최대 허용 처리 시간
        self.max_stack_depth = 100  # 최대 스택 깊이
    
    def validate_memory(self, memory_bytes: int) -> bool:
        """메모리 사용량 검증"""
        return (memory_bytes / 1024) <= self.max_memory_kb
    
    def validate_time(self, time_ms: int) -> bool:
        """실행시간 검증"""
        return time_ms <= self.max_time_ms
    
    def validate_stack(self, stack_depth: int) -> bool:
        """스택 깊이 검증"""
        return stack_depth <= self.max_stack_depth
    
    def get_violations(self, metrics: PerformanceMetrics) -> List[str]:
        """제약 조건 위반 사항 반환"""
        violations = []
        
        if metrics.memory_peak_kb > self.max_memory_kb:
            violations.append(f"Memory overflow: {metrics.memory_peak_kb:.1f}KB > {self.max_memory_kb}KB")
        
        if metrics.max_time_ms > self.max_time_ms:
            violations.append(f"Time overflow: {metrics.max_time_ms}ms > {self.max_time_ms}ms")
        
        if metrics.timeout_count > 0:
            violations.append(f"Timeouts occurred: {metrics.timeout_count} times")
        
        if metrics.stack_overflow_count > 0:
            violations.append(f"Stack overflows: {metrics.stack_overflow_count} times")
        
        return violations


class PerformanceMonitor:
    """성능 모니터링 시스템"""
    
    def __init__(self):
        self.validator = ArduinoConstraintValidator()
        self.reset_statistics()
    
    def reset_statistics(self):
        """통계 초기화"""
        self.metrics_by_combination = {}  # 18가지 조합별 메트릭
        self.global_stats = {
            'total_tests': 0,
            'start_time': time.time(),
            'arduino_resets': 0
        }
    
    def get_combination_key(self, generator_name: str, algorithm_name: str) -> str:
        """조합 키 생성"""
        return f"{generator_name}_{algorithm_name}"
    
    def start_test(self, generator_name: str, algorithm_name: str) -> str:
        """테스트 시작"""
        combination_key = self.get_combination_key(generator_name, algorithm_name)
        
        if combination_key not in self.metrics_by_combination:
            self.metrics_by_combination[combination_key] = PerformanceMetrics()
        
        # Arduino 상태 저장
        arduino_stats = arduino.get_stats()
        
        return combination_key
    
    def end_test(self, combination_key: str, result, weights: List[int]):
        """테스트 종료 및 결과 기록"""
        if combination_key not in self.metrics_by_combination:
            return
        
        metrics = self.metrics_by_combination[combination_key]
        
        # 결과 분석
        is_success = result.is_success if hasattr(result, 'is_success') else False
        error = result.error if hasattr(result, 'error') else abs(2000 - result.total_weight)
        time_ms = result.execution_time_ms if hasattr(result, 'execution_time_ms') else 0
        memory_bytes = result.memory_used_bytes if hasattr(result, 'memory_used_bytes') else 0
        timeout = result.timeout if hasattr(result, 'timeout') else False
        
        # 스택 오버플로우 검사
        arduino_stats = arduino.get_stats()
        stack_overflow = arduino_stats['max_stack_depth'] > self.validator.max_stack_depth
        
        # 메트릭에 추가
        metrics.add_result(is_success, error, time_ms, memory_bytes, timeout, stack_overflow)
        
        self.global_stats['total_tests'] += 1
        
        # 제약 조건 검증
        violations = self.validator.get_violations(metrics)
        if violations:
            print(f"Constraint violations in {combination_key}:")
            for violation in violations:
                print(f"  - {violation}")
    
    def get_combination_report(self, combination_key: str) -> Optional[Dict[str, Any]]:
        """특정 조합의 성능 리포트 반환"""
        if combination_key not in self.metrics_by_combination:
            return None
        
        metrics = self.metrics_by_combination[combination_key]
        violations = self.validator.get_violations(metrics)
        
        report = metrics.to_dict()
        report['combination'] = combination_key
        report['violations'] = violations
        report['arduino_compatible'] = len(violations) == 0
        
        return report
    
    def get_all_combinations_report(self) -> Dict[str, Dict[str, Any]]:
        """모든 조합의 성능 리포트 반환"""
        reports = {}
        
        for combination_key in self.metrics_by_combination:
            reports[combination_key] = self.get_combination_report(combination_key)
        
        return reports
    
    def get_ranking(self, sort_by: str = 'success_rate') -> List[Tuple[str, Dict[str, Any]]]:
        """조합별 순위 반환"""
        reports = self.get_all_combinations_report()
        
        # 정렬 기준에 따라 순위 매기기
        if sort_by == 'success_rate':
            sorted_items = sorted(reports.items(), 
                                key=lambda x: x[1]['success_rate'], reverse=True)
        elif sort_by == 'avg_time_ms':
            sorted_items = sorted(reports.items(), 
                                key=lambda x: x[1]['avg_time_ms'])
        elif sort_by == 'memory_peak_kb':
            sorted_items = sorted(reports.items(), 
                                key=lambda x: x[1]['memory_peak_kb'])
        elif sort_by == 'avg_error':
            sorted_items = sorted(reports.items(), 
                                key=lambda x: x[1]['avg_error'])
        else:
            sorted_items = list(reports.items())
        
        return sorted_items
    
    def export_csv(self, filename: str):
        """CSV 형태로 결과 내보내기"""
        reports = self.get_all_combinations_report()
        
        if not reports:
            print("No data to export")
            return

        # If filename has no directory component, place it under
        # src/python/result/<script_basename>/
        if not os.path.isabs(filename) and not os.path.dirname(filename):
            module_dir = os.path.dirname(__file__)
            script_name = os.path.splitext(os.path.basename(__file__))[0]
            output_dir = os.path.join(module_dir, 'result', script_name)
            os.makedirs(output_dir, exist_ok=True)
            filename = os.path.join(output_dir, filename)

        # CSV 헤더 생성
        sample_report = next(iter(reports.values()))
        headers = ['combination'] + [k for k in sample_report.keys() if k != 'violations']

        with open(filename, 'w', newline='', encoding='utf-8') as f:
            f.write(','.join(headers) + '\n')

            for combination_key, report in reports.items():
                row = [str(report.get(header, '')) for header in headers]
                f.write(','.join(row) + '\n')

        print(f"Results exported to {filename}")
    
    def print_summary(self):
        """요약 정보 출력"""
        print("\n" + "="*70)
        print("ARDUINO R4 MINIMA PERFORMANCE SUMMARY")
        print("="*70)
        
        total_time = time.time() - self.global_stats['start_time']
        print(f"Total Tests: {self.global_stats['total_tests']}")
        print(f"Total Time: {total_time:.1f}s")
        print(f"Arduino Resets: {self.global_stats['arduino_resets']}")
        
        # 상위 5개 조합 출력
        rankings = self.get_ranking('success_rate')
        
        print(f"\nTop 5 Combinations by Success Rate:")
        print("-" * 70)
        print(f"{'Rank':<5} {'Combination':<25} {'Success%':<10} {'Avg Time':<10} {'Memory KB':<10}")
        print("-" * 70)
        
        for i, (combination, report) in enumerate(rankings[:5]):
            print(f"{i+1:<5} {combination:<25} {report['success_rate']:<10.1f} "
                  f"{report['avg_time_ms']:<10.1f} {report['memory_peak_kb']:<10.1f}")
        
        # Arduino 호환성 체크
        compatible_count = sum(1 for _, report in rankings if report['arduino_compatible'])
        print(f"\nArduino R4 Compatible Combinations: {compatible_count}/{len(rankings)}")
        
        # 메모리 사용량 통계
        arduino_stats = arduino.get_stats()
        print(f"\nArduino Memory Status:")
        print(f"  Used: {arduino_stats['memory_used']} bytes")
        print(f"  Free: {arduino_stats['memory_free']} bytes")
        print(f"  Usage: {arduino_stats['memory_usage_percent']:.1f}%")
        print(f"  Max Stack Depth: {arduino_stats['max_stack_depth']}")


# 글로벌 모니터 인스턴스
performance_monitor = PerformanceMonitor()


if __name__ == "__main__":
    # 기본 테스트
    print("Performance Monitor Test")
    print("=" * 40)
    
    # 샘플 데이터로 테스트
    from dataclasses import dataclass
    
    @dataclass
    class MockResult:
        is_success: bool
        error: int
        execution_time_ms: int
        memory_used_bytes: int
        timeout: bool = False
        total_weight: int = 2000
    
    # 테스트 데이터 추가
    test_combinations = [
        ("Arduino_Random", "Random_Sampling"),
        ("XorShift32", "Greedy_LocalSearch"), 
        ("Hybrid", "Dynamic_Programming")
    ]
    
    for gen_name, algo_name in test_combinations:
        combination_key = performance_monitor.start_test(gen_name, algo_name)
        
        # 모의 결과 생성
        for _ in range(100):
            result = MockResult(
                is_success=True,
                error=25,
                execution_time_ms=150,
                memory_used_bytes=1200
            )
            performance_monitor.end_test(combination_key, result, [600] * 12)
    
    # 결과 출력
    performance_monitor.print_summary()
    
    # CSV 내보내기 테스트
    performance_monitor.export_csv("test_results.csv")
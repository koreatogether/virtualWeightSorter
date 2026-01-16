"""
Unit tests for the random number generator implementations
"""

import pytest
import sys
from pathlib import Path

# Add Python source to path
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root / "src" / "python"))

from random_generator import RandomNumberGenerator

class TestRandomNumberGenerator:
    
    def test_generator_initialization(self):
        """테스트: 생성기 초기화"""
        generator = RandomNumberGenerator()
        assert generator.previous_number is None
        assert generator.stats.total_count == 0
    
    def test_number_generation_range(self):
        """테스트: 생성된 숫자가 0, 1, 2 범위 내에 있는지"""
        generator = RandomNumberGenerator()
        
        for _ in range(100):
            number = generator.generate_number()
            assert number in [0, 1, 2], f"Generated number {number} is out of range"
    
    def test_no_consecutive_identical_numbers(self):
        """테스트: 연속된 동일한 숫자가 생성되지 않는지"""
        generator = RandomNumberGenerator()
        
        previous = None
        for _ in range(100):
            current = generator.generate_number()
            
            if previous is not None:
                assert current != previous, f"Consecutive identical numbers: {previous} -> {current}"
            
            previous = current
    
    def test_first_generation_any_valid_number(self):
        """테스트: 첫 번째 생성은 모든 유효한 숫자 가능"""
        valid_first_numbers = set()
        
        for _ in range(100):
            generator = RandomNumberGenerator()
            first_number = generator.generate_number()
            valid_first_numbers.add(first_number)
        
        # 충분한 시도 후에는 모든 숫자(0, 1, 2)가 첫 번째로 나와야 함
        assert len(valid_first_numbers) >= 2, "First generation should produce varied numbers"
    
    def test_lookup_table_logic(self):
        """테스트: 룩업 테이블 로직 검증"""
        generator = RandomNumberGenerator()
        
        # 각 이전 숫자에 대해 모든 후보를 테스트
        test_cases = [
            # (previous, candidate, expected_result)
            (0, 0, 1),  # 0이 나왔는데 0이 후보면 -> 1
            (0, 1, 1),  # 0이 나왔는데 1이 후보면 -> 1  
            (0, 2, 2),  # 0이 나왔는데 2가 후보면 -> 2
            (1, 0, 0),  # 1이 나왔는데 0이 후보면 -> 0
            (1, 1, 0),  # 1이 나왔는데 1이 후보면 -> 0
            (1, 2, 2),  # 1이 나왔는데 2가 후보면 -> 2
            (2, 0, 0),  # 2가 나왔는데 0이 후보면 -> 0
            (2, 1, 1),  # 2가 나왔는데 1이 후보면 -> 1
            (2, 2, 0),  # 2가 나왔는데 2가 후보면 -> 0
        ]
        
        for previous, candidate, expected in test_cases:
            result = generator.lookup_table[previous][candidate]
            assert result == expected, f"Lookup table error: {previous}[{candidate}] should be {expected}, got {result}"
    
    def test_statistics_tracking(self):
        """테스트: 통계 추적 기능"""
        generator = RandomNumberGenerator()
        
        # 숫자 생성
        for _ in range(10):
            generator.generate_number()
        
        stats = generator.get_statistics()
        
        assert stats['total_generated'] == 10
        assert stats['elapsed_time_seconds'] > 0
        assert sum(stats['frequency_analysis'][i]['count'] for i in range(3)) == 10
    
    def test_reset_functionality(self):
        """테스트: 리셋 기능"""
        generator = RandomNumberGenerator()
        
        # 몇 개 숫자 생성
        for _ in range(5):
            generator.generate_number()
        
        # 리셋
        generator.reset()
        
        assert generator.previous_number is None
        assert generator.stats.total_count == 0
    
    def test_no_infinite_loops(self):
        """테스트: 무한 루프 방지 (타임아웃 테스트)"""
        import signal
        
        def timeout_handler(signum, frame):
            raise TimeoutError("Generation took too long - possible infinite loop")
        
        # 짧은 타임아웃 설정 (Unix 시스템에서만 작동)
        if hasattr(signal, 'alarm'):
            signal.signal(signal.SIGALRM, timeout_handler)
            signal.alarm(1)  # 1초 타임아웃
        
        try:
            generator = RandomNumberGenerator()
            
            # 많은 숫자 생성 테스트
            for _ in range(1000):
                number = generator.generate_number()
                assert number in [0, 1, 2]
            
        finally:
            if hasattr(signal, 'alarm'):
                signal.alarm(0)  # 타임아웃 해제
    
    def test_distribution_fairness(self):
        """테스트: 분포의 공정성 (통계적 테스트)"""
        generator = RandomNumberGenerator()
        
        # 큰 샘플로 분포 테스트
        sample_size = 3000
        for _ in range(sample_size):
            generator.generate_number()
        
        stats = generator.get_statistics()
        
        # 각 숫자의 빈도가 너무 편향되지 않았는지 확인
        # (완전히 균등할 필요는 없지만 극도로 편향되면 안 됨)
        for i in range(3):
            frequency = stats['frequency_analysis'][i]['percentage']
            assert 20 <= frequency <= 50, f"Number {i} frequency {frequency}% is too biased"


class TestPerformance:
    
    def test_generation_performance(self):
        """테스트: 성능 기준 확인"""
        from random_generator import PerformanceBenchmark
        
        # 작은 샘플로 성능 테스트
        result = PerformanceBenchmark.run_benchmark(1000)
        
        assert result['iterations'] == 1000
        assert result['total_time'] > 0
        assert result['iterations_per_second'] > 100  # 초당 최소 100개 생성 가능해야 함


if __name__ == "__main__":
    # 테스트 실행
    pytest.main([__file__, "-v"])
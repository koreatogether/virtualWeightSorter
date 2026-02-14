"""
Phase 2 문서 기반 조합 알고리즘 3종 구현
- Algorithm 1: Random Sampling (기준선)
- Algorithm 2: Greedy + Local Search  
- Algorithm 3: Dynamic Programming (Arduino R4 제한적 구현)

목표: 2000g (±70g 허용, 즉 1930-2070g)
"""

import random
import time
from typing import List, Tuple, Optional, Set
from abc import ABC, abstractmethod
from arduino_r4_simulator import arduino, millis
from dataclasses import dataclass


@dataclass
class CombinationResult:
    """조합 결과 데이터 클래스"""
    selected_indices: List[int]  # 선택된 포도 인덱스들
    total_weight: int           # 총 무게
    is_success: bool           # 목표 달성 여부 (1930-2070g)
    error: int                 # |2000 - total_weight|
    execution_time_ms: int     # 실행 시간
    memory_used_bytes: int     # 메모리 사용량
    iterations: int            # 반복 횟수
    timeout: bool              # 타임아웃 발생 여부


class CombinationAlgorithm(ABC):
    """조합 알고리즘 베이스 클래스"""
    
    def __init__(self, name: str, target_weight: int = 2000, tolerance: int = 70):
        self.name = name
        self.target_weight = target_weight
        self.tolerance = tolerance
        self.min_weight = target_weight - tolerance  # 1930g
        self.max_weight = target_weight + tolerance  # 2070g
        self.timeout_ms = 5000  # 5초 타임아웃
        
        # 통계
        self.total_calls = 0
        self.total_success = 0
        self.total_time_ms = 0
        self.stack_overflows = 0
        self.timeouts = 0
    
    @abstractmethod
    def find_combination(self, weights: List[int]) -> CombinationResult:
        """주어진 무게 리스트에서 최적 조합 찾기"""
        pass
    
    def is_valid_weight(self, total_weight: int) -> bool:
        """목표 무게 범위 내 확인"""
        return self.min_weight <= total_weight <= self.max_weight
    
    def calculate_error(self, total_weight: int) -> int:
        """목표 무게와의 오차 계산"""
        return abs(self.target_weight - total_weight)
    
    def get_stats(self) -> dict:
        """알고리즘 통계 반환"""
        success_rate = (self.total_success / max(1, self.total_calls)) * 100
        avg_time = self.total_time_ms / max(1, self.total_calls)
        
        return {
            'name': self.name,
            'total_calls': self.total_calls,
            'success_rate': success_rate,
            'avg_time_ms': avg_time,
            'stack_overflows': self.stack_overflows,
            'timeouts': self.timeouts
        }


class RandomSamplingAlgorithm(CombinationAlgorithm):
    """Algorithm 1: Random Sampling (기준선)"""
    
    def __init__(self, max_iterations: int = 1000):
        super().__init__("Random_Sampling")
        self.max_iterations = max_iterations
        # Arduino R4 제약: 500회로 축소
        if arduino.specs.SRAM_SIZE <= 32768:
            self.max_iterations = 500
    
    def find_combination(self, weights: List[int]) -> CombinationResult:
        start_time = millis()
        start_memory = arduino._memory_used
        
        if not arduino.allocate_memory(len(weights) * 4):  # int 배열 메모리
            return CombinationResult([], 0, False, self.target_weight, 0, 0, 0, False)
        
        try:
            best_combination = []
            best_weight = 0
            best_error = float('inf')
            iterations = 0
            
            for iteration in range(self.max_iterations):
                # 타임아웃 체크
                current_time = millis()
                if current_time - start_time > self.timeout_ms:
                    self.timeouts += 1
                    break
                
                # 무작위 조합 생성 (3-8개 선택)
                combination_size = random.randint(3, 8)
                combination_size = min(combination_size, len(weights))
                
                selected_indices = random.sample(range(len(weights)), combination_size)
                total_weight = sum(weights[i] for i in selected_indices)
                
                arduino.simulate_instruction_delay('arithmetic', combination_size)
                arduino.simulate_instruction_delay('memory_access', combination_size)
                
                # 더 나은 조합인지 확인
                current_error = self.calculate_error(total_weight)
                if current_error < best_error:
                    best_combination = selected_indices.copy()
                    best_weight = total_weight
                    best_error = current_error
                    
                    # 목표 달성 시 조기 종료
                    if self.is_valid_weight(total_weight):
                        break
                
                iterations = iteration + 1
                arduino.simulate_instruction_delay('comparison', 2)
            
            execution_time = millis() - start_time
            memory_used = arduino._memory_used - start_memory
            is_success = self.is_valid_weight(best_weight)
            timeout_occurred = (millis() - start_time) > self.timeout_ms
            
            self.total_calls += 1
            if is_success:
                self.total_success += 1
            self.total_time_ms += execution_time
            
            return CombinationResult(
                selected_indices=best_combination,
                total_weight=best_weight,
                is_success=is_success,
                error=best_error,
                execution_time_ms=execution_time,
                memory_used_bytes=memory_used,
                iterations=iterations,
                timeout=timeout_occurred
            )
            
        finally:
            arduino.free_memory(len(weights) * 4)


class GreedyLocalSearchAlgorithm(CombinationAlgorithm):
    """Algorithm 2: Greedy + Local Search"""
    
    def __init__(self):
        super().__init__("Greedy_LocalSearch")
    
    def find_combination(self, weights: List[int]) -> CombinationResult:
        start_time = millis()
        start_memory = arduino._memory_used
        
        # 메모리 할당: 가중치 배열 + 인덱스 배열 + 비트마스크
        required_memory = len(weights) * 8 + 64  # 여유분 포함
        if not arduino.allocate_memory(required_memory):
            return CombinationResult([], 0, False, self.target_weight, 0, 0, 0, False)
        
        try:
            # 1단계: 큰 무게부터 정렬 (탐욕적 선택)
            indexed_weights = [(weights[i], i) for i in range(len(weights))]
            indexed_weights.sort(reverse=True)  # 큰 무게부터
            
            arduino.simulate_instruction_delay('arithmetic', len(weights) * 3)  # 정렬 비용
            
            # 2단계: 탐욕적 선택 (2000g 초과 시 중단)
            selected_indices = []
            current_weight = 0
            
            for weight, index in indexed_weights:
                if current_weight + weight <= self.max_weight:
                    selected_indices.append(index)
                    current_weight += weight
                    
                    arduino.simulate_instruction_delay('arithmetic', 1)
                    
                    # 목표 달성 시 조기 종료
                    if self.is_valid_weight(current_weight):
                        break
            
            # 3단계: Local Search (2-swap 최적화)
            if not self.is_valid_weight(current_weight):
                selected_indices, current_weight = self._local_search_2swap(
                    weights, selected_indices, current_weight, start_time
                )
            
            execution_time = millis() - start_time
            memory_used = arduino._memory_used - start_memory
            is_success = self.is_valid_weight(current_weight)
            error = self.calculate_error(current_weight)
            timeout_occurred = execution_time > self.timeout_ms
            
            self.total_calls += 1
            if is_success:
                self.total_success += 1
            self.total_time_ms += execution_time
            
            return CombinationResult(
                selected_indices=selected_indices,
                total_weight=current_weight,
                is_success=is_success,
                error=error,
                execution_time_ms=execution_time,
                memory_used_bytes=memory_used,
                iterations=len(indexed_weights) + 10,  # 정렬 + local search
                timeout=timeout_occurred
            )
            
        except Exception as e:
            self.stack_overflows += 1
            return CombinationResult([], 0, False, self.target_weight, 0, 0, 0, False)
        
        finally:
            arduino.free_memory(required_memory)
    
    def _local_search_2swap(self, weights: List[int], selected_indices: List[int], 
                           current_weight: int, start_time: int) -> Tuple[List[int], int]:
        """2-swap 로컬 서치"""
        best_indices = selected_indices.copy()
        best_weight = current_weight
        best_error = self.calculate_error(current_weight)
        
        selected_set = set(selected_indices)
        unselected_indices = [i for i in range(len(weights)) if i not in selected_set]
        
        # 타임아웃 체크를 위한 반복 제한
        max_swaps = min(50, len(selected_indices) * len(unselected_indices))
        swap_count = 0
        
        for i in selected_indices:
            if swap_count >= max_swaps:
                break
                
            # 타임아웃 체크
            if millis() - start_time > self.timeout_ms:
                self.timeouts += 1
                break
                
            for j in unselected_indices:
                swap_count += 1
                
                # i를 제거하고 j를 추가
                new_weight = current_weight - weights[i] + weights[j]
                new_error = self.calculate_error(new_weight)
                
                arduino.simulate_instruction_delay('arithmetic', 3)
                
                if new_error < best_error:
                    best_indices = [idx if idx != i else j for idx in selected_indices]
                    best_weight = new_weight
                    best_error = new_error
                    
                    # 목표 달성 시 즉시 반환
                    if self.is_valid_weight(new_weight):
                        return best_indices, best_weight
        
        return best_indices, best_weight


class DynamicProgrammingAlgorithm(CombinationAlgorithm):
    """Algorithm 3: Dynamic Programming (Arduino R4 제한적 구현)"""
    
    def __init__(self):
        super().__init__("Dynamic_Programming")
        # Arduino R4 Minima 메모리 제약 고려
        self.approximation_factor = 10  # 10g 단위로 반올림
        self.max_capacity = 2100 // self.approximation_factor  # 210 (2100g/10g)
        
        # 실제 사용 가능 메모리 계산 (32KB 중 약 28KB 사용 가능)
        available_memory = arduino.specs.SRAM_SIZE - 4000  # 시스템 예약 메모리
        required_memory = self.max_capacity * 4  # int 배열
        
        if required_memory > available_memory:
            # 메모리 부족 시 더 큰 근사치 사용
            self.approximation_factor = 20  # 20g 단위
            self.max_capacity = 2100 // self.approximation_factor  # 105
    
    def find_combination(self, weights: List[int]) -> CombinationResult:
        start_time = millis()
        start_memory = arduino._memory_used
        
        # 메모리 할당 체크
        required_memory = self.max_capacity * 4 + len(weights) * 4
        if not arduino.allocate_memory(required_memory):
            # DP 메모리 부족 시 그리디로 폴백
            greedy_algo = GreedyLocalSearchAlgorithm()
            return greedy_algo.find_combination(weights)
        
        try:
            # 무게를 근사치로 변환
            approx_weights = [w // self.approximation_factor for w in weights]
            target_approx = self.target_weight // self.approximation_factor
            
            arduino.simulate_instruction_delay('arithmetic', len(weights))
            
            # DP 테이블 초기화
            dp = [False] * (self.max_capacity + 1)
            dp[0] = True  # 0은 항상 가능
            parent = [-1] * (self.max_capacity + 1)  # 역추적용
            
            arduino.simulate_instruction_delay('memory_access', self.max_capacity)
            
            # DP 수행
            for i, weight in enumerate(approx_weights):
                # 타임아웃 체크
                if millis() - start_time > self.timeout_ms:
                    self.timeouts += 1
                    break
                
                # 역순으로 업데이트 (중복 선택 방지)
                for capacity in range(self.max_capacity, weight - 1, -1):
                    if dp[capacity - weight]:
                        if not dp[capacity]:
                            dp[capacity] = True
                            parent[capacity] = i
                    
                    arduino.simulate_instruction_delay('memory_access', 1)
                    arduino.simulate_instruction_delay('comparison', 1)
            
            # 최적해 찾기 (목표에 가장 가까운 값)
            best_capacity = self._find_best_capacity(dp, target_approx)
            
            # 역추적으로 선택된 아이템 찾기
            selected_indices = self._backtrack(parent, approx_weights, best_capacity)
            
            # 실제 무게 계산
            total_weight = sum(weights[i] for i in selected_indices)
            
            execution_time = millis() - start_time
            memory_used = arduino._memory_used - start_memory
            is_success = self.is_valid_weight(total_weight)
            error = self.calculate_error(total_weight)
            timeout_occurred = execution_time > self.timeout_ms
            
            self.total_calls += 1
            if is_success:
                self.total_success += 1
            self.total_time_ms += execution_time
            
            return CombinationResult(
                selected_indices=selected_indices,
                total_weight=total_weight,
                is_success=is_success,
                error=error,
                execution_time_ms=execution_time,
                memory_used_bytes=memory_used,
                iterations=len(weights) * self.max_capacity // 100,  # 대략적 반복 수
                timeout=timeout_occurred
            )
            
        except Exception as e:
            self.stack_overflows += 1
            return CombinationResult([], 0, False, self.target_weight, 0, 0, 0, False)
        
        finally:
            arduino.free_memory(required_memory)
    
    def _find_best_capacity(self, dp: List[bool], target: int) -> int:
        """목표에 가장 가까운 달성 가능한 용량 찾기"""
        best_capacity = 0
        min_error = float('inf')
        
        for capacity in range(len(dp)):
            if dp[capacity]:
                error = abs(target - capacity)
                if error < min_error:
                    min_error = error
                    best_capacity = capacity
                    
                arduino.simulate_instruction_delay('comparison', 1)
        
        return best_capacity
    
    def _backtrack(self, parent: List[int], weights: List[int], capacity: int) -> List[int]:
        """역추적으로 선택된 아이템 찾기"""
        selected = []
        current_capacity = capacity
        
        while current_capacity > 0 and parent[current_capacity] != -1:
            item_index = parent[current_capacity]
            selected.append(item_index)
            current_capacity -= weights[item_index]
            
            arduino.simulate_instruction_delay('memory_access', 1)
        
        return selected


# 알고리즘 팩토리
def create_all_algorithms() -> List[CombinationAlgorithm]:
    """모든 조합 알고리즘 인스턴스 생성"""
    return [
        RandomSamplingAlgorithm(),
        GreedyLocalSearchAlgorithm(),
        DynamicProgrammingAlgorithm()
    ]


if __name__ == "__main__":
    # 기본 테스트
    algorithms = create_all_algorithms()
    test_weights = [520, 580, 610, 650, 700, 530, 590, 620, 660, 510, 540, 630]
    
    print("Combination Algorithms Test")
    print("=" * 50)
    print(f"Test weights: {test_weights}")
    print(f"Total weight: {sum(test_weights)}g")
    print(f"Target: 2000g (±70g)")
    print()
    
    for algo in algorithms:
        print(f"{algo.name} Test:")
        result = algo.find_combination(test_weights)
        
        if result.selected_indices:
            selected_weights = [test_weights[i] for i in result.selected_indices]
            print(f"  Selected indices: {result.selected_indices}")
            print(f"  Selected weights: {selected_weights}")
            print(f"  Total weight: {result.total_weight}g")
            print(f"  Success: {result.is_success}")
            print(f"  Error: {result.error}g")
            print(f"  Time: {result.execution_time_ms}ms")
            print(f"  Memory: {result.memory_used_bytes} bytes")
            print(f"  Iterations: {result.iterations}")
        else:
            print(f"  Failed to find combination")
        
        print(f"  Algorithm stats: {algo.get_stats()}")
        print()
        
    print(f"Arduino stats: {arduino.get_stats()}")
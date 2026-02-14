"""
Phase 1-5 문서 기반 난수 생성기 6종 구현
- Arduino random() (LFSR 기반)
- XorShift32 (32비트 경량 PRNG)
- Hybrid (XorShift32 + analogRead 노이즈 혼합)
- 숙련자 모델 (가우시안 분포 + 균등화 보정)
- 초보자 모델 (특정 구간 선호 + 피로도 누적)
- 랜덤 작업자 (100세트마다 패턴 변경)
"""

import random
import math
from typing import List, Optional
from abc import ABC, abstractmethod
from arduino_r4_simulator import arduino, millis, analogRead


class RandomGenerator(ABC):
    """난수 생성기 베이스 클래스"""
    
    def __init__(self, name: str):
        self.name = name
        self.memory_usage = 0  # 바이트 단위
        self.call_count = 0
        self.total_time_ms = 0
    
    @abstractmethod
    def generate_weights(self, count: int = 12) -> List[int]:
        """500-700g 범위에서 count개의 무게 생성"""
        pass
    
    def get_stats(self) -> dict:
        """생성기 통계 반환"""
        avg_time = self.total_time_ms / max(1, self.call_count)
        return {
            'name': self.name,
            'memory_usage': self.memory_usage,
            'call_count': self.call_count,
            'avg_time_ms': avg_time,
            'total_time_ms': self.total_time_ms
        }


class ArduinoRandomGenerator(RandomGenerator):
    """Arduino 기본 random() 함수 (LFSR 기반) 시뮬레이션"""
    
    def __init__(self):
        super().__init__("Arduino_Random")
        self.memory_usage = 4  # seed용 4바이트
        self._seed_initialized = False
    
    def generate_weights(self, count: int = 12) -> List[int]:
        start_time = millis()
        
        # Arduino seed 방식: millis() + analogRead(A0)
        if not self._seed_initialized:
            seed_value = millis() + analogRead(0)
            random.seed(seed_value)
            self._seed_initialized = True
            arduino.simulate_instruction_delay('function_call', 1)
        
        weights = []
        for _ in range(count):
            # Arduino random() 특성: LFSR 기반으로 약간의 편향성
            raw_value = random.randint(500, 700)
            
            # Arduino random()의 미세한 편향 시뮬레이션
            if raw_value > 650:  # 높은 값에서 약간 감소 편향
                raw_value = int(raw_value * 0.98)
            elif raw_value < 550:  # 낮은 값에서 약간 증가 편향  
                raw_value = int(raw_value * 1.02)
            
            weights.append(max(500, min(700, raw_value)))
            arduino.simulate_instruction_delay('random', 1)
        
        end_time = millis()
        self.call_count += 1
        self.total_time_ms += (end_time - start_time)
        
        return weights


class XorShift32Generator(RandomGenerator):
    """XorShift32 32비트 경량 PRNG (RAM 4바이트만 사용)"""
    
    def __init__(self):
        super().__init__("XorShift32")
        self.memory_usage = 4  # state용 4바이트
        self._state = None
    
    def _xorshift32(self) -> int:
        """XorShift32 알고리즘"""
        if self._state is None:
            # 초기 시드: millis() + analogRead(A0)
            self._state = (millis() + analogRead(0)) & 0xFFFFFFFF
            if self._state == 0:
                self._state = 1  # 0이면 안됨
        
        arduino.simulate_instruction_delay('arithmetic', 4)  # XOR 연산 4회
        
        self._state ^= self._state << 13
        self._state ^= self._state >> 17
        self._state ^= self._state << 5
        self._state &= 0xFFFFFFFF  # 32비트 유지
        
        return self._state
    
    def generate_weights(self, count: int = 12) -> List[int]:
        start_time = millis()
        
        weights = []
        for _ in range(count):
            # XorShift32로 500-700 범위 생성
            random_value = self._xorshift32()
            weight = 500 + (random_value % 201)  # 500-700 범위
            weights.append(weight)
        
        end_time = millis()
        self.call_count += 1
        self.total_time_ms += (end_time - start_time)
        
        return weights


class HybridGenerator(RandomGenerator):
    """Hybrid: XorShift32 + analogRead(A0) 노이즈 혼합"""
    
    def __init__(self):
        super().__init__("Hybrid")
        self.memory_usage = 4  # XorShift32 state용 4바이트
        self._xorshift = XorShift32Generator()
    
    def generate_weights(self, count: int = 12) -> List[int]:
        start_time = millis()
        
        weights = []
        for _ in range(count):
            # XorShift32 기본값
            xor_weights = self._xorshift.generate_weights(1)
            base_weight = xor_weights[0]
            
            # analogRead 노이즈 혼합 (±20g 범위)
            analog_noise = analogRead(0)  # 0-1023 범위
            noise_factor = ((analog_noise % 41) - 20)  # ±20 범위
            
            hybrid_weight = base_weight + noise_factor
            weights.append(max(500, min(700, hybrid_weight)))
            
            arduino.simulate_instruction_delay('analog_read', 1)
        
        end_time = millis()
        self.call_count += 1
        self.total_time_ms += (end_time - start_time)
        
        return weights


class ExpertGenerator(RandomGenerator):
    """숙련자 모델: 가우시안 분포 + 균등화 보정"""
    
    def __init__(self):
        super().__init__("Expert")
        self.memory_usage = 8  # 평균, 표준편차 저장용
        self._mean = 600  # 평균 무게
        self._std_dev = 40  # 표준편차
    
    def generate_weights(self, count: int = 12) -> List[int]:
        start_time = millis()
        
        weights = []
        
        # 가우시안 분포로 기본 생성
        for _ in range(count):
            # Box-Muller 변환으로 정규분포 생성
            u1 = random.random()
            u2 = random.random()
            z = math.sqrt(-2 * math.log(u1)) * math.cos(2 * math.pi * u2)
            
            gaussian_weight = int(self._mean + z * self._std_dev)
            weights.append(max(500, min(700, gaussian_weight)))
            
            arduino.simulate_instruction_delay('arithmetic', 10)  # 복잡한 수학 연산
        
        # 균등화 보정: 극단값들을 중간으로 조정
        weights = self._equalization_correction(weights)
        
        end_time = millis()
        self.call_count += 1
        self.total_time_ms += (end_time - start_time)
        
        return weights
    
    def _equalization_correction(self, weights: List[int]) -> List[int]:
        """균등화 보정 알고리즘"""
        # 구간별 카운트: [500-540], [540-580], [580-620], [620-660], [660-700]
        bins = [0] * 5
        for weight in weights:
            bin_idx = min(4, (weight - 500) // 40)
            bins[bin_idx] += 1
        
        # 과도하게 집중된 구간의 값들을 다른 구간으로 재분배
        corrected_weights = weights.copy()
        target_per_bin = len(weights) // 5  # 이상적 균등 분포
        
        for i, count in enumerate(bins):
            if count > target_per_bin + 1:  # 과도하게 많은 구간
                # 해당 구간의 일부 값들을 인접 구간으로 이동
                excess = count - target_per_bin
                bin_start = 500 + i * 40
                bin_end = bin_start + 40
                
                for j, weight in enumerate(corrected_weights):
                    if bin_start <= weight < bin_end and excess > 0:
                        # 인접 구간으로 이동
                        if i > 0:
                            corrected_weights[j] = weight - 20
                        elif i < 4:
                            corrected_weights[j] = weight + 20
                        excess -= 1
        
        return [max(500, min(700, w)) for w in corrected_weights]


class BeginnerGenerator(RandomGenerator):
    """초보자 모델: 특정 구간 선호(580-620g 60%) + 피로도 누적"""
    
    def __init__(self):
        super().__init__("Beginner")
        self.memory_usage = 8  # 피로도, 선호 구간 정보
        self._fatigue_level = 0.0  # 0.0-1.0 피로도
        self._preferred_min = 580
        self._preferred_max = 620
        self._set_count = 0
    
    def generate_weights(self, count: int = 12) -> List[int]:
        start_time = millis()
        
        weights = []
        preferred_count = int(count * 0.6)  # 60%는 선호 구간
        
        # 피로도에 따른 선호 구간 확대
        fatigue_expansion = int(self._fatigue_level * 20)  # 최대 20g 확대
        current_min = max(500, self._preferred_min - fatigue_expansion)
        current_max = min(700, self._preferred_max + fatigue_expansion)
        
        # 선호 구간 60% 생성
        for _ in range(preferred_count):
            weight = random.randint(current_min, current_max)
            weights.append(weight)
            arduino.simulate_instruction_delay('random', 1)
        
        # 나머지 40%는 전체 범위에서 생성
        remaining_count = count - preferred_count
        for _ in range(remaining_count):
            weight = random.randint(500, 700)
            weights.append(weight)
            arduino.simulate_instruction_delay('random', 1)
        
        # 피로도 누적 (100세트마다 0.1씩 증가)
        self._set_count += 1
        if self._set_count % 100 == 0:
            self._fatigue_level = min(1.0, self._fatigue_level + 0.1)
        
        end_time = millis()
        self.call_count += 1
        self.total_time_ms += (end_time - start_time)
        
        random.shuffle(weights)  # 순서 섞기
        return weights


class RandomWorkerGenerator(RandomGenerator):
    """랜덤 작업자: 매 100세트마다 패턴 변경"""
    
    def __init__(self):
        super().__init__("RandomWorker")
        self.memory_usage = 12  # 패턴 정보, 카운터 등
        self._set_count = 0
        self._current_pattern = "uniform"
        self._pattern_params = {}
        self._change_pattern()
    
    def generate_weights(self, count: int = 12) -> List[int]:
        start_time = millis()
        
        # 100세트마다 패턴 변경
        if self._set_count > 0 and self._set_count % 100 == 0:
            self._change_pattern()
        
        weights = self._generate_by_pattern(count)
        self._set_count += 1
        
        end_time = millis()
        self.call_count += 1
        self.total_time_ms += (end_time - start_time)
        
        return weights
    
    def _change_pattern(self):
        """패턴 변경 (5가지 패턴 순환)"""
        patterns = ["uniform", "normal", "bimodal", "skewed_low", "skewed_high"]
        pattern_idx = (self._set_count // 100) % len(patterns)
        self._current_pattern = patterns[pattern_idx]
        
        # 패턴별 파라미터 설정
        if self._current_pattern == "uniform":
            self._pattern_params = {}
        elif self._current_pattern == "normal":
            self._pattern_params = {"mean": 600, "std": 30}
        elif self._current_pattern == "bimodal":
            self._pattern_params = {"peak1": 550, "peak2": 650}
        elif self._current_pattern == "skewed_low":
            self._pattern_params = {"bias": 0.3}  # 낮은 값 선호
        elif self._current_pattern == "skewed_high":
            self._pattern_params = {"bias": 0.7}  # 높은 값 선호
    
    def _generate_by_pattern(self, count: int) -> List[int]:
        """현재 패턴에 따른 무게 생성"""
        weights = []
        
        if self._current_pattern == "uniform":
            # 균등 분포
            for _ in range(count):
                weight = random.randint(500, 700)
                weights.append(weight)
                arduino.simulate_instruction_delay('random', 1)
        
        elif self._current_pattern == "normal":
            # 정규 분포
            mean = self._pattern_params["mean"]
            std = self._pattern_params["std"]
            for _ in range(count):
                weight = int(random.gauss(mean, std))
                weights.append(max(500, min(700, weight)))
                arduino.simulate_instruction_delay('arithmetic', 5)
        
        elif self._current_pattern == "bimodal":
            # 이봉 분포
            peak1 = self._pattern_params["peak1"]
            peak2 = self._pattern_params["peak2"]
            for _ in range(count):
                if random.random() < 0.5:
                    weight = random.randint(peak1 - 25, peak1 + 25)
                else:
                    weight = random.randint(peak2 - 25, peak2 + 25)
                weights.append(max(500, min(700, weight)))
                arduino.simulate_instruction_delay('random', 2)
        
        elif self._current_pattern == "skewed_low":
            # 낮은 값 편향
            bias = self._pattern_params["bias"]
            for _ in range(count):
                u = random.random()
                # 편향 적용: 낮은 값에 더 많은 확률
                skewed_u = u ** (1/bias)
                weight = int(500 + skewed_u * 200)
                weights.append(max(500, min(700, weight)))
                arduino.simulate_instruction_delay('arithmetic', 3)
        
        elif self._current_pattern == "skewed_high":
            # 높은 값 편향
            bias = self._pattern_params["bias"]
            for _ in range(count):
                u = random.random()
                # 편향 적용: 높은 값에 더 많은 확률
                skewed_u = u ** bias
                weight = int(500 + skewed_u * 200)
                weights.append(max(500, min(700, weight)))
                arduino.simulate_instruction_delay('arithmetic', 3)
        
        return weights


# 생성기 팩토리
def create_all_generators() -> List[RandomGenerator]:
    """모든 생성기 인스턴스 생성"""
    return [
        ArduinoRandomGenerator(),
        XorShift32Generator(),
        HybridGenerator(),
        ExpertGenerator(),
        BeginnerGenerator(),
        RandomWorkerGenerator()
    ]


if __name__ == "__main__":
    # 기본 테스트
    generators = create_all_generators()
    
    print("Random Generators Test")
    print("=" * 50)
    
    for gen in generators:
        print(f"\n{gen.name} Test:")
        weights = gen.generate_weights(12)
        print(f"  Generated weights: {weights}")
        print(f"  Stats: {gen.get_stats()}")
        
        # 구간별 분포 확인
        bins = [0] * 5  # [500-540], [540-580], [580-620], [620-660], [660-700]
        for weight in weights:
            bin_idx = min(4, (weight - 500) // 40)
            bins[bin_idx] += 1
        print(f"  Distribution: {bins}")
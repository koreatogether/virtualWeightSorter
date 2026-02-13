"""
Arduino R4 Minima Mock Simulator
- Arduino R4 Minima 하드웨어 제약사항을 정확히 시뮬레이션
- 32KB RAM, 256KB Flash 메모리 제한
- 16MHz ARM Cortex-M4 처리 성능
- analogRead(), millis(), random() 함수 모의
"""

import time
import random
import threading
from typing import List, Optional, Dict, Any
from dataclasses import dataclass
from enum import Enum


class ArduinoError(Exception):
    """Arduino 시뮬레이션 관련 예외"""
    pass


@dataclass
class ArduinoSpecs:
    """Arduino R4 Minima 하드웨어 사양"""
    SRAM_SIZE: int = 32768  # 32KB RAM
    FLASH_SIZE: int = 262144  # 256KB Flash
    EEPROM_SIZE: int = 4096  # 4KB EEPROM
    CLOCK_SPEED: int = 16000000  # 16MHz
    MAX_ANALOG_READ: int = 1023
    MAX_MILLIS: int = 4294967295  # 2^32 - 1 (49.7일 후 오버플로우)


class ArduinoR4Minima:
    """Arduino R4 Minima 하드웨어 시뮬레이터"""
    
    def __init__(self):
        self.specs = ArduinoSpecs()
        self._boot_time = time.time()
        self._memory_used = 0
        self._stack_depth = 0
        self._max_stack_depth = 0
        self._analog_noise_level = 50  # analogRead 노이즈 레벨
        self._random_seed = None
        
        # 실행 시간 시뮬레이션을 위한 명령어 가중치
        self._instruction_cycles = {
            'arithmetic': 1,    # 산술연산
            'comparison': 1,    # 비교연산  
            'memory_access': 2, # 메모리 접근
            'function_call': 5, # 함수 호출
            'loop': 1,         # 반복문
            'random': 10,      # 난수 생성
            'analog_read': 100 # 아날로그 읽기
        }
    
    def millis(self) -> int:
        """Arduino millis() 함수 시뮬레이션"""
        elapsed_ms = int((time.time() - self._boot_time) * 1000)
        return elapsed_ms % (self.specs.MAX_MILLIS + 1)
    
    def analogRead(self, pin: int) -> int:
        """Arduino analogRead() 함수 시뮬레이션 (노이즈만)"""
        if pin < 0 or pin > 7:  # A0-A7
            raise ArduinoError(f"Invalid analog pin: {pin}")
        
        # analogRead 기본 노이즈 시뮬레이션
        base_noise = random.randint(-self._analog_noise_level, self._analog_noise_level)
        
        noise_value = int(512 + base_noise)  # 중앙값 512에서 노이즈 추가
        return max(0, min(self.specs.MAX_ANALOG_READ, noise_value))
    
    def random_arduino(self, max_value: int) -> int:
        """Arduino random() 함수 시뮬레이션 (LFSR 기반)"""
        if self._random_seed is None:
            # Arduino 특성: millis() + analogRead(A0) 시드
            self._random_seed = self.millis() + self.analogRead(0)
            random.seed(self._random_seed)
        
        # LFSR 특성 시뮬레이션: 약간의 편향성 추가
        raw_random = random.randint(0, max_value - 1)
        # Arduino random()의 미세한 편향 시뮬레이션
        if raw_random > max_value * 0.9:
            raw_random = int(raw_random * 0.98)  # 높은 값 약간 감소
        
        return raw_random
    
    def allocate_memory(self, size: bytes) -> bool:
        """메모리 할당 시뮬레이션"""
        if self._memory_used + size > self.specs.SRAM_SIZE:
            return False
        self._memory_used += size
        return True
    
    def free_memory(self, size: bytes):
        """메모리 해제 시뮬레이션"""
        self._memory_used = max(0, self._memory_used - size)
    
    def get_free_memory(self) -> int:
        """사용 가능한 메모리 크기 반환"""
        return self.specs.SRAM_SIZE - self._memory_used
    
    def push_stack(self, size: bytes = 4) -> bool:
        """스택 푸시 시뮬레이션"""
        if self._memory_used + self._stack_depth * 4 + size > self.specs.SRAM_SIZE:
            raise ArduinoError("Stack Overflow!")
        
        self._stack_depth += 1
        self._max_stack_depth = max(self._max_stack_depth, self._stack_depth)
        return True
    
    def pop_stack(self, size: bytes = 4):
        """스택 팝 시뮬레이션"""
        if self._stack_depth > 0:
            self._stack_depth -= 1
    
    def simulate_instruction_delay(self, instruction_type: str, count: int = 1):
        """명령어 실행 시간 시뮬레이션"""
        cycles = self._instruction_cycles.get(instruction_type, 1) * count
        delay_seconds = cycles / self.specs.CLOCK_SPEED
        time.sleep(delay_seconds)
    
    def set_analog_noise(self, noise_level: int):
        """analogRead 노이즈 레벨 설정"""
        self._analog_noise_level = noise_level
    
    def get_stats(self) -> Dict[str, Any]:
        """하드웨어 통계 반환"""
        return {
            'memory_used': self._memory_used,
            'memory_free': self.get_free_memory(),
            'memory_usage_percent': (self._memory_used / self.specs.SRAM_SIZE) * 100,
            'max_stack_depth': self._max_stack_depth,
            'current_stack_depth': self._stack_depth,
            'uptime_ms': self.millis(),
            'analog_noise_level': self._analog_noise_level
        }
    
    def reset(self):
        """Arduino 리셋 시뮬레이션"""
        self._boot_time = time.time()
        self._memory_used = 0
        self._stack_depth = 0
        self._max_stack_depth = 0
        self._random_seed = None


# 글로벌 Arduino 인스턴스
arduino = ArduinoR4Minima()

# Arduino 함수들을 전역 함수로 제공
def millis() -> int:
    return arduino.millis()

def analogRead(pin: int) -> int:
    return arduino.analogRead(pin)

def random_func(max_value: int) -> int:
    return arduino.random_arduino(max_value)

def free_memory() -> int:
    return arduino.get_free_memory()


if __name__ == "__main__":
    # 기본 테스트
    print("Arduino R4 Minima Simulator Test")
    print("=" * 40)
    
    print(f"Boot time: {millis()}ms")
    print(f"Analog A0: {analogRead(0)}")
    print(f"Random(1000): {random_func(1000)}")
    print(f"Free memory: {free_memory()} bytes")
    
    print("\nHardware Stats:")
    stats = arduino.get_stats()
    for key, value in stats.items():
        print(f"  {key}: {value}")
"""
Arduino Uno R4 WiFi Mock Simulator
정확한 하드웨어 사양을 반영한 Arduino 함수 시뮬레이션

Hardware Specifications:
- MCU: Renesas RA4M1 (ARM Cortex-M4)
- Clock Speed: 48MHz
- Flash Memory: 256KB
- SRAM: 32KB
- EEPROM: 8KB (emulated)
- Digital Pins: 14 (6 PWM)
- Analog Inputs: 6 (12-bit ADC)
- Operating Voltage: 5V
"""

import random
import time
from dataclasses import dataclass
from enum import Enum
from typing import Any, Dict, List, Optional


class PinMode(Enum):
    INPUT = 0
    OUTPUT = 1
    INPUT_PULLUP = 2


@dataclass
class HardwareSpecs:
    """Arduino Uno R4 WiFi 하드웨어 사양"""

    # MCU 사양
    mcu_name: str = "Renesas RA4M1"
    architecture: str = "ARM Cortex-M4"
    clock_speed_hz: int = 48_000_000  # 48MHz

    # 메모리 사양
    flash_memory_bytes: int = 256 * 1024  # 256KB
    sram_bytes: int = 32 * 1024  # 32KB
    eeprom_bytes: int = 8 * 1024  # 8KB (emulated)

    # I/O 사양
    digital_pins: int = 14
    analog_pins: int = 6
    pwm_pins: List[int] = None

    # ADC 사양
    adc_resolution: int = 12  # 12-bit ADC
    adc_max_value: int = 4095  # 2^12 - 1

    # 전압 사양
    operating_voltage: float = 5.0  # 5V
    analog_reference: float = 5.0  # 5V

    def __post_init__(self):
        if self.pwm_pins is None:
            self.pwm_pins = [3, 5, 6, 9, 10, 11]  # PWM 지원 핀


class ArduinoUnoR4WiFiMock:
    """Arduino Uno R4 WiFi 정확한 하드웨어 시뮬레이션"""

    def __init__(self, seed: Optional[int] = None):
        self.specs = HardwareSpecs()

        # 시간 관련
        self.start_time = time.time()
        self.micros_start = time.perf_counter()

        # 랜덤 시드 설정
        if seed is not None:
            random.seed(seed)
            self._random_seed = seed
        else:
            self._random_seed = int(time.time())
            random.seed(self._random_seed)

        # 메모리 시뮬레이션
        self.sram_usage = 0
        self.flash_usage = 0
        self.eeprom_data = bytearray(self.specs.eeprom_bytes)

        # 핀 상태 관리
        self.digital_pins_mode = [PinMode.INPUT] * self.specs.digital_pins
        self.digital_pins_value = [0] * self.specs.digital_pins
        self.analog_pins_value = [0] * self.specs.analog_pins

        # PWM 상태
        self.pwm_values = dict.fromkeys(self.specs.pwm_pins, 0)

        # Serial 출력 버퍼
        self.serial_output = []
        self.serial_baud_rate = 9600

        # 성능 카운터
        self.instruction_count = 0
        self.function_calls = {}

        # 인터럽트 시뮬레이션
        self.interrupts_enabled = True
        self.interrupt_handlers = {}

        print("Arduino Uno R4 WiFi Mock initialized")
        print(
            f"MCU: {self.specs.mcu_name} @ {self.specs.clock_speed_hz/1_000_000:.0f}MHz"
        )
        print(
            f"SRAM: {self.specs.sram_bytes//1024}KB, Flash: {self.specs.flash_memory_bytes//1024}KB"
        )
        print(f"Random seed: {self._random_seed}")

    def _count_instruction(self, cycles: int = 1):
        """명령어 사이클 카운트 (성능 측정용)"""
        self.instruction_count += cycles

    def _count_function_call(self, func_name: str):
        """함수 호출 횟수 카운트"""
        self.function_calls[func_name] = self.function_calls.get(func_name, 0) + 1

    # ==================== 시간 관련 함수 ====================

    def millis(self) -> int:
        """
        Arduino millis() 함수 시뮬레이션
        48MHz 클럭 기준으로 정확한 타이밍 계산
        """
        self._count_function_call("millis")
        self._count_instruction(4)  # millis() 함수 호출 오버헤드

        elapsed_seconds = time.time() - self.start_time
        # 48MHz 클럭 기준으로 밀리초 계산
        millis_value = int(elapsed_seconds * 1000) % (2**32)
        return millis_value

    def micros(self) -> int:
        """
        Arduino micros() 함수 시뮬레이션
        48MHz 클럭 기준 마이크로초 정밀도
        """
        self._count_function_call("micros")
        self._count_instruction(6)  # micros() 함수 호출 오버헤드

        elapsed_seconds = time.perf_counter() - self.micros_start
        # 48MHz 클럭 기준으로 마이크로초 계산
        micros_value = int(elapsed_seconds * 1_000_000) % (2**32)
        return micros_value

    def delay(self, ms: int):
        """
        Arduino delay() 함수 시뮬레이션
        실제 시간 지연 + 클럭 사이클 계산
        """
        self._count_function_call("delay")
        # delay() 함수는 많은 클럭 사이클 소모
        cycles = ms * (self.specs.clock_speed_hz // 1000)
        self._count_instruction(cycles)

        time.sleep(ms / 1000.0)

    def delayMicroseconds(self, us: int):
        """Arduino delayMicroseconds() 함수 시뮬레이션"""
        self._count_function_call("delayMicroseconds")
        cycles = us * (self.specs.clock_speed_hz // 1_000_000)
        self._count_instruction(cycles)

        time.sleep(us / 1_000_000.0)

    # ==================== 랜덤 함수 ====================

    def randomSeed(self, seed: int):
        """Arduino randomSeed() 함수 시뮬레이션"""
        self._count_function_call("randomSeed")
        self._count_instruction(10)

        random.seed(seed)
        self._random_seed = seed

    def random_range(self, min_val: int, max_val: int) -> int:
        """
        Arduino random(min, max) 함수 시뮬레이션
        Arduino는 max 값을 포함하지 않음 (exclusive)
        """
        self._count_function_call("random")
        self._count_instruction(20)  # 랜덤 생성은 상대적으로 무거운 연산

        if min_val >= max_val:
            return min_val

        return random.randint(min_val, max_val - 1)

    def random_max(self, max_val: int) -> int:
        """Arduino random(max) 함수 시뮬레이션"""
        return self.random_range(0, max_val)

    # ==================== 디지털 I/O ====================

    def pinMode(self, pin: int, mode: PinMode):
        """Arduino pinMode() 함수 시뮬레이션"""
        self._count_function_call("pinMode")
        self._count_instruction(5)

        if 0 <= pin < self.specs.digital_pins:
            self.digital_pins_mode[pin] = mode

    def digitalWrite(self, pin: int, value: int):
        """Arduino digitalWrite() 함수 시뮬레이션"""
        self._count_function_call("digitalWrite")
        self._count_instruction(8)

        if 0 <= pin < self.specs.digital_pins:
            if self.digital_pins_mode[pin] == PinMode.OUTPUT:
                self.digital_pins_value[pin] = 1 if value else 0

    def digitalRead(self, pin: int) -> int:
        """Arduino digitalRead() 함수 시뮬레이션"""
        self._count_function_call("digitalRead")
        self._count_instruction(6)

        if 0 <= pin < self.specs.digital_pins:
            if self.digital_pins_mode[pin] in [PinMode.INPUT, PinMode.INPUT_PULLUP]:
                # 실제 하드웨어에서는 노이즈나 플로팅 상태 시뮬레이션
                if self.digital_pins_mode[pin] == PinMode.INPUT_PULLUP:
                    return 1  # 풀업 저항으로 HIGH
                return self.digital_pins_value[pin]
        return 0

    # ==================== 아날로그 I/O ====================

    def analogRead(self, pin: int) -> int:
        """
        Arduino analogRead() 함수 시뮬레이션
        12-bit ADC (0-4095) 시뮬레이션
        """
        self._count_function_call("analogRead")
        # ADC 변환은 상당한 시간 소요 (약 100 클럭 사이클)
        self._count_instruction(100)

        if 0 <= pin < self.specs.analog_pins:
            # 기본값에 ADC 노이즈 추가 (실제 하드웨어 특성 반영)
            base_value = self.analog_pins_value[pin]
            noise = random.gauss(0, 2)  # 평균 0, 표준편차 2의 가우시안 노이즈

            result = int(base_value + noise)
            return max(0, min(self.specs.adc_max_value, result))

        return 0

    def analogWrite(self, pin: int, value: int):
        """Arduino analogWrite() (PWM) 함수 시뮬레이션"""
        self._count_function_call("analogWrite")
        self._count_instruction(12)

        if pin in self.specs.pwm_pins:
            # PWM 값은 0-255 범위
            pwm_value = max(0, min(255, value))
            self.pwm_values[pin] = pwm_value

    # ==================== Serial 통신 ====================

    def Serial_begin(self, baud_rate: int = 9600):
        """Arduino Serial.begin() 함수 시뮬레이션"""
        self._count_function_call("Serial.begin")
        self._count_instruction(50)  # Serial 초기화는 상당한 오버헤드

        self.serial_baud_rate = baud_rate
        self.serial_output.clear()
        print(f"Serial initialized at {baud_rate} baud")

    def Serial_print(self, value: Any):
        """Arduino Serial.print() 함수 시뮬레이션"""
        self._count_function_call("Serial.print")
        # Serial 출력은 상당한 시간 소요 (baud rate 의존)
        char_count = len(str(value))
        cycles = char_count * (self.specs.clock_speed_hz // self.serial_baud_rate) * 10
        self._count_instruction(cycles)

        output = str(value)
        self.serial_output.append(output)
        print(output, end="")

    def Serial_println(self, value: Any = ""):
        """Arduino Serial.println() 함수 시뮬레이션"""
        self.Serial_print(str(value) + "\n")

    # ==================== 메모리 관리 ====================

    def allocate_sram(self, bytes_needed: int) -> bool:
        """SRAM 메모리 할당 시뮬레이션"""
        if self.sram_usage + bytes_needed <= self.specs.sram_bytes:
            self.sram_usage += bytes_needed
            return True
        return False  # Out of memory

    def free_sram(self, bytes_freed: int):
        """SRAM 메모리 해제 시뮬레이션"""
        self.sram_usage = max(0, self.sram_usage - bytes_freed)

    def get_free_memory(self) -> int:
        """사용 가능한 SRAM 메모리 반환"""
        return self.specs.sram_bytes - self.sram_usage

    # ==================== 성능 모니터링 ====================

    def get_performance_stats(self) -> Dict[str, Any]:
        """성능 통계 반환"""
        elapsed_time = time.time() - self.start_time

        return {
            "elapsed_time_seconds": elapsed_time,
            "instruction_count": self.instruction_count,
            "instructions_per_second": (
                self.instruction_count / elapsed_time if elapsed_time > 0 else 0
            ),
            "function_calls": self.function_calls.copy(),
            "sram_usage_bytes": self.sram_usage,
            "sram_usage_percent": (self.sram_usage / self.specs.sram_bytes) * 100,
            "free_memory_bytes": self.get_free_memory(),
            "clock_speed_hz": self.specs.clock_speed_hz,
            "random_seed": self._random_seed,
        }

    def reset_performance_counters(self):
        """성능 카운터 리셋"""
        self.instruction_count = 0
        self.function_calls.clear()
        self.start_time = time.time()
        self.micros_start = time.perf_counter()

    # ==================== 하드웨어 정보 ====================

    def get_hardware_info(self) -> Dict[str, Any]:
        """하드웨어 정보 반환"""
        return {
            "board_name": "Arduino Uno R4 WiFi",
            "mcu": self.specs.mcu_name,
            "architecture": self.specs.architecture,
            "clock_speed_mhz": self.specs.clock_speed_hz / 1_000_000,
            "flash_memory_kb": self.specs.flash_memory_bytes // 1024,
            "sram_kb": self.specs.sram_bytes // 1024,
            "eeprom_kb": self.specs.eeprom_bytes // 1024,
            "digital_pins": self.specs.digital_pins,
            "analog_pins": self.specs.analog_pins,
            "pwm_pins": self.specs.pwm_pins,
            "adc_resolution": self.specs.adc_resolution,
            "operating_voltage": self.specs.operating_voltage,
        }


# ==================== 편의 함수들 ====================


def create_arduino_mock(seed: Optional[int] = None) -> ArduinoUnoR4WiFiMock:
    """Arduino Mock 인스턴스 생성 편의 함수"""
    return ArduinoUnoR4WiFiMock(seed=seed)


# ==================== 테스트 코드 ====================

if __name__ == "__main__":
    # 기본 테스트
    arduino = create_arduino_mock(seed=12345)

    print("\n=== Hardware Info ===")
    hw_info = arduino.get_hardware_info()
    for key, value in hw_info.items():
        print(f"{key}: {value}")

    print("\n=== Basic Function Test ===")
    arduino.Serial_begin(9600)
    arduino.Serial_println("Arduino Uno R4 WiFi Mock Test")

    # 랜덤 숫자 생성 테스트
    print("\nRandom numbers (0-2):")
    for i in range(10):
        num = arduino.random_range(0, 3)
        arduino.Serial_println(f"Random {i+1}: {num}")

    # 성능 통계
    print("\n=== Performance Stats ===")
    stats = arduino.get_performance_stats()
    for key, value in stats.items():
        if isinstance(value, float):
            print(f"{key}: {value:.2f}")
        else:
            print(f"{key}: {value}")

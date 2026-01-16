#!/usr/bin/env python3
"""
센서 로그 관리 서비스
개별 센서에 대한 상세 로그 정보를 수집하고 표시합니다.
"""

import time
from collections import deque
from dataclasses import dataclass, field
from datetime import datetime


@dataclass
class SensorStats:
    """개별 센서 통계 정보"""

    sensor_addr: str
    sensor_id: str

    # 측정 통계
    total_measurements: int = 0
    successful_measurements: int = 0
    failed_measurements: int = 0

    # 온도 통계
    min_temperature: float = float("inf")
    max_temperature: float = float("-inf")
    last_temperature: float = 0.0
    temperature_sum: float = 0.0

    # 통신 통계
    connection_failures: int = 0
    last_failure_time: str | None = None
    consecutive_successes: int = 0
    max_consecutive_successes: int = 0

    # 임계값 위반
    high_threshold_violations: int = 0
    low_threshold_violations: int = 0

    # 최근 측정값들 (최대 100개 보관)
    recent_temperatures: deque = field(default_factory=lambda: deque(maxlen=100))
    recent_timestamps: deque = field(default_factory=lambda: deque(maxlen=100))

    # 로그 카운터
    log_counter: int = 0

    def get_average_temperature(self) -> float:
        """평균 온도 계산"""
        if self.successful_measurements > 0:
            return round(self.temperature_sum / self.successful_measurements, 1)
        return 0.0

    def get_success_rate(self) -> float:
        """측정 성공률 계산"""
        if self.total_measurements > 0:
            return round(
                (self.successful_measurements / self.total_measurements) * 100, 1
            )
        return 0.0

    def get_temperature_change_rate(self) -> float:
        """최근 온도 변화율 계산 (°C/분)"""
        if len(self.recent_temperatures) < 2:
            return 0.0

        # 최근 5개 측정값으로 변화율 계산
        recent_count = min(5, len(self.recent_temperatures))
        if recent_count < 2:
            return 0.0

        temp_diff = (
            self.recent_temperatures[-1] - self.recent_temperatures[-recent_count]
        )
        time_diff = self.recent_timestamps[-1] - self.recent_timestamps[-recent_count]

        if time_diff > 0:
            # 초당 변화율을 분당으로 변환
            return float(round((temp_diff / time_diff) * 60, 2))
        return 0.0


class SensorLogger:
    """센서 로그 관리자"""

    def __init__(self) -> None:
        self.sensors: dict[str, SensorStats] = {}
        self.system_start_time = time.time()
        self.log_interval = 10  # 10개 측정마다 로그 출력

    def update_sensor_data(self, sensor_data: dict) -> None:
        """센서 데이터 업데이트"""
        sensor_addr = sensor_data.get("sensor_addr", "UNKNOWN")
        sensor_id = sensor_data.get("sensor_id", "00")
        temperature = sensor_data.get("temperature", 0.0)
        th_value = sensor_data.get("th_value", 30.0)
        tl_value = sensor_data.get("tl_value", 15.0)

        # 센서 통계 객체 생성 또는 가져오기
        if sensor_addr not in self.sensors:
            self.sensors[sensor_addr] = SensorStats(
                sensor_addr=sensor_addr, sensor_id=sensor_id
            )

        stats = self.sensors[sensor_addr]
        stats.sensor_id = sensor_id  # ID 업데이트

        # 측정 통계 업데이트
        stats.total_measurements += 1
        current_time = time.time()

        # 온도값 유효성 검사
        if self._is_valid_temperature(temperature):
            stats.successful_measurements += 1
            stats.consecutive_successes += 1
            stats.max_consecutive_successes = max(
                stats.max_consecutive_successes, stats.consecutive_successes
            )

            # 온도 통계 업데이트
            stats.last_temperature = temperature
            stats.temperature_sum += temperature
            stats.min_temperature = min(stats.min_temperature, temperature)
            stats.max_temperature = max(stats.max_temperature, temperature)

            # 최근 측정값 저장
            stats.recent_temperatures.append(temperature)
            stats.recent_timestamps.append(current_time)

            # 임계값 위반 체크
            if temperature > th_value:
                stats.high_threshold_violations += 1
            elif temperature < tl_value:
                stats.low_threshold_violations += 1

        else:
            stats.failed_measurements += 1
            stats.connection_failures += 1
            stats.consecutive_successes = 0
            stats.last_failure_time = datetime.now().strftime("%H:%M:%S")

        # 로그 출력 (10개마다)
        stats.log_counter += 1
        if stats.log_counter >= self.log_interval:
            self._print_sensor_log(stats)
            stats.log_counter = 0

    def _is_valid_temperature(self, temperature: float) -> bool:
        """온도값 유효성 검사"""
        return -55.0 <= temperature <= 125.0 and temperature not in [-127.0, 85.0]

    def _print_sensor_log(self, stats: SensorStats) -> None:
        """센서 로그 출력"""
        current_time = datetime.now().strftime("%H:%M:%S")
        uptime = int(time.time() - self.system_start_time)
        uptime_str = (
            f"{uptime // 3600:02d}:{(uptime % 3600) // 60:02d}:{uptime % 60:02d}"
        )

        change_rate = stats.get_temperature_change_rate()
        change_indicator = ""
        if abs(change_rate) > 1.0:  # 1°C/분 이상 변화
            change_indicator = (
                f" ({'↗️' if change_rate > 0 else '↘️'}{abs(change_rate):.1f}°C/min)"
            )

        print(f"📊 [{current_time}] 센서 {stats.sensor_id} ({stats.sensor_addr[-8:]})")
        print(f"   🌡️  온도: {stats.last_temperature:.1f}°C{change_indicator}")
        print(
            f"   📈 통계: 평균 {stats.get_average_temperature():.1f}°C | 범위 {stats.min_temperature:.1f}~{stats.max_temperature:.1f}°C"
        )
        print(
            f"   📡 통신: 성공률 {stats.get_success_rate():.1f}% | 연속성공 {stats.consecutive_successes}회"
        )

        if stats.connection_failures > 0:
            print(
                f"   ⚠️  장애: 통신끊김 {stats.connection_failures}회 | 마지막 {stats.last_failure_time}"
            )

        if stats.high_threshold_violations > 0 or stats.low_threshold_violations > 0:
            print(
                f"   🚨 임계값: 고온 {stats.high_threshold_violations}회 | 저온 {stats.low_threshold_violations}회"
            )

        print(f"   ⏱️  시스템 가동시간: {uptime_str}")
        print()

    def print_system_summary(self) -> None:
        """전체 시스템 요약 출력"""
        if not self.sensors:
            print("📊 센서 데이터가 없습니다.")
            return

        current_time = datetime.now().strftime("%H:%M:%S")
        uptime = int(time.time() - self.system_start_time)
        uptime_str = (
            f"{uptime // 3600:02d}:{(uptime % 3600) // 60:02d}:{uptime % 60:02d}"
        )

        print("=" * 60)
        print(f"📊 시스템 요약 [{current_time}] | 가동시간: {uptime_str}")
        print("=" * 60)

        total_measurements = sum(s.total_measurements for s in self.sensors.values())
        total_failures = sum(s.connection_failures for s in self.sensors.values())

        print(f"🔢 전체 센서: {len(self.sensors)}개")
        print(f"📈 총 측정: {total_measurements}회")
        print(f"⚠️  총 장애: {total_failures}회")
        print()

        for _addr, stats in self.sensors.items():
            status = "🟢" if stats.consecutive_successes > 0 else "🔴"
            print(
                f"{status} 센서 {stats.sensor_id}: {stats.last_temperature:.1f}°C "
                f"(성공률 {stats.get_success_rate():.1f}%, 장애 {stats.connection_failures}회)"
            )

        print("=" * 60)
        print()

    def get_sensor_stats(self, sensor_addr: str) -> SensorStats | None:
        """특정 센서 통계 반환"""
        return self.sensors.get(sensor_addr)

    def set_log_interval(self, interval: int) -> None:
        """로그 출력 간격 설정"""
        self.log_interval = max(1, interval)
        print(f"📊 로그 출력 간격: {self.log_interval}개 측정마다")

    def log_event(self, event_type: str, event_data: dict) -> None:
        """특별한 이벤트를 로그에 기록"""
        try:
            current_time = self._get_current_time()

            if event_type == "ID_CHANGE":
                old_id = event_data.get("old_id", "??")
                new_id = event_data.get("new_id", "??")
                message = f"🔄 ID 변경: {old_id} → {new_id}"
                print(f"[{current_time}] {message}")

                # 센서 통계에도 기록 (해당 센서가 있다면)
                for addr, stats in self.sensors.items():
                    if stats.sensor_id == old_id:
                        # ID 업데이트
                        stats.sensor_id = new_id
                        print(
                            f"📝 센서 {addr} 통계에서 ID 업데이트: {old_id} → {new_id}"
                        )
                        break

            elif event_type == "CONNECTION_CHANGE":
                status = event_data.get("status", "unknown")
                port = event_data.get("port", "unknown")
                message = f"🔌 연결 상태 변경: {status} (포트: {port})"
                print(f"[{current_time}] {message}")

            else:
                message = event_data.get("message", f"이벤트: {event_type}")
                print(f"[{current_time}] 📋 {message}")

        except Exception as e:
            print(f"❌ 이벤트 로깅 오류: {e}")

    def _get_current_time(self) -> str:
        """현재 시간을 문자열로 반환"""
        return datetime.now().strftime("%H:%M:%S")


# 전역 로거 인스턴스
sensor_logger = SensorLogger()

"""DS18B20 센서 데이터 관리 및 정렬"""

from dataclasses import dataclass
from typing import Any


@dataclass
class SensorData:
    """단일 센서 데이터"""

    temperature: float
    sensor_addr: str
    sensor_id: str
    user_sensor_id: int
    th_value: int
    tl_value: int
    measurement_interval: int
    timestamp: int
    priority_group: int  # 1=설정됨, 2=미설정(00), 3=기타

    @classmethod
    def from_json(cls, json_data: dict) -> "SensorData":
        """JSON 데이터로부터 SensorData 생성"""
        sensor_id = json_data.get("sensor_id", "00")

        # 우선순위 그룹 결정
        if sensor_id in ["01", "02", "03", "04", "05", "06", "07", "08"]:
            priority_group = 1  # EEPROM 설정된 센서
        elif sensor_id == "00":
            priority_group = 2  # EEPROM 미설정 센서
        else:
            priority_group = 3  # 기타/오류

        return cls(
            temperature=json_data.get("temperature", 0.0),
            sensor_addr=json_data.get("sensor_addr", ""),
            sensor_id=sensor_id,
            user_sensor_id=json_data.get("user_sensor_id", 0),
            th_value=json_data.get("th_value", 30),
            tl_value=json_data.get("tl_value", 15),
            measurement_interval=json_data.get("measurement_interval", 1000),
            timestamp=json_data.get("timestamp", 0),
            priority_group=priority_group,
        )


class SensorDataManager:
    """센서 데이터 수집, 정렬, 관리"""

    def __init__(self) -> None:
        self._sensors: dict[str, SensorData] = {}  # key: sensor_addr
        self._last_update_time = 0

    def add_sensor_data(self, json_data: dict) -> None:
        """새로운 센서 데이터 추가/업데이트"""
        sensor = SensorData.from_json(json_data)
        self._sensors[sensor.sensor_addr] = sensor
        self._last_update_time = sensor.timestamp

    def get_sorted_sensors(self) -> list[SensorData]:
        """우선순위에 따라 정렬된 센서 리스트 반환"""
        sensors = list(self._sensors.values())

        # 정렬 키 함수
        def sort_key(sensor: SensorData) -> tuple:
            if sensor.priority_group == 1:
                # 1순위: sensor_id로 정렬 (01, 02, 03, ...)
                return (sensor.priority_group, int(sensor.sensor_id))
            else:
                # 2순위, 3순위: 고유주소로 정렬 (문자열 사전순)
                return (sensor.priority_group, sensor.sensor_addr)

        return sorted(sensors, key=sort_key)

    def get_sensor_by_address(self, address: str) -> SensorData | None:
        """주소로 특정 센서 데이터 조회"""
        return self._sensors.get(address)

    def get_sensor_by_id(self, sensor_id: str) -> SensorData | None:
        """센서 ID로 특정 센서 데이터 조회"""
        for sensor in self._sensors.values():
            if sensor.sensor_id == sensor_id:
                return sensor
        return None

    def get_sensor_count(self) -> int:
        """연결된 센서 개수"""
        return len(self._sensors)

    def get_sensors_by_group(self) -> dict[int, list[SensorData]]:
        """그룹별로 분류된 센서 딕셔너리"""
        groups: dict[int, list[SensorData]] = {1: [], 2: [], 3: []}

        for sensor in self.get_sorted_sensors():
            groups[sensor.priority_group].append(sensor)

        return groups

    def clear_old_sensors(self, max_age_seconds: int = 10) -> int:
        """오래된 센서 데이터 제거 (연결 해제된 센서)"""
        current_time = self._last_update_time
        removed_count = 0

        # 제거할 센서 주소 목록
        to_remove = []
        for addr, sensor in self._sensors.items():
            if current_time - sensor.timestamp > max_age_seconds * 1000:  # ms 단위
                to_remove.append(addr)

        # 실제 제거
        for addr in to_remove:
            del self._sensors[addr]
            removed_count += 1

        return removed_count

    def get_display_info(self, max_sensors: int = 8) -> list[dict[str, Any]]:
        """대시보드 표시용 센서 정보 반환"""
        sorted_sensors = self.get_sorted_sensors()
        display_list = []

        # 최대 센서 개수만큼만 반환
        for sensor in sorted_sensors[:max_sensors]:
            # 주소 포맷팅 (16자리를 4-4-4-4로 분할)
            addr = sensor.sensor_addr
            formatted_addr = (
                f"{addr[:4]}:{addr[4:8]}:{addr[8:12]}:{addr[12:16]}"
                if len(addr) >= 16
                else addr
            )

            display_info = {
                "sensor_id": sensor.sensor_id,
                "temperature": sensor.temperature,
                "address": sensor.sensor_addr,
                "formatted_address": formatted_addr,
                "user_id": sensor.user_sensor_id,
                "th_value": sensor.th_value,
                "tl_value": sensor.tl_value,
                "priority_group": sensor.priority_group,
                "group_name": self._get_group_name(sensor.priority_group),
                "is_configured": sensor.priority_group == 1,
            }
            display_list.append(display_info)

        return display_list

    def _get_group_name(self, priority_group: int) -> str:
        """우선순위 그룹 이름"""
        names = {1: "설정됨", 2: "미설정", 3: "오류"}
        return names.get(priority_group, "알수없음")

    def get_summary(self) -> dict[str, Any]:
        """센서 현황 요약"""
        groups = self.get_sensors_by_group()

        return {
            "total_sensors": self.get_sensor_count(),
            "configured_sensors": len(groups[1]),
            "unconfigured_sensors": len(groups[2]),
            "error_sensors": len(groups[3]),
            "last_update": self._last_update_time,
        }


# 전역 센서 데이터 매니저 인스턴스
sensor_manager = SensorDataManager()

"""실시간 센서 데이터 연동 콜백"""

from typing import Any

from dash import Input, Output

from ..services.port_manager import port_manager
from ..services.sensor_data_manager import sensor_manager
from ..services.sensor_logger import sensor_logger


def register_sensor_data_callbacks(app: Any) -> None:
    """센서 데이터 관련 콜백 등록"""

    @app.callback(
        [
            Output("main-sensor-id", "children"),
            Output("main-temp-value", "children"),
            Output("main-addr-0", "children"),
            Output("main-addr-1", "children"),
        ],
        Input("sensor-data-store", "data"),
        prevent_initial_call=True,
    )
    def update_temperature_card(
        sensor_data: dict[str, Any],
    ) -> tuple[str, str, str, str]:
        """센서 데이터 저장소 기반으로 온도 카드 업데이트"""
        status = sensor_data.get("status")

        # 연결 상태 확인
        if status == "disconnected":
            return _get_disconnected_display()

        if status == "error":
            return _get_error_display()

        # 센서 데이터 처리
        sensors = sensor_data.get("sensors", [])
        if sensors:
            return _format_primary_sensor_display(sensors[0])
        else:
            return _get_no_sensor_display()

    @app.callback(
        Output("sensor-data-store", "data"),
        Input("sensor-data-interval", "n_intervals"),
        prevent_initial_call=True,
    )
    def collect_sensor_data(n_intervals: int) -> dict[str, Any]:
        """센서 데이터 수집 및 저장소 업데이트"""

        if not port_manager.is_connected():
            return {"sensors": [], "last_update": 0, "status": "disconnected"}

        try:
            # Arduino에서 센서 데이터 읽기
            sensor_data = port_manager.read_sensor_data(timeout=1.0)

            if sensor_data:
                # TH/TL 디버깅: 수신된 센서 데이터 출력
                if "th_value" in sensor_data and "tl_value" in sensor_data:
                    sensor_addr = sensor_data.get("sensor_addr", "Unknown")
                    th_val = sensor_data.get("th_value", "Unknown")
                    tl_val = sensor_data.get("tl_value", "Unknown")
                    print(
                        f"📊 센서 데이터 수신: {sensor_addr} - TH={th_val}, TL={tl_val}"
                    )

                # 센서 데이터 매니저에 추가
                sensor_manager.add_sensor_data(sensor_data)

                # 센서 로거에도 데이터 추가
                sensor_logger.update_sensor_data(sensor_data)

                # 오래된 센서 데이터 정리 (10초 이상 업데이트 없는 센서)
                removed_count = sensor_manager.clear_old_sensors(max_age_seconds=10)

                # 현재 센서 상태 반환
                display_info = sensor_manager.get_display_info(max_sensors=8)
                summary = sensor_manager.get_summary()

                return {
                    "sensors": display_info,
                    "summary": summary,
                    "removed_sensors": removed_count,
                    "status": "connected",
                    "last_update": summary["last_update"],
                }
            else:
                # 데이터 없음 - 기존 상태 유지
                summary = sensor_manager.get_summary()
                return {
                    "sensors": sensor_manager.get_display_info(max_sensors=8),
                    "summary": summary,
                    "status": "no_data",
                    "last_update": summary["last_update"],
                }

        except Exception as e:
            return {
                "sensors": [],
                "error": str(e),
                "status": "error",
                "last_update": 0,
            }

    @app.callback(
        Output("sensor-count-display", "children"),
        Input("sensor-data-store", "data"),
        prevent_initial_call=True,
    )
    def update_sensor_count(sensor_data: dict[str, Any]) -> str:
        """센서 개수 표시 업데이트"""

        if sensor_data.get("status") == "disconnected":
            return "연결 안됨"

        summary = sensor_data.get("summary", {})
        total = summary.get("total_sensors", 0)
        configured = summary.get("configured_sensors", 0)
        unconfigured = summary.get("unconfigured_sensors", 0)

        if total == 0:
            return "센서 없음"

        return f"총 {total}개 (설정:{configured}, 미설정:{unconfigured})"

    @app.callback(
        [
            Output("sensor-data-interval", "disabled"),
            Output("sensor-data-interval", "interval"),
        ],
        Input("connection-status", "children"),
        prevent_initial_call=True,
    )
    def control_sensor_data_collection(connection_status: str) -> tuple[bool, int]:
        """연결 상태에 따라 센서 데이터 수집 제어"""

        if "연결됨" in connection_status:
            # 연결됨 - 1초마다 데이터 수집
            return False, 1000
        else:
            # 연결 안됨 - 데이터 수집 중지 (10초 간격으로 느리게)
            return True, 10000


def _get_disconnected_display() -> tuple[str, str, str, str]:
    """연결 안됨 상태 표시"""
    return "00", "연결 안됨", "0000:0000", "0000:0000"


def _get_error_display() -> tuple[str, str, str, str]:
    """데이터 오류 상태 표시"""
    return "00", "데이터 오류", "0000:0000", "0000:0000"


def _get_no_sensor_display() -> tuple[str, str, str, str]:
    """센서 없음 상태 표시"""
    return "00", "센서 없음", "0000:0000", "0000:0000"


def _format_primary_sensor_display(
    primary_sensor: dict[str, Any],
) -> tuple[str, str, str, str]:
    """주 센서 데이터 표시 포맷팅"""
    sensor_id = primary_sensor.get("sensor_id", "00")
    temperature = f"{primary_sensor.get('temperature', 0.0):.1f}°C"

    addr_line1, addr_line2 = _format_sensor_address(primary_sensor)

    return sensor_id, temperature, addr_line1, addr_line2


def _format_sensor_address(sensor: dict[str, Any]) -> tuple[str, str]:
    """센서 주소 포맷팅"""
    formatted_addr = sensor.get("formatted_address", "0000:0000:0000:0000")
    addr_parts = formatted_addr.split(":")

    addr_line1 = (
        f"{addr_parts[0]}:{addr_parts[1]}" if len(addr_parts) >= 2 else "0000:0000"
    )
    addr_line2 = (
        f"{addr_parts[2]}:{addr_parts[3]}" if len(addr_parts) >= 4 else "0000:0000"
    )

    return addr_line1, addr_line2

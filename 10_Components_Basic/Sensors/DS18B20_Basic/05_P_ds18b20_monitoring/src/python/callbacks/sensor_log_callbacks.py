"""센서 로그 콜백 함수들

센서 로그 카드의 실시간 업데이트를 처리합니다.
"""

from typing import Any

from dash import Input, Output

from ..components.sensor_log_card import sensor_log_card
from ..services.sensor_logger import sensor_logger


def register_sensor_log_callbacks(app: Any) -> None:
    """센서 로그 관련 콜백 등록"""

    @app.callback(
        Output("sensor-log-container", "children"),
        [Input("sensor-data-interval", "n_intervals")],
        prevent_initial_call=False,
    )
    def update_sensor_log_display(n_intervals: int | None) -> list:
        """센서 로그 디스플레이 업데이트"""

        # 센서 로거에서 통계 데이터 가져오기
        if not sensor_logger.sensors:
            return [sensor_log_card(sensors_stats=None)]  # 기본 빈 카드

        # 센서 통계를 딕셔너리 형태로 변환
        sensors_stats = {}

        for addr, stats in sensor_logger.sensors.items():
            # SensorStats 객체를 딕셔너리로 변환
            stats_dict = {
                "sensor_id": stats.sensor_id,
                "sensor_addr": stats.sensor_addr,
                "last_temperature": stats.last_temperature,
                "average_temperature": stats.get_average_temperature(),
                "min_temperature": stats.min_temperature,
                "max_temperature": stats.max_temperature,
                "total_measurements": stats.total_measurements,
                "successful_measurements": stats.successful_measurements,
                "failed_measurements": stats.failed_measurements,
                "consecutive_successes": stats.consecutive_successes,
                "max_consecutive_successes": stats.max_consecutive_successes,
                "connection_failures": stats.connection_failures,
                "last_failure_time": stats.last_failure_time,
                "high_threshold_violations": stats.high_threshold_violations,
                "low_threshold_violations": stats.low_threshold_violations,
                "temperature_change_rate": stats.get_temperature_change_rate(),
                "th_value": 30.0,  # 기본값 (실제로는 센서 데이터에서 가져와야 함)
                "tl_value": 15.0,  # 기본값
            }
            sensors_stats[addr] = stats_dict

        # 모든 센서 데이터를 하나의 로그 카드에 표시
        return [
            sensor_log_card(
                sensors_stats=sensors_stats,
                system_start_time=sensor_logger.system_start_time,
                id_prefix="main-log",
            )
        ]


def integrate_sensor_logger_with_data_callbacks() -> Any:
    """센서 데이터 콜백과 센서 로거 통합"""

    # 기존 센서 데이터 콜백에서 센서 로거 업데이트 호출
    # 이 함수는 sensor_data_callbacks.py에서 호출되어야 함

    def update_sensor_logger(sensor_data: dict[str, Any]) -> None:
        """센서 데이터를 센서 로거에 업데이트"""
        if sensor_data and "temperature" in sensor_data:
            sensor_logger.update_sensor_data(sensor_data)

    return update_sensor_logger

from typing import Any

from .interval_callbacks import register_interval_callbacks
from .sensor_data_callbacks import register_sensor_data_callbacks
from .sensor_id_callbacks import register_sensor_id_callbacks
from .sensor_log_callbacks import register_sensor_log_callbacks
from .threshold_callbacks import register_threshold_callbacks


def register_callbacks(app: Any) -> None:
    """Register callbacks on the provided Dash app.

    Registers all application callbacks including sensor data updates.
    """
    # 실시간 센서 데이터 콜백 등록
    register_sensor_data_callbacks(app)

    # 센서 ID 변경 콜백 등록
    register_sensor_id_callbacks(app)

    # 센서 로그 콜백 등록
    register_sensor_log_callbacks(app)

    # 임계값 설정 콜백 등록
    register_threshold_callbacks(app)

    # 측정주기 설정 콜백 등록
    register_interval_callbacks(app)

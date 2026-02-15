from dash import dcc, html

from ..components.connection_controls import connection_controls
from ..components.control_buttons import control_buttons
from ..components.interval_modal import interval_modal
from ..components.sensor_id_modal import sensor_id_modal, sensor_id_stores
from ..components.sensor_log_card import sensor_log_card
from ..components.temperature_card import temperature_card
from ..components.threshold_modal import threshold_modal


def get_layout() -> html.Div:
    """Return the app layout.

    Keep layout creation in a function to avoid import-time side-effects.
    """
    return html.Div(
        children=[
            html.Div(
                children=[
                    html.H1(
                        "온도센서 (DS18B20) 대시보드",
                        className="app-title",
                    ),
                    connection_controls(),
                ],
                className="header-container",
            ),
            html.Div(
                children=[
                    html.Div(
                        children=[
                            temperature_card(),
                            control_buttons(),
                            # Graph card outline (300px wide, same height as temperature card)
                            html.Div(
                                children=[
                                    html.Div(
                                        "실시간 그래프",
                                        style={
                                            "textAlign": "center",
                                            "padding": "10px",
                                        },
                                    ),
                                    html.Div(
                                        id="sensor-count-display",
                                        style={
                                            "fontSize": "12px",
                                            "textAlign": "center",
                                            "color": "#666",
                                        },
                                    ),
                                ],
                                className="graph-card",
                            ),
                            # Log card: fills remaining horizontal space to the right of the graph
                            html.Div(
                                id="sensor-log-container",
                                children=[
                                    sensor_log_card()  # 기본 빈 로그 카드
                                ],
                                className="log-card",
                            ),
                        ],
                        id="card_group",
                        className="card-group",
                    )
                ],
                id="content",
                className="content",
            ),
            # 실시간 데이터 업데이트를 위한 숨겨진 컴포넌트들
            dcc.Interval(
                id="port-scan-interval",
                interval=5000,  # 5초마다 포트 스캔
                n_intervals=0,
            ),
            dcc.Interval(
                id="sensor-data-interval",
                interval=10000,  # 10초마다 센서 데이터 수집
                n_intervals=0,
            ),
            dcc.Store(
                id="sensor-data-store", data={"sensors": [], "status": "disconnected"}
            ),
            # 센서 ID 변경 모달
            sensor_id_modal(),
            # 센서 ID 데이터 저장소
            sensor_id_stores(),
            # 임계값 설정 모달
            threshold_modal(),
            # 측정주기 설정 모달
            interval_modal(),
            # 결과 메시지 표시 영역 (숨김)
            html.Div(id="sensor-id-result", style={"display": "none"}),
            html.Div(id="threshold-result", style={"display": "none"}),
            html.Div(id="interval-result", style={"display": "none"}),
        ],
        className="app-root",
    )

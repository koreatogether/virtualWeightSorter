"""센서 ID 변경 모달 컴포넌트"""

import dash_bootstrap_components as dbc
from dash import dcc, html


def sensor_id_modal() -> html.Div:
    """개별 센서 ID 변경 팝업 (진짜 팝업 버전)"""
    return html.Div(
        [
            # 팝업 창
            html.Div(
                [
                    # 현재 센서 ID 표시
                    html.Div(
                        id="current-sensor-info",
                        children=[html.Span("ID: --", className="popup-sensor-info")],
                        className="text-center mb-2",
                    ),
                    # ID 선택 그리드 (4x2 미니 버튼)
                    html.Div(
                        [
                            html.Div(
                                [
                                    html.Button(
                                        f"{i:02d}",
                                        id=f"id-btn-{i:02d}",
                                        className="popup-id-btn",
                                    )
                                    for i in range(1, 5)
                                ],
                                className="popup-btn-row",
                            ),
                            html.Div(
                                [
                                    html.Button(
                                        f"{i:02d}",
                                        id=f"id-btn-{i:02d}",
                                        className="popup-id-btn",
                                    )
                                    for i in range(5, 9)
                                ],
                                className="popup-btn-row",
                            ),
                        ],
                        className="popup-btn-grid",
                    ),
                    # 유효성 검사 결과
                    html.Div(id="sensor-id-validation", className="popup-validation"),
                    # 액션 버튼
                    html.Div(
                        [
                            html.Button(
                                "✕",
                                id="sensor-id-cancel-btn",
                                className="popup-action-btn popup-cancel-btn",
                            ),
                            html.Button(
                                "✓",
                                id="sensor-id-confirm-btn",
                                className="popup-action-btn popup-confirm-btn",
                                disabled=True,
                            ),
                        ],
                        className="popup-actions",
                    ),
                    # 숨겨진 정보 저장
                    html.Div(id="selected-id-info", style={"display": "none"}),
                ],
                className="sensor-id-popup-content",
            ),
        ],
        id="sensor-id-modal",
        className="sensor-id-popup-overlay",
        style={"display": "none"},  # 기본적으로 숨김
    )


def format_sensor_info(sensors: list) -> list:
    """센서 정보를 표시용으로 포맷팅"""
    if not sensors:
        return [html.P("연결된 센서가 없습니다.", className="text-muted")]

    sensor_cards = []
    for i, sensor in enumerate(sensors, 1):
        sensor_id = sensor.get("sensor_id", "00")
        temperature = sensor.get("temperature", 0.0)
        formatted_addr = sensor.get("formatted_address", "")
        group_name = sensor.get("group_name", "미설정")

        # 상태에 따른 색상
        color = "success" if sensor_id != "00" else "warning"

        card = dbc.Card(
            [
                dbc.CardBody(
                    [
                        html.H6(f"센서 #{i}", className="card-title"),
                        html.P(
                            [
                                html.Strong(f"ID: {sensor_id}"),
                                " | ",
                                f"온도: {temperature:.1f}°C",
                                html.Br(),
                                html.Small(
                                    f"주소: {formatted_addr}", className="text-muted"
                                ),
                                html.Br(),
                                html.Small(
                                    f"상태: {group_name}", className="text-muted"
                                ),
                            ],
                            className="card-text mb-0",
                        ),
                    ]
                )
            ],
            color=color,
            outline=True,
            className="mb-2",
        )

        sensor_cards.append(card)

    return sensor_cards


def sensor_id_stores() -> html.Div:
    """센서 ID 모달에서 사용할 데이터 저장소들"""
    return html.Div(
        [
            dcc.Store(id="sensor-address-store", data=""),
            dcc.Store(id="selected-id-store", data=""),
        ],
        style={"display": "none"},
    )

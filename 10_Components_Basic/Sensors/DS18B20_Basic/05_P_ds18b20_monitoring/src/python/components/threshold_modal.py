"""임계값 설정 모달 컴포넌트

TH(상한)/TL(하한) 온도 임계값을 설정하는 모달 컴포넌트입니다.
"""

from typing import Any

import dash_bootstrap_components as dbc
from dash import dcc, html


def threshold_modal() -> html.Div:
    """임계값 설정 모달 컴포넌트 생성

    Returns:
        임계값 설정 모달 Div 컴포넌트
    """
    return html.Div(
        id="threshold-modal",
        className="threshold-modal-overlay",
        style={"display": "none"},
        children=[
            html.Div(
                className="threshold-modal-content",
                children=[
                    # 모달 헤더
                    html.Div(
                        [
                            html.H4(
                                "🌡️ 온도 임계값 설정", className="threshold-modal-title"
                            ),
                            html.Button(
                                "✕",
                                id="threshold-close-btn",
                                className="threshold-close-btn",
                                n_clicks=0,
                            ),
                        ],
                        className="threshold-modal-header",
                    ),
                    # 현재 센서 정보 표시
                    html.Div(
                        id="current-threshold-sensor-info",
                        className="threshold-sensor-info",
                        children=[],
                    ),
                    # 임계값 입력 섹션
                    html.Div(
                        [
                            html.Div(
                                [
                                    html.Label(
                                        "🔥 상한 온도 (TH)", className="threshold-label"
                                    ),
                                    dbc.InputGroup(
                                        [
                                            dbc.Input(
                                                id="th-input",
                                                type="number",
                                                placeholder="예: 30",
                                                min=-55,
                                                max=125,
                                                step=0.1,
                                                className="threshold-input",
                                            ),
                                            dbc.InputGroupText("°C"),
                                        ],
                                        className="threshold-input-group",
                                    ),
                                    html.Small(
                                        "설정하지 않으려면 비워두세요 (-55°C ~ 125°C)",
                                        className="threshold-hint",
                                    ),
                                ],
                                className="threshold-field",
                            ),
                            html.Div(
                                [
                                    html.Label(
                                        "❄️ 하한 온도 (TL)", className="threshold-label"
                                    ),
                                    dbc.InputGroup(
                                        [
                                            dbc.Input(
                                                id="tl-input",
                                                type="number",
                                                placeholder="예: 15",
                                                min=-55,
                                                max=125,
                                                step=0.1,
                                                className="threshold-input",
                                            ),
                                            dbc.InputGroupText("°C"),
                                        ],
                                        className="threshold-input-group",
                                    ),
                                    html.Small(
                                        "설정하지 않으려면 비워두세요 (-55°C ~ 125°C)",
                                        className="threshold-hint",
                                    ),
                                ],
                                className="threshold-field",
                            ),
                        ],
                        className="threshold-inputs",
                    ),
                    # 유효성 검사 메시지
                    html.Div(
                        id="threshold-validation",
                        className="threshold-validation",
                        children=[],
                    ),
                    # 모달 푸터 (버튼들)
                    html.Div(
                        [
                            dbc.Button(
                                "취소",
                                id="threshold-cancel-btn",
                                className="threshold-cancel-btn threshold-modal-btn",
                                n_clicks=0,
                            ),
                            dbc.Button(
                                "확인",
                                id="threshold-confirm-btn",
                                className="threshold-confirm-btn threshold-modal-btn",
                                n_clicks=0,
                                disabled=False,
                            ),
                        ],
                        className="threshold-modal-footer",
                    ),
                ],
            ),
            # 숨겨진 스토어 (현재 센서 주소)
            dcc.Store(id="threshold-sensor-address-store", data=""),
        ],
    )


def create_threshold_sensor_info(
    sensor_addr: str, sensor_id: str, current_th: float, current_tl: float
) -> list[Any]:
    """임계값 설정 모달의 현재 센서 정보 생성

    Args:
        sensor_addr: 센서 주소
        sensor_id: 센서 ID
        current_th: 현재 상한 온도
        current_tl: 현재 하한 온도

    Returns:
        센서 정보 표시용 컴포넌트 리스트
    """
    # 센서 주소 포맷팅 (8자씩 나누기)
    addr_part1 = sensor_addr[:8] if len(sensor_addr) >= 8 else sensor_addr
    addr_part2 = sensor_addr[8:] if len(sensor_addr) > 8 else ""

    return [
        html.Div(
            [
                html.Div(
                    [
                        html.Span("센서 ID: ", className="threshold-sensor-info-label"),
                        html.Span(
                            f"#{sensor_id}", className="threshold-sensor-info-value"
                        ),
                    ],
                    className="threshold-sensor-info-row",
                ),
                html.Div(
                    [
                        html.Span(
                            "센서 주소: ", className="threshold-sensor-info-label"
                        ),
                        html.Span(
                            f"{addr_part1} {addr_part2}",
                            className="threshold-sensor-info-value",
                        ),
                    ],
                    className="threshold-sensor-info-row",
                ),
                html.Div(
                    [
                        html.Span(
                            "현재 설정: ", className="threshold-sensor-info-label"
                        ),
                        html.Span(
                            f"TH={current_th:.1f}°C, TL={current_tl:.1f}°C",
                            className="threshold-sensor-info-value",
                        ),
                    ],
                    className="threshold-sensor-info-row",
                ),
            ],
        )
    ]

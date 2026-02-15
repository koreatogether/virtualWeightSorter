"""측정주기 설정 모달 컴포넌트

사용자가 2~60초 범위에서 측정 주기를 선택하는 모달 UI를 제공합니다.
"""

from typing import Any

import dash_bootstrap_components as dbc
from dash import dcc, html


def interval_modal() -> html.Div:
    """측정주기 설정 모달 컴포넌트 생성"""
    return html.Div(
        id="interval-modal",
        className="threshold-modal-overlay",  # 기존 스타일 재사용
        style={"display": "none"},
        children=[
            html.Div(
                className="threshold-modal-content",
                children=[
                    # 헤더
                    html.Div(
                        [
                            html.H4(
                                "⏱️ 측정 주기 설정", className="threshold-modal-title"
                            ),
                            html.Button(
                                "✕",
                                id="interval-close-btn",
                                className="threshold-close-btn",
                                n_clicks=0,
                            ),
                        ],
                        className="threshold-modal-header",
                    ),
                    # 현재 주기 정보/안내
                    html.Div(
                        id="current-interval-info",
                        className="threshold-sensor-info",
                        children=[],
                    ),
                    # 입력 섹션 (슬라이더 + 숫자 입력 동기화)
                    html.Div(
                        [
                            html.Label("주기(초)", className="threshold-label"),
                            dbc.InputGroup(
                                [
                                    dcc.Slider(
                                        id="interval-slider",
                                        min=2,
                                        max=60,
                                        step=1,
                                        value=10,
                                        marks={2: "2", 10: "10", 30: "30", 60: "60"},
                                        updatemode="drag",
                                    ),
                                    dbc.InputGroupText(" "),  # 간격용
                                    dbc.Input(
                                        id="interval-input",
                                        type="number",
                                        min=2,
                                        max=60,
                                        step=1,
                                        value=10,
                                        className="threshold-input",
                                    ),
                                    dbc.InputGroupText("초"),
                                ],
                                className="threshold-input-group",
                            ),
                            html.Small(
                                "2초 ~ 60초 범위에서 선택하세요.",
                                className="threshold-hint",
                            ),
                        ],
                        className="threshold-inputs",
                    ),
                    # 푸터 (버튼들)
                    html.Div(
                        [
                            dbc.Button(
                                "취소",
                                id="interval-cancel-btn",
                                className="threshold-cancel-btn threshold-modal-btn",
                                n_clicks=0,
                            ),
                            dbc.Button(
                                "확인",
                                id="interval-confirm-btn",
                                className="threshold-confirm-btn threshold-modal-btn",
                                n_clicks=0,
                                disabled=False,
                            ),
                        ],
                        className="threshold-modal-footer",
                    ),
                ],
            ),
            # 숨겨진 저장소: 현재 주기(초)
            dcc.Store(id="interval-current-seconds", data=10),
        ],
    )


def create_interval_info(current_seconds: int | None) -> list[Any]:
    """모달 상단에 표시할 현재 주기 정보"""
    secs = current_seconds if isinstance(current_seconds, int) else 10
    return [
        html.Div(
            [
                html.Span("현재 측정 주기: ", className="threshold-sensor-info-label"),
                html.Span(f"{secs}초", className="threshold-sensor-info-value"),
            ],
            className="threshold-sensor-info-row",
        )
    ]

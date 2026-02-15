from dash import dcc, html


def connection_controls() -> html.Div:
    """COM 포트 연결 제어 컴포넌트"""
    return html.Div(
        children=[
            # COM 포트 드롭다운 (동적으로 업데이트됨)
            dcc.Dropdown(
                id="com-port-dropdown",
                options=[],  # 빈 리스트로 시작, 콜백에서 업데이트
                value=None,
                placeholder="COM 포트 스캔 중...",
                className="com-dropdown",
            ),
            # 연결 제어 버튼들
            html.Button(
                "연결", id="connect-btn", className="header-btn connect", disabled=True
            ),
            html.Button(
                "해제",
                id="disconnect-btn",
                className="header-btn disconnect",
                disabled=True,
            ),
            html.Button(
                "진단",
                id="diagnose-btn",
                className="header-btn diagnose",
                disabled=False,
            ),
            # 연결 상태 표시
            html.Div(
                "○ 연결 안됨", id="connection-status", className="connection-status"
            ),
        ],
        className="connection-controls",
    )

"""Arduino 연결 관련 콜백 함수들"""

from typing import Any

from dash import Input, Output, State
from dash.exceptions import PreventUpdate

from ..services.diagnostics import (
    diagnose_connection_failure,
    diagnose_sensor_communication,
)
from ..services.port_manager import port_manager


def register_connection_callbacks(app: Any) -> None:
    """연결 관련 콜백 등록"""

    @app.callback(
        [
            Output("com-port-dropdown", "options"),
            Output("com-port-dropdown", "placeholder"),
        ],
        Input("port-scan-interval", "n_intervals"),  # 5초마다 스캔
    )
    def update_port_list(n_intervals: int) -> tuple[list[dict[str, str]], str]:
        """사용 가능한 COM 포트 목록 업데이트"""
        try:
            ports = port_manager.scan_ports()

            if not ports:
                return [], "사용 가능한 COM 포트 없음"

            options = [{"label": port, "value": port} for port in ports]
            placeholder = f"{len(ports)}개 포트 발견"

            return options, placeholder

        except Exception as e:
            return [], f"포트 스캔 오류: {e!s}"

    @app.callback(
        [
            Output("connect-btn", "disabled", allow_duplicate=True),
            Output("disconnect-btn", "disabled", allow_duplicate=True),
            Output("diagnose-btn", "disabled", allow_duplicate=True),
        ],
        Input("com-port-dropdown", "value"),
        prevent_initial_call=True,
    )
    def update_button_states(selected_port: str | None) -> tuple[bool, bool, bool]:
        """포트 선택 시 버튼 상태 업데이트"""
        if selected_port:
            # 포트 선택됨
            if port_manager.is_connected():
                # 이미 연결된 상태
                return True, False, False  # 연결(비활성), 해제(활성), 진단(활성)
            else:
                # 연결 안된 상태
                return False, True, False  # 연결(활성), 해제(비활성), 진단(활성)
        else:
            # 포트 미선택 - 연결만 비활성화, 진단은 활성화 (연결 없이도 포트 진단 가능)
            return True, True, False

    @app.callback(
        [
            Output("connection-status", "children"),
            Output("connection-status", "className"),
            Output("connect-btn", "disabled"),
            Output("disconnect-btn", "disabled"),
            Output("diagnose-btn", "disabled"),
        ],
        Input("connect-btn", "n_clicks"),
        State("com-port-dropdown", "value"),
        prevent_initial_call=True,
    )
    def handle_connect(
        n_clicks: int | None, selected_port: str | None
    ) -> tuple[str, str, bool, bool, bool]:
        """연결 버튼 클릭 처리 (진단 포함)"""
        if not n_clicks or not selected_port:
            raise PreventUpdate

        # 연결 시도
        success = port_manager.connect(selected_port)

        if success:
            # Arduino 통신 테스트
            comm_result = port_manager.test_communication()
            if comm_result:
                # 연결 성공
                status_text = f"● 연결됨 ({selected_port})"
                status_class = "connection-status connected"

                # 로그 메시지 추가 (별도 콜백에서 처리)
                return status_text, status_class, True, False, False
            else:
                # 통신 실패 - 연결 해제하고 진단
                port_manager.disconnect()
                status_text = "✗ 통신 실패"
                status_class = "connection-status error"

                return status_text, status_class, False, True, True
        else:
            # 연결 실패
            status_text = "✗ 연결 실패"
            status_class = "connection-status error"

            return status_text, status_class, False, True, True

    @app.callback(
        [
            Output("connection-status", "children", allow_duplicate=True),
            Output("connection-status", "className", allow_duplicate=True),
            Output("connect-btn", "disabled", allow_duplicate=True),
            Output("disconnect-btn", "disabled", allow_duplicate=True),
            Output("diagnose-btn", "disabled", allow_duplicate=True),
        ],
        Input("disconnect-btn", "n_clicks"),
        prevent_initial_call=True,
    )
    def handle_disconnect(n_clicks: int | None) -> tuple[str, str, bool, bool, bool]:
        """해제 버튼 클릭 처리"""
        if not n_clicks:
            raise PreventUpdate

        if port_manager.is_connected():
            port_manager.disconnect()
            status_text = "○ 연결 해제됨"
            status_class = "connection-status disconnected"
        else:
            status_text = "○ 이미 연결 해제됨"
            status_class = "connection-status disconnected"

        return status_text, status_class, False, True, True

    @app.callback(
        Output("connection-status", "children", allow_duplicate=True),
        Output("connection-status", "className", allow_duplicate=True),
        Input("diagnose-btn", "n_clicks"),
        State("com-port-dropdown", "value"),
        prevent_initial_call=True,
    )
    def handle_diagnose(
        n_clicks: int | None, selected_port: str | None
    ) -> tuple[str, str]:
        """진단 버튼 클릭 처리"""
        if not n_clicks:
            raise PreventUpdate

        if not port_manager.is_connected():
            return _diagnose_disconnected_state(selected_port)
        else:
            return _diagnose_connected_state()


def _diagnose_disconnected_state(selected_port: str | None) -> tuple[str, str]:
    """연결되지 않은 상태에서 진단"""
    if not selected_port:
        return "⚠️ 포트를 선택하세요", "connection-status error"

    diagnostic = diagnose_connection_failure(selected_port)
    if diagnostic.success:
        return "✅ 진단 완료 - 연결 가능", "connection-status connected"
    else:
        return f"❌ {diagnostic.message}", "connection-status error"


def _diagnose_connected_state() -> tuple[str, str]:
    """연결된 상태에서 진단"""
    current_port = port_manager.get_current_port()
    if not current_port:
        return "❌ 연결된 포트 정보 없음", "connection-status error"

    diagnostic = diagnose_sensor_communication(current_port)
    if diagnostic.success:
        return "✅ 진단 완료 - 통신 정상", "connection-status connected"
    else:
        return f"❌ {diagnostic.message}", "connection-status error"

    # 로그 메시지를 위한 별도 콜백 (센서 로그 영역이 구현되면 활성화)
    # @app.callback(
    #     Output("sensor-log", "children", allow_duplicate=True),
    #     [Input("connect-btn", "n_clicks"),
    #      Input("disconnect-btn", "n_clicks"),
    #      Input("diagnose-btn", "n_clicks")],
    #     [State("com-port-dropdown", "value"),
    #      State("connection-status", "children")],
    #     prevent_initial_call=True
    # )
    # def update_log_messages(connect_clicks, disconnect_clicks, diagnose_clicks, selected_port, status):
    #     """로그 메시지 업데이트"""
    #     trigger = ctx.triggered_id

    #     if trigger == "connect-btn":
    #         if "연결됨" in status:
    #             return [create_log_entry(f"✅ {selected_port} 연결 성공 - Arduino 응답 확인됨")]
    #         else:
    #             diagnostic = diagnose_connection_failure(selected_port)
    #             return format_diagnostic_message(diagnostic)
    #     elif trigger == "disconnect-btn":
    #         return [create_log_entry("🔌 연결 해제 완료")]
    #     elif trigger == "diagnose-btn":
    #         if port_manager.is_connected():
    #             diagnostic = diagnose_sensor_communication(selected_port)
    #             return format_diagnostic_message(diagnostic)
    #         else:
    #             diagnostic = diagnose_connection_failure(selected_port)
    #             return format_diagnostic_message(diagnostic)

    #     raise PreventUpdate

"""측정주기 설정 콜백

2~60초 범위에서 측정 주기를 설정하고 장치에 반영합니다.
"""

from __future__ import annotations

import json
import time
from typing import Any, cast

import dash_bootstrap_components as dbc
from dash import Input, Output, State, callback_context
from dash.exceptions import PreventUpdate

from ..components.interval_modal import create_interval_info
from ..services.port_manager import port_manager


def _create_error_alert(message: str) -> dbc.Alert:
    return dbc.Alert(f"❌ {message}", color="danger")


def _create_success_alert(message: str) -> dbc.Alert:
    return dbc.Alert(f"✅ {message}", color="success")


def _send_arduino_command(command: str) -> bool:
    sc = getattr(port_manager, "_serial_connection", None)
    if not port_manager.is_connected() or sc is None:
        return False
    try:
        sc.write(command.encode())
        sc.flush()
        return True
    except Exception:
        return False


def _wait_for_response(timeout: float = 5.0) -> dict[str, Any] | None:
    start = time.time()
    buf = ""
    while time.time() - start < timeout:
        sc = getattr(port_manager, "_serial_connection", None)
        if sc and sc.in_waiting > 0:
            raw = sc.read(sc.in_waiting).decode("utf-8", errors="ignore")
            buf += raw
            # Parse complete lines only from buffer to handle fragmented JSON
            while "\n" in buf:
                line, buf = buf.split("\n", 1)
                line = line.strip()
                if not (line.startswith("{") and line.endswith("}")):
                    continue
                try:
                    parsed = json.loads(line)
                except json.JSONDecodeError:
                    continue
                if isinstance(parsed, dict) and parsed.get("type") in (
                    "response",
                    "system_status",
                ):
                    return cast(dict[str, Any], parsed)
        time.sleep(0.05)
    return None


def register_interval_callbacks(app: Any) -> None:
    # 모달 열기/닫기
    @app.callback(
        [
            Output("interval-modal", "style", allow_duplicate=True),
            Output("current-interval-info", "children", allow_duplicate=True),
            Output("interval-current-seconds", "data", allow_duplicate=True),
        ],
        [
            Input("btn-interval", "n_clicks"),
            Input("interval-cancel-btn", "n_clicks"),
            Input("interval-close-btn", "n_clicks"),
            Input("interval-confirm-btn", "n_clicks"),
        ],
        [State("interval-current-seconds", "data")],
        prevent_initial_call=True,
    )
    def toggle_modal(
        open_clicks: int | None,
        cancel_clicks: int | None,
        close_clicks: int | None,
        confirm_clicks: int | None,
        current_secs: int | None,
    ) -> tuple[dict, list, int]:
        ctx = callback_context
        if not ctx.triggered:
            raise PreventUpdate
        trigger_id = ctx.triggered[0]["prop_id"].split(".")[0]

        if trigger_id == "btn-interval":
            # 장치 상태에서 현재 주기 가져오기 시도
            info_children: list
            secs = current_secs or 10
            if port_manager.is_connected():
                cmd = json.dumps({"type": "command", "command": "get_status"}) + "\n"
                if _send_arduino_command(cmd):
                    resp = _wait_for_response(timeout=1.5)
                    if resp and resp.get("type") == "system_status":
                        secs_from_dev = int(
                            (resp.get("measurement_interval") or 1000) // 1000
                        )
                        # 범위 보정
                        if 2 <= secs_from_dev <= 60:
                            secs = secs_from_dev
            info_children = create_interval_info(secs)
            return {"display": "block"}, info_children, int(secs)

        # 닫기
        return {"display": "none"}, [], int(current_secs or 10)

    # 슬라이더/숫자 동기화 및 검증 메시지
    @app.callback(
        [Output("interval-input", "value"), Output("interval-slider", "value")],
        [Input("interval-slider", "value"), Input("interval-input", "value")],
        [State("interval-current-seconds", "data")],
        prevent_initial_call=True,
    )
    def sync_inputs(
        slider_val: int | None, input_val: int | None, current_secs: int | None
    ) -> tuple[int, int]:
        base = int(current_secs or 10)
        # 어떤 입력이 트리거됐는지 확인
        trig = (
            callback_context.triggered[0]["prop_id"].split(".")[0]
            if callback_context.triggered
            else ""
        )
        if trig == "interval-slider" and isinstance(slider_val, int | float):
            v = max(2, min(60, int(slider_val)))
            return v, v
        if trig == "interval-input" and isinstance(input_val, int | float):
            v = max(2, min(60, int(input_val)))
            return v, v
        return base, base

    # 확인 눌러 적용
    @app.callback(
        Output("interval-result", "children", allow_duplicate=True),
        Input("interval-confirm-btn", "n_clicks"),
        [State("interval-input", "value")],
        prevent_initial_call=True,
    )
    def apply_interval(n_clicks: int | None, value: int | None) -> list[Any]:
        if not n_clicks:
            raise PreventUpdate
        if value is None:
            return [_create_error_alert("값이 비어 있습니다.")]
        secs = int(value)
        if secs < 2 or secs > 60:
            return [_create_error_alert("2~60초 범위로 입력해 주세요.")]
        if not port_manager.is_connected():
            return [_create_error_alert("Arduino가 연결되지 않았습니다.")]

        # 장치에 전송 (기존 set_config 경로 활용: config_type=measurement_interval, new_value=초)
        payload = {
            "type": "command",
            "command": "set_config",
            "config_type": "measurement_interval",
            "new_value": secs,
        }
        cmd = json.dumps(payload) + "\n"
        if not _send_arduino_command(cmd):
            return [_create_error_alert("명령 전송 실패")]

        resp = _wait_for_response(timeout=3.0)
        if not resp or resp.get("type") != "response":
            return [_create_error_alert("Arduino 응답을 받지 못했습니다.")]

        if resp.get("status") == "success":
            return [
                _create_success_alert(
                    f"측정 주기가 {secs}초로 설정되었습니다. (EEPROM 자동 저장) "
                )
            ]
        return [_create_error_alert(f"설정 실패: {resp.get('message', '')}")]

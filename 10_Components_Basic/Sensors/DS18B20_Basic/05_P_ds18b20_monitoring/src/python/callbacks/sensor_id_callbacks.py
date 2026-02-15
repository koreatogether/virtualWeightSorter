"""센서 ID 변경 관련 콜백"""

from typing import Any, cast

import dash_bootstrap_components as dbc
from dash import Input, Output, State, callback_context, html
from dash.exceptions import PreventUpdate

from ..services.port_manager import port_manager
from ..services.sensor_data_manager import sensor_manager


def _handle_modal_open(
    addr_part1: str | None, addr_part2: str | None
) -> tuple[dict[str, str], list[Any], str]:
    """모달 열기 처리 로직"""
    # Arduino 연결 상태 확인
    if not port_manager.is_connected():
        return (
            {"display": "none"},
            [html.P("Arduino가 연결되지 않았습니다.", className="text-danger")],
            "",
        )

    # 현재 표시된 센서 정보에서 주소 추출
    current_address = _extract_sensor_address(addr_part1, addr_part2)
    if not current_address:
        return (
            {"display": "none"},
            [html.P("센서 정보를 찾을 수 없습니다.", className="text-danger")],
            "",
        )

    # 센서 매니저에서 해당 센서 찾기
    current_sensor = sensor_manager.get_sensor_by_address(current_address)
    if not current_sensor:
        return (
            {"display": "none"},
            [html.P("센서 데이터를 찾을 수 없습니다.", className="text-danger")],
            "",
        )

    sensor_info = _format_current_sensor_info(current_sensor)
    return {"display": "block"}, sensor_info, current_sensor.sensor_addr


def _handle_modal_close() -> tuple[dict[str, str], list[Any], str]:
    """모달 닫기 처리 로직"""
    return {"display": "none"}, [], ""


def _handle_id_button_click(
    trigger_id: str, sensor_address: str | None
) -> tuple[list[Any], list[Any], bool, str]:
    """ID 버튼 클릭 처리 로직"""
    if not trigger_id.startswith("id-btn-"):
        raise PreventUpdate

    selected_id = trigger_id.replace("id-btn-", "")

    # 중복 체크 (센서 주소가 있을 때만)
    if sensor_address:
        is_duplicate, validation_message = _check_id_duplicate(
            selected_id, sensor_address
        )
    else:
        # 센서 주소가 없어도 ID 선택은 가능
        is_duplicate = False
        validation_message = [
            html.Span(f"ID {selected_id} 선택됨", className="popup-validation success")
        ]

    return [], validation_message, is_duplicate, selected_id


def _handle_id_change_execution(
    sensor_address: str | None, selected_id: str | None
) -> list[Any]:
    """ID 변경 실행 로직"""
    if not sensor_address or not selected_id:
        raise PreventUpdate

    if not port_manager.is_connected():
        return [_create_error_alert("Arduino가 연결되지 않았습니다.")]

    try:
        return _execute_sensor_id_change(sensor_address, selected_id)
    except Exception as e:
        return [_create_error_alert(f"오류 발생: {e}")]


def register_sensor_id_callbacks(app: Any) -> None:
    """센서 ID 변경 관련 콜백 등록"""

    @app.callback(
        [
            Output("sensor-id-modal", "style"),
            Output("current-sensor-info", "children"),
            Output("sensor-address-store", "data", allow_duplicate=True),
        ],
        [
            Input("btn-id", "n_clicks"),
            Input("sensor-id-cancel-btn", "n_clicks"),
            Input("sensor-id-confirm-btn", "n_clicks"),
        ],
        [
            State("sensor-id-modal", "style"),
            State("sensor-address-store", "data"),
            State("main-sensor-id", "children"),  # 현재 표시된 센서 ID
            State("main-addr-0", "children"),  # 현재 표시된 센서 주소 (첫 번째 부분)
            State("main-addr-1", "children"),  # 현재 표시된 센서 주소 (두 번째 부분)
        ],
        prevent_initial_call=True,
    )
    def toggle_sensor_id_modal(
        open_clicks: int | None,
        cancel_clicks: int | None,
        confirm_clicks: int | None,
        current_style: dict | None,
        sensor_address: str | None,
        current_sensor_id: str | None,
        addr_part1: str | None,
        addr_part2: str | None,
    ) -> tuple[dict, list, str]:
        """센서 ID 모달 열기/닫기 및 센서 정보 표시"""

        ctx = callback_context
        if not ctx.triggered:
            raise PreventUpdate

        trigger_id = ctx.triggered[0]["prop_id"].split(".")[0]

        if trigger_id == "btn-id":
            return _handle_modal_open(addr_part1, addr_part2)

        elif trigger_id in ["sensor-id-cancel-btn", "sensor-id-confirm-btn"]:
            return _handle_modal_close()

        # 기본값 (변화 없음)
        current_display = (
            current_style.get("display", "none") if current_style else "none"
        )
        return {"display": current_display}, [], sensor_address or ""

    # ID 버튼 클릭 처리 (01-08 버튼들)
    @app.callback(
        [
            Output("selected-id-info", "children"),
            Output("sensor-id-validation", "children"),
            Output("sensor-id-confirm-btn", "disabled"),
            Output("selected-id-store", "data", allow_duplicate=True),
        ],
        [Input(f"id-btn-{i:02d}", "n_clicks") for i in range(1, 9)],
        [
            State("sensor-address-store", "data"),
            State("selected-id-store", "data"),
        ],
        prevent_initial_call=True,
    )
    def handle_id_button_clicks(*args: Any) -> tuple[list, list, bool, str]:
        """ID 버튼 클릭 처리 및 중복 체크"""
        ctx = callback_context
        if not ctx.triggered:
            raise PreventUpdate

        trigger_id = ctx.triggered[0]["prop_id"].split(".")[0]
        sensor_address = args[-2]  # State에서 센서 주소

        return _handle_id_button_click(trigger_id, sensor_address)

    # ID 변경 실행
    @app.callback(
        Output("sensor-id-result", "children", allow_duplicate=True),
        Input("sensor-id-confirm-btn", "n_clicks"),
        [
            State("sensor-address-store", "data"),
            State("selected-id-store", "data"),
        ],
        prevent_initial_call=True,
    )
    def execute_sensor_id_change(
        confirm_clicks: int | None, sensor_address: str | None, selected_id: str | None
    ) -> list:
        """센서 ID 변경 실행"""
        if not confirm_clicks:
            raise PreventUpdate

        return _handle_id_change_execution(sensor_address, selected_id)


def _create_error_alert(message: str) -> dbc.Alert:
    """에러 알림 생성"""
    return dbc.Alert(f"❌ {message}", color="danger")


def _create_success_alert(message: str) -> dbc.Alert:
    """성공 알림 생성"""
    return dbc.Alert(f"✅ {message}", color="success")


def _create_warning_alert(message: str) -> dbc.Alert:
    """경고 알림 생성"""
    return dbc.Alert(f"⏱️ {message}", color="warning")


def _extract_sensor_address(addr_part1: str | None, addr_part2: str | None) -> str:
    """UI에서 표시된 주소 부분들을 16자리 주소로 복원"""
    if not addr_part1 or not addr_part2:
        return ""

    # "XXXX:XXXX" 형태에서 콜론 제거
    part1_clean = addr_part1.replace(":", "")
    part2_clean = addr_part2.replace(":", "")

    # 16자리 주소 복원
    full_address = part1_clean + part2_clean

    # 16자리가 아니면 빈 문자열 반환
    if len(full_address) != 16:
        return ""

    return full_address


def _format_current_sensor_info(sensor: Any) -> list:
    """현재 센서 정보 포맷팅"""
    return [
        html.P(
            [
                html.Strong(f"ID: {sensor.sensor_id}"),
                " | ",
                f"온도: {sensor.temperature:.1f}°C",
                html.Br(),
                html.Small(
                    f"주소: {sensor.sensor_addr[:4]}:{sensor.sensor_addr[4:8]}:{sensor.sensor_addr[8:12]}:{sensor.sensor_addr[12:16]}",
                    className="text-muted",
                ),
            ],
            className="mb-0",
        )
    ]


def _check_id_duplicate(
    selected_id: str, current_sensor_address: str
) -> tuple[bool, list]:
    """ID 중복 체크"""
    current_sensors = sensor_manager.get_sorted_sensors()

    # 현재 센서의 기존 ID는 제외하고 체크
    used_ids = {
        sensor.sensor_id
        for sensor in current_sensors
        if sensor.sensor_id != "00" and sensor.sensor_addr != current_sensor_address
    }

    if selected_id in used_ids:
        warning = [
            dbc.Alert(
                f"⚠️ ID {selected_id}는 이미 다른 센서에서 사용 중입니다.",
                color="warning",
                className="mb-0",
            )
        ]
        return True, warning  # 중복됨, 버튼 비활성화

    # 사용 가능한 ID
    success = [
        dbc.Alert(
            f"✅ ID {selected_id} 사용 가능합니다.", color="success", className="mb-0"
        )
    ]
    return False, success  # 중복 아님, 버튼 활성화


def _execute_sensor_id_change(selected_sensor: str, new_id: str) -> list:
    """센서 ID 변경 실행"""
    command = _build_sensor_id_command(selected_sensor, new_id)

    if not _send_command(command):
        return [_create_error_alert("명령 전송 실패")]

    response = _wait_for_response(timeout=5.0)
    return _process_response(response, new_id)


def _build_sensor_id_command(sensor_address: str, new_id: str) -> str:
    """센서 ID 변경 명령 생성 (Arduino JSON 형식)"""
    import json

    # Arduino 펌웨어에서 지원하는 정확한 형식 사용
    command_data = {
        "type": "command",
        "command": "set_sensor_data",  # Arduino에서 지원하는 명령어
        "new_value": int(new_id),  # 정수 형태로 전송
    }

    command = json.dumps(command_data) + "\n"
    print(f"🔧 Arduino 명령 전송: {command.strip()}")
    return command


def _send_command(command: str) -> bool:
    """명령 전송"""
    try:
        print(f"🔄 Arduino 명령 전송 시도: {command.strip()}")

        # 연결 상태 확인
        if not port_manager.is_connected():
            print("❌ Arduino가 연결되지 않음")
            return False

        if not hasattr(port_manager, "_serial_connection"):
            print("❌ 시리얼 연결 객체가 없음")
            return False

        if port_manager._serial_connection is None:
            print("❌ 시리얼 연결이 None")
            return False

        # 명령 전송
        bytes_written = port_manager._serial_connection.write(command.encode())
        print(f"✅ 명령 전송 완료: {bytes_written} bytes")

        # 버퍼 플러시
        port_manager._serial_connection.flush()
        print("✅ 버퍼 플러시 완료")

        return True

    except Exception as e:
        print(f"❌ 명령 전송 실패: {e}")
        return False


def _wait_for_response(timeout: float) -> str | None:
    """Arduino 응답 대기 (텍스트 응답)"""
    import time

    print(f"⏳ Arduino 응답 대기 중... (최대 {timeout}초)")
    start_time = time.time()

    while time.time() - start_time < timeout:
        try:
            if (
                port_manager.is_connected()
                and hasattr(port_manager, "_serial_connection")
                and port_manager._serial_connection is not None
            ):
                # 대기 중인 데이터 확인
                waiting = port_manager._serial_connection.in_waiting
                if waiting > 0:
                    print(f"📨 {waiting} bytes 수신됨")
                    response_bytes = port_manager._serial_connection.readline()
                    response = cast(
                        str, response_bytes.decode("utf-8", errors="ignore").strip()
                    )
                    if isinstance(response, str) and response:
                        print(f"📤 Arduino 응답: {response}")
                        return response
        except Exception as e:
            print(f"❌ 응답 읽기 오류: {e}")
            pass

        time.sleep(0.1)

    print("⏰ Arduino 응답 시간 초과")
    return None


def _process_response(response: str | None, new_id: str) -> list:
    """Arduino JSON 응답 처리"""
    if response is None:
        return [_create_warning_alert("Arduino 응답 시간 초과")]
    import json

    try:
        # JSON 응답 파싱
        response_data = json.loads(response)

        if response_data.get("type") == "response":
            status = response_data.get("status")
            message = response_data.get("message", "")

            if status == "success":
                # 성공 응답 처리
                old_id = response_data.get("old_id", "??")

                # 로그에 ID 변경 기록
                _log_id_change(old_id, new_id)

                return [
                    _create_success_alert(
                        f"✅ 센서 ID가 성공적으로 {old_id} → {new_id}로 변경되었습니다!"
                    )
                ]
            elif status == "error":
                return [_create_error_alert(f"❌ ID 변경 실패: {message}")]
            else:
                return [_create_warning_alert(f"⚠️ 알 수 없는 상태: {status}")]
        else:
            return [_create_warning_alert(f"⚠️ 알 수 없는 응답 형식: {response}")]

    except json.JSONDecodeError:
        # JSON이 아닌 경우 기존 텍스트 처리 방식 사용
        if "SUCCESS" in response:
            return [_create_success_alert(f"✅ ID 변경 완료: {response}")]
        elif "ERROR" in response:
            return [_create_error_alert(f"❌ ID 변경 실패: {response}")]
        else:
            return [_create_warning_alert(f"⚠️ 예상치 못한 응답: {response}")]


def _log_id_change(old_id: str, new_id: str) -> None:
    """ID 변경을 로그에 기록하고 센서 매니저 업데이트"""
    try:
        from ..services.sensor_logger import sensor_logger

        # 로그 메시지 생성
        log_message = f"ID 변경: {old_id} → {new_id}"

        # 센서 로거에 기록 (특별한 이벤트로)
        sensor_logger.log_event(
            "ID_CHANGE",
            {
                "old_id": old_id,
                "new_id": new_id,
                "message": log_message,
                "timestamp": sensor_logger._get_current_time(),
            },
        )

        print(f"📝 로그 기록: {log_message}")

        # 센서 매니저에도 ID 변경 알림
        _notify_sensor_manager_id_change(old_id, new_id)

    except Exception as e:
        print(f"❌ 로그 기록 실패: {e}")


def _notify_sensor_manager_id_change(old_id: str, new_id: str) -> None:
    """센서 매니저에 ID 변경 알림"""
    try:
        # 센서 매니저의 모든 센서를 확인하여 ID 업데이트
        sensors = sensor_manager.get_sorted_sensors()

        for sensor in sensors:
            if sensor.sensor_id == old_id:
                print(f"🔄 센서 매니저에서 ID 업데이트: {old_id} → {new_id}")
                # 센서 객체의 ID 직접 업데이트
                sensor.sensor_id = new_id

                # 센서 매니저의 내부 딕셔너리도 업데이트 필요할 수 있음
                if hasattr(sensor_manager, "_sensors"):
                    for addr, sensor_obj in sensor_manager._sensors.items():
                        if sensor_obj.sensor_id == old_id:
                            sensor_obj.sensor_id = new_id
                            print(f"✅ 센서 {addr} ID 업데이트 완료")
                break

    except Exception as e:
        print(f"❌ 센서 매니저 업데이트 실패: {e}")

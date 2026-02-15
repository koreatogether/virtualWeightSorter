"""임계값 설정 콜백

TH(상한)/TL(하한) 온도 임계값 설정과 관련된 모든 콜백 함수들입니다.
"""

import json
import time
from typing import Any

import dash_bootstrap_components as dbc
from dash import Input, Output, State, callback_context, html
from dash.exceptions import PreventUpdate

from ..components.threshold_modal import create_threshold_sensor_info
from ..services.port_manager import port_manager
from ..services.sensor_data_manager import sensor_manager


def _handle_threshold_modal_open_fast(
    sensor_id_display: str | None, addr_part1: str | None, addr_part2: str | None
) -> tuple[dict[str, str], list[Any], str, float | None, float | None]:
    """임계값 모달 열기: 즉시 표시용 경량 경로

    - 시리얼 질의 없이, 현재 대시보드가 알고 있는 최근 값으로 즉시 시드합니다.
    - 팝업은 바로 뜨고, 정밀 값은 별도 콜백에서 실시간 조회하여 업데이트합니다.
    """
    # UI로부터 신원 파악
    ui_id = _extract_sensor_id(sensor_id_display)
    ui_addr_full = _extract_sensor_address(addr_part1, addr_part2)

    identifier = ui_id or ui_addr_full or ""
    # 센서 매니저에서 최근 값 가져오기
    th_from_mgr: float | None = None
    tl_from_mgr: float | None = None
    disp_addr = ui_addr_full or "Unknown"
    disp_id = ui_id or "00"
    if ui_id:
        sensor_obj = sensor_manager.get_sensor_by_id(ui_id)
        if sensor_obj:
            disp_addr = sensor_obj.sensor_addr or disp_addr
            try:
                th_from_mgr = float(sensor_obj.th_value)
                tl_from_mgr = float(sensor_obj.tl_value)
            except Exception:
                th_from_mgr, tl_from_mgr = None, None

    # 표시용 구성
    display_th = th_from_mgr if th_from_mgr is not None else None
    display_tl = tl_from_mgr if tl_from_mgr is not None else None

    # children 생성 (None이면 기본값 텍스트로 표시)
    info_th = display_th if display_th is not None else 30.0
    info_tl = display_tl if display_tl is not None else 15.0
    sensor_info = create_threshold_sensor_info(disp_addr, disp_id, info_th, info_tl)

    return {"display": "block"}, sensor_info, identifier, display_th, display_tl


def _handle_threshold_modal_open(
    sensor_id_display: str | None, addr_part1: str | None, addr_part2: str | None
) -> tuple[dict[str, str], list[Any], str, float | None, float | None]:
    """임계값 모달 열기 처리 로직 (ID 기반)

    재연결/하드리셋 직후에도 EEPROM의 현재값을 신뢰성 있게 가져오기 위해
    먼저 system_status를 요청하여 ID/주소/TH/TL을 우선 시드합니다.
    """
    # Arduino 연결 상태 확인
    if not port_manager.is_connected():
        return (
            {"display": "none"},
            [html.P("Arduino가 연결되지 않았습니다.", className="text-danger")],
            "",
            None,
            None,
        )

    # 1) system_status 우선 조회 (하드리셋 직후 대비) - 짧은 재시도 게이트
    status = _fetch_system_status_with_retries(retries=3, delay=0.25, timeout=0.6)

    # 상태 기반 후보값
    status_id = ""
    status_addr = ""
    status_th = None
    status_tl = None
    if status:
        # 다양한 필드명을 호환
        for id_key in ("user_sensor_id", "selected_sensor_id", "sensor_id"):
            sid = status.get(id_key)
            if isinstance(sid, int) and 1 <= sid <= 8:
                status_id = f"{sid:02d}"
                break
            if isinstance(sid, str) and sid.zfill(2) in [
                f"{i:02d}" for i in range(1, 9)
            ]:
                status_id = sid.zfill(2)
                break
        for addr_key in ("selected_sensor_addr", "sensor_addr", "addr"):
            saddr = status.get(addr_key)
            if isinstance(saddr, str) and saddr:
                status_addr = saddr
                break
        status_th = status.get("th_value")
        status_tl = status.get("tl_value")

    # 2) UI에서 전달된 값과 병합
    ui_id = _extract_sensor_id(sensor_id_display)
    # 주소는 항상 한 번 계산해 둔다(동일성 비교에 사용).
    # 자리표시자(예: 0X0X...)는 무시하기 위해 16자리 HEX 유효성까지 검사한다.
    ui_addr_full = _extract_sensor_address(addr_part1, addr_part2)
    current_sensor_id = ui_id or status_id
    current_address = ""
    if not current_sensor_id:
        # 주소로 보완
        current_address = ui_addr_full or status_addr
        # 주소도 없으면 중단
        if not current_address:
            return (
                {"display": "none"},
                [html.P("센서 식별 정보를 찾지 못했습니다.", className="text-danger")],
                "",
                None,
                None,
            )

    # 3) 상태에 TH/TL이 있으면 그대로 표시
    if (status_th is not None) and (status_tl is not None):
        # system_status의 값은 '선택된 센서' 전역값이므로, 대상 센서와 신원(ID/주소)이 일치할 때만 신뢰
        identity_match = False
        target_id = current_sensor_id  # 우리가 편집하려는 ID
        # 대상 주소: 센서 매니저에서 ID로 찾거나, UI에서 받은 주소 또는 status의 주소
        target_addr = ""
        if target_id:
            sensor_obj = sensor_manager.get_sensor_by_id(target_id)
            if sensor_obj:
                target_addr = sensor_obj.sensor_addr
        if not target_addr:
            target_addr = ui_addr_full or ""
        # ID 매칭 우선, 그다음 주소 매칭(대소문자 무시)
        if (target_id and status_id and target_id == f"{int(status_id):02d}") or (
            target_addr
            and status_addr
            and target_addr.upper() == str(status_addr).upper()
        ):
            identity_match = True

        if identity_match:
            final_id = target_id or status_id or "00"
            final_addr = target_addr or status_addr or current_address or "Unknown"
            try:
                display_th = float(status_th)
                display_tl = float(status_tl)
            except Exception:
                display_th, display_tl = 30.0, 15.0
            sensor_info = create_threshold_sensor_info(
                final_addr, final_id, display_th, display_tl
            )
            identifier = (
                final_id
                if final_id in [f"{i:02d}" for i in range(1, 9)]
                else final_addr
            )
            return (
                {"display": "block"},
                sensor_info,
                identifier,
                float(display_th),
                float(display_tl),
            )

    # 4) 없으면 ID 기반 실시간 조회로 보완 (먼저 최근 수집된 센서 데이터 활용 → 실패 시 아두이노 질의 재시도)
    if current_sensor_id:
        # 주소 표시용 조회
        sensor_obj = sensor_manager.get_sensor_by_id(current_sensor_id)
        disp_addr = sensor_obj.sensor_addr if sensor_obj else (status_addr or "Unknown")
        # 4-1) 센서 데이터 매니저 값 확보 (최후 폴백으로 사용)
        th_from_mgr: float | None = None
        tl_from_mgr: float | None = None
        if sensor_obj:
            try:
                th_from_mgr = float(sensor_obj.th_value)
                tl_from_mgr = float(sensor_obj.tl_value)
            except Exception:
                th_from_mgr, tl_from_mgr = None, None
        # 라이브 1차 조회 시도 (짧은 타임아웃)
        from contextlib import suppress

        with suppress(Exception):
            th_live, tl_live = _get_sensor_thresholds_from_arduino_by_id(
                current_sensor_id, timeout=0.8
            )
            if th_live is not None and tl_live is not None:
                sensor_info = create_threshold_sensor_info(
                    disp_addr, current_sensor_id, float(th_live), float(tl_live)
                )
                return (
                    {"display": "block"},
                    sensor_info,
                    current_sensor_id,
                    float(th_live),
                    float(tl_live),
                )
        # 안전하게 넘어가서 재시도 루프로 진행

        # 4-2) 직접 질의 (리셋 직후 타이밍 문제 완화 위해 소규모 재시도)
        print(f"🔍 TH/TL 실시간 조회 시작: Sensor ID {current_sensor_id}")
        current_th: float | None = None
        current_tl: float | None = None
        for _ in range(3):
            current_th, current_tl = _get_sensor_thresholds_from_arduino_by_id(
                current_sensor_id, timeout=0.9
            )
            if current_th is not None and current_tl is not None:
                break
            # 잠깐 대기 후 재시도 (부팅 직후 응답 지연 대비)
            time.sleep(0.3)
        print(
            f"🎯 Arduino에서 실시간 TH/TL 결과: ID {current_sensor_id} - TH={current_th}, TL={current_tl}"
        )
        # 라이브 조회가 모두 실패했다면, 매니저 값이 있으면 사용, 없으면 기본값
        display_th = (
            current_th
            if current_th is not None
            else (th_from_mgr if th_from_mgr is not None else 30.0)
        )
        display_tl = (
            current_tl
            if current_tl is not None
            else (tl_from_mgr if tl_from_mgr is not None else 15.0)
        )
        # 라이브 조회 실패 후 캐시 사용 시 명시적 로깅
        if (current_th is None or current_tl is None) and (
            th_from_mgr is not None and tl_from_mgr is not None
        ):
            with suppress(Exception):
                print(
                    f"[TH/TL FETCH DEBUG] Falling back to SensorManager cached values: TH={float(th_from_mgr)}, TL={float(tl_from_mgr)}"
                )
        sensor_info = create_threshold_sensor_info(
            disp_addr, current_sensor_id, display_th, display_tl
        )
        return (
            {"display": "block"},
            sensor_info,
            current_sensor_id,
            float(display_th),
            float(display_tl),
        )

    # 5) 마지막 폴백: 주소만 알 때 기본값 표기
    disp_addr = current_address or status_addr or "Unknown"
    sensor_info = create_threshold_sensor_info(disp_addr, "00", 30.0, 15.0)
    return {"display": "block"}, sensor_info, disp_addr, 30.0, 15.0


def _handle_threshold_modal_close() -> (
    tuple[dict[str, str], list[Any], str, None, None]
):
    """임계값 모달 닫기 처리 로직"""
    return {"display": "none"}, [], "", None, None


def _handle_threshold_validation(
    th_value: float | None, tl_value: float | None
) -> tuple[list[Any], bool]:
    """임계값 유효성 검사"""
    validation_messages = []
    is_valid = True

    # 둘 다 비어있으면 오류
    if th_value is None and tl_value is None:
        validation_messages.append(
            html.Div(
                "❌ TH 또는 TL 중 최소 하나는 입력해주세요.",
                className="validation-error",
            )
        )
        is_valid = False
        return validation_messages, is_valid

    # 범위 검사
    if th_value is not None and (th_value < -55 or th_value > 125):
        validation_messages.append(
            html.Div(
                "❌ TH 값은 -55°C ~ 125°C 범위여야 합니다.",
                className="validation-error",
            )
        )
        is_valid = False

    if tl_value is not None and (tl_value < -55 or tl_value > 125):
        validation_messages.append(
            html.Div(
                "❌ TL 값은 -55°C ~ 125°C 범위여야 합니다.",
                className="validation-error",
            )
        )
        is_valid = False

    # TH > TL 검사 (둘 다 입력된 경우)
    if th_value is not None and tl_value is not None and th_value <= tl_value:
        validation_messages.append(
            html.Div("❌ TH 값은 TL 값보다 커야 합니다.", className="validation-error")
        )
        is_valid = False

    # 성공 메시지
    if is_valid:
        if th_value is not None and tl_value is not None:
            validation_messages.append(
                html.Div(
                    f"✅ 설정 가능: TH={th_value:.1f}°C, TL={tl_value:.1f}°C",
                    className="validation-success",
                )
            )
        elif th_value is not None:
            validation_messages.append(
                html.Div(
                    f"✅ 설정 가능: TH={th_value:.1f}°C", className="validation-success"
                )
            )
        else:
            validation_messages.append(
                html.Div(
                    f"✅ 설정 가능: TL={tl_value:.1f}°C", className="validation-success"
                )
            )

    return validation_messages, is_valid


def _handle_threshold_change_execution(
    sensor_identifier: str | None, th_value: float | None, tl_value: float | None
) -> list[Any]:
    """임계값 변경 실행 로직 (ID 기반)"""
    if not sensor_identifier:
        raise PreventUpdate

    if not port_manager.is_connected():
        return [_create_error_alert("Arduino가 연결되지 않았습니다.")]

    try:
        return _execute_threshold_change(sensor_identifier, th_value, tl_value)
    except Exception as e:
        return [_create_error_alert(f"오류 발생: {e}")]


def _execute_threshold_change(
    sensor_identifier: str, th_value: float | None, tl_value: float | None
) -> list[Any]:
    """Arduino에 임계값 변경 명령 전송 (ID/주소 자동 감지)"""
    print(
        f"🔧 _execute_threshold_change called - Identifier: {sensor_identifier}, TH: {th_value}, TL: {tl_value}"
    )

    # 센서 식별자가 ID 형식인지 확인 (01-08)
    if sensor_identifier in ["01", "02", "03", "04", "05", "06", "07", "08"]:
        # ID 기반 명령 (새로운 주요 방식)
        print(f"🆔 Using sensor ID based command: {sensor_identifier}")
        command_data: dict[str, Any] = {
            "type": "command",
            "command": "set_threshold",
            "sensor_id": int(sensor_identifier),
        }
    else:
        # 주소 기반 명령 (호환성)
        print(f"📍 Using sensor address based command: {sensor_identifier}")
        command_data = {
            "type": "command",
            "command": "set_threshold",
            "sensor_addr": sensor_identifier,
        }

    # 설정할 값만 포함
    if th_value is not None:
        command_data["th_value"] = float(th_value)
    if tl_value is not None:
        command_data["tl_value"] = float(tl_value)

    command = json.dumps(command_data) + "\\n"

    # 명령 전송
    success = _send_arduino_command(command)
    if not success:
        return [_create_error_alert("명령 전송에 실패했습니다.")]

    # 응답 대기 및 처리
    response = _wait_for_arduino_response()
    if not response:
        return [_create_error_alert("Arduino 응답을 받지 못했습니다.")]

    return _process_threshold_response(response, th_value, tl_value)


def _send_arduino_command(command: str) -> bool:
    """Arduino에 명령 전송"""
    try:
        # 연결 상태 확인
        if not port_manager.is_connected():
            return False

        if not hasattr(port_manager, "_serial_connection"):
            return False

        if port_manager._serial_connection is None:
            return False

        # 명령 전송
        port_manager._serial_connection.write(command.encode())

        # 버퍼 플러시
        port_manager._serial_connection.flush()

        return True

    except Exception:
        return False


def _drain_serial_input(max_time: float = 0.2) -> None:
    """빠르게 시리얼 입력 버퍼를 비워서 이전 응답의 간섭을 줄임"""
    sc = getattr(port_manager, "_serial_connection", None)
    if not sc:
        return
    end_time = time.time() + max_time
    try:
        # reset_input_buffer 가 더 빠르지만, 남은 줄을 읽어 로그에도 남김
        sc.reset_input_buffer()
        while time.time() < end_time and sc.in_waiting > 0:
            _ = sc.read(sc.in_waiting)
            time.sleep(0.02)
    except Exception:
        pass


def _wait_for_arduino_response(
    timeout: float = 5.0,
    required_keys: set[str] | None = None,
    accepted_types: set[str] | None = None,
) -> str | None:
    """Arduino 응답 대기

    required_keys 가 지정되면 해당 키들이 포함된 JSON 응답만 유효로 간주합니다.
    """
    try:
        start_time = time.time()
        response_buffer = ""
        accepted = accepted_types or {"response"}

        # 누적 버퍼를 유지하며 개행 단위로 완성된 JSON만 파싱한다
        while time.time() - start_time < timeout:
            sc = getattr(port_manager, "_serial_connection", None)
            if sc and sc.in_waiting > 0:
                data = sc.read(sc.in_waiting)
                chunk = data.decode("utf-8", errors="ignore")
                response_buffer += chunk

                # 디버그 메시지 출력 (TH/TL/EEPROM/COMM 관련 위주)
                for debug_line in chunk.split("\n"):
                    d = debug_line.strip()
                    if d and (
                        "[TH/TL DEBUG]" in d
                        or "[EEPROM DEBUG]" in d
                        or "[SAVE DEBUG]" in d
                        or "[COMM DEBUG]" in d
                        or "[GET CONFIG DEBUG]" in d
                    ):
                        print(f"🔍 Arduino Debug: {d}")

            # 완성된 라인들만 파싱하고 마지막 미완성 조각은 버퍼에 남긴다
            if "\n" in response_buffer:
                parts = response_buffer.split("\n")
                complete_lines = parts[:-1]
                response_buffer = parts[-1]

                for raw_line in complete_lines:
                    line = raw_line.strip()
                    if not (line.startswith("{") and line.endswith("}")):
                        continue
                    try:
                        json_data = json.loads(line)
                        if json_data.get("type") in accepted and (
                            required_keys is None
                            or required_keys.issubset(json_data.keys())
                        ):
                            return line
                    except json.JSONDecodeError:
                        continue

            time.sleep(0.05)

        return None

    except Exception:
        return None


def _process_threshold_response(
    response: str, th_value: float | None, tl_value: float | None
) -> list[Any]:
    """Arduino 응답 처리"""
    try:
        # JSON 응답 파싱
        response_data = json.loads(response)

        if response_data.get("type") == "response":
            status = response_data.get("status")
            message = response_data.get("message", "")

            if status == "success":
                # 성공 응답 처리
                success_msg = "✅ 임계값이 성공적으로 설정되었습니다!"
                if th_value is not None and tl_value is not None:
                    success_msg += f" (TH={th_value:.1f}°C, TL={tl_value:.1f}°C)"
                elif th_value is not None:
                    success_msg += f" (TH={th_value:.1f}°C)"
                elif tl_value is not None:
                    success_msg += f" (TL={tl_value:.1f}°C)"

                return [_create_success_alert(success_msg)]
            else:
                # 실패 응답 처리
                return [_create_error_alert(f"설정 실패: {message}")]
        else:
            return [_create_error_alert("잘못된 응답 형식입니다.")]

    except json.JSONDecodeError:
        return [_create_error_alert("응답 파싱 오류가 발생했습니다.")]
    except Exception as e:
        return [_create_error_alert(f"응답 처리 중 오류: {e}")]


def _extract_sensor_id(sensor_id_display: str | None) -> str:
    """UI에서 표시된 센서 ID 추출"""
    print(f"🔍 [SENSOR ID DEBUG] Input sensor_id: '{sensor_id_display}'")

    if not sensor_id_display:
        print("❌ [SENSOR ID DEBUG] Missing sensor ID!")
        return ""

    # 센서 ID 정리 (공백 제거, 숫자만 추출)
    clean_id = sensor_id_display.strip()

    # 01-08 범위 체크
    if clean_id in ["01", "02", "03", "04", "05", "06", "07", "08"]:
        print(f"✅ [SENSOR ID DEBUG] Valid sensor ID: '{clean_id}'")
        return clean_id
    else:
        print(f"❌ [SENSOR ID DEBUG] Invalid sensor ID: '{clean_id}' (must be 01-08)")
        return ""


def _extract_sensor_address(addr_part1: str | None, addr_part2: str | None) -> str:
    """UI에서 표시된 주소 부분들을 16자리 주소로 복원"""
    print(f"🔍 [ADDRESS DEBUG] Input parts: '{addr_part1}' + '{addr_part2}'")

    if not addr_part1 or not addr_part2:
        print("❌ [ADDRESS DEBUG] Missing address parts!")
        return ""

    # 콜론 제거하여 16자리 주소 생성
    clean_part1 = addr_part1.replace(":", "")
    clean_part2 = addr_part2.replace(":", "")
    full_address = clean_part1 + clean_part2

    print(f"🔍 [ADDRESS DEBUG] Cleaned parts: '{clean_part1}' + '{clean_part2}'")
    print(
        f"🔍 [ADDRESS DEBUG] Full address: '{full_address}' (length: {len(full_address)})"
    )

    # 16자리이면서 HEX 문자열인지 확인
    if len(full_address) == 16:
        result = full_address.upper()
        try:
            int(result, 16)
        except ValueError:
            print(f"❌ [ADDRESS DEBUG] Invalid hex characters in address: '{result}'")
            return ""
        print(f"✅ [ADDRESS DEBUG] Final result: '{result}'")
        return result
    else:
        print(f"❌ [ADDRESS DEBUG] Invalid length: {len(full_address)} (expected: 16)")

    return ""


def _create_error_alert(message: str) -> dbc.Alert:
    """에러 알림 생성"""
    return dbc.Alert(f"❌ {message}", color="danger")


def _create_success_alert(message: str) -> dbc.Alert:
    """성공 알림 생성"""
    return dbc.Alert(f"✅ {message}", color="success")


def _create_warning_alert(message: str) -> dbc.Alert:
    """경고 알림 생성"""
    return dbc.Alert(f"⏱️ {message}", color="warning")


def _get_sensor_thresholds_from_arduino_by_id(
    sensor_id: str, timeout: float = 3.0
) -> tuple[float | None, float | None]:
    """Arduino에서 센서 ID 기반으로 TH/TL 실시간 조회 (새로운 주요 함수)"""
    print(f"🔍 [TH/TL FETCH DEBUG] Fetching thresholds for sensor ID: {sensor_id}")

    try:
        # 센서 ID를 정수로 변환
        sensor_id_int = int(sensor_id)
        if sensor_id_int < 1 or sensor_id_int > 8:
            print(f"❌ [TH/TL FETCH DEBUG] Invalid sensor ID range: {sensor_id_int}")
            return None, None

        # 기존 응답 잔여분 제거 (간헐적 기본값 노출 원인 차단)
        _drain_serial_input()

        # Arduino에 get_sensor_config 명령 전송 (sensor_id 기반)
        command_data = {
            "type": "command",
            "command": "get_sensor_config",
            "sensor_id": sensor_id_int,
        }

        command = json.dumps(command_data) + "\\n"

        # 명령 전송
        success = _send_arduino_command(command)
        if not success:
            print("❌ [TH/TL FETCH DEBUG] Failed to send command")
            return None, None

        # 응답 대기: 반드시 th_value / tl_value 키가 포함된 응답만 채택
        response = _wait_for_arduino_response(
            timeout=max(0.2, float(timeout)), required_keys={"th_value", "tl_value"}
        )
        if not response:
            print("❌ [TH/TL FETCH DEBUG] No response received")
            return None, None

        print(f"🔍 [TH/TL FETCH DEBUG] Raw response: {response}")

        # JSON 응답 파싱
        response_data = json.loads(response)

        if (
            response_data.get("type") == "response"
            and response_data.get("status") == "success"
        ):
            th_value = response_data.get("th_value")
            tl_value = response_data.get("tl_value")
            print(
                f"✅ [TH/TL FETCH DEBUG] Parsed values - TH: {th_value}, TL: {tl_value}"
            )
            return th_value, tl_value
        else:
            print("❌ [TH/TL FETCH DEBUG] Invalid response format or error")
            return None, None

    except Exception as e:
        print(f"❌ [TH/TL FETCH DEBUG] Exception: {e}")
        return None, None


def _get_sensor_thresholds_from_arduino(sensor_address: str) -> tuple[float, float]:
    """Arduino에서 특정 센서의 현재 TH/TL 값을 가져옴"""
    print(f"🔧 get_sensor_thresholds_from_arduino 호출됨: {sensor_address}")

    try:
        if not port_manager.is_connected():
            print("❌ Arduino 연결되지 않음")
            return 30.0, 15.0  # 기본값

        print("✅ Arduino 연결 확인됨")

        # Arduino에 센서별 TH/TL 조회 명령 전송
        command_data = {
            "type": "command",
            "command": "get_sensor_config",
            "sensor_addr": sensor_address,
        }
        command = json.dumps(command_data) + "\\n"
        print(f"📤 Arduino 조회 명령: {command.strip()}")

        # 이전 응답 비우기
        _drain_serial_input()

        # 명령 전송
        success = _send_arduino_command(command)
        if not success:
            print("❌ 명령 전송 실패")
            return 30.0, 15.0

        print("✅ 명령 전송 성공")

        # 응답 대기 (TH/TL 키 필수)
        response = _wait_for_arduino_response(
            timeout=2.0, required_keys={"th_value", "tl_value"}
        )
        if not response:
            print("❌ Arduino 응답 타임아웃")
            return 30.0, 15.0

        print(f"📥 Arduino 응답 수신: {response}")

        # 응답 파싱
        response_data = json.loads(response)
        if (
            response_data.get("type") == "response"
            and response_data.get("status") == "success"
        ):
            # 응답에서 TH/TL 값 추출
            th_value = float(response_data.get("th_value", 30.0))
            tl_value = float(response_data.get("tl_value", 15.0))
            print(f"✅ TH/TL 값 파싱 성공: TH={th_value}, TL={tl_value}")
            return th_value, tl_value
        else:
            print(f"❌ 응답 형식 오류: {response_data}")

    except Exception as e:
        print(f"❌ Arduino TH/TL 조회 실패: {e}")

    return 30.0, 15.0  # 실패시 기본값


def _fetch_system_status_with_retries(
    retries: int = 3, delay: float = 0.25, timeout: float = 0.6
) -> dict[str, Any] | None:
    """system_status를 소규모 재시도로 가져옴.

    장치 초기화 직후 첫 응답 지연을 흡수하여, 모달 첫 오픈 시 신원을 확보하도록 돕는다.
    """
    if not port_manager.is_connected():
        return None

    status: dict[str, Any] | None = None
    for _ in range(max(1, int(retries))):
        try:
            _drain_serial_input()
            status_cmd = json.dumps({"type": "command", "command": "get_status"}) + "\n"
            _send_arduino_command(status_cmd)
            status_line = _wait_for_arduino_response(
                timeout=max(0.2, float(timeout)), accepted_types={"system_status"}
            )
            if status_line:
                tmp = json.loads(status_line)
                if isinstance(tmp, dict) and tmp.get("type") == "system_status":
                    status = tmp
                    break
        except Exception:
            status = None
        time.sleep(max(0.0, float(delay)))

    return status


def register_threshold_callbacks(app: Any) -> None:
    """임계값 설정 관련 콜백 등록"""

    # 임계값 모달 열기/닫기 처리
    @app.callback(
        [
            Output("threshold-modal", "style", allow_duplicate=True),
            Output("current-threshold-sensor-info", "children", allow_duplicate=True),
            Output("threshold-sensor-address-store", "data", allow_duplicate=True),
            Output("th-input", "value", allow_duplicate=True),
            Output("tl-input", "value", allow_duplicate=True),
        ],
        [
            Input("btn-thtl", "n_clicks"),
            Input("threshold-cancel-btn", "n_clicks"),
            Input("threshold-close-btn", "n_clicks"),
            Input("threshold-confirm-btn", "n_clicks"),
        ],
        [
            State("threshold-modal", "style"),
            State("threshold-sensor-address-store", "data"),
            State("main-sensor-id", "children"),  # 현재 표시된 센서 ID
            State("main-addr-0", "children"),  # 현재 표시된 센서 주소 (첫 번째 부분)
            State("main-addr-1", "children"),  # 현재 표시된 센서 주소 (두 번째 부분)
        ],
        prevent_initial_call=True,
    )
    def toggle_threshold_modal(
        open_clicks: int | None,
        cancel_clicks: int | None,
        close_clicks: int | None,
        confirm_clicks: int | None,
        current_style: dict | None,
        sensor_address: str | None,
        current_sensor_id: str | None,
        addr_part1: str | None,
        addr_part2: str | None,
    ) -> tuple[dict, list, str, float | None, float | None]:
        """임계값 모달 열기/닫기 및 센서 정보 표시"""
        ctx = callback_context
        if not ctx.triggered:
            raise PreventUpdate

        trigger_id = ctx.triggered[0]["prop_id"].split(".")[0]

        if trigger_id == "btn-thtl":
            # 즉시 오픈: 캐시 값으로 시드 (빠른 반응)
            return _handle_threshold_modal_open_fast(
                current_sensor_id, addr_part1, addr_part2
            )

        elif trigger_id in [
            "threshold-cancel-btn",
            "threshold-close-btn",
            "threshold-confirm-btn",
        ]:
            return _handle_threshold_modal_close()

        # 기본값 (변화 없음)
        current_display = (
            current_style.get("display", "none") if current_style else "none"
        )
        return {"display": current_display}, [], sensor_address or "", None, None

    # 팝업 오픈 직후 정밀 값으로 업데이트 (느린 작업 분리)
    @app.callback(
        [
            Output(
                "current-threshold-sensor-info",
                "children",
                allow_duplicate=True,
            ),
            Output("th-input", "value", allow_duplicate=True),
            Output("tl-input", "value", allow_duplicate=True),
        ],
        Input("btn-thtl", "n_clicks"),
        [
            State("main-sensor-id", "children"),
            State("main-addr-0", "children"),
            State("main-addr-1", "children"),
        ],
        prevent_initial_call=True,
    )
    def refresh_threshold_modal_values(
        open_clicks: int | None,
        current_sensor_id: str | None,
        addr_part1: str | None,
        addr_part2: str | None,
    ) -> tuple[list[Any], float | None, float | None]:
        # 장치에서 가능한 정밀 값 조회(이전에 느렸던 작업)
        style, sensor_info, identifier, th_val, tl_val = _handle_threshold_modal_open(
            current_sensor_id, addr_part1, addr_part2
        )
        # style 은 이 콜백에서 변경하지 않음
        return sensor_info, th_val, tl_val

    # 임계값 입력 유효성 검사
    @app.callback(
        [
            Output("threshold-validation", "children"),
            Output("threshold-confirm-btn", "disabled"),
        ],
        [
            Input("th-input", "value"),
            Input("tl-input", "value"),
        ],
        prevent_initial_call=True,
    )
    def validate_threshold_inputs(
        th_value: float | None, tl_value: float | None
    ) -> tuple[list, bool]:
        """임계값 입력 유효성 검사"""
        validation_messages, is_valid = _handle_threshold_validation(th_value, tl_value)
        return validation_messages, not is_valid

    # 임계값 변경 실행
    @app.callback(
        Output("threshold-result", "children", allow_duplicate=True),
        Input("threshold-confirm-btn", "n_clicks"),
        [
            State("threshold-sensor-address-store", "data"),
            State("th-input", "value"),
            State("tl-input", "value"),
        ],
        prevent_initial_call=True,
    )
    def execute_threshold_change(
        confirm_clicks: int | None,
        sensor_address: str | None,
        th_value: float | None,
        tl_value: float | None,
    ) -> list:
        """임계값 변경 실행"""
        if not confirm_clicks:
            raise PreventUpdate

        return _handle_threshold_change_execution(sensor_address, th_value, tl_value)

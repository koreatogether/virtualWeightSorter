"""Arduino 연결 및 센서 통신 진단 로직"""

import time
from dataclasses import dataclass

import serial
import serial.tools.list_ports

from .port_manager import port_manager


@dataclass
class DiagnosticResult:
    """진단 결과"""

    success: bool
    issue_type: str  # 'port_busy', 'hardware_disconnected', 'communication_error', 'sensor_missing'
    message: str
    recommendation: str


def is_port_accessible(port: str) -> bool:
    """포트 접근 가능성 테스트"""
    try:
        # 짧은 시간 동안 포트 열기 시도
        test_serial = serial.Serial(port, 9600, timeout=1)
        test_serial.close()
        return True
    except serial.SerialException:
        return False
    except Exception:
        return False


def test_port_response(port: str, timeout: float) -> bool:
    """포트에서 기본 응답 테스트"""
    try:
        test_serial = serial.Serial(port, 9600, timeout=timeout)
        time.sleep(0.5)  # 안정화

        # DTR 신호로 Arduino 리셋 시도
        test_serial.dtr = False
        time.sleep(0.1)
        test_serial.dtr = True
        time.sleep(1.5)  # 부팅 대기

        # 버퍼 클리어
        test_serial.reset_input_buffer()

        # 간단한 문자 전송
        test_serial.write(b"\\n")

        # 응답 대기 (어떤 응답이라도)
        response = test_serial.read(100)

        test_serial.close()
        return len(response) > 0 or test_serial.in_waiting > 0

    except Exception:
        return False


def test_arduino_communication(port: str) -> bool:
    """Arduino와 실제 통신 테스트"""
    try:
        test_serial = serial.Serial(port, 9600, timeout=3)
        time.sleep(0.5)

        result = _perform_ping_test(test_serial)
        test_serial.close()
        return result

    except Exception:
        return False


def _perform_ping_test(test_serial: serial.Serial) -> bool:
    """핑 테스트 수행"""
    test_serial.write(b"ping\\n")
    return _wait_for_pong_response(test_serial, timeout=3.0)


def _wait_for_pong_response(test_serial: serial.Serial, timeout: float) -> bool:
    """pong 응답 대기"""
    start_time = time.time()

    while time.time() - start_time < timeout:
        if test_serial.in_waiting > 0:
            line = test_serial.readline().decode("utf-8", errors="ignore").strip()
            if line and "pong" in line.lower():
                return True

    return False


def diagnose_connection_failure(port: str) -> DiagnosticResult:
    """연결 실패 원인 진단"""
    # 1. 포트 존재 확인
    port_check = _check_port_availability(port)
    if port_check:
        return port_check

    # 2. 포트 접근성 테스트
    access_check = _check_port_accessibility(port)
    if access_check:
        return access_check

    # 3. 하드웨어 연결 테스트
    hardware_check = _check_hardware_connection(port)
    if hardware_check:
        return hardware_check

    # 4. Arduino 통신 테스트
    communication_check = _check_arduino_communication(port)
    if communication_check:
        return communication_check

    return DiagnosticResult(
        success=True,
        issue_type="none",
        message="진단 완료 - 연결 가능",
        recommendation="",
    )


def _check_port_availability(port: str) -> DiagnosticResult | None:
    """포트 존재 확인"""
    available_ports = [p.device for p in serial.tools.list_ports.comports()]
    if port not in available_ports:
        return DiagnosticResult(
            success=False,
            issue_type="port_not_found",
            message=f"COM 포트 {port}를 찾을 수 없음",
            recommendation="포트 목록을 새로고침하고 다른 포트를 선택하세요. USB 케이블이 제대로 연결되었는지 확인하세요.",
        )
    return None


def _check_port_accessibility(port: str) -> DiagnosticResult | None:
    """포트 접근성 테스트"""
    if not is_port_accessible(port):
        return DiagnosticResult(
            success=False,
            issue_type="port_busy",
            message=f"COM 포트 {port} 접근 실패",
            recommendation="다른 프로그램에서 사용 중일 수 있습니다. Arduino IDE, 시리얼 모니터, 다른 Python 프로그램을 종료 후 다시 시도하세요.",
        )
    return None


def _check_hardware_connection(port: str) -> DiagnosticResult | None:
    """하드웨어 연결 테스트"""
    if not test_port_response(port, 2.0):
        return DiagnosticResult(
            success=False,
            issue_type="hardware_disconnected",
            message=f"COM 포트 {port}에서 응답 없음",
            recommendation="USB 케이블 연결 상태를 확인하고, Arduino 보드의 전원 LED가 켜져 있는지 확인하세요. 다른 USB 포트로 시도해보세요.",
        )
    return None


def _check_arduino_communication(port: str) -> DiagnosticResult | None:
    """Arduino 통신 테스트"""
    if not test_arduino_communication(port):
        return DiagnosticResult(
            success=False,
            issue_type="communication_error",
            message="Arduino는 연결되었으나 통신 프로토콜 오류",
            recommendation="Arduino에 올바른 펌웨어가 업로드되어 있는지 확인하세요. 보드 타입과 통신 속도(9600 bps)를 확인하세요.",
        )
    return None


def diagnose_sensor_communication(port: str) -> DiagnosticResult:
    """센서 통신 진단 (연결 후 호출)"""

    if not port_manager.is_connected():
        return DiagnosticResult(
            success=False,
            issue_type="not_connected",
            message="Arduino에 연결되지 않음",
            recommendation="먼저 Arduino에 연결하세요.",
        )

    # Arduino 핑 테스트
    if not port_manager.test_communication():
        return DiagnosticResult(
            success=False,
            issue_type="communication_error",
            message="Arduino 핑 테스트 실패",
            recommendation="펌웨어가 올바르게 동작하지 않습니다. Arduino 코드를 다시 업로드하고 시리얼 모니터로 수동 테스트해보세요.",
        )

    # TODO: 센서 스캔 테스트는 나중에 Arduino 펌웨어와 함께 구현
    # sensors = scan_sensors(port)
    # if not sensors:
    #     return DiagnosticResult(
    #         success=False,
    #         issue_type='sensor_missing',
    #         message='DS18B20 센서를 찾을 수 없음',
    #         recommendation='센서 배선을 확인하세요. 데이터 핀이 올바른 디지털 핀에 연결되고 풀업 저항(4.7kΩ)이 연결되어 있는지 확인하세요.'
    #     )

    return DiagnosticResult(
        success=True,
        issue_type="none",
        message="진단 완료 - Arduino 통신 정상",
        recommendation="",
    )


def get_port_info(port: str) -> str:
    """포트 상세 정보 반환"""
    try:
        ports = serial.tools.list_ports.comports()
        for p in ports:
            if p.device == port:
                return f"{p.device}: {p.description} [{p.manufacturer}]"
        return f"{port}: 정보 없음"
    except Exception:
        return f"{port}: 정보 확인 불가"

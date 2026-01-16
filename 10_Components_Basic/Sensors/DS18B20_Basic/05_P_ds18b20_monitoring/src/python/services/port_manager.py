"""COM 포트 관리 및 Arduino 연결 처리"""

import time
from dataclasses import dataclass
from datetime import datetime
from typing import Any

import serial
import serial.tools.list_ports


@dataclass
class ConnectionStatus:
    """연결 상태 정보"""

    is_connected: bool
    port: str | None
    baudrate: int | None
    last_error: str | None
    connected_since: datetime | None


class PortManager:
    """COM 포트 검색, 연결, 상태 관리"""

    def __init__(self) -> None:
        self._serial_connection: serial.Serial | None = None
        self._current_port: str | None = None
        self._baudrate: int = 115200  # Arduino 통신 속도에 맞춰 변경
        self._last_error: str | None = None
        self._connected_since: datetime | None = None

    def scan_ports(self) -> list[str]:
        """사용 가능한 COM 포트 목록 반환 (COM0, COM1 제외)"""
        try:
            ports = serial.tools.list_ports.comports()
            available_ports = []

            for port in ports:
                # COM0, COM1 제외
                if port.device not in ["COM0", "COM1"]:
                    available_ports.append(port.device)

            # 포트 번호로 정렬
            available_ports.sort(key=lambda x: int(x.replace("COM", "")))
            return available_ports

        except Exception as e:
            self._last_error = f"포트 스캔 오류: {e!s}"
            return []

    def connect(self, port: str, timeout: float = 3.0) -> bool:
        """지정된 포트로 연결 시도"""
        if self.is_connected():
            self.disconnect()

        try:
            self._serial_connection = serial.Serial(
                port=port,
                baudrate=self._baudrate,
                timeout=timeout,
                write_timeout=timeout,
            )

            # 연결 안정화를 위한 짧은 대기
            time.sleep(0.5)

            # 버퍼 클리어
            self._serial_connection.reset_input_buffer()
            self._serial_connection.reset_output_buffer()

            self._current_port = port
            self._connected_since = datetime.now()
            self._last_error = None

            return True

        except serial.SerialException as e:
            self._last_error = f"포트 {port} 연결 실패: {e!s}"
            self._serial_connection = None
            self._current_port = None
            return False
        except Exception as e:
            self._last_error = f"예상치 못한 오류: {e!s}"
            self._serial_connection = None
            self._current_port = None
            return False

    def disconnect(self) -> None:
        """현재 연결 해제"""
        if self._serial_connection and self._serial_connection.is_open:
            try:
                self._serial_connection.close()
            except Exception as e:
                self._last_error = f"연결 해제 오류: {e!s}"

        self._serial_connection = None
        self._current_port = None
        self._connected_since = None

    def is_connected(self) -> bool:
        """연결 상태 확인"""
        return self._serial_connection is not None and self._serial_connection.is_open

    def get_current_port(self) -> str | None:
        """현재 연결된 포트 반환"""
        return self._current_port if self.is_connected() else None

    def get_status(self) -> ConnectionStatus:
        """현재 연결 상태 반환"""
        return ConnectionStatus(
            is_connected=self.is_connected(),
            port=self._current_port,
            baudrate=self._baudrate if self.is_connected() else None,
            last_error=self._last_error,
            connected_since=self._connected_since,
        )

    def send_data(self, data: str) -> bool:
        """데이터 전송"""
        if not self.is_connected() or self._serial_connection is None:
            self._last_error = "연결되지 않음"
            return False

        try:
            self._serial_connection.write(data.encode("utf-8"))
            return True
        except Exception as e:
            self._last_error = f"데이터 전송 오류: {e!s}"
            return False

    def read_data(self, timeout: float = 2.0) -> str | None:
        """데이터 읽기"""
        if not self.is_connected() or self._serial_connection is None:
            self._last_error = "연결되지 않음"
            return None

        try:
            # 원래 타임아웃 저장
            original_timeout = self._serial_connection.timeout
            self._serial_connection.timeout = timeout

            # 줄 단위로 읽기
            line = self._serial_connection.readline().decode("utf-8").strip()

            # 타임아웃 복원
            self._serial_connection.timeout = original_timeout

            return line if line else None

        except Exception as e:
            self._last_error = f"데이터 읽기 오류: {e!s}"
            return None

    def test_communication(self) -> bool:
        """Arduino와 통신 테스트 (JSON 센서 데이터 확인)"""
        if not self.is_connected() or self._serial_connection is None:
            return False

        try:
            self._clear_input_buffer()
            return self._wait_for_sensor_data(timeout=5.0)
        except Exception as e:
            self._last_error = f"통신 테스트 오류: {e!s}"
            return False

    def _clear_input_buffer(self) -> None:
        """입력 버퍼 클리어"""
        if self._serial_connection:
            self._serial_connection.reset_input_buffer()

    def _wait_for_sensor_data(self, timeout: float) -> bool:
        """센서 데이터 수신 대기"""

        start_time = time.time()

        while time.time() - start_time < timeout:
            if self._serial_connection and self._serial_connection.in_waiting > 0:
                line = self._read_line_safe()
                if line and self._is_valid_sensor_data(line):
                    return True
            time.sleep(0.1)

        return False

    def _read_line_safe(self) -> str | None:
        """안전한 라인 읽기"""
        if not self._serial_connection:
            return None

        try:
            line = (
                self._serial_connection.readline()
                .decode("utf-8", errors="ignore")
                .strip()
            )
            return line if line else None
        except Exception:
            return None

    def _is_valid_sensor_data(self, line: str) -> bool:
        """유효한 센서 데이터인지 확인"""
        import json

        try:
            data = json.loads(line)
            return data.get("type") == "sensor_data" and "temperature" in data
        except json.JSONDecodeError:
            return False

    def read_sensor_data(self, timeout: float = 2.0) -> dict[str, Any] | None:
        """센서 데이터 JSON 읽기"""
        if not self.is_connected() or self._serial_connection is None:
            return None

        try:
            return self._read_json_data(timeout, "sensor_data")
        except Exception as e:
            self._last_error = f"센서 데이터 읽기 오류: {e!s}"
            return None

    def _read_json_data(
        self, timeout: float, expected_type: str
    ) -> dict[str, Any] | None:
        """JSON 데이터 읽기 (타입 필터링)"""

        start_time = time.time()

        while time.time() - start_time < timeout:
            if self._serial_connection and self._serial_connection.in_waiting > 0:
                line = self._read_line_safe()
                if line:
                    data = self._parse_json_safe(line)
                    if data and data.get("type") == expected_type:
                        return data
            time.sleep(0.05)

        return None

    def _parse_json_safe(self, line: str) -> dict[str, Any] | None:
        """안전한 JSON 파싱"""
        import json

        try:
            data = json.loads(line)
            return data if isinstance(data, dict) else None
        except json.JSONDecodeError:
            return None


# 전역 포트 매니저 인스턴스
port_manager = PortManager()

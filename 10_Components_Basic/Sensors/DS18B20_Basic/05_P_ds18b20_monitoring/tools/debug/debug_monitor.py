#!/usr/bin/env python3
"""
🔍 DS18B20 대시보드 디버그 모니터링 시스템
- 모든 중요한 이벤트를 실시간 추적
- 개별 센서창 활성화 문제 진단
- 함수 호출, 변수 상태, 콜백 등록 과정 감시
"""

import logging
from datetime import datetime
from pathlib import Path
from typing import ClassVar

# 로그 디렉토리 생성
LOG_DIR = Path("debug_logs")
LOG_DIR.mkdir(exist_ok=True)

# 시간별 로그 파일
timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
log_file = LOG_DIR / f"dashboard_debug_{timestamp}.log"


# 커스텀 포맷터
class DebugFormatter(logging.Formatter):
    """색상과 아이콘이 포함된 디버그 포맷터"""

    COLORS: ClassVar[dict[str, str]] = {
        "DEBUG": "\033[36m",  # 청록색
        "INFO": "\033[32m",  # 녹색
        "WARNING": "\033[33m",  # 노란색
        "ERROR": "\033[31m",  # 빨간색
        "CRITICAL": "\033[35m",  # 자주색
        "RESET": "\033[0m",  # 리셋
    }

    ICONS: ClassVar[dict[str, str]] = {
        "sensor_init": "🆕",
        "callback_reg": "📞",
        "connection": "🔌",
        "ui_update": "🔄",
        "error": "❌",
        "success": "✅",
        "warning": "⚠️",
        "data": "📊",
        "debug": "🔍",
    }

    def format(self, record):
        # 레벨별 색상 적용
        color = self.COLORS.get(record.levelname, self.COLORS["RESET"])
        reset = self.COLORS["RESET"]

        # 아이콘 추가
        icon = ""
        msg = record.getMessage()
        for key, emoji in self.ICONS.items():
            if key in msg.lower():
                icon = emoji + " "
                break

        # 포맷팅
        formatted = f"{color}[{record.asctime}] {icon}{record.levelname:<8} | {record.name:<25} | {msg}{reset}"
        return formatted


# 로거 설정
def setup_debug_logger():
    """디버그 전용 로거 설정"""
    logger = logging.getLogger("DASHBOARD_DEBUG")
    logger.setLevel(logging.DEBUG)

    # 기존 핸들러 제거
    for handler in logger.handlers[:]:
        logger.removeHandler(handler)

    # 파일 핸들러
    file_handler = logging.FileHandler(log_file, encoding="utf-8")
    file_handler.setLevel(logging.DEBUG)
    file_formatter = logging.Formatter(
        "[%(asctime)s] %(levelname)-8s | %(name)-25s | %(message)s", datefmt="%H:%M:%S"
    )
    file_handler.setFormatter(file_formatter)

    # 콘솔 핸들러 (색상 포함)
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.INFO)
    console_handler.setFormatter(DebugFormatter(datefmt="%H:%M:%S"))

    logger.addHandler(file_handler)
    logger.addHandler(console_handler)

    return logger


# 전역 디버그 로거
debug_logger = setup_debug_logger()


class SensorActivationMonitor:
    """센서 활성화 과정 전용 모니터"""

    def __init__(self):
        self.logger = logging.getLogger("SENSOR_ACTIVATION")
        self.sensor_states = {}
        self.callback_registry = {}
        self.ui_elements = {}

    def log_sensor_detection(self, sensor_id, address):
        """센서 감지 로그"""
        self.logger.info(f"🆕 SENSOR_INIT: 센서 {sensor_id} 감지됨 (addr: {address})")
        self.sensor_states[sensor_id] = {
            "detected": True,
            "address": address,
            "callbacks_registered": False,
            "ui_created": False,
            "data_received": False,
        }

    def log_callback_registration(self, sensor_id, callback_type, success=True):
        """콜백 등록 로그"""
        status = "✅" if success else "❌"
        self.logger.info(
            f"📞 CALLBACK_REG: {status} 센서 {sensor_id} {callback_type} 콜백 등록"
        )

        if sensor_id not in self.callback_registry:
            self.callback_registry[sensor_id] = {}
        self.callback_registry[sensor_id][callback_type] = success

        if sensor_id in self.sensor_states:
            self.sensor_states[sensor_id]["callbacks_registered"] = success

    def log_ui_creation(self, sensor_id, element_type, success=True):
        """UI 요소 생성 로그"""
        status = "✅" if success else "❌"
        self.logger.info(
            f"🔄 UI_UPDATE: {status} 센서 {sensor_id} {element_type} UI 생성"
        )

        if sensor_id not in self.ui_elements:
            self.ui_elements[sensor_id] = {}
        self.ui_elements[sensor_id][element_type] = success

        if sensor_id in self.sensor_states:
            self.sensor_states[sensor_id]["ui_created"] = success

    def log_data_received(self, sensor_id, temperature):
        """데이터 수신 로그"""
        self.logger.info(f"📊 DATA: 센서 {sensor_id} 데이터 수신 - {temperature}°C")

        if sensor_id in self.sensor_states:
            self.sensor_states[sensor_id]["data_received"] = True

    def diagnose_activation_issues(self):
        """활성화 문제 진단"""
        self.logger.info("🔍 DEBUG: ===== 센서 활성화 진단 시작 =====")

        for sensor_id, state in self.sensor_states.items():
            self.logger.info(f"🔍 DEBUG: 센서 {sensor_id} 상태:")
            self.logger.info(f"  - 감지됨: {state['detected']}")
            self.logger.info(f"  - 콜백등록: {state['callbacks_registered']}")
            self.logger.info(f"  - UI생성: {state['ui_created']}")
            self.logger.info(f"  - 데이터수신: {state['data_received']}")

            # 문제 진단
            if not state["callbacks_registered"]:
                self.logger.error(
                    f"❌ ERROR: 센서 {sensor_id} 콜백 등록 실패 - 개별창 비활성화 원인!"
                )
            if not state["ui_created"]:
                self.logger.error(f"❌ ERROR: 센서 {sensor_id} UI 생성 실패")
            if not state["data_received"]:
                self.logger.warning(f"⚠️ WARNING: 센서 {sensor_id} 데이터 수신 없음")

        self.logger.info("🔍 DEBUG: ===== 진단 완료 =====")

    def diagnose_ui_structure(self):
        """UI 구조 심층 진단"""
        self.logger.info("🔍 DEBUG: ===== UI 구조 심층 진단 시작 =====")

        try:
            import sys

            sys.path.insert(0, "src/python")
            from dashboard.data_manager import data_manager

            # 현재 등록된 센서들 확인
            sensors = data_manager.get_sensors_list()
            self.logger.info(f"📊 DATA: 등록된 센서 수: {len(sensors)}")

            for sensor_id in sensors:
                self.logger.info(f"🔍 DEBUG: 센서 {sensor_id} UI 구조 분석:")

                # 개별 섹션 확인
                section_id = f"sensor-{sensor_id}-section"
                value_id = f"sensor-{sensor_id}-value"
                button_id = f"btn-{sensor_id}"
                graph_id = f"graph-{sensor_id}"
                log_id = f"log-{sensor_id}"

                self.logger.info(f"  - 섹션 ID: {section_id}")
                self.logger.info(f"  - 값 표시 ID: {value_id}")
                self.logger.info(f"  - 버튼 ID: {button_id}")
                self.logger.info(f"  - 그래프 ID: {graph_id}")
                self.logger.info(f"  - 로그 ID: {log_id}")

        except Exception as e:
            self.logger.error(f"❌ ERROR: UI 구조 분석 실패 - {e}")

        self.logger.info("🔍 DEBUG: ===== UI 구조 진단 완료 =====")

    def diagnose_callback_functions(self):
        """콜백 함수 등록 상태 진단"""
        self.logger.info("🔍 DEBUG: ===== 콜백 함수 진단 시작 =====")

        try:
            import sys

            sys.path.insert(0, "src/python")
            from dashboard.app import app

            # Dash 앱의 콜백 등록 상태 확인
            callbacks = app.callback_map if hasattr(app, "callback_map") else {}
            self.logger.info(f"📞 CALLBACK_REG: 총 등록된 콜백 수: {len(callbacks)}")

            # 개별 센서 버튼 콜백 확인
            button_callbacks = [cb for cb in callbacks if "btn-" in str(cb)]
            self.logger.info(f"📞 CALLBACK_REG: 버튼 콜백 수: {len(button_callbacks)}")

            for cb in button_callbacks:
                self.logger.info(f"📞 CALLBACK_REG: 등록된 버튼 콜백: {cb}")

        except Exception as e:
            self.logger.error(f"❌ ERROR: 콜백 함수 분석 실패 - {e}")

        self.logger.info("🔍 DEBUG: ===== 콜백 함수 진단 완료 =====")

    def full_diagnostic(self):
        """전체 진단 실행"""
        self.diagnose_activation_issues()
        self.diagnose_ui_structure()
        self.diagnose_callback_functions()


class ConnectionMonitor:
    """연결 과정 전용 모니터"""

    def __init__(self):
        self.logger = logging.getLogger("CONNECTION")
        self.connection_steps = []

    def log_port_connection(self, port, success=True):
        """포트 연결 로그"""
        status = "✅" if success else "❌"
        self.logger.info(f"🔌 CONNECTION: {status} 포트 {port} 연결 시도")
        self.connection_steps.append(f"포트연결_{port}_{success}")

    def log_arduino_handshake(self, success=True):
        """아두이노 핸드셰이크 로그"""
        status = "✅" if success else "❌"
        self.logger.info(f"🔌 CONNECTION: {status} 아두이노 핸드셰이크")
        self.connection_steps.append(f"핸드셰이크_{success}")

    def log_data_stream_start(self, success=True):
        """데이터 스트림 시작 로그"""
        status = "✅" if success else "❌"
        self.logger.info(f"📊 DATA: {status} 데이터 스트림 시작")
        self.connection_steps.append(f"데이터스트림_{success}")


# 전역 모니터 인스턴스
sensor_monitor = SensorActivationMonitor()
connection_monitor = ConnectionMonitor()


def patch_dashboard_logging():
    """대시보드 모듈에 디버그 로깅 패치"""
    debug_logger.info("🔍 DEBUG: 대시보드 로깅 패치 시작")

    try:
        # data_manager 패치
        import sys

        sys.path.insert(0, "src/python")

        from dashboard.data_manager import DataManager

        # 원본 메서드 백업
        original_add_sensor_data = DataManager.add_sensor_data
        original_get_sensors_list = DataManager.get_sensors_list

        def patched_add_sensor_data(self, data):
            """센서 데이터 추가 패치"""
            sensor_id = data.get("sensor_display_id", "unknown")
            temp = data.get("temperature", "unknown")
            sensor_monitor.log_data_received(sensor_id, temp)
            return original_add_sensor_data(self, data)

        def patched_get_sensors_list(self):
            """센서 목록 조회 패치"""
            sensors = original_get_sensors_list(self)
            debug_logger.debug(f"🔍 DEBUG: 활성 센서 목록 조회됨: {sensors}")
            return sensors

        # 패치 적용
        DataManager.add_sensor_data = patched_add_sensor_data
        DataManager.get_sensors_list = patched_get_sensors_list

        debug_logger.info("✅ SUCCESS: 대시보드 로깅 패치 완료")

    except Exception as e:
        debug_logger.error(f"❌ ERROR: 패치 실패 - {e}")


def start_monitoring():
    """모니터링 시작"""
    debug_logger.info("🚀 디버그 모니터링 시스템 시작")
    debug_logger.info(f"📝 로그 파일: {log_file}")
    debug_logger.info("=" * 60)

    # 대시보드 패치
    patch_dashboard_logging()

    return {
        "sensor_monitor": sensor_monitor,
        "connection_monitor": connection_monitor,
        "debug_logger": debug_logger,
    }


if __name__ == "__main__":
    monitors = start_monitoring()
    debug_logger.info("🔍 디버그 모니터 준비 완료 - 대시보드를 시작하세요!")

    # 5초 후 자동 심층 진단 실행
    import time

    time.sleep(5)
    debug_logger.info("🔍 DEBUG: 자동 심층 진단 시작...")
    try:
        sensor_monitor.full_diagnostic()
    except Exception as e:
        debug_logger.error(f"❌ ERROR: 자동 진단 실패 - {e}")

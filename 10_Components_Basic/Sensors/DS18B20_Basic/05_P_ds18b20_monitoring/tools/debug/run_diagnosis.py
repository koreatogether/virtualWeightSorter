#!/usr/bin/env python3
"""
개별 센서창 비활성화 문제 진단 스크립트
"""

import sys
import time


def main():
    # Add project src to sys.path for local imports
    sys.path.insert(0, "src/python")

    # Local imports placed inside function to avoid E402
    from tools.debug.debug_monitor import debug_logger, sensor_monitor

    debug_logger.info("🔍 DEBUG: 별도 진단 스크립트 시작")

    # 5초 기다려서 대시보드가 안정화되도록
    debug_logger.info("⏳ 대시보드 안정화 대기 중...")
    time.sleep(5)

    try:
        # 전체 진단 실행
        debug_logger.info("🔍 DEBUG: 전체 진단 시작...")
        sensor_monitor.full_diagnostic()

        # 추가 분석
        debug_logger.info("🔍 DEBUG: ===== 추가 분석 =====")

        # 대시보드 모듈 직접 분석 (지연 로드)
        from dashboard.app import app
        from dashboard.data_manager import data_manager

        # 현재 상태 분석
        sensors = data_manager.get_sensors_list()
        debug_logger.info(f"📊 DATA: 현재 활성 센서: {sensors}")

        # 콜백 분석
        if hasattr(app, "callback_map"):
            callbacks = app.callback_map
            debug_logger.info(f"📞 CALLBACK_REG: 총 콜백 수: {len(callbacks)}")

            # 개별 센서 관련 콜백 찾기
            sensor_callbacks = []
            for callback_id in callbacks:
                if any(sensor_id in str(callback_id) for sensor_id in sensors):
                    sensor_callbacks.append(callback_id)

            debug_logger.info(
                f"📞 CALLBACK_REG: 센서 관련 콜백 수: {len(sensor_callbacks)}"
            )

            for cb in sensor_callbacks:
                debug_logger.info(f"📞 CALLBACK_REG: 콜백: {cb}")
        else:
            debug_logger.error("❌ ERROR: callback_map이 존재하지 않습니다!")

    except Exception as e:
        debug_logger.error(f"❌ ERROR: 진단 실패 - {e}")
        import traceback

        debug_logger.error(f"❌ ERROR: 상세 오류 - {traceback.format_exc()}")


if __name__ == "__main__":
    main()

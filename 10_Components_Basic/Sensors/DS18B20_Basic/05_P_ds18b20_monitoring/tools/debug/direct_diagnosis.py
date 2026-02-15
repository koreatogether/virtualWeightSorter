#!/usr/bin/env python3
"""
직접 진단 스크립트 - 브라우저 없이도 콜백 등록 상태 확인
"""

import sys
import time

from tools.debug.debug_monitor import debug_logger


def main():
    debug_logger.info("🔍 DEBUG: 직접 진단 스크립트 시작")

    # 5초 기다려서 대시보드 안정화
    debug_logger.info("⏳ 대시보드 안정화 대기 중...")
    time.sleep(5)

    try:
        # Ensure project src is on path and import dashboard modules lazily
        sys.path.insert(0, "src/python")
        from dashboard.app import app, update_sensor_sections
        from dashboard.data_manager import data_manager

        debug_logger.info("🔍 DEBUG: 현재 데이터 매니저 상태 확인")

        # 데이터 매니저에서 직접 센서 상태 확인
        sensors = data_manager.get_sensors_list()
        debug_logger.info(f"📊 DATA: 현재 등록된 센서: {sensors}")
        debug_logger.info(
            f"📊 DATA: display_id_to_addr: {data_manager.display_id_to_addr}"
        )
        debug_logger.info(
            f"📊 DATA: sensor_data 키들: {list(data_manager.sensor_data.keys())}"
        )

        # 글로벌 변수들은 함수 내부에서만 존재하므로 직접 함수를 호출해서 확인
        debug_logger.info(
            "🔍 DEBUG: 글로벌 변수는 함수 내부에서만 존재 - 함수 호출로 확인"
        )

        # update_sensor_sections 함수를 직접 호출하여 콜백 등록 과정 추적
        debug_logger.info("🔍 DEBUG: update_sensor_sections 함수 직접 호출")

        # 가짜 interval count와 현재 섹션으로 함수 호출
        result = update_sensor_sections(n=1, current_sections=None)

        debug_logger.info(f"🔍 DEBUG: 함수 결과: {type(result)}")

        # 콜백 등록 상태 확인 (app.callback_map을 통해)
        debug_logger.info("🔍 DEBUG: ===== 콜백 등록 상태 확인 =====")
        if hasattr(app, "callback_map"):
            callbacks = app.callback_map
            debug_logger.info(f"📞 CALLBACK_REG: 총 콜백 수: {len(callbacks)}")

            # 개별 센서 관련 콜백 찾기
            sensor_callbacks = []
            for callback_id in callbacks:
                for sensor_id in sensors:
                    if f"btn-{sensor_id}" in str(
                        callback_id
                    ) or f"sensor-{sensor_id}" in str(callback_id):
                        sensor_callbacks.append((sensor_id, callback_id))

            debug_logger.info(
                f"📞 CALLBACK_REG: 센서 관련 콜백 수: {len(sensor_callbacks)}"
            )

            for sensor_id, cb in sensor_callbacks:
                debug_logger.info(f"📞 CALLBACK_REG: 센서 {sensor_id} 콜백: {cb}")
        else:
            debug_logger.error("❌ ERROR: callback_map이 존재하지 않습니다!")

        # 최종 진단
        debug_logger.info("🔍 DEBUG: ===== 최종 진단 =====")
        for sensor_id in sensors:
            # 각 센서별 예상 콜백 ID들
            expected_callbacks = [
                f"btn-{sensor_id}",
                f"sensor-{sensor_id}-value",
                f"graph-{sensor_id}",
                f"log-{sensor_id}",
            ]

            registered_count = 0
            if hasattr(app, "callback_map"):
                for expected_cb in expected_callbacks:
                    for callback_id in app.callback_map:
                        if expected_cb in str(callback_id):
                            registered_count += 1
                            break

            debug_logger.info(
                f"센서 {sensor_id}: {registered_count}/{len(expected_callbacks)} 콜백 등록됨"
            )

    except Exception as e:
        debug_logger.error(f"❌ ERROR: 직접 진단 실패 - {e}")
        import traceback

        debug_logger.error(f"❌ ERROR: 상세 오류:\n{traceback.format_exc()}")


if __name__ == "__main__":
    main()

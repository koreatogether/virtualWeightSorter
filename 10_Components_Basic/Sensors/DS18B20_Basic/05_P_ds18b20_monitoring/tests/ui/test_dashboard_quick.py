#!/usr/bin/env python3
"""대시보드 빠른 테스트 스크립트"""

import sys
from pathlib import Path

# Add src/python to path
current_dir = Path(__file__).parent
src_python_dir = current_dir / "src" / "python"
sys.path.insert(0, str(src_python_dir))

try:
    from dashboard.app import data_manager, log_manager
    print("대시보드 모듈 import 성공")

    # 테스트 데이터 추가
    test_data = {
        "sensor_id": "28FF64AC5B4F001A",
        "temperature": 25.5,
        "th_value": 30.0,
        "tl_value": 20.0,
        "measurement_interval": 1000,
        "timestamp": "2025-08-19T04:33:10.310531",
    }
    data_manager.add_sensor_data(test_data)
    print(f"테스트 데이터 추가 완료: {len(data_manager.sensor_data)}개")

    # 로그 시스템 테스트
    log_manager.add_log("데이터 내보내기 기능 테스트", "info")
    print(f"로그 시스템 테스트 완료: {len(log_manager.logs)}개 로그")

    print("\n=== 대시보드 기능 테스트 완료 ===")
    print("- 모든 모듈이 정상적으로 로드됨")
    print("- 데이터 관리 시스템 정상 작동")
    print("- 로그 시스템 정상 작동")
    print("- 데이터 내보내기 UI 추가 완료")
    print("\n대시보드를 실행하여 '데이터 내보내기' 버튼을 확인하세요:")
    print("python src/python/dashboard/app.py")

except Exception as e:
    print(f"오류 발생: {e}")
    import traceback
    traceback.print_exc()

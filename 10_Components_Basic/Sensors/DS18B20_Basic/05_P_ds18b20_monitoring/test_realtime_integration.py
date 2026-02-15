#!/usr/bin/env python3
"""실시간 센서 데이터 연동 테스트"""

import sys
import time
from pathlib import Path

# 프로젝트 루트를 Python 경로에 추가
project_root = Path(__file__).parent
sys.path.insert(0, str(project_root))

from src.python.services.port_manager import port_manager
from src.python.services.sensor_data_manager import sensor_manager


def test_port_scanning():
    """포트 스캔 테스트"""
    print("🔍 COM 포트 스캔 테스트...")
    ports = port_manager.scan_ports()
    print(f"   발견된 포트: {ports}")
    return len(ports) > 0


def test_sensor_data_manager():
    """센서 데이터 매니저 테스트"""
    print("📊 센서 데이터 매니저 테스트...")
    
    # 테스트 센서 데이터
    test_data = {
        "type": "sensor_data",
        "temperature": 25.3,
        "sensor_addr": "285882840000000E",
        "sensor_id": "01",
        "user_sensor_id": 1234,
        "th_value": 30,
        "tl_value": 15,
        "measurement_interval": 1000,
        "timestamp": int(time.time() * 1000)
    }
    
    # 센서 데이터 추가
    sensor_manager.add_sensor_data(test_data)
    
    # 정렬된 센서 확인
    sorted_sensors = sensor_manager.get_sorted_sensors()
    print(f"   정렬된 센서 개수: {len(sorted_sensors)}")
    
    if sorted_sensors:
        sensor = sorted_sensors[0]
        print(f"   첫 번째 센서: ID={sensor.sensor_id}, 온도={sensor.temperature}°C")
    
    # 표시 정보 확인
    display_info = sensor_manager.get_display_info()
    print(f"   표시 정보: {len(display_info)}개 센서")
    
    # 요약 정보 확인
    summary = sensor_manager.get_summary()
    print(f"   요약: {summary}")
    
    return len(sorted_sensors) > 0


def test_connection_status():
    """연결 상태 테스트"""
    print("🔌 연결 상태 테스트...")
    
    status = port_manager.get_status()
    print(f"   연결 상태: {status.is_connected}")
    print(f"   현재 포트: {status.port}")
    print(f"   마지막 오류: {status.last_error}")
    
    return True


def main():
    """메인 테스트 함수"""
    print("🚀 실시간 센서 데이터 연동 테스트 시작")
    print("=" * 50)
    
    tests = [
        ("포트 스캔", test_port_scanning),
        ("센서 데이터 매니저", test_sensor_data_manager),
        ("연결 상태", test_connection_status),
    ]
    
    results = []
    
    for test_name, test_func in tests:
        try:
            result = test_func()
            results.append((test_name, "✅ 통과" if result else "❌ 실패"))
            print()
        except Exception as e:
            results.append((test_name, f"❌ 오류: {e}"))
            print(f"   오류 발생: {e}")
            print()
    
    print("=" * 50)
    print("📋 테스트 결과 요약:")
    for test_name, result in results:
        print(f"   {test_name}: {result}")
    
    # 전체 결과
    passed = sum(1 for _, result in results if "✅" in result)
    total = len(results)
    
    print(f"\n🎯 전체 결과: {passed}/{total} 테스트 통과")
    
    if passed == total:
        print("🎉 모든 테스트가 성공적으로 완료되었습니다!")
        print("\n💡 다음 단계:")
        print("   1. 웹 브라우저에서 http://127.0.0.1:8050 접속")
        print("   2. COM 포트 선택 후 Arduino 연결")
        print("   3. 실시간 센서 데이터 확인")
    else:
        print("⚠️ 일부 테스트가 실패했습니다. 로그를 확인하세요.")
    
    return passed == total


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
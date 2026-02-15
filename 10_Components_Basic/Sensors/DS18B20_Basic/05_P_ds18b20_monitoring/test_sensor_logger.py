#!/usr/bin/env python3
"""
센서 로거 테스트 스크립트
실제 센서 데이터로 로거 기능을 테스트합니다.
"""

import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager
from src.python.services.sensor_logger import sensor_logger

def test_sensor_logger():
    """센서 로거 테스트"""
    print("🧪 센서 로거 테스트 시작")
    print("=" * 50)
    
    # 로그 간격을 5개로 설정 (테스트용)
    sensor_logger.set_log_interval(5)
    
    # Arduino 연결 시도
    print("🔌 Arduino 연결 중...")
    success = port_manager.auto_connect()
    
    if not success:
        print("❌ Arduino 연결 실패!")
        return
    
    print(f"✅ Arduino 연결 성공: {port_manager.get_current_port()}")
    
    # 30초간 센서 데이터 수집
    print("\n📊 30초간 센서 데이터 수집 및 로그 테스트...")
    print("(5개 측정마다 로그가 출력됩니다)")
    print("-" * 50)
    
    start_time = time.time()
    data_count = 0
    
    try:
        while time.time() - start_time < 30:
            # 센서 데이터 읽기
            data = port_manager.read_sensor_data(timeout=2.0)
            
            if data:
                data_count += 1
                print(f"📥 [{data_count:2d}] 센서 {data.get('sensor_id', '00')}: "
                      f"{data.get('temperature', 0):.1f}°C "
                      f"(주소: {data.get('sensor_addr', 'UNKNOWN')[-8:]})")
                
                # 센서 로거에 데이터 전달
                sensor_logger.update_sensor_data(data)
            else:
                print("⏳ 데이터 대기 중...")
            
            time.sleep(0.8)
    
    except KeyboardInterrupt:
        print("\n⏹️ 사용자가 테스트를 중단했습니다.")
    
    # 최종 요약 출력
    print("\n" + "=" * 50)
    print("📊 테스트 완료 - 시스템 요약")
    sensor_logger.print_system_summary()
    
    # 개별 센서 상세 정보
    print("📋 개별 센서 상세 정보:")
    for addr, stats in sensor_logger.sensors.items():
        print(f"\n🔍 센서 {stats.sensor_id} ({addr[-8:]}):")
        print(f"   📈 총 측정: {stats.total_measurements}회")
        print(f"   ✅ 성공: {stats.successful_measurements}회 ({stats.get_success_rate():.1f}%)")
        print(f"   ❌ 실패: {stats.failed_measurements}회")
        print(f"   🌡️ 온도 범위: {stats.min_temperature:.1f}°C ~ {stats.max_temperature:.1f}°C")
        print(f"   📊 평균 온도: {stats.get_average_temperature():.1f}°C")
        print(f"   📡 연속 성공: {stats.consecutive_successes}회 (최대: {stats.max_consecutive_successes}회)")
        
        if stats.connection_failures > 0:
            print(f"   ⚠️ 통신 장애: {stats.connection_failures}회 (마지막: {stats.last_failure_time})")
        
        change_rate = stats.get_temperature_change_rate()
        if abs(change_rate) > 0.1:
            direction = "상승" if change_rate > 0 else "하강"
            print(f"   📈 온도 변화: {direction} {abs(change_rate):.2f}°C/분")

def test_manual_data():
    """수동 데이터로 로거 테스트"""
    print("\n🧪 수동 데이터 테스트")
    print("=" * 30)
    
    # 테스트 데이터
    test_data = [
        {"sensor_addr": "285882840000000E", "sensor_id": "01", "temperature": 25.5, "th_value": 30.0, "tl_value": 15.0},
        {"sensor_addr": "285882840000000E", "sensor_id": "01", "temperature": 25.7, "th_value": 30.0, "tl_value": 15.0},
        {"sensor_addr": "285882840000000E", "sensor_id": "01", "temperature": 25.9, "th_value": 30.0, "tl_value": 15.0},
        {"sensor_addr": "285882840000000E", "sensor_id": "01", "temperature": 26.1, "th_value": 30.0, "tl_value": 15.0},
        {"sensor_addr": "285882840000000E", "sensor_id": "01", "temperature": 26.3, "th_value": 30.0, "tl_value": 15.0},  # 5번째 - 로그 출력
        {"sensor_addr": "285C82850000005D", "sensor_id": "02", "temperature": 24.2, "th_value": 30.0, "tl_value": 15.0},
        {"sensor_addr": "285C82850000005D", "sensor_id": "02", "temperature": -127.0, "th_value": 30.0, "tl_value": 15.0},  # 오류 데이터
        {"sensor_addr": "285C82850000005D", "sensor_id": "02", "temperature": 24.5, "th_value": 30.0, "tl_value": 15.0},
        {"sensor_addr": "285C82850000005D", "sensor_id": "02", "temperature": 31.5, "th_value": 30.0, "tl_value": 15.0},  # 임계값 초과
        {"sensor_addr": "285C82850000005D", "sensor_id": "02", "temperature": 24.8, "th_value": 30.0, "tl_value": 15.0},  # 5번째 - 로그 출력
    ]
    
    # 새로운 로거 인스턴스 생성
    from src.python.services.sensor_logger import SensorLogger
    test_logger = SensorLogger()
    test_logger.set_log_interval(5)
    
    print("📥 테스트 데이터 입력 중...")
    for i, data in enumerate(test_data):
        print(f"[{i+1:2d}] 센서 {data['sensor_id']}: {data['temperature']:.1f}°C")
        test_logger.update_sensor_data(data)
        time.sleep(0.2)
    
    print("\n📊 테스트 요약:")
    test_logger.print_system_summary()

if __name__ == "__main__":
    print("🚀 센서 로거 테스트 도구")
    print("1. 실제 센서 데이터 테스트")
    print("2. 수동 데이터 테스트")
    
    choice = input("\n선택하세요 (1 또는 2): ").strip()
    
    if choice == "1":
        test_sensor_logger()
    elif choice == "2":
        test_manual_data()
    else:
        print("❌ 잘못된 선택입니다.")
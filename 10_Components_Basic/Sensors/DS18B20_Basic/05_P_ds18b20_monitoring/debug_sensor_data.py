#!/usr/bin/env python3
"""Arduino 센서 데이터 실시간 모니터링 및 디버깅"""

import sys
import time
import json
from pathlib import Path

# 프로젝트 루트를 Python 경로에 추가
project_root = Path(__file__).parent
sys.path.insert(0, str(project_root))

from src.python.services.port_manager import port_manager
from src.python.services.sensor_data_manager import sensor_manager


def scan_and_connect():
    """포트 스캔 및 연결"""
    print("🔍 COM 포트 스캔 중...")
    ports = port_manager.scan_ports()
    
    if not ports:
        print("❌ 사용 가능한 COM 포트가 없습니다.")
        return False
    
    print(f"📡 발견된 포트: {ports}")
    
    # 첫 번째 포트로 연결 시도
    target_port = ports[0]
    print(f"🔌 {target_port}에 연결 시도 중...")
    
    success = port_manager.connect(target_port)
    
    if success:
        print(f"✅ {target_port} 연결 성공!")
        
        # 통신 테스트
        comm_test = port_manager.test_communication()
        if comm_test:
            print("✅ Arduino 통신 테스트 성공!")
            return True
        else:
            print("❌ Arduino 통신 테스트 실패")
            return False
    else:
        print(f"❌ {target_port} 연결 실패")
        return False


def monitor_sensor_data(duration=30):
    """센서 데이터 실시간 모니터링"""
    print(f"📊 {duration}초 동안 센서 데이터 모니터링 시작...")
    print("=" * 80)
    
    start_time = time.time()
    data_count = 0
    unique_sensors = set()
    
    while time.time() - start_time < duration:
        try:
            # Arduino에서 센서 데이터 읽기
            sensor_data = port_manager.read_sensor_data(timeout=2.0)
            
            if sensor_data:
                data_count += 1
                
                # 센서 정보 추출
                sensor_id = sensor_data.get("sensor_id", "??")
                sensor_addr = sensor_data.get("sensor_addr", "")
                temperature = sensor_data.get("temperature", 0.0)
                user_id = sensor_data.get("user_sensor_id", 0)
                
                unique_sensors.add((sensor_id, sensor_addr))
                
                # 실시간 출력
                timestamp = time.strftime("%H:%M:%S")
                print(f"[{timestamp}] ID:{sensor_id} | 온도:{temperature:6.1f}°C | 주소:{sensor_addr} | 사용자ID:{user_id}")
                
                # 센서 데이터 매니저에 추가
                sensor_manager.add_sensor_data(sensor_data)
                
                # JSON 원본 데이터도 출력 (처음 5개만)
                if data_count <= 5:
                    print(f"    📄 원본 JSON: {json.dumps(sensor_data, ensure_ascii=False)}")
                    print()
            
            time.sleep(0.5)  # 0.5초 대기
            
        except KeyboardInterrupt:
            print("\n⏹️ 사용자가 모니터링을 중단했습니다.")
            break
        except Exception as e:
            print(f"❌ 오류 발생: {e}")
            time.sleep(1)
    
    print("=" * 80)
    print(f"📈 모니터링 완료: {data_count}개 데이터 수신")
    print(f"🔢 고유 센서 개수: {len(unique_sensors)}")
    
    return unique_sensors


def analyze_sensor_priority():
    """센서 우선순위 분석"""
    print("🔍 센서 우선순위 분석...")
    
    # 현재 센서 매니저의 센서들 확인
    sorted_sensors = sensor_manager.get_sorted_sensors()
    
    if not sorted_sensors:
        print("❌ 센서 데이터가 없습니다.")
        return
    
    print(f"📊 총 {len(sorted_sensors)}개 센서 발견")
    print()
    
    # 그룹별 분류
    groups = sensor_manager.get_sensors_by_group()
    
    for group_id, group_sensors in groups.items():
        if not group_sensors:
            continue
            
        group_name = {1: "설정됨 (01-08)", 2: "미설정 (00)", 3: "오류/기타"}.get(group_id, "알수없음")
        print(f"🏷️ {group_name} 그룹: {len(group_sensors)}개")
        
        for i, sensor in enumerate(group_sensors, 1):
            print(f"   {i}. ID:{sensor.sensor_id} | 온도:{sensor.temperature:.1f}°C | 주소:{sensor.sensor_addr[:16]}")
        print()
    
    # 표시 정보 확인
    display_info = sensor_manager.get_display_info(max_sensors=8)
    
    print("🖥️ 대시보드 표시 순서:")
    for i, info in enumerate(display_info, 1):
        print(f"   {i}. ID:{info['sensor_id']} | 온도:{info['temperature']:.1f}°C | 그룹:{info['group_name']}")
    
    if display_info:
        primary = display_info[0]
        print(f"\n🎯 메인 카드 표시 센서: ID={primary['sensor_id']}, 온도={primary['temperature']:.1f}°C")
        
        if primary['sensor_id'] == '00':
            print("⚠️ 주의: 메인 카드에 '00' 센서가 표시되고 있습니다!")
            print("   - 01~08 ID를 가진 센서가 있는지 확인하세요.")
            print("   - 센서 우선순위 로직을 점검하세요.")


def main():
    """메인 함수"""
    print("🚀 Arduino 센서 데이터 디버깅 도구")
    print("=" * 50)
    
    # 1. 연결
    if not scan_and_connect():
        print("❌ Arduino 연결에 실패했습니다.")
        return False
    
    print()
    
    # 2. 센서 데이터 모니터링
    unique_sensors = monitor_sensor_data(duration=15)  # 15초 모니터링
    
    print()
    
    # 3. 고유 센서 목록 출력
    print("📋 발견된 고유 센서 목록:")
    for sensor_id, sensor_addr in sorted(unique_sensors):
        priority_group = 1 if sensor_id in ['01', '02', '03', '04', '05', '06', '07', '08'] else (2 if sensor_id == '00' else 3)
        group_name = {1: "설정됨", 2: "미설정", 3: "오류"}.get(priority_group)
        print(f"   ID:{sensor_id} | 주소:{sensor_addr} | 그룹:{group_name}")
    
    print()
    
    # 4. 우선순위 분석
    analyze_sensor_priority()
    
    # 5. 연결 해제
    port_manager.disconnect()
    print("\n🔌 Arduino 연결 해제 완료")
    
    return True


if __name__ == "__main__":
    try:
        success = main()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n⏹️ 프로그램이 중단되었습니다.")
        port_manager.disconnect()
        sys.exit(0)
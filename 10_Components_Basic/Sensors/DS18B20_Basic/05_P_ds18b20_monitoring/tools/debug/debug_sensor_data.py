#!/usr/bin/env python3
"""Arduino 센서 데이터 실시간 모니터링 및 디버그"""

import sys
import time
import json
from pathlib import Path

# 프로젝트 루트를 Python 경로에 추가
project_root = Path(__file__).parents[2]
sys.path.insert(0, str(project_root))

from src.python.services.port_manager import port_manager
from src.python.services.sensor_data_manager import sensor_manager


def scan_and_connect():
    """
포트 스캔 및 연결
"""
    print("🔍 COM 포트 스캔 중...")
    ports = port_manager.scan_ports()
    
    if not ports:
        print("❌ 사용 가능한 COM 포트가 없습니다.")
        return False
    
    print(f"🔊 발견된 포트: {ports}")
    
    target_port = ports[0]
    print(f"🔌 {target_port}에 연결 시도 중...")
    
    success = port_manager.connect(target_port)
    
    if success:
        print(f"✅ {target_port} 연결 성공!")
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
    """
센서 데이터 모니터링
"""
    print(f"📊 {duration}초 동안 센서 데이터 모니터링 시작...")
    print("=" * 80)
    
    start_time = time.time()
    data_count = 0
    unique_sensors = set()
    
    while time.time() - start_time < duration:
        try:
            sensor_data = port_manager.read_sensor_data(timeout=2.0)
            
            if sensor_data:
                data_count += 1
                
                sensor_id = sensor_data.get("sensor_id", "??")
                sensor_addr = sensor_data.get("sensor_addr", "")
                temperature = sensor_data.get("temperature", 0.0)
                user_id = sensor_data.get("user_sensor_id", 0)
                
                unique_sensors.add((sensor_id, sensor_addr))
                
                timestamp = time.strftime("%H:%M:%S")
                print(f"[{timestamp}] ID:{sensor_id} | 온도:{temperature:6.1f}°C | 주소:{sensor_addr} | 사용자ID:{user_id}")
                
                sensor_manager.add_sensor_data(sensor_data)
                
                if data_count <= 5:
                    print(f"    📄 원본 JSON: {json.dumps(sensor_data, ensure_ascii=False)}")
                    print()
            
            time.sleep(0.5)
            
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


def main():
    if not scan_and_connect():
        print("❌ Arduino 연결에 실패했습니다.")
        return False
    
    unique_sensors = monitor_sensor_data(duration=15)
    
    print()
    print("📋 발견된 센서 목록:")
    for sensor_id, sensor_addr in sorted(unique_sensors):
        print(f"   ID:{sensor_id} | 주소:{sensor_addr}")
    
    port_manager.disconnect()
    print("\n🔌 Arduino 연결 해제 완료")
    return True


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        port_manager.disconnect()
        print("\n프로그램 종료")

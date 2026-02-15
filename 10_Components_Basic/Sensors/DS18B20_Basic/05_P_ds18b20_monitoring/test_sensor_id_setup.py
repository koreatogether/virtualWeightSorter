#!/usr/bin/env python3
"""센서 ID 설정 테스트 스크립트"""

import sys
import time
import json
from pathlib import Path

# 프로젝트 루트를 Python 경로에 추가
project_root = Path(__file__).parent
sys.path.insert(0, str(project_root))

from src.python.services.port_manager import port_manager


def connect_arduino():
    """Arduino 연결"""
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
        return True
    else:
        print(f"❌ {target_port} 연결 실패")
        return False


def send_command(command_dict):
    """Arduino에 명령 전송"""
    command_json = json.dumps(command_dict)
    print(f"📤 명령 전송: {command_json}")
    
    success = port_manager.send_data(command_json + "\n")
    if not success:
        print("❌ 명령 전송 실패")
        return None
    
    # 응답 대기
    for i in range(10):  # 최대 10초 대기
        response_data = port_manager.read_data(timeout=1.0)
        if response_data:
            try:
                response = json.loads(response_data)
                print(f"📥 응답 수신: {json.dumps(response, ensure_ascii=False)}")
                return response
            except json.JSONDecodeError:
                print(f"📥 원본 응답: {response_data}")
                continue
        time.sleep(0.1)
    
    print("⏱️ 응답 시간 초과")
    return None


def list_sensors():
    """센서 목록 조회"""
    print("\n🔍 센서 목록 조회...")
    command = {
        "type": "command",
        "command": "list_sensors"
    }
    
    response = send_command(command)
    if response and response.get("status") == "success":
        addresses = response.get("addresses", "[]")
        count = response.get("count", 0)
        print(f"✅ {count}개 센서 발견")
        
        # 주소 파싱
        try:
            addr_list = json.loads(addresses)
            for i, addr in enumerate(addr_list):
                print(f"   센서 {i}: {addr}")
            return addr_list
        except:
            print(f"   주소 목록: {addresses}")
            return []
    else:
        print("❌ 센서 목록 조회 실패")
        return []


def set_sensor_id(sensor_index, new_id):
    """특정 센서에 ID 설정"""
    print(f"\n🏷️ 센서 {sensor_index}에 ID {new_id:02d} 설정 중...")
    
    # 1. 센서 선택
    select_command = {
        "type": "command",
        "command": "select_sensor_index",
        "index": sensor_index
    }
    
    response = send_command(select_command)
    if not response or response.get("status") != "success":
        print(f"❌ 센서 {sensor_index} 선택 실패")
        return False
    
    time.sleep(0.5)
    
    # 2. ID 설정
    set_command = {
        "type": "command",
        "command": "set_sensor_data",
        "new_value": new_id
    }
    
    response = send_command(set_command)
    if response and response.get("status") == "success":
        print(f"✅ 센서 {sensor_index}에 ID {new_id:02d} 설정 완료!")
        return True
    else:
        print(f"❌ 센서 {sensor_index} ID 설정 실패")
        return False


def verify_sensor_data():
    """센서 데이터 확인"""
    print("\n📊 센서 데이터 확인 중...")
    
    for i in range(10):  # 10번 시도
        sensor_data = port_manager.read_sensor_data(timeout=2.0)
        if sensor_data:
            sensor_id = sensor_data.get("sensor_id", "??")
            sensor_addr = sensor_data.get("sensor_addr", "")
            temperature = sensor_data.get("temperature", 0.0)
            user_id = sensor_data.get("user_sensor_id", 0)
            
            print(f"📡 ID:{sensor_id} | 온도:{temperature:.1f}°C | 주소:{sensor_addr} | 사용자ID:{user_id}")
            
            if sensor_id != "00":
                print(f"✅ 센서 ID가 {sensor_id}로 설정되었습니다!")
                return True
        
        time.sleep(1)
    
    print("❌ 여전히 모든 센서가 ID 00입니다.")
    return False


def main():
    """메인 함수"""
    print("🚀 센서 ID 설정 테스트")
    print("=" * 50)
    
    # 1. Arduino 연결
    if not connect_arduino():
        return False
    
    # 2. 센서 목록 조회
    sensor_addresses = list_sensors()
    if not sensor_addresses:
        return False
    
    # 3. 첫 번째 센서에 ID 01 설정
    print(f"\n🎯 첫 번째 센서에 ID 01을 설정해보겠습니다.")
    success = set_sensor_id(0, 1)
    
    if success:
        # 4. 설정 결과 확인
        time.sleep(2)
        verify_sensor_data()
    
    # 5. 연결 해제
    port_manager.disconnect()
    print("\n🔌 Arduino 연결 해제 완료")
    
    return success


if __name__ == "__main__":
    try:
        success = main()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n⏹️ 프로그램이 중단되었습니다.")
        port_manager.disconnect()
        sys.exit(0)
#!/usr/bin/env python3
"""센서 명령 테스트 (더 안정적인 버전)"""

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
    
    target_port = ports[0]
    print(f"🔌 {target_port}에 연결 시도 중...")
    
    success = port_manager.connect(target_port)
    if success:
        print(f"✅ {target_port} 연결 성공!")
        return True
    else:
        print(f"❌ {target_port} 연결 실패")
        return False


def send_command_and_wait(command_dict, wait_time=3):
    """명령 전송 후 응답 대기"""
    command_json = json.dumps(command_dict)
    print(f"📤 명령 전송: {command_json}")
    
    # 기존 버퍼 클리어
    time.sleep(0.5)
    while port_manager._serial_connection and port_manager._serial_connection.in_waiting > 0:
        port_manager._serial_connection.read_all()
    
    # 명령 전송
    success = port_manager.send_data(command_json + "\n")
    if not success:
        print("❌ 명령 전송 실패")
        return None
    
    print(f"⏳ {wait_time}초 대기 중...")
    time.sleep(wait_time)
    
    # 모든 응답 수집
    responses = []
    for i in range(20):  # 최대 20번 시도
        response_data = port_manager.read_data(timeout=0.1)
        if response_data:
            try:
                response = json.loads(response_data)
                responses.append(response)
                print(f"📥 응답 {len(responses)}: {json.dumps(response, ensure_ascii=False)}")
                
                # response 타입이면 명령 응답으로 간주
                if response.get("type") == "response":
                    return response
            except json.JSONDecodeError:
                print(f"📥 원본 데이터: {response_data}")
        else:
            break
    
    print(f"📊 총 {len(responses)}개 응답 수신")
    
    # response 타입 찾기
    for resp in responses:
        if resp.get("type") == "response":
            return resp
    
    return responses[0] if responses else None


def test_ping():
    """핑 테스트"""
    print("\n🏓 핑 테스트...")
    command = {"type": "ping"}
    
    response = send_command_and_wait(command, wait_time=2)
    if response and response.get("status") == "success" and response.get("message") == "pong":
        print("✅ 핑 테스트 성공!")
        return True
    else:
        print("❌ 핑 테스트 실패")
        return False


def get_system_status():
    """시스템 상태 조회"""
    print("\n📊 시스템 상태 조회...")
    command = {
        "type": "command",
        "command": "get_status"
    }
    
    response = send_command_and_wait(command, wait_time=2)
    if response and response.get("type") == "system_status":
        print("✅ 시스템 상태 수신!")
        print(f"   센서 연결: {response.get('sensor_connected')}")
        print(f"   센서 주소: {response.get('sensor_addr')}")
        print(f"   사용자 ID: {response.get('user_sensor_id')}")
        print(f"   마지막 온도: {response.get('last_temperature')}°C")
        return response
    else:
        print("❌ 시스템 상태 조회 실패")
        return None


def set_first_sensor_id():
    """첫 번째 센서에 ID 1 설정"""
    print("\n🏷️ 첫 번째 센서에 ID 1 설정...")
    
    # 센서 인덱스 0 선택
    select_command = {
        "type": "command",
        "command": "select_sensor_index",
        "index": 0
    }
    
    response = send_command_and_wait(select_command, wait_time=2)
    if not response or response.get("status") != "success":
        print("❌ 센서 선택 실패")
        return False
    
    print("✅ 센서 0 선택 완료")
    
    # ID 1 설정
    set_command = {
        "type": "command",
        "command": "set_sensor_data",
        "new_value": 1
    }
    
    response = send_command_and_wait(set_command, wait_time=3)
    if response and response.get("status") == "success":
        print("✅ 센서 ID 1 설정 완료!")
        return True
    else:
        print("❌ 센서 ID 설정 실패")
        print(f"   응답: {response}")
        return False


def monitor_sensor_data_after_setup():
    """설정 후 센서 데이터 모니터링"""
    print("\n📊 설정 후 센서 데이터 확인...")
    
    found_id_01 = False
    
    for i in range(15):  # 15초 모니터링
        sensor_data = port_manager.read_sensor_data(timeout=1.0)
        if sensor_data:
            sensor_id = sensor_data.get("sensor_id", "??")
            sensor_addr = sensor_data.get("sensor_addr", "")
            temperature = sensor_data.get("temperature", 0.0)
            user_id = sensor_data.get("user_sensor_id", 0)
            
            timestamp = time.strftime("%H:%M:%S")
            print(f"[{timestamp}] ID:{sensor_id} | 온도:{temperature:.1f}°C | 주소:{sensor_addr[:16]} | 사용자ID:{user_id}")
            
            if sensor_id == "01":
                found_id_01 = True
                print("🎉 ID 01 센서 발견!")
        
        time.sleep(1)
    
    if found_id_01:
        print("✅ 센서 ID 설정이 성공적으로 적용되었습니다!")
        return True
    else:
        print("❌ 여전히 ID 01 센서가 보이지 않습니다.")
        return False


def main():
    """메인 함수"""
    print("🚀 센서 명령 테스트")
    print("=" * 50)
    
    # 1. Arduino 연결
    if not connect_arduino():
        return False
    
    # 2. 핑 테스트
    if not test_ping():
        return False
    
    # 3. 시스템 상태 확인
    status = get_system_status()
    if not status:
        return False
    
    # 4. 센서 ID 설정
    if set_first_sensor_id():
        # 5. 설정 후 모니터링
        monitor_sensor_data_after_setup()
    
    # 6. 연결 해제
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
#!/usr/bin/env python3
"""센서 ID 수정 테스트"""

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


def send_command_simple(command_dict):
    """간단한 명령 전송"""
    command_json = json.dumps(command_dict)
    print(f"📤 명령: {command_json}")
    
    success = port_manager.send_data(command_json + "\n")
    if not success:
        print("❌ 전송 실패")
        return None
    
    # 응답 대기
    time.sleep(2)
    
    responses = []
    for i in range(10):
        response_data = port_manager.read_data(timeout=0.2)
        if response_data:
            try:
                response = json.loads(response_data)
                responses.append(response)
                if response.get("type") == "response":
                    print(f"📥 응답: {response.get('status')} - {response.get('message')}")
                    return response
            except:
                pass
    
    print(f"📊 {len(responses)}개 응답 수신 (response 타입 없음)")
    return None


def set_sensor_id_step_by_step():
    """단계별 센서 ID 설정"""
    print("\n🎯 단계별 센서 ID 설정 시작")
    
    # 1단계: 센서 0 선택
    print("\n1️⃣ 센서 0 선택...")
    select_cmd = {
        "type": "command",
        "command": "select_sensor_index",
        "index": 0
    }
    
    response = send_command_simple(select_cmd)
    if not response or response.get("status") != "success":
        print("❌ 센서 선택 실패")
        return False
    
    print("✅ 센서 0 선택 완료")
    
    # 2단계: ID 1 설정
    print("\n2️⃣ 센서 ID 1 설정...")
    set_cmd = {
        "type": "command",
        "command": "set_sensor_data",
        "new_value": 1
    }
    
    response = send_command_simple(set_cmd)
    if response and response.get("status") == "success":
        print("✅ 센서 ID 1 설정 완료!")
        print(f"   메시지: {response.get('message')}")
        return True
    else:
        print("❌ 센서 ID 설정 실패")
        if response:
            print(f"   응답: {response}")
        return False


def monitor_changes():
    """변경사항 모니터링"""
    print("\n📊 변경사항 모니터링 (10초)...")
    
    found_01 = False
    
    for i in range(20):  # 10초 모니터링
        sensor_data = port_manager.read_sensor_data(timeout=0.5)
        if sensor_data:
            sensor_id = sensor_data.get("sensor_id", "??")
            sensor_addr = sensor_data.get("sensor_addr", "")
            temperature = sensor_data.get("temperature", 0.0)
            user_id = sensor_data.get("user_sensor_id", 0)
            
            timestamp = time.strftime("%H:%M:%S")
            print(f"[{timestamp}] ID:{sensor_id} | 온도:{temperature:.1f}°C | 주소:{sensor_addr[:16]} | 사용자ID:{user_id}")
            
            if sensor_id == "01":
                found_01 = True
                print("🎉 ID 01 센서 발견!")
                break
    
    return found_01


def main():
    """메인 함수"""
    print("🚀 센서 ID 수정 테스트")
    print("=" * 50)
    
    # 1. Arduino 연결
    if not connect_arduino():
        return False
    
    # 2. 센서 ID 설정
    if set_sensor_id_step_by_step():
        # 3. 변경사항 확인
        success = monitor_changes()
        
        if success:
            print("\n🎉 센서 ID 설정이 성공적으로 완료되었습니다!")
        else:
            print("\n❌ 센서 ID가 여전히 변경되지 않았습니다.")
    
    # 4. 연결 해제
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
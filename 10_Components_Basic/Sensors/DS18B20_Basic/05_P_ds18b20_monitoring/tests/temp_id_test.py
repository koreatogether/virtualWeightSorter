#!/usr/bin/env python3
"""임시 센서 ID 변경 테스트 스크립트 (테스트 폴더로 이동)"""

import sys
import os
import json
import time

# 프로젝트 루트를 Python 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

def test_sensor_id_change():
    """센서 ID 변경 테스트"""
    try:
        from src.python.services.port_manager import port_manager
        
        if not port_manager.is_connected():
            print("Arduino가 연결되지 않았습니다.")
            return
        
        print("Arduino 센서 ID 변경 테스트 시작")
        print("=" * 50)
        
        # 1. 현재 센서 상태 확인
        print("\n1. 시스템 상태 확인")
        status_cmd = json.dumps({
            "type": "command",
            "command": "get_status"
        }) + "\n"
        
        if send_command(status_cmd):
            response = wait_for_response(timeout=3.0)
            if response:
                print(f"응답: {response}")
                try:
                    resp_data = json.loads(response)
                    print(f"현재 센서 ID: {resp_data.get('user_sensor_id', '??')}")
                    print(f"센서 주소: {resp_data.get('sensor_addr', '??')}")
                except:
                    pass

        # 2. 센서 목록 확인
        print("\n2. 센서 목록 확인")
        list_cmd = json.dumps({
            "type": "command", 
            "command": "list_sensors"
        }) + "\n"
        
        sensor_addresses = []
        if send_command(list_cmd):
            response = wait_for_response(timeout=3.0)
            if response:
                try:
                    resp_data = json.loads(response)
                    sensor_addresses = resp_data.get('addresses', [])
                    print(f"발견된 센서: {len(sensor_addresses)}개")
                    for i, addr in enumerate(sensor_addresses):
                        print(f"  센서 {i+1}: {addr}")
                except:
                    print(f"응답: {response}")

        if not sensor_addresses:
            print("센서를 찾을 수 없습니다.")
            return
            
        # 3. ID 변경 테스트 (첫 번째 센서)
        test_sensor_addr = sensor_addresses[0]
        new_id = "05"  # 테스트용 ID
        
        print(f"\n3. 센서 ID 변경 테스트")
        print(f"대상 센서: {test_sensor_addr}")
        print(f"새 ID: {new_id}")
        
        # 방법 1: 간단한 텍스트 명령
        print("\n방법 1: 텍스트 명령 테스트")
        text_cmd = f"SET_SENSOR_ID:{test_sensor_addr}:{new_id}\n"
        print(f"명령: {text_cmd.strip()}")
        
        if send_command(text_cmd):
            response = wait_for_response(timeout=5.0)
            if response:
                print(f"응답: {response}")
        
        time.sleep(1)
        
        # 방법 2: JSON 명령 (Arduino 펌웨어에서 지원하는 정확한 형식)
        print("\n방법 2: JSON 명령 테스트 - setSensorId")
        json_cmd = json.dumps({
            "type": "command",
            "command": "setSensorId",
            "sensor_addr": test_sensor_addr,
            "new_id": new_id
        }) + "\n"
        print(f"명령: {json_cmd.strip()}")
        
        if send_command(json_cmd):
            response = wait_for_response(timeout=5.0)
            if response:
                print(f"응답: {response}")
        
        time.sleep(1)
        
        # 방법 3: 다른 JSON 형식
        print("\n방법 3: JSON 명령 테스트 - set_sensor_data")
        json_cmd2 = json.dumps({
            "type": "command",
            "command": "set_sensor_data",
            "new_value": int(new_id)
        }) + "\n"
        print(f"명령: {json_cmd2.strip()}")
        
        if send_command(json_cmd2):
            response = wait_for_response(timeout=5.0)
            if response:
                print(f"응답: {response}")
        
        # 4. 최종 상태 확인
        print("\n4. 최종 상태 확인")
        if send_command(status_cmd):
            response = wait_for_response(timeout=3.0)
            if response:
                try:
                    resp_data = json.loads(response)
                    print(f"최종 센서 ID: {resp_data.get('user_sensor_id', '??')}")
                except:
                    print(f"응답: {response}")
        
        print("\n" + "=" * 50)
        print("센서 ID 변경 테스트 완료")
        
    except Exception as e:
        print(f"테스트 오류: {e}")


def send_command(command: str) -> bool:
    """Arduino에 명령 전송"""
    try:
        from src.python.services.port_manager import port_manager
        
        if not port_manager.is_connected():
            return False
            
        if not hasattr(port_manager, "_serial_connection") or port_manager._serial_connection is None:
            return False
        
        bytes_written = port_manager._serial_connection.write(command.encode())
        port_manager._serial_connection.flush()
        
        print(f"전송 완료: {bytes_written} bytes")
        return True
        
    except Exception as e:
        print(f"명령 전송 실패: {e}")
        return False


def wait_for_response(timeout: float) -> str | None:
    """Arduino 응답 대기"""
    try:
        from src.python.services.port_manager import port_manager
        
        print(f"응답 대기 중... (최대 {timeout}초)")
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            if (port_manager.is_connected() and 
                hasattr(port_manager, "_serial_connection") and 
                port_manager._serial_connection is not None):
                
                waiting = port_manager._serial_connection.in_waiting
                if waiting > 0:
                    response_bytes = port_manager._serial_connection.readline()
                    response = response_bytes.decode("utf-8", errors="ignore").strip()
                    if response:
                        return response
            
            time.sleep(0.1)
        
        print("응답 시간 초과")
        return None
        
    except Exception as e:
        print(f"응답 읽기 오류: {e}")
        return None


if __name__ == "__main__":
    test_sensor_id_change()

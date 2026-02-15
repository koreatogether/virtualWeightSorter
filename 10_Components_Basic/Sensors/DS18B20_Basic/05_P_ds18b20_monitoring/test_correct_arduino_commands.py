#!/usr/bin/env python3
"""올바른 Arduino 명령 형식 테스트"""

import sys
import os
import json
import time

# 프로젝트 루트를 Python 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

def test_correct_commands():
    """올바른 Arduino 명령 형식으로 테스트"""
    try:
        from src.python.services.port_manager import port_manager
        from src.python.callbacks.sensor_id_callbacks import _send_command, _wait_for_response
        
        if not port_manager.is_connected():
            print("❌ Arduino가 연결되지 않았습니다.")
            return
        
        print("🧪 올바른 Arduino 명령 형식 테스트")
        print("=" * 50)
        
        # 1. 시스템 상태 확인
        print("\n1️⃣ 시스템 상태 확인")
        status_cmd = json.dumps({
            "type": "command",
            "command": "get_status"
        }) + "\n"
        
        success = _send_command(status_cmd)
        if success:
            response = _wait_for_response(timeout=3.0)
            if response:
                print(f"   📤 응답: {response}")
                try:
                    resp_data = json.loads(response)
                    if resp_data.get("type") == "system_status":
                        print(f"   ✅ 시스템 상태 수신 성공!")
                        print(f"   📊 센서 연결: {resp_data.get('sensor_connected')}")
                        print(f"   📊 센서 주소: {resp_data.get('sensor_addr')}")
                        print(f"   📊 사용자 ID: {resp_data.get('user_sensor_id')}")
                except:
                    pass
        
        # 2. 센서 목록 확인
        print("\n2️⃣ 센서 목록 확인")
        list_cmd = json.dumps({
            "type": "command", 
            "command": "list_sensors"
        }) + "\n"
        
        success = _send_command(list_cmd)
        if success:
            response = _wait_for_response(timeout=3.0)
            if response:
                print(f"   📤 응답: {response}")
                try:
                    resp_data = json.loads(response)
                    if resp_data.get("status") == "success":
                        print(f"   ✅ 센서 목록 수신 성공!")
                        print(f"   📊 센서 개수: {resp_data.get('count')}")
                        addresses = resp_data.get('addresses', [])
                        for i, addr in enumerate(addresses):
                            print(f"   📊 센서 {i}: {addr}")
                        
                        # 첫 번째 센서로 ID 변경 테스트
                        if addresses:
                            test_sensor_addr = addresses[0]
                            print(f"\n3️⃣ 센서 ID 변경 테스트 (주소: {test_sensor_addr})")
                            
                            # 방법 1: 간단한 텍스트 명령
                            print("\n   방법 1: 텍스트 명령 (SET_SENSOR_ID:주소:ID)")
                            text_cmd = f"SET_SENSOR_ID:{test_sensor_addr}:99\n"
                            print(f"   명령: {text_cmd.strip()}")
                            
                            success = _send_command(text_cmd)
                            if success:
                                response = _wait_for_response(timeout=3.0)
                                if response:
                                    print(f"   📤 응답: {response}")
                                    try:
                                        resp_data = json.loads(response)
                                        if resp_data.get("status") == "success":
                                            print(f"   ✅ 텍스트 명령 성공!")
                                        else:
                                            print(f"   ❌ 텍스트 명령 실패: {resp_data.get('message')}")
                                    except:
                                        pass
                            
                            time.sleep(1)
                            
                            # 방법 2: JSON 명령 (set_sensor_data)
                            print("\n   방법 2: JSON 명령 (set_sensor_data)")
                            json_cmd = json.dumps({
                                "type": "command",
                                "command": "set_sensor_data", 
                                "new_value": 88
                            }) + "\n"
                            print(f"   명령: {json_cmd.strip()}")
                            
                            success = _send_command(json_cmd)
                            if success:
                                response = _wait_for_response(timeout=3.0)
                                if response:
                                    print(f"   📤 응답: {response}")
                                    try:
                                        resp_data = json.loads(response)
                                        if resp_data.get("status") == "success":
                                            print(f"   ✅ JSON 명령 성공!")
                                        else:
                                            print(f"   ❌ JSON 명령 실패: {resp_data.get('message')}")
                                    except:
                                        pass
                            
                except:
                    pass
        
        # 4. 최종 상태 확인
        print("\n4️⃣ 최종 상태 확인")
        success = _send_command(status_cmd)
        if success:
            response = _wait_for_response(timeout=3.0)
            if response:
                try:
                    resp_data = json.loads(response)
                    if resp_data.get("type") == "system_status":
                        print(f"   📊 최종 사용자 ID: {resp_data.get('user_sensor_id')}")
                except:
                    pass
        
        print("\n" + "=" * 50)
        print("🧪 올바른 명령 형식 테스트 완료")
        
    except Exception as e:
        print(f"❌ 테스트 오류: {e}")

if __name__ == "__main__":
    test_correct_commands()
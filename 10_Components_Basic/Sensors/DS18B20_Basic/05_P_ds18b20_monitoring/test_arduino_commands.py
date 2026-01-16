#!/usr/bin/env python3
"""Arduino 명령어 형식 테스트"""

import sys
import os
import json
import time

# 프로젝트 루트를 Python 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

def test_various_commands():
    """다양한 명령 형식 테스트"""
    try:
        from src.python.services.port_manager import port_manager
        from src.python.callbacks.sensor_id_callbacks import _send_command, _wait_for_response
        
        if not port_manager.is_connected():
            print("❌ Arduino가 연결되지 않았습니다.")
            return
        
        print("🧪 Arduino 명령어 형식 테스트")
        print("=" * 50)
        
        # 테스트할 명령 형식들
        test_commands = [
            # 1. 기본 형식
            {
                "type": "command",
                "action": "setSensorId",
                "sensorAddress": "285882840000000E",
                "newId": "04"
            },
            # 2. snake_case 형식
            {
                "type": "command", 
                "action": "set_sensor_id",
                "sensor_address": "285882840000000E",
                "new_id": "04"
            },
            # 3. 대문자 형식
            {
                "type": "command",
                "action": "SET_SENSOR_ID",
                "sensor_address": "285882840000000E", 
                "new_id": "04"
            },
            # 4. 간단한 형식
            {
                "command": "setSensorId",
                "address": "285882840000000E",
                "id": "04"
            },
            # 5. 다른 형식
            {
                "cmd": "changeSensorId",
                "addr": "285882840000000E",
                "newId": "04"
            }
        ]
        
        for i, cmd_data in enumerate(test_commands, 1):
            print(f"\n{i}️⃣ 테스트 명령 {i}")
            command = json.dumps(cmd_data) + "\n"
            print(f"   명령: {command.strip()}")
            
            success = _send_command(command)
            if success:
                response = _wait_for_response(timeout=3.0)
                if response:
                    try:
                        resp_data = json.loads(response)
                        status = resp_data.get("status", "unknown")
                        message = resp_data.get("message", "")
                        
                        if status == "success":
                            print(f"   ✅ 성공: {message}")
                            break  # 성공하면 중단
                        elif status == "error":
                            print(f"   ❌ 실패: {message}")
                        else:
                            print(f"   ❓ 알 수 없음: {response}")
                    except:
                        print(f"   📤 응답: {response}")
                else:
                    print(f"   ⏰ 응답 없음")
            else:
                print(f"   ❌ 전송 실패")
            
            time.sleep(1)  # 1초 대기
        
        print("\n" + "=" * 50)
        print("🧪 테스트 완료")
        
    except Exception as e:
        print(f"❌ 테스트 오류: {e}")

if __name__ == "__main__":
    test_various_commands()
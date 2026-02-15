#!/usr/bin/env python3
"""ID 변경 디버그 테스트"""

import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

def test_id_change_communication():
    """ID 변경 통신 테스트"""
    try:
        from src.python.services.port_manager import port_manager
        from src.python.services.sensor_data_manager import sensor_manager
        from src.python.callbacks.sensor_id_callbacks import (
            _build_sensor_id_command,
            _send_command,
            _wait_for_response
        )
        
        print("🧪 ID 변경 통신 테스트 시작")
        print("=" * 50)
        
        # 1. 연결 상태 확인
        print("1️⃣ 연결 상태 확인")
        is_connected = port_manager.is_connected()
        print(f"   Arduino 연결: {is_connected}")
        
        if not is_connected:
            print("❌ Arduino가 연결되지 않았습니다.")
            print("💡 해결 방법:")
            print("   1. Arduino 시뮬레이터 실행: python test_simple_simulator.py")
            print("   2. 대시보드에서 COM 포트 연결")
            return
        
        # 2. 현재 센서 목록 확인
        print("\n2️⃣ 현재 센서 목록")
        sensors = sensor_manager.get_sorted_sensors()
        print(f"   연결된 센서 수: {len(sensors)}")
        
        for i, sensor in enumerate(sensors):
            print(f"   센서 {i+1}: ID {sensor.sensor_id}, 주소 {sensor.sensor_addr[:16]}...")
        
        if not sensors:
            print("❌ 연결된 센서가 없습니다.")
            return
        
        # 3. 첫 번째 센서로 ID 변경 테스트
        test_sensor = sensors[0]
        old_id = test_sensor.sensor_id
        new_id = "99"  # 테스트용 ID
        
        print(f"\n3️⃣ ID 변경 테스트")
        print(f"   대상 센서: {test_sensor.sensor_addr[:16]}...")
        print(f"   현재 ID: {old_id}")
        print(f"   새 ID: {new_id}")
        
        # 4. 명령 생성 및 전송
        print(f"\n4️⃣ 명령 전송")
        command = _build_sensor_id_command(test_sensor.sensor_addr, new_id)
        print(f"   생성된 명령: {command.strip()}")
        
        success = _send_command(command)
        if not success:
            print("❌ 명령 전송 실패")
            return
        
        # 5. 응답 대기
        print(f"\n5️⃣ Arduino 응답 대기")
        response = _wait_for_response(timeout=10.0)
        
        if response:
            print(f"✅ 응답 수신: {response}")
            
            # 6. 응답 분석
            print(f"\n6️⃣ 응답 분석")
            if "SUCCESS" in response:
                print("✅ ID 변경 성공!")
                
                # 7. 센서 데이터 확인
                print(f"\n7️⃣ 변경된 센서 데이터 확인 (10초 대기)")
                time.sleep(2)
                
                updated_sensors = sensor_manager.get_sorted_sensors()
                print(f"   업데이트된 센서 수: {len(updated_sensors)}")
                
                found_new_id = False
                for sensor in updated_sensors:
                    print(f"   센서: ID {sensor.sensor_id}, 주소 {sensor.sensor_addr[:16]}...")
                    if sensor.sensor_id == new_id:
                        found_new_id = True
                        print(f"   ✅ 새 ID {new_id} 확인됨!")
                
                if not found_new_id:
                    print(f"   ⚠️ 새 ID {new_id}가 아직 센서 데이터에 반영되지 않음")
                    print("   💡 Arduino에서 새 데이터를 전송할 때까지 기다려보세요")
                
            elif "ERROR" in response:
                print(f"❌ ID 변경 실패: {response}")
            else:
                print(f"❓ 알 수 없는 응답: {response}")
        else:
            print("❌ Arduino 응답 없음")
            print("💡 확인사항:")
            print("   1. Arduino 시뮬레이터가 실행 중인가?")
            print("   2. 시뮬레이터에서 명령을 받았는가?")
            print("   3. 시리얼 통신이 정상인가?")
        
        print("\n" + "=" * 50)
        print("🧪 테스트 완료")
        
    except Exception as e:
        print(f"❌ 테스트 중 오류: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_id_change_communication()
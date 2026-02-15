#!/usr/bin/env python3
"""
자동 센서 ID 설정 스크립트
"""

import serial
import time
import sys

def find_arduino_port():
    """Arduino 포트를 검색합니다."""
    # COM4가 이미 확인되어있으므로 직용 사용
    return "COM4"

def send_sensor_id_commands():
    """
센서 ID 설정 명령어들을 순차적으로 전송합니다.
"""
    
    # 센서 주소와 설정할 ID 매핑
    sensor_commands = [
        ("285C82850000005D", "02"),  # user_id: 1026
        ("28E6AA830000005A", "03"),  # user_id: 771
        ("28E79B850000002D", "04"),  # user_id: 260
        ("28FF641F43B82384", "05"),  # user_id: 517
    ]
    
    print("🔍 Arduino 포트 검색 중...")
    port = find_arduino_port()
    
    if not port:
        print("❌ Arduino를 찾을 수 없습니다!")
        return False
    
    print(f"✅ Arduino 발견: {port}")
    
    try:
        # 시리얼 연걸
        ser = serial.Serial(port, 115200, timeout=5)
        time.sleep(2)
        
        print("🚀 센서 ID 설정 시작...")
        print("=" * 50)
        
        for i, (sensor_addr, sensor_id) in enumerate(sensor_commands, 1):
            command = f"SET_SENSOR_ID:{sensor_addr}:{sensor_id}"
            
            print(f"\n📡 [{i}/4] 명령어 전송: {command}")
            
            # 명령어 전송
            ser.write(f"{command}\n".encode())
            
            # 응답 대기 (최대 10초)
            start_time = time.time()
            response_received = False
            
            while time.time() - start_time < 10:
                if ser.in_waiting > 0:
                    response = ser.readline().decode('utf-8', errors='ignore').strip()
                    if response:
                        print(f"📥 응답: {response}")
                        
                        if "Sensor ID set successfully" in response:
                            print(f"✅ 센서 {sensor_addr} → ID '{sensor_id}' 설정 완료!")
                            response_received = True
                            break
                        elif "Error" in response or "Failed" in response:
                            print(f"❌ 설정 실패: {response}")
                            break
                
                time.sleep(0.1)
            
            if not response_received:
                print(f"⚠️  응답 시간 초과 (센서: {sensor_addr})")
            
            # 다음 명령어 전 약간 대기
            time.sleep(2)
        
        print("\n" + "=" * 50)
        print("🎉 모든 센서 ID 설정 명령어 전송 완료!")
        
        ser.close()
        return True
        
    except Exception as e:
        print(f"❌ 오류 발생: {e}")
        return False

if __name__ == "__main__":
    print("🏯 DS18B20 센서 자동 설정 도구")
    print("=" * 50)
    
    success = send_sensor_id_commands()
    
    if success:
        print("\n✨ 센서 데이터를 확인해보세요!")
    else:
        print("\n❌ 설정 중 문제가 발생했습니다.")

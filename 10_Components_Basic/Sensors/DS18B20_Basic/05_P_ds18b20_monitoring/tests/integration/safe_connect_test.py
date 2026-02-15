#!/usr/bin/env python3
"""
안전한 Arduino 연결 테스트
"""
import time

import serial


def safe_connect_test():
    print("=== 안전한 Arduino 연결 테스트 ===")

    try:
        # 매우 조심스럽게 연결
        print("1. 포트 열기 (리셋 방지)")
        ser = serial.Serial()
        ser.port = 'COM4'
        ser.baudrate = 115200
        ser.timeout = 2
        ser.write_timeout = 2

        # 리셋 방지 설정
        ser.dtr = None  # None으로 설정하여 상태 변경 방지
        ser.rts = None  # None으로 설정하여 상태 변경 방지

        ser.open()
        print("2. 포트 연결 성공")

        # 추가 리셋 방지
        ser.dtr = False
        ser.rts = False

        print("3. Arduino 안정화 대기 (5초)...")
        time.sleep(5)

        print("4. 수신된 데이터:")
        for i in range(15):  # 15번 시도
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8').strip()
                    if line:
                        print(f"   [{i+1}] {line}")
                except Exception as e:
                    print(f"   [{i+1}] 디코딩 오류: {e}")
            else:
                print(f"   [{i+1}] 대기 중... ({ser.in_waiting} bytes)")
            time.sleep(1)

        ser.close()
        print("5. 연결 종료")

    except Exception as e:
        print(f"오류: {e}")
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    safe_connect_test()

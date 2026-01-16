#!/usr/bin/env python3
"""
다양한 보드레이트로 아두이노 시리얼 테스트
"""

import time

import serial


def test_multiple_baudrates():
    """다양한 보드레이트로 테스트"""
    baudrates = [9600, 19200, 38400, 57600, 115200, 230400]

    for baudrate in baudrates:
        print(f"\n보드레이트 {baudrate} 테스트 중...")
        try:
            ser = serial.Serial(
                port="COM4",
                baudrate=baudrate,
                timeout=3,
                rtscts=False,
                dsrdtr=False
            )

            # Arduino 리셋 방지
            ser.dtr = False
            ser.rts = False
            time.sleep(0.5)

            print(f"   연결 성공: COM4@{baudrate}")

            # 5초간 데이터 수신 시도
            start_time = time.time()
            received_data = False

            while time.time() - start_time < 5:
                if ser.in_waiting > 0:
                    data = ser.readline()
                    try:
                        decoded = data.decode('utf-8').strip()
                        print(f"   수신: {decoded}")
                        received_data = True
                        break
                    except:
                        print(f"   원시 데이터: {data}")
                        received_data = True
                        break
                time.sleep(0.1)

            if not received_data:
                print("   데이터 수신 없음")

            ser.close()

        except Exception as e:
            print(f"   연결 실패: {e}")

if __name__ == "__main__":
    print("다중 보드레이트 아두이노 테스트")
    print("=" * 50)
    test_multiple_baudrates()
    print("\n테스트 완료!")
    input("아무 키나 눌러 종료...")

#!/usr/bin/env python3
"""
Arduino 리셋 방지 연결 테스트
"""
import time

import serial


def no_reset_connect():
    print("=== Arduino 리셋 방지 연결 테스트 ===")

    try:
        # 가장 안전한 연결 방법
        ser = serial.Serial(
            port='COM4',
            baudrate=115200,
            timeout=3,
            write_timeout=3,
            # 모든 제어 신호 비활성화
            dsrdtr=False,
            rtscts=False,
            xonxoff=False
        )

        # 연결 후 즉시 제어 신호 비활성화
        if hasattr(ser, 'dtr'):
            ser.dtr = False
        if hasattr(ser, 'rts'):
            ser.rts = False

        print("✓ 포트 연결 성공 (리셋 방지 모드)")

        # 짧은 대기 후 데이터 확인
        time.sleep(1)

        print("데이터 수신 시도:")
        for i in range(20):
            try:
                # 논블로킹 읽기
                if ser.in_waiting > 0:
                    data = ser.read(ser.in_waiting)
                    text = data.decode('utf-8', errors='ignore').strip()
                    if text:
                        for line in text.split('\n'):
                            if line.strip():
                                print(f"  📨 {line.strip()}")
                else:
                    print(f"  ⏳ 대기 중... ({i+1}/20)")
                time.sleep(0.5)
            except Exception as e:
                print(f"  ❌ 읽기 오류: {e}")

        ser.close()
        print("✓ 연결 종료")

    except Exception as e:
        print(f"❌ 연결 오류: {e}")

if __name__ == "__main__":
    no_reset_connect()

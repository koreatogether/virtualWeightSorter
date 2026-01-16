#!/usr/bin/env python3
"""
아두이노 시리얼 데이터 직접 읽기 디버그 도구
COM4 포트에서 원시 데이터를 읽어서 확인
"""

import time

import serial


def debug_serial_data(port="COM4", baudrate=115200, timeout=5):
    """시리얼 포트에서 원시 데이터 읽기"""
    try:
        print(f"COM4 포트 연결 시도... (Baudrate: {baudrate})")

        # 시리얼 포트 열기
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=1,
            rtscts=False,   # RTS/CTS 흐름 제어 비활성화
            dsrdtr=False    # DSR/DTR 흐름 제어 비활성화
        )

        # Arduino 리셋 방지
        ser.dtr = False
        ser.rts = False
        time.sleep(0.1)

        print(f"✅ 시리얼 포트 연결 성공: {port}")
        print("아두이노에서 데이터 수신 대기 중...")
        print("=" * 50)

        start_time = time.time()
        data_count = 0

        while time.time() - start_time < timeout:
            if ser.in_waiting > 0:
                # 원시 바이트 데이터 읽기
                raw_data = ser.readline()

                try:
                    # UTF-8로 디코딩 시도
                    decoded_data = raw_data.decode('utf-8').strip()
                    data_count += 1

                    print(f"[{data_count:03d}] 수신: {decoded_data}")
                    print(f"     원시: {raw_data}")
                    print(f"     길이: {len(raw_data)} bytes")

                    # JSON 형태인지 확인
                    if decoded_data.startswith('{') and decoded_data.endswith('}'):
                        print("     📊 JSON 형태 데이터 감지!")
                    print("-" * 40)

                except UnicodeDecodeError as e:
                    print(f"[ERR] 디코딩 실패: {raw_data} - {e}")

            time.sleep(0.1)

        if data_count == 0:
            print("❌ 타임아웃: 아두이노에서 데이터를 수신하지 못했습니다.")
            print("\n가능한 원인:")
            print("1. 아두이노가 데이터를 전송하지 않음")
            print("2. Baudrate 불일치 (현재: 115200)")
            print("3. 아두이노 프로그램이 실행되지 않음")
            print("4. DS18B20 센서 연결 문제")
        else:
            print(f"✅ 총 {data_count}개의 메시지 수신됨")

        ser.close()

    except serial.SerialException as e:
        print(f"❌ 시리얼 포트 오류: {e}")

    except Exception as e:
        print(f"❌ 예상치 못한 오류: {e}")

if __name__ == "__main__":
    print("🔧 아두이노 시리얼 데이터 디버그 도구")
    print("=" * 50)

    # 5초 동안 데이터 수신 시도
    debug_serial_data(timeout=10)

    print("\n디버그 완료. 아무 키나 눌러 종료...")
    input()

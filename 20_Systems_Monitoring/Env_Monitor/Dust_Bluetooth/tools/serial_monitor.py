#!/usr/bin/env python3
"""
간단한 Arduino 시리얼 모니터
COM4:9600으로 연결하여 데이터를 수신합니다.
"""

import serial
import time
import sys

def connect_arduino(port='COM4', baudrate=9600):
    """Arduino에 연결하고 데이터를 수신합니다."""
    try:
        # 시리얼 연결
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"시리얼 연결 성공 - {port}:{baudrate}")
        
        # Arduino 리셋 신호
        print("Arduino 리셋 신호를 보냅니다...")
        ser.setDTR(False)
        time.sleep(1)
        ser.setDTR(True)
        time.sleep(2)
        
        print("수신 대기 중... (Ctrl+C로 종료)")
        print("-" * 50)
        
        # 데이터 수신 루프
        while True:
            if ser.in_waiting > 0:
                try:
                    data = ser.readline().decode('utf-8').strip()
                    if data:
                        timestamp = time.strftime("%H:%M:%S")
                        print(f"[{timestamp}] {data}")
                except UnicodeDecodeError:
                    print("[DEBUG] 디코딩 오류 발생")
            time.sleep(0.01)  # CPU 사용량 최적화
            
    except serial.SerialException as e:
        print(f"시리얼 연결 오류: {e}")
        print("Arduino가 COM4에 연결되어 있는지 확인하세요.")
    except KeyboardInterrupt:
        print("\n사용자가 중단했습니다.")
    except Exception as e:
        print(f"예상치 못한 오류: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("시리얼 연결 종료")

if __name__ == "__main__":
    connect_arduino()
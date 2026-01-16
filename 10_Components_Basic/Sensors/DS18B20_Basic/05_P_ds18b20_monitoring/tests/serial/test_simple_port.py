#!/usr/bin/env python3
"""간단한 Arduino 포트 감지 테스트"""

import serial.tools.list_ports

# Arduino VID:PID 목록
ARDUINO_IDENTIFIERS = [
    '2341:0043',  # Arduino Uno R3
    '2341:8037',  # Arduino Micro
    '2341:0001',  # Arduino Leonardo
    '2341:0036',  # Arduino Leonardo (bootloader)
    '2341:1002',  # Arduino 보드 (사용자의 보드)
    '10C4:EA60',  # ESP32 DevKit (Silicon Labs)
    '1A86:7523',  # CH340 (중국 클론)
    '0403:6001',  # FTDI FT232
]

def calculate_arduino_score(port):
    """Arduino 호환성 점수 계산"""
    score = 0

    if port.hwid:
        hwid_upper = port.hwid.upper()

        # VID:PID 매치 확인
        for vid_pid in ARDUINO_IDENTIFIERS:
            if vid_pid.upper() in hwid_upper:
                if '2341:' in vid_pid:  # Arduino 공식 VID
                    score += 50
                elif '10C4:EA60' in vid_pid:  # ESP32
                    score += 45
                else:  # 클론/FTDI
                    score += 40
                break

    # 설명 기반 점수
    if port.description:
        desc_lower = port.description.lower()
        if 'arduino' in desc_lower:
            score += 20
        elif any(word in desc_lower for word in ['usb serial', 'usb 직렬']):
            score += 10

    return score

def main():
    print("=== Arduino Port Detection Test ===\n")

    ports = list(serial.tools.list_ports.comports())
    print(f"System ports: {len(ports)}")

    arduino_ports = []

    for port in ports:
        print(f"\nPort: {port.device}")
        print(f"   Description: {port.description}")
        print(f"   HWID: {port.hwid}")
        print(f"   Manufacturer: {port.manufacturer}")

        score = calculate_arduino_score(port)
        is_arduino = score >= 15

        print(f"   Compatibility Score: {score}/100")
        print(f"   Arduino Compatible: {'YES' if is_arduino else 'NO'}")

        if is_arduino:
            arduino_ports.append(port.device)

    print(f"\nArduino Compatible Ports: {len(arduino_ports)}")
    for port in arduino_ports:
        print(f"   - {port}")

    if arduino_ports:
        print("\nArduino board detected! You can connect via Port Manager.")
    else:
        print("\nNo Arduino compatible ports found.")

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Arduino 포트 감지 테스트"""

import sys
from pathlib import Path

# Add src/python to Python path
current_dir = Path(__file__).parent
src_python_dir = current_dir / "src" / "python"
if str(src_python_dir) not in sys.path:
    sys.path.insert(0, str(src_python_dir))

import serial.tools.list_ports
from dashboard.port_manager import ArduinoDetector, PortManager


def test_port_detection():
    """포트 감지 테스트"""
    print("=== 시리얼 포트 감지 테스트 ===\n")

    # 시스템 포트 목록
    ports = list(serial.tools.list_ports.comports())
    print(f"시스템 포트 수: {len(ports)}")

    for port in ports:
        print(f"\n📍 포트: {port.device}")
        print(f"   설명: {port.description}")
        print(f"   HWID: {port.hwid}")
        print(f"   제조사: {port.manufacturer}")

        # 호환성 점수 계산
        score = ArduinoDetector.calculate_compatibility_score(port)
        is_compatible = ArduinoDetector.is_arduino_compatible(port)

        print(f"   호환성 점수: {score}/100")
        print(f"   Arduino 호환: {'✅ YES' if is_compatible else '❌ NO'}")

    print("\n" + "="*50)

    # PortManager를 통한 스캔
    port_manager = PortManager()
    scanned_ports = port_manager.scan_ports()

    print(f"포트 매니저 스캔 결과: {len(scanned_ports)}개 포트")

    for port_info in scanned_ports:
        print(f"\n🔍 {port_info.device} - 호환성: {port_info.compatibility_score}% - 상태: {port_info.status}")
        if port_info.compatibility_score >= 15:
            print("   ✅ Arduino 호환 포트로 감지됨!")
        else:
            print("   ❌ 호환성이 낮음")

if __name__ == "__main__":
    test_port_detection()

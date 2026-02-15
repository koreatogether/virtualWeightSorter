#!/usr/bin/env python3
"""COM4 포트 사용 현황 확인"""

import time

import psutil
import serial
import serial.tools.list_ports


def check_com4_usage():
    print("=== COM4 포트 사용 현황 확인 ===")

    # 1. 시리얼 포트 목록 확인
    print("\n1. 시스템 시리얼 포트 목록:")
    ports = serial.tools.list_ports.comports()
    for port in ports:
        print(f"   {port.device}: {port.description}")
        if "COM4" in port.device:
            print(f"      제조사: {port.manufacturer}")
            print(f"      하드웨어 ID: {port.hwid}")

    # 2. COM4 연결 시도
    print("\n2. COM4 직접 연결 테스트:")
    try:
        with serial.Serial('COM4', 115200, timeout=1) as ser:
            print("   ✅ COM4 연결 성공 - 포트가 사용 가능함")
            time.sleep(1)
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                print(f"   📥 수신된 데이터: {data}")
    except serial.SerialException as e:
        print(f"   ❌ COM4 연결 실패: {e}")
        if "being used" in str(e) or "Access is denied" in str(e):
            print("   🔒 다른 프로세스가 COM4를 사용 중입니다")
    except Exception as e:
        print(f"   ❌ 기타 오류: {e}")

    # 3. Python 프로세스들의 열린 파일 확인
    print("\n3. Python 프로세스들의 COM 포트 사용 확인:")
    for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
        try:
            if proc.info['name'] == 'python.exe':
                pid = proc.info['pid']
                cmdline = ' '.join(proc.info['cmdline'] or [])
                print(f"\n   PID {pid}:")
                print(f"      명령줄: {cmdline[:100]}...")

                # 열린 파일 확인
                try:
                    open_files = proc.open_files()
                    com_files = [f for f in open_files if 'COM' in f.path.upper()]
                    if com_files:
                        print("      🔗 사용 중인 COM 포트:")
                        for f in com_files:
                            print(f"         {f.path}")
                    else:
                        print("      📝 COM 포트 사용 없음")
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    print("      ⚠️  프로세스 정보 접근 불가")

        except (psutil.NoSuchProcess, psutil.ZombieProcess):
            continue

    print("\n=== 확인 완료 ===")

if __name__ == "__main__":
    check_com4_usage()

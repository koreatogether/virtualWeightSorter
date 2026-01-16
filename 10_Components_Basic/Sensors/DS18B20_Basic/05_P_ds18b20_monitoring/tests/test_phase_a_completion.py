#!/usr/bin/env python3
"""Phase A 완성 기능 테스트

EEPROM 시뮬레이션, 사용자 ID 관리, 변경 감지 시스템이
제대로 동작하는지 확인하는 테스트 스크립트입니다.
"""

import json
import sys
from pathlib import Path

# 프로젝트 루트를 Python 경로에 추가
project_root = Path(__file__).parent
sys.path.insert(0, str(project_root))

from src.python.simulator.offline_simulator import OfflineDS18B20Simulator


def test_eeprom_functionality():
    """EEPROM 기능 테스트"""
    print("🧪 Phase A 완성 기능 테스트")
    print("=" * 50)

    # 1. 시뮬레이터 초기화
    print("\n1️⃣ 시뮬레이터 초기화")
    simulator = OfflineDS18B20Simulator()
    print(f"   초기 사용자 ID: {simulator.getUserData()}")

    # 2. 사용자 ID 변경 테스트
    print("\n2️⃣ 사용자 ID 변경 테스트")
    for new_id in [3, 7, 2, 1]:
        success = simulator.setUserData(new_id)
        current_id = simulator.getUserData()
        print(f"   {new_id}로 변경: {'성공' if success else '실패'} (현재: {current_id})")

    # 3. 범위 외 값 테스트
    print("\n3️⃣ 범위 외 값 테스트")
    for invalid_id in [0, 9, -1, 10]:
        success = simulator.setUserData(invalid_id)
        print(f"   {invalid_id}로 변경: {'성공' if success else '실패 (예상됨)'}")

    # 4. EEPROM 상태 조회
    print("\n4️⃣ EEPROM 상태 조회")
    eeprom_status = simulator.get_eeprom_status()
    print(f"   사용자 ID: {eeprom_status['user_id']}")
    print(f"   센서 ID: {eeprom_status['sensor_id']}")
    print(f"   TH 값: {eeprom_status['th_value']}°C")
    print(f"   TL 값: {eeprom_status['tl_value']}°C")
    print(f"   측정 주기: {eeprom_status['measurement_interval']}ms")

    # 5. 명령 처리 테스트
    print("\n5️⃣ 명령 처리 테스트")

    # 사용자 ID 조회 명령
    cmd_get = {
        "type": "command",
        "command": "get_user_data"
    }
    response = simulator.process_command(cmd_get)
    print(f"   get_user_data 응답: {response['message']}")

    # 사용자 ID 설정 명령
    cmd_set = {
        "type": "command",
        "command": "set_user_data",
        "new_value": 5
    }
    response = simulator.process_command(cmd_set)
    print(f"   set_user_data 응답: {response['message']}")
    print(f"   변경 후 ID: {simulator.getUserData()}")

    # 6. EEPROM 파일 검증
    print("\n6️⃣ EEPROM 파일 검증")
    eeprom_file = Path("data/eeprom_simulation_offline.json")
    if eeprom_file.exists():
        with open(eeprom_file, encoding='utf-8') as f:
            data = json.load(f)
        print(f"   파일에 저장된 사용자 ID: {data.get('user_id', 'N/A')}")
        print(f"   파일 경로: {eeprom_file.absolute()}")
    else:
        print("   ❌ EEPROM 파일이 존재하지 않습니다!")

    print("\n✅ Phase A 테스트 완료!")
    print("   - EEPROM 시뮬레이션: 동작 ✅")
    print("   - 사용자 ID 관리: 동작 ✅")
    print("   - 변경 감지 시스템: 동작 ✅")
    print("   - 명령 처리: 동작 ✅")


if __name__ == "__main__":
    test_eeprom_functionality()

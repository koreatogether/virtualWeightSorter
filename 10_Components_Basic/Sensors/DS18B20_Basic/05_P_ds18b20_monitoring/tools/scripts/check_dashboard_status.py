#!/usr/bin/env python3
"""
Comprehensive dashboard connection and sensor health check (moved to tools/scripts)
"""
from pathlib import Path
import sys
import time

project_root = Path(__file__).parents[2]
sys.path.insert(0, str(project_root))

from src.python.services.port_manager import port_manager
from src.python.services.sensor_logger import sensor_logger

def comprehensive_check():
    print("=== 대시보드 연결 상태 종합 점검 ===")

    is_connected = port_manager.is_connected()
    current_port = port_manager.get_current_port()
    status = port_manager.get_status()

    print(f"\n📊 현재 상태:")
    print(f"  연결됨: {is_connected}")
    print(f"  포트: {current_port}")
    print(f"  Baudrate: {status.baudrate}")
    print(f"  연결시작: {status.connected_since}")
    print(f"  마지막 오류: {status.last_error}")

    if is_connected:
        sensors_found = []
        for i in range(5):
            data = port_manager.read_sensor_data(timeout=2.0)
            if data:
                sensor_info = {
                    'temperature': data.get('temperature'),
                    'address': data.get('sensor_addr'),
                    'user_id': data.get('user_sensor_id'),
                    'th': data.get('th_value'),
                    'tl': data.get('tl_value')
                }
                addr = sensor_info.get('address') or ''
                if addr not in [s.get('address') or '' for s in sensors_found]:
                    sensors_found.append(sensor_info)
                    print(f"  센서 {len(sensors_found)}: {sensor_info.get('temperature')}°C (ID:{sensor_info.get('user_id')}, Addr:{addr[:16]})")
                sensor_logger.update_sensor_data(data)
                if len(sensors_found) >= 5:
                    break
            else:
                print(f"  시도 {i+1}: 데이터 없음")
            time.sleep(0.5)

        print(f"\n📈 발견된 센서: {len(sensors_found)}")

        # basic throughput test
        print("\n🔁 통신 품질 테스트 (10초)")
        success_count = 0
        total_count = 0
        start_time = time.time()
        while time.time() - start_time < 10:
            data = port_manager.read_sensor_data(timeout=1.0)
            total_count += 1
            if data:
                success_count += 1
            time.sleep(0.8)
        success_rate = (success_count / total_count) * 100 if total_count > 0 else 0
        print(f"  성공률: {success_rate:.1f}% ({success_count}/{total_count})")

        # diagnostics (if available)
        try:
            if isinstance(current_port, str):
                from src.python.services.diagnostics import diagnose_sensor_communication
                diagnostic = diagnose_sensor_communication(current_port)
                print(f"  진단 결과: {'✅ 성공' if diagnostic.success else '❌ 실패'}")
                print(f"  메시지: {diagnostic.message}")
        except Exception:
            pass

        # disconnect and reconnect test
        port_manager.disconnect()
        print(f"  해제 후 상태: {'연결 안됨' if not port_manager.is_connected() else '여전히 연결'}")
        reconnect_success = port_manager.connect(current_port) if current_port else False
        print(f"  재연결: {'✅' if reconnect_success else '❌'}")

    else:
        print("\n⚠️ 연결되어 있지 않습니다 - 포트를 확인하세요")

    print('\n=== 점검 완료 ===')

if __name__ == "__main__":
    try:
        comprehensive_check()
    except Exception as e:
        print(f"❌ 점검 중 오류: {e}")
        import traceback
        traceback.print_exc()

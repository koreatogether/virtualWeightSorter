"""대시보드 연결 상태 종합 체크"""
import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager
from src.python.services.sensor_logger import sensor_logger

def comprehensive_check():
    """종합적인 연결 상태 체크"""
    print("=== 대시보드 연결 상태 종합 체크 ===")
    
    # 1. 현재 연결 상태 확인
    is_connected = port_manager.is_connected()
    current_port = port_manager.get_current_port()
    status = port_manager.get_status()
    
    print(f"\\n📊 현재 상태:")
    print(f"  연결됨: {is_connected}")
    print(f"  포트: {current_port}")
    print(f"  통신속도: {status.baudrate}")
    print(f"  연결 시간: {status.connected_since}")
    print(f"  마지막 오류: {status.last_error}")
    
    if is_connected:
        # 2. JSON 센서 데이터 수신 테스트
        print(f"\\n🌡️ 센서 데이터 수신 테스트:")
        
        sensors_found = []
        for i in range(5):  # 5번 시도
            data = port_manager.read_sensor_data(timeout=2.0)
            if data:
                sensor_info = {
                    'temperature': data.get('temperature'),
                    'address': data.get('sensor_addr'),
                    'user_id': data.get('user_sensor_id'),
                    'th': data.get('th_value'),
                    'tl': data.get('tl_value')
                }
                
                # 중복 센서 체크
                addr = sensor_info['address']
                if addr not in [s['address'] for s in sensors_found]:
                    sensors_found.append(sensor_info)
                    print(f"  센서 {len(sensors_found)}: {sensor_info['temperature']}°C (ID:{sensor_info['user_id']}, Addr:{addr[:16]})")
                
                # 센서 로거에 데이터 업데이트
                sensor_logger.update_sensor_data(data)
                
                if len(sensors_found) >= 5:  # 5개 센서 모두 발견
                    break
            else:
                print(f"  시도 {i+1}: 데이터 없음")
            
            time.sleep(0.5)
        
        print(f"\\n📈 발견된 센서: {len(sensors_found)}개")
        
        # 3. 통신 품질 테스트
        print(f"\\n🔄 통신 품질 테스트 (10초):")
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
        
        # 4. 버튼 동작 시뮬레이션 테스트
        print(f"\\n🔘 버튼 동작 테스트:")
        
        # 진단 테스트
        from src.python.services.diagnostics import diagnose_sensor_communication
        diagnostic = diagnose_sensor_communication(current_port)
        print(f"  진단 결과: {'✅ 성공' if diagnostic.success else '❌ 실패'}")
        print(f"  진단 메시지: {diagnostic.message}")
        
        # 해제/재연결 테스트
        print(f"  해제 테스트...")
        port_manager.disconnect()
        print(f"  해제 후 상태: {'❌ 해제됨' if not port_manager.is_connected() else '⚠️ 여전히 연결'}")
        
        print(f"  재연결 테스트...")
        reconnect_success = port_manager.connect(current_port)
        print(f"  재연결 결과: {'✅ 성공' if reconnect_success else '❌ 실패'}")
        
    else:
        print("\\n⚠️ 현재 연결되어 있지 않음")
    
    print(f"\\n=== 체크 완료 ===")

if __name__ == "__main__":
    try:
        comprehensive_check()
    except Exception as e:
        print(f"❌ 체크 중 오류: {e}")
        import traceback
        traceback.print_exc()
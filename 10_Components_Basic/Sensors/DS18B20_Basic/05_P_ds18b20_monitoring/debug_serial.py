"""Arduino 시리얼 통신 디버깅 스크립트"""
import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager

def debug_serial_communication():
    """시리얼 통신 상세 디버깅"""
    print("=== Arduino 시리얼 통신 디버깅 ===")
    
    # 포트 스캔
    ports = port_manager.scan_ports()
    print(f"사용 가능한 포트: {ports}")
    
    if not ports:
        print("❌ 사용 가능한 포트가 없습니다.")
        return
    
    port = ports[0]  # COM4
    print(f"\\n{port} 연결 시도...")
    
    # 연결
    success = port_manager.connect(port)
    print(f"연결 결과: {success}")
    
    if not success:
        print("❌ 연결 실패")
        return
    
    print("\\n=== 수동 통신 테스트 ===")
    
    # 1. 초기 데이터 확인
    print("1. 초기 Arduino 출력 확인 (3초 대기)...")
    time.sleep(3)
    initial_data = port_manager.read_data(timeout=1.0)
    if initial_data:
        print(f"   Arduino 초기 메시지: '{initial_data}'")
    else:
        print("   초기 메시지 없음")
    
    # 2. ping 테스트
    print("\\n2. ping 명령 전송...")
    send_success = port_manager.send_data("ping\\n")
    print(f"   전송 성공: {send_success}")
    
    if send_success:
        print("   응답 대기 (3초)...")
        response = port_manager.read_data(timeout=3.0)
        if response:
            print(f"   응답: '{response}'")
            if "pong" in response.lower():
                print("   ✅ ping-pong 성공!")
            else:
                print("   ⚠️ 예상과 다른 응답")
        else:
            print("   ❌ 응답 없음")
    
    # 3. 원시 데이터 확인
    print("\\n3. 원시 시리얼 데이터 확인...")
    try:
        if port_manager._serial_connection and port_manager._serial_connection.is_open:
            # 직접 시리얼 포트 접근
            serial_conn = port_manager._serial_connection
            
            # 버퍼에 남은 데이터 확인
            waiting = serial_conn.in_waiting
            print(f"   대기 중인 바이트: {waiting}")
            
            if waiting > 0:
                raw_data = serial_conn.read(waiting)
                print(f"   원시 데이터: {raw_data}")
                try:
                    decoded = raw_data.decode('utf-8')
                    print(f"   디코딩된 데이터: '{decoded}'")
                except UnicodeDecodeError as e:
                    print(f"   디코딩 오류: {e}")
    
    except Exception as e:
        print(f"   원시 데이터 확인 오류: {e}")
    
    # 연결 해제
    print("\\n연결 해제...")
    port_manager.disconnect()
    print("디버깅 완료")

if __name__ == "__main__":
    try:
        debug_serial_communication()
    except Exception as e:
        print(f"❌ 디버깅 중 오류: {e}")
        import traceback
        traceback.print_exc()
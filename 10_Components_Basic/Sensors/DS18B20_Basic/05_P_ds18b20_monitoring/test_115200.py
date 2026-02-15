"""115200 bps로 Arduino 연결 테스트"""
import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager

def test_115200_connection():
    """115200 bps로 연결 및 통신 테스트"""
    print("=== 115200 bps Arduino 연결 테스트 ===")
    
    # 1. 포트 스캔
    ports = port_manager.scan_ports()
    print(f"사용 가능한 포트: {ports}")
    
    if not ports:
        print("❌ 포트 없음")
        return
    
    port = ports[0]
    print(f"\\n{port} 연결 시도 (115200 bps)...")
    
    # 2. 연결 시도
    success = port_manager.connect(port)
    print(f"연결 결과: {'✅ 성공' if success else '❌ 실패'}")
    
    if not success:
        print(f"오류: {port_manager.get_status().last_error}")
        return
    
    # 3. 초기 데이터 확인
    print("\\n초기 Arduino 출력 확인...")
    time.sleep(2)
    initial_data = port_manager.read_data(timeout=1.0)
    if initial_data:
        print(f"초기 메시지: '{initial_data}'")
    
    # 4. 통신 테스트
    print("\\nping-pong 테스트...")
    comm_result = port_manager.test_communication()
    print(f"통신 결과: {'✅ 성공' if comm_result else '❌ 실패'}")
    
    # 5. 수동 ping 테스트
    if not comm_result:
        print("\\n수동 통신 테스트...")
        send_ok = port_manager.send_data("ping\\n")
        print(f"ping 전송: {'✅' if send_ok else '❌'}")
        
        if send_ok:
            response = port_manager.read_data(timeout=3.0)
            print(f"응답: '{response}'" if response else "응답 없음")
    
    # 6. 현재 Arduino 프로그램 확인
    print("\\nArduino 현재 상태 확인...")
    try:
        # 몇 개의 명령 시도해보기
        test_commands = ["help", "status", "info", "version", "?"]
        
        for cmd in test_commands:
            print(f"'{cmd}' 명령 시도...")
            port_manager.send_data(f"{cmd}\\n")
            time.sleep(1)
            response = port_manager.read_data(timeout=2.0)
            if response:
                print(f"  응답: '{response}'")
                break
        else:
            print("  알려진 명령에 응답하지 않음")
            
    except Exception as e:
        print(f"테스트 중 오류: {e}")
    
    # 연결 해제
    print("\\n연결 해제...")
    port_manager.disconnect()
    print("테스트 완료")

if __name__ == "__main__":
    test_115200_connection()
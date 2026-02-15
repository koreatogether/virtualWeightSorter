"""Arduino 연결 기능 테스트 스크립트"""
import sys
import os

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager
from src.python.services.diagnostics import diagnose_connection_failure, diagnose_sensor_communication

def test_port_scan():
    """포트 스캔 테스트"""
    print("=== 포트 스캔 테스트 ===")
    ports = port_manager.scan_ports()
    print(f"사용 가능한 포트: {ports}")
    return ports

def test_connection(port):
    """연결 테스트"""
    print(f"\n=== {port} 연결 테스트 ===")
    
    # 연결 시도
    success = port_manager.connect(port)
    status = port_manager.get_status()
    
    print(f"연결 결과: {success}")
    print(f"연결 상태: {status}")
    
    if success:
        print("통신 테스트 중...")
        comm_result = port_manager.test_communication()
        print(f"통신 결과: {comm_result}")
        
        # 진단 테스트
        diagnostic = diagnose_sensor_communication(port)
        print(f"진단 결과: {diagnostic}")
    else:
        print("연결 실패 진단 중...")
        diagnostic = diagnose_connection_failure(port)
        print(f"진단 결과: {diagnostic}")
    
    return success

def test_disconnect():
    """연결 해제 테스트"""
    print("\n=== 연결 해제 테스트 ===")
    port_manager.disconnect()
    status = port_manager.get_status()
    print(f"해제 후 상태: {status}")

def main():
    """메인 테스트 실행"""
    try:
        # 1. 포트 스캔
        ports = test_port_scan()
        
        if not ports:
            print("❌ 사용 가능한 포트가 없습니다.")
            return
        
        # 2. 첫 번째 포트로 연결 테스트
        test_port = ports[0]
        success = test_connection(test_port)
        
        # 3. 연결된 경우 해제 테스트
        if success:
            test_disconnect()
        
        print("\n=== 테스트 완료 ===")
        
    except Exception as e:
        print(f"❌ 테스트 중 오류 발생: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
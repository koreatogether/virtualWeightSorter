"""간단한 대시보드 상태 체크"""
import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager

def simple_status_check():
    """간단한 상태 체크"""
    print("=== Dashboard Status Check ===")
    
    # 현재 연결 상태
    is_connected = port_manager.is_connected()
    current_port = port_manager.get_current_port()
    status = port_manager.get_status()
    
    print(f"Connected: {is_connected}")
    print(f"Port: {current_port}")
    print(f"Baudrate: {status.baudrate}")
    
    if is_connected:
        print("\\nTesting sensor data reception...")
        
        # 5개 센서 데이터 수신 테스트
        sensor_count = 0
        for i in range(10):  # 10번 시도
            data = port_manager.read_sensor_data(timeout=1.5)
            if data:
                temp = data.get('temperature')
                addr = data.get('sensor_addr', 'Unknown')
                print(f"  Data {i+1}: {temp}C from {addr[:16]}")
                sensor_count += 1
                if sensor_count >= 5:
                    break
            time.sleep(0.5)
        
        print(f"\\nReceived data from {sensor_count} sensors")
        
        # 통신 테스트
        comm_test = port_manager.test_communication()
        print(f"Communication test: {comm_test}")
        
    else:
        print("Not connected - cannot test sensor data")
    
    print("\\nCheck complete!")

if __name__ == "__main__":
    simple_status_check()
"""JSON 센서 데이터 통신 테스트"""
import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager

def test_json_communication():
    """JSON 센서 데이터 통신 테스트"""
    print("=== JSON 센서 데이터 통신 테스트 ===")
    
    # 연결
    success = port_manager.connect("COM4")
    print(f"Connection: {success}")
    
    if not success:
        print("Connection failed")
        return
    
    # 통신 테스트 (JSON 데이터 확인)
    print("\\nTesting communication...")
    comm_result = port_manager.test_communication()
    print(f"JSON Communication: {comm_result}")
    
    if comm_result:
        print("\\n=== Reading sensor data ===")
        # 여러 센서 데이터 읽기
        for i in range(10):
            data = port_manager.read_sensor_data(timeout=3.0)
            if data:
                temp = data.get('temperature')
                addr = data.get('sensor_addr', 'Unknown')[:16]  # 앞부분만
                user_id = data.get('user_sensor_id')
                print(f"Sensor {i+1}: {temp}C, Addr: {addr}, ID: {user_id}")
            else:
                print(f"Sensor {i+1}: No data")
            
            time.sleep(0.5)
    
    # 연결 해제
    print("\\nDisconnecting...")
    port_manager.disconnect()
    print("Test complete!")

if __name__ == "__main__":
    test_json_communication()
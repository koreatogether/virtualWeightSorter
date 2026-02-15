"""센서 정렬 로직 테스트"""
import sys
import os

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.sensor_data_manager import SensorDataManager

def test_sensor_sorting():
    """센서 정렬 테스트"""
    print("=== DS18B20 센서 정렬 테스트 ===")
    
    manager = SensorDataManager()
    
    # 테스트 데이터 (다양한 우선순위)
    test_sensors = [
        # 2순위: EEPROM 미설정 (sensor_id: 00)
        {"type": "sensor_data", "temperature": 28.5, "sensor_addr": "285882840000000E", "sensor_id": "00", "user_sensor_id": 1281, "th_value": 30, "tl_value": 15, "timestamp": 1000},
        {"type": "sensor_data", "temperature": 28.1, "sensor_addr": "285C82850000005D", "sensor_id": "00", "user_sensor_id": 1026, "th_value": 30, "tl_value": 15, "timestamp": 1001},
        {"type": "sensor_data", "temperature": 28.2, "sensor_addr": "28E6AA830000005A", "sensor_id": "00", "user_sensor_id": 771, "th_value": 30, "tl_value": 15, "timestamp": 1002},
        
        # 1순위: EEPROM 설정됨 (sensor_id: 01-08)
        {"type": "sensor_data", "temperature": 29.5, "sensor_addr": "28FF641F43B82384", "sensor_id": "03", "user_sensor_id": 517, "th_value": 30, "tl_value": 15, "timestamp": 1003},
        {"type": "sensor_data", "temperature": 27.8, "sensor_addr": "28E79B850000002D", "sensor_id": "01", "user_sensor_id": 260, "th_value": 30, "tl_value": 15, "timestamp": 1004},
        {"type": "sensor_data", "temperature": 30.1, "sensor_addr": "28AA1B2C3D4E5F60", "sensor_id": "05", "user_sensor_id": 100, "th_value": 30, "tl_value": 15, "timestamp": 1005},
    ]
    
    # 센서 데이터 추가
    print("\\n테스트 데이터 추가...")
    for sensor_data in test_sensors:
        manager.add_sensor_data(sensor_data)
        print(f"  추가: ID={sensor_data['sensor_id']}, Addr={sensor_data['sensor_addr'][:16]}, Temp={sensor_data['temperature']}°C")
    
    # 정렬 결과 확인
    print(f"\\n=== 정렬 결과 (총 {manager.get_sensor_count()}개 센서) ===")
    sorted_sensors = manager.get_sorted_sensors()
    
    for i, sensor in enumerate(sorted_sensors, 1):
        group_name = ["", "설정됨", "미설정", "오류"][sensor.priority_group]
        print(f"{i}. [{group_name}] ID:{sensor.sensor_id}, {sensor.temperature}°C, Addr:{sensor.sensor_addr[:16]}")
    
    # 그룹별 분류 확인
    print(f"\\n=== 그룹별 분류 ===")
    groups = manager.get_sensors_by_group()
    for group_num, sensors in groups.items():
        if sensors:
            group_name = ["", "EEPROM 설정됨", "EEPROM 미설정", "오류"][group_num]
            print(f"{group_name}: {len(sensors)}개")
            for sensor in sensors:
                print(f"  - ID:{sensor.sensor_id}, Addr:{sensor.sensor_addr[:16]}")
    
    # 대시보드 표시용 정보
    print(f"\\n=== 대시보드 표시용 (최대 8개) ===")
    display_info = manager.get_display_info(max_sensors=8)
    for i, info in enumerate(display_info, 1):
        print(f"{i}. ID:{info['sensor_id']} ({info['group_name']}) {info['temperature']}°C")
        print(f"   주소: {info['formatted_address']}")
    
    # 요약 정보
    print(f"\\n=== 센서 현황 요약 ===")
    summary = manager.get_summary()
    print(f"전체: {summary['total_sensors']}개")
    print(f"설정됨: {summary['configured_sensors']}개")
    print(f"미설정: {summary['unconfigured_sensors']}개")
    print(f"오류: {summary['error_sensors']}개")

if __name__ == "__main__":
    test_sensor_sorting()
#!/usr/bin/env python3
"""데이터 내보내기 기능 테스트 스크립트"""

import random
import sys
from datetime import datetime, timedelta
from pathlib import Path

# Add src/python to path
current_dir = Path(__file__).parent
src_python_dir = current_dir / "src" / "python"
sys.path.insert(0, str(src_python_dir))

from dashboard.data_manager import DataManager


def generate_test_data(data_manager: DataManager, count: int = 50):
    """테스트용 센서 데이터 생성"""
    print(f"테스트 데이터 {count}개 생성 중...")

    base_time = datetime.now() - timedelta(hours=2)
    base_temp = 25.0

    for i in range(count):
        # 온도 변화 시뮬레이션 (24-28도 범위)
        temp_variation = random.uniform(-1.0, 1.0)
        temperature = base_temp + temp_variation + (i % 10) * 0.1

        # 시간 증가
        timestamp = base_time + timedelta(minutes=i * 2)

        # 센서 데이터 생성
        sensor_data = {
            "sensor_id": "28FF64AC5B4F001A",
            "temperature": round(temperature, 1),
            "th_value": 30.0,
            "tl_value": 20.0,
            "measurement_interval": 1000,
            "timestamp": timestamp.isoformat(),
        }

        data_manager.add_sensor_data(sensor_data)

    print(f"테스트 데이터 생성 완료: {len(data_manager.sensor_data)}개")


def test_export_features(data_manager: DataManager):
    """내보내기 기능 테스트"""
    print("\n=== 데이터 내보내기 기능 테스트 ===")

    # 테스트 파일 경로
    test_dir = Path("test_exports")
    test_dir.mkdir(exist_ok=True)

    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')

    # CSV 내보내기 테스트
    csv_file = test_dir / f"test_export_{timestamp}.csv"
    print(f"\n1. CSV 내보내기 테스트: {csv_file}")
    success = data_manager.export_data(str(csv_file), "csv")
    print(f"   결과: {'성공' if success else '실패'}")
    if csv_file.exists():
        print(f"   파일 크기: {csv_file.stat().st_size} 바이트")

    # JSON 내보내기 테스트
    json_file = test_dir / f"test_export_{timestamp}.json"
    print(f"\n2. JSON 내보내기 테스트: {json_file}")
    success = data_manager.export_data(str(json_file), "json")
    print(f"   결과: {'성공' if success else '실패'}")
    if json_file.exists():
        print(f"   파일 크기: {json_file.stat().st_size} 바이트")

    # 통계 출력
    stats = data_manager.get_statistics()
    print("\n3. 데이터 통계:")
    print(f"   총 데이터 수: {stats.get('total_data_count', 0)}")
    print(f"   최소 온도: {stats.get('min_temperature', 'N/A')}°C")
    print(f"   최대 온도: {stats.get('max_temperature', 'N/A')}°C")
    print(f"   평균 온도: {stats.get('avg_temperature', 'N/A')}°C")

    return test_dir


def main():
    """메인 테스트 함수"""
    print("DS18B20 데이터 내보내기 기능 테스트")
    print("=" * 50)

    # DataManager 초기화
    data_manager = DataManager(max_data_points=1000)

    # 테스트 데이터 생성
    generate_test_data(data_manager, 50)

    # 내보내기 기능 테스트
    test_dir = test_export_features(data_manager)

    print("\n=== 테스트 완료 ===")
    print(f"생성된 파일들은 {test_dir} 폴더에서 확인할 수 있습니다.")
    print("대시보드를 실행하여 '데이터 내보내기' 버튼을 테스트해보세요.")


if __name__ == "__main__":
    main()

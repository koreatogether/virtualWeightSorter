#!/usr/bin/env python3
"""수정된 통합 테스트"""

# Set UTF-8 encoding for console output
import io
import sys
import time
from pathlib import Path

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Add src/python to Python path
current_dir = Path(__file__).parent
src_python_dir = current_dir / "src" / "python"
sys.path.insert(0, str(src_python_dir))

from dashboard.data_manager import DataManager
from dashboard.serial_handler_fixed import SerialHandler


def main():
    print("수정된 통합 시스템 테스트")
    print("=" * 40)

    # 데이터 매니저 생성
    data_manager = DataManager()

    def on_data_received(data):
        """Arduino에서 데이터 수신 시 처리"""
        try:
            print(f"[수신] 온도: {data['temperature']}°C, 센서: {data.get('sensor_addr')}")
            print(f"      타임스탬프: {data.get('timestamp')} (타입: {type(data.get('timestamp'))})")

            # 데이터 매니저에 추가
            data_manager.add_sensor_data(data)

            # 최신 데이터 확인
            latest = data_manager.get_latest_data()
            if latest:
                print(f"[저장] 센서ID: {latest['sensor_id']}, 온도: {latest['temperature']}°C")
                print(f"      저장된 타임스탬프: {latest['timestamp']}")

        except Exception as e:
            print(f"[오류] 데이터 처리 중 예외: {e}")
            import traceback
            traceback.print_exc()

    def on_response_received(response):
        print(f"[응답] {response.get('status')} - {response.get('message')}")

    def on_error(error_msg):
        print(f"[오류] {error_msg}")

    # 시리얼 핸들러 생성 및 설정
    handler = SerialHandler(port="COM4", baudrate=115200)
    handler.set_callbacks(
        data_callback=on_data_received,
        response_callback=on_response_received,
        error_callback=on_error
    )

    try:
        print("COM4 포트 연결 시도...")
        if handler.connect():
            print("포트 연결 성공!")

            if handler.start_reading():
                print("데이터 읽기 시작")
                print("15초간 데이터를 수신하고 그래프 데이터를 확인합니다...")
                print("-" * 40)

                # 15초간 데이터 수신 및 저장
                for i in range(15):
                    time.sleep(1)

                    # 데이터 매니저 상태 확인
                    try:
                        graph_data = data_manager.get_graph_data(10)  # 최근 10개
                        latest_data = data_manager.get_latest_data()
                        stats = data_manager.get_statistics()

                        print(f"[{i+1:2d}초] 그래프 데이터: {len(graph_data.get('timestamps', []))}개")

                        if latest_data:
                            print(f"      최신: {latest_data['temperature']}°C @ {latest_data['timestamp']}")

                        if i == 5:  # 중간에 통계 출력
                            print(f"      통계: 평균={stats.get('avg_temp', 0):.1f}°C, "
                                  f"최고={stats.get('max_temp', 0):.1f}°C, "
                                  f"최저={stats.get('min_temp', 0):.1f}°C")

                    except Exception as e:
                        print(f"      데이터 매니저 오류: {e}")

                print("-" * 40)
                print("최종 결과:")

                try:
                    graph_data = data_manager.get_graph_data()
                    latest_data = data_manager.get_latest_data()
                    stats = data_manager.get_statistics()

                    print(f"총 수집된 데이터: {len(graph_data.get('timestamps', []))}개")
                    print(f"온도 범위: {stats.get('min_temp', 0):.1f}°C ~ {stats.get('max_temp', 0):.1f}°C")
                    print(f"평균 온도: {stats.get('avg_temp', 0):.1f}°C")

                    if latest_data:
                        print(f"마지막 데이터: {latest_data['temperature']}°C")
                        print(f"센서 ID: {latest_data['sensor_id']}")

                    # 그래프 데이터 샘플 출력
                    if graph_data.get('timestamps'):
                        print("그래프 데이터 샘플:")
                        timestamps = graph_data['timestamps']
                        temperatures = graph_data['temperatures']
                        for i in range(min(3, len(timestamps))):
                            print(f"  {i+1}. {timestamps[i]} -> {temperatures[i]}°C")

                    print("✅ 통합 테스트 성공!")

                except Exception as e:
                    print(f"❌ 최종 결과 처리 중 오류: {e}")
                    import traceback
                    traceback.print_exc()

            else:
                print("데이터 읽기 시작 실패")
        else:
            print("포트 연결 실패")

    except KeyboardInterrupt:
        print("\n사용자에 의해 테스트 중단")
    except Exception as e:
        print(f"예외 발생: {e}")
        import traceback
        traceback.print_exc()
    finally:
        print("연결 종료...")
        handler.stop_reading()
        handler.disconnect()
        print("테스트 종료")

if __name__ == "__main__":
    main()

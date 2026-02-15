#!/usr/bin/env python3
"""대시보드 연결 시뮬레이션 테스트"""

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


def simulate_dashboard_connection():
    print("웹 대시보드 연결 시뮬레이션")
    print("=" * 40)

    # 대시보드와 동일한 구조로 설정
    data_manager = DataManager()

    def on_data_received(data):
        """대시보드와 동일한 데이터 처리"""
        try:
            data_manager.add_sensor_data(data)
            print(f"[대시보드] 데이터 저장됨: {data['temperature']}°C")

            # 그래프 데이터 확인
            graph_data = data_manager.get_graph_data(5)
            print(f"[대시보드] 그래프 데이터: {len(graph_data.get('timestamps', []))}개")

        except Exception as e:
            print(f"[대시보드] 데이터 처리 오류: {e}")

    def on_response_received(response):
        print(f"[대시보드] Arduino 응답: {response.get('status')} - {response.get('message')}")

    def on_error(error_msg):
        print(f"[대시보드] 오류: {error_msg}")

    # 웹 대시보드와 동일한 시리얼 핸들러 설정
    serial_handler = SerialHandler(port="COM4", baudrate=115200)
    serial_handler.set_callbacks(
        data_callback=on_data_received,
        response_callback=on_response_received,
        error_callback=on_error
    )

    try:
        print("웹 대시보드처럼 COM4 연결 시도...")

        # 웹 대시보드의 연결 로직 시뮬레이션
        connected = serial_handler.connect("COM4")
        print(f"연결 결과: {connected}")

        if connected:
            print("✅ 연결 성공! 데이터 읽기 시작...")

            started = serial_handler.start_reading()
            print(f"읽기 시작 결과: {started}")

            if started:
                print("10초간 데이터 수신 및 대시보드 시뮬레이션...")
                print("-" * 40)

                for i in range(10):
                    time.sleep(1)

                    # 연결 상태 및 데이터 확인
                    status = serial_handler.get_connection_status()
                    graph_data = data_manager.get_graph_data()
                    latest = data_manager.get_latest_data()

                    print(f"[{i+1:2d}초] 연결: {status['is_connected']}, "
                          f"큐: {status['data_queue_size']}, "
                          f"그래프: {len(graph_data.get('timestamps', []))}개")

                    if latest:
                        print(f"      최신 온도: {latest['temperature']}°C")

                print("-" * 40)
                print("✅ 대시보드 연결 시뮬레이션 성공!")

                # 최종 상태 확인
                graph_data = data_manager.get_graph_data()
                stats = data_manager.get_statistics()
                print(f"최종 그래프 데이터: {len(graph_data.get('timestamps', []))}개")
                print(f"온도 통계: 평균 {stats.get('avg_temperature', 0)}°C")

            else:
                print("❌ 데이터 읽기 시작 실패")
        else:
            print("❌ COM4 연결 실패")

    except KeyboardInterrupt:
        print("\n사용자 중단")
    except Exception as e:
        print(f"예외 발생: {e}")
        import traceback
        traceback.print_exc()
    finally:
        print("연결 정리...")
        serial_handler.stop_reading()
        serial_handler.disconnect()
        print("시뮬레이션 종료")

if __name__ == "__main__":
    simulate_dashboard_connection()

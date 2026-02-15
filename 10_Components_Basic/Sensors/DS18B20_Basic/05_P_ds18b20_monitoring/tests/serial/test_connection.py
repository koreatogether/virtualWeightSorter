#!/usr/bin/env python3
"""대시보드 연결 테스트"""

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
    print("대시보드 연결 기능 테스트")
    print("=" * 40)

    # 데이터 매니저 생성
    data_manager = DataManager()

    def on_data_received(data):
        """Arduino에서 데이터 수신 시 처리"""
        try:
            data_manager.add_sensor_data(data)
            print(f"데이터 추가됨: {data['temperature']}°C (센서: {data.get('sensor_addr', 'N/A')})")

            # 현재 데이터 상태 출력
            data_manager.get_recent_data(1)
            graph_data = data_manager.get_graph_data()
            print(f"저장된 데이터 개수: {len(graph_data.get('timestamps', []))}")

        except Exception as e:
            print(f"데이터 처리 오류: {e}")

    def on_response_received(response):
        print(f"Arduino 응답: {response.get('status')} - {response.get('message')}")

    def on_error(error_msg):
        print(f"연결 오류: {error_msg}")

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
                print("15초간 Arduino 데이터를 수신하고 데이터 매니저에 저장...")
                print("-" * 40)

                # 15초간 데이터 수신 및 저장
                for i in range(15):
                    time.sleep(1)
                    status = handler.get_connection_status()
                    graph_data = data_manager.get_graph_data()
                    recent_data = data_manager.get_recent_data(5)

                    print(f"[{i+1:2d}초] 큐: {status['data_queue_size']}, 총 데이터: {len(graph_data.get('timestamps', []))}, 최근 5개: {len(recent_data)}")

                    # 최근 데이터 중 하나 출력
                    if recent_data:
                        latest = recent_data[-1]
                        print(f"      최신 온도: {latest['temperature']}°C @ {latest['timestamp'].strftime('%H:%M:%S')}")

                print("-" * 40)
                print("최종 결과:")
                graph_data = data_manager.get_graph_data()
                latest_data = data_manager.get_latest_data()
                stats = data_manager.get_statistics()

                print(f"총 수집된 데이터: {len(graph_data.get('timestamps', []))}개")

                if latest_data:
                    print(f"마지막 온도: {latest_data['temperature']}°C")
                    print(f"센서 주소: {latest_data.get('sensor_addr', 'N/A')}")
                    print(f"수신 시각: {latest_data['timestamp'].strftime('%Y-%m-%d %H:%M:%S')}")

                print(f"통계 정보: {stats}")

                print("테스트 성공!")

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

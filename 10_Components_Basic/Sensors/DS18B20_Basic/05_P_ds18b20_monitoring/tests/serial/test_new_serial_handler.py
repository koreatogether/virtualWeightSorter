#!/usr/bin/env python3
"""새 시리얼 핸들러 테스트"""

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

from dashboard.serial_handler_fixed import SerialHandler


def main():
    print("새로운 시리얼 핸들러 테스트")
    print("=" * 40)

    def on_data_received(data):
        print(f"센서 데이터 수신: {data.get('temperature')}°C")
        print(f"   센서 주소: {data.get('sensor_addr')}")
        print(f"   타임스탬프: {data.get('timestamp')}")

    def on_response_received(response):
        print(f"응답 수신: {response.get('status')} - {response.get('message')}")

    def on_error(error_msg):
        print(f"오류: {error_msg}")

    # 시리얼 핸들러 생성
    handler = SerialHandler(port="COM4", baudrate=115200)
    handler.set_callbacks(
        data_callback=on_data_received,
        response_callback=on_response_received,
        error_callback=on_error
    )

    try:
        print("COM4 포트 연결 시도...")
        if handler.connect():
            print("COM4 포트 연결 성공!")

            if handler.start_reading():
                print("데이터 읽기 시작")
                print("10초간 Arduino 데이터를 수신합니다...")
                print("-" * 40)

                # 10초간 데이터 수신
                for i in range(10):
                    time.sleep(1)
                    status = handler.get_connection_status()
                    print(f"[{i+1:2d}초] 연결: {status['is_connected']}, 큐 크기: {status['data_queue_size']}")

                print("-" * 40)
                print("테스트 완료!")

            else:
                print("데이터 읽기 시작 실패")
        else:
            print("COM4 포트 연결 실패")

    except KeyboardInterrupt:
        print("\n사용자에 의해 테스트 중단")
    except Exception as e:
        print(f"예외 발생: {e}")
    finally:
        print("연결 종료 중...")
        handler.stop_reading()
        handler.disconnect()
        print("테스트 종료")

if __name__ == "__main__":
    main()

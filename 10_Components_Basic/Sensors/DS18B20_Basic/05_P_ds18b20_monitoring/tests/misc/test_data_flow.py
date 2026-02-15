#!/usr/bin/env python3
"""Arduino 데이터 플로우 테스트
실제 Arduino에서 대시보드까지 데이터가 제대로 흐르는지 확인
"""

import json
import sys
import time

import serial

# dashboard 모듈 경로 추가
sys.path.append('src/python/dashboard')

from data_manager import data_manager
from serial_handler_fixed import SerialHandler


def test_direct_serial_communication():
    """COM4에서 직접 데이터 수신 테스트"""
    print("=== 직접 시리얼 통신 테스트 ===")

    try:
        # 직접 시리얼 연결
        with serial.Serial('COM4', 115200, timeout=2) as ser:
            print("OK COM4 연결 성공")

            # 버퍼 클리어
            ser.reset_input_buffer()
            ser.reset_output_buffer()

            print("Arduino에서 데이터 수신 대기 중...")

            for i in range(10):  # 10번 시도
                if ser.in_waiting > 0:
                    data = ser.read(ser.in_waiting)
                    try:
                        text = data.decode('utf-8', errors='ignore')
                        print(f"DATA 원시 데이터: {text.strip()}")

                        # JSON 파싱 시도
                        lines = text.strip().split('\n')
                        for line in lines:
                            line = line.strip()
                            if line.startswith('{') and line.endswith('}'):
                                try:
                                    json_data = json.loads(line)
                                    print(f"OK JSON 파싱 성공: {json_data}")

                                    if json_data.get('type') == 'sensor_data':
                                        print(f"TEMP 온도 데이터: {json_data.get('temperature')}°C")
                                        print(f"SENSOR 센서 주소: {json_data.get('sensor_addr')}")
                                        return json_data

                                except json.JSONDecodeError as e:
                                    print(f"ERROR JSON 파싱 실패: {e}")
                    except UnicodeDecodeError as e:
                        print(f"ERROR 텍스트 디코딩 실패: {e}")

                time.sleep(1)
                print(f"대기 중... ({i+1}/10)")

            print("ERROR 10초 동안 데이터 수신 없음")
            return None

    except Exception as e:
        print(f"ERROR 시리얼 연결 실패: {e}")
        return None

def test_serial_handler():
    """SerialHandler를 통한 데이터 수신 테스트"""
    print("\n=== SerialHandler 테스트 ===")

    # 데이터 수신 콜백
    received_data = []

    def on_data_received(data):
        print(f"DATA SerialHandler 데이터 수신: {data}")
        received_data.append(data)

        # data_manager에 추가
        try:
            data_manager.add_sensor_data(data)
            print("OK data_manager에 데이터 추가 성공")

            # 최신 데이터 확인
            latest = data_manager.get_latest_data()
            if latest:
                print(f"STATS data_manager 최신 데이터: {latest['temperature']}°C")
            else:
                print("ERROR data_manager에서 최신 데이터 없음")

        except Exception as e:
            print(f"ERROR data_manager 데이터 추가 실패: {e}")

    def on_error(msg):
        print(f"ERROR SerialHandler 오류: {msg}")

    # SerialHandler 초기화
    handler = SerialHandler('COM4', 115200)
    handler.set_callbacks(
        data_callback=on_data_received,
        error_callback=on_error
    )

    try:
        # 연결 및 읽기 시작
        if handler.connect():
            print("OK SerialHandler 연결 성공")

            if handler.start_reading():
                print("OK 데이터 읽기 시작")

                # 15초 대기
                print("15초 동안 데이터 수신 대기...")
                time.sleep(15)

                print(f"STATS 총 {len(received_data)}개 데이터 수신됨")

                # data_manager 통계 확인
                stats = data_manager.get_statistics()
                print(f"STATS data_manager 통계: {stats}")

                return len(received_data) > 0
            else:
                print("ERROR 데이터 읽기 시작 실패")
                return False
        else:
            print("ERROR SerialHandler 연결 실패")
            return False

    finally:
        handler.stop_reading()
        handler.disconnect()

def test_data_manager():
    """data_manager 기능 테스트"""
    print("\n=== data_manager 기능 테스트 ===")

    # 현재 상태 확인
    latest = data_manager.get_latest_data()
    if latest:
        print(f"STATS 최신 데이터: {latest}")
    else:
        print("ERROR 저장된 데이터 없음")

    stats = data_manager.get_statistics()
    print(f"STATS 통계: {stats}")

    # 그래프 데이터 확인
    graph_data = data_manager.get_graph_data(limit=10)
    print(f"GRAPH 그래프 데이터 포인트 수: {len(graph_data['temperatures'])}")

    return latest is not None

def main():
    print("Arduino → Dashboard 데이터 플로우 테스트")
    print("=" * 50)

    # 1. 직접 시리얼 통신 테스트
    direct_data = test_direct_serial_communication()

    # 2. SerialHandler 테스트
    handler_success = test_serial_handler()

    # 3. data_manager 테스트
    manager_success = test_data_manager()

    print("\n" + "=" * 50)
    print("테스트 결과 요약:")
    print(f"SERIAL 직접 시리얼 통신: {'OK 성공' if direct_data else 'ERROR 실패'}")
    print(f"HANDLER SerialHandler: {'OK 성공' if handler_success else 'ERROR 실패'}")
    print(f"MANAGER data_manager: {'OK 성공' if manager_success else 'ERROR 실패'}")

    if direct_data and not handler_success:
        print("\nDIAG 진단: Arduino는 데이터를 보내지만 SerialHandler가 처리하지 못함")
    elif handler_success and not manager_success:
        print("\nDIAG 진단: SerialHandler는 작동하지만 data_manager에 저장되지 않음")
    elif not direct_data:
        print("\nDIAG 진단: Arduino에서 데이터를 보내지 않거나 COM4 연결 문제")
    else:
        print("\nDIAG 진단: 모든 구성요소가 정상 작동함")

if __name__ == "__main__":
    main()

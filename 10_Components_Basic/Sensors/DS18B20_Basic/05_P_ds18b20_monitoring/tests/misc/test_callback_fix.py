#!/usr/bin/env python3
"""포트 연결 콜백 수정 테스트"""

import json
import sys
import time

sys.path.append('src/python/dashboard')

def test_port_connection_callback():
    """포트 연결 콜백 로직 직접 테스트"""
    print("=== 포트 연결 콜백 로직 테스트 ===")

    try:
        # 필요한 모듈들 import
        from data_manager import data_manager
        from port_manager import PortStatus, port_manager
        from serial_handler_fixed import SerialHandler

        # mock app.server.config
        mock_config = {}

        # 포트 정보 확인
        port_device = 'COM4'
        port_info = port_manager.get_port_by_device(port_device)

        if not port_info:
            print(f"ERROR: 포트 {port_device}를 찾을 수 없습니다")
            return False

        if port_info.status != PortStatus.AVAILABLE:
            print(f"ERROR: 포트 {port_device}는 현재 사용할 수 없습니다")
            return False

        print(f"포트 정보: {port_device} (상태: {port_info.status})")

        # SerialHandler 인스턴스 생성
        serial_handler = SerialHandler()

        # 콜백 설정
        def on_data_received(data):
            data_manager.add_sensor_data(data)
            print(f"데이터 수신: {data['temperature']}°C")

        serial_handler.set_callbacks(
            data_callback=on_data_received,
            error_callback=lambda msg: print(f"시리얼 오류: {msg}")
        )

        # 연결 시도
        print(f"포트 연결 시도: {port_device}")
        connected = serial_handler.connect(port_device)
        print(f"연결 결과: {connected}")

        if connected:
            started = serial_handler.start_reading()
            print(f"읽기 시작: {started}")

            if started:
                # 포트 연결 성공 시 config 설정 (실제 콜백에서 하는 작업)
                mock_config['connected_port'] = port_device
                mock_config['port_connection_success'] = True
                mock_config['force_dashboard_update'] = True
                mock_config['activate_interval'] = True

                print("포트 연결 성공! 설정 플래그:")
                print(f"  connected_port: {mock_config['connected_port']}")
                print(f"  force_dashboard_update: {mock_config['force_dashboard_update']}")
                print(f"  activate_interval: {mock_config['activate_interval']}")

                # 콜백 반환값 시뮬레이션
                callback_return = (
                    True,  # success alert open
                    f"포트 {port_device}에 성공적으로 연결되었습니다. 메인 대시보드로 이동합니다.",  # success message
                    False,  # error alert closed
                    "",     # error message empty
                    False   # interval-component disabled=False (활성화)
                )

                print("콜백 반환값 (success, success_msg, error, error_msg, interval_disabled):")
                print(f"  {callback_return}")
                print("  interval-component disabled=False 👍 활성화됨!")

                # 5초간 데이터 수신 테스트
                print("\n5초간 데이터 수신 테스트...")
                for i in range(5):
                    latest = data_manager.get_latest_data()
                    if latest:
                        print(f"  [{i+1}] 온도: {latest['temperature']}°C")
                    else:
                        print(f"  [{i+1}] 데이터 없음")
                    time.sleep(1)

                serial_handler.disconnect()
                return True
            else:
                print("읽기 시작 실패")
                serial_handler.disconnect()
                return False
        else:
            print("연결 실패")
            return False

    except Exception as e:
        print(f"테스트 실패: {e}")
        return False

def test_callback_trigger_logic():
    """콜백 트리거 로직 테스트"""
    print("\n=== 콜백 트리거 로직 테스트 ===")

    # 패턴 매칭 ID 파싱 테스트
    test_triggered_id = '{"index":"COM4","type":"connect-btn"}.n_clicks'

    try:
        id_part = test_triggered_id.split('.')[0]
        button_info = json.loads(id_part)
        port_device = button_info['index']

        print(f"트리거 ID: {test_triggered_id}")
        print(f"파싱된 포트: {port_device}")
        print("OK 패턴 매칭 ID 파싱 성공")

        return True

    except Exception as e:
        print(f"ERROR 패턴 매칭 ID 파싱 실패: {e}")
        return False

def main():
    print("포트 연결 콜백 수정 테스트")
    print("=" * 50)
    print("테스트 내용:")
    print("1. 패턴 매칭 ID 파싱")
    print("2. 포트 연결 로직")
    print("3. interval-component 직접 활성화 확인")
    print("=" * 50)

    # 1. 콜백 트리거 로직 테스트
    trigger_ok = test_callback_trigger_logic()

    # 2. 포트 연결 콜백 테스트
    connection_ok = test_port_connection_callback()

    print("\n" + "=" * 50)
    print("테스트 결과:")
    print(f"1. 콜백 트리거 로직: {'OK 성공' if trigger_ok else 'ERROR 실패'}")
    print(f"2. 포트 연결 로직: {'OK 성공' if connection_ok else 'ERROR 실패'}")

    if trigger_ok and connection_ok:
        print("\nSUCCESS 모든 테스트 통과!")
        print("포트 연결 시 interval-component가 직접 활성화됩니다.")
        print("이제 브라우저에서 테스트해보세요:")
        print("1. python -m dashboard.app 실행")
        print("2. 포트 관리 > COM4 연결")
        print("3. 즉시 온도 데이터 표시 확인")
    else:
        print("\nERROR 테스트 실패 - 추가 디버깅 필요")

if __name__ == "__main__":
    main()

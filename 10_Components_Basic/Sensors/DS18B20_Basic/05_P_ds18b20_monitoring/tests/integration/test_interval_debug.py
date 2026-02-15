#!/usr/bin/env python3
"""Interval-component 활성화 디버깅 도구"""

import subprocess
import sys
import time

sys.path.append('src/python/dashboard')

def test_connection_logic():
    """포트 연결 로직 직접 테스트"""
    print("=== 포트 연결 로직 직접 테스트 ===")

    try:
        # dashboard 모듈 로드
        from data_manager import data_manager
        from serial_handler_fixed import SerialHandler

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

        # COM4 연결 시도
        print("COM4 연결 시도...")
        connected = serial_handler.connect('COM4')
        print(f"연결 결과: {connected}")

        if connected:
            started = serial_handler.start_reading()
            print(f"읽기 시작: {started}")

            if started:
                print("10초 동안 데이터 수신 테스트...")
                for i in range(10):
                    latest = data_manager.get_latest_data()
                    stats = data_manager.get_statistics()

                    if latest:
                        print(f"[{i+1}] 온도: {latest['temperature']}°C | 총 데이터: {stats['total_data_count']}")
                    else:
                        print(f"[{i+1}] 데이터 없음 | 총 데이터: {stats['total_data_count']}")

                    time.sleep(1)

                serial_handler.disconnect()
                return True

        return False

    except Exception as e:
        print(f"테스트 실패: {e}")
        return False

def test_config_flags():
    """앱 설정 플래그 테스트"""
    print("\n=== 앱 설정 플래그 테스트 ===")

    try:
        # 가상의 앱 설정
        config = {}

        # 포트 연결 시뮬레이션
        config['connected_port'] = 'COM4'
        config['force_dashboard_update'] = True
        config['activate_interval'] = True

        print(f"connected_port: {config.get('connected_port')}")
        print(f"force_dashboard_update: {config.get('force_dashboard_update')}")
        print(f"activate_interval: {config.get('activate_interval')}")

        # update_connection_from_port 로직 시뮬레이션
        connected_port = config.get('connected_port')
        force_update = config.get('force_dashboard_update', False)

        if connected_port or force_update:
            if force_update:
                config['force_dashboard_update'] = False
                print("force_dashboard_update 플래그 리셋됨")

            print("interval-component가 활성화되어야 함 (disabled=False)")
            return True

        return False

    except Exception as e:
        print(f"설정 테스트 실패: {e}")
        return False

def test_dashboard_startup_sequence():
    """대시보드 시작 순서 테스트"""
    print("\n=== 대시보드 시작 순서 테스트 ===")

    try:
        # 대시보드 앱 시작
        print("1. 대시보드 서버 시작...")
        process = subprocess.Popen([
            sys.executable, '-m', 'dashboard.app'
        ], cwd='src/python', stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        print("2. 서버 시작 대기 (10초)...")
        time.sleep(10)

        # 프로세스가 살아있는지 확인
        if process.poll() is None:
            print("3. 서버 시작 성공!")

            print("4. 이제 수동으로 브라우저에서 포트 연결 테스트를 수행하세요")
            print("   - http://127.0.0.1:8050 접속")
            print("   - 포트 관리 > COM4 연결")
            print("   - 브라우저 개발자 도구(F12)에서 네트워크 탭 확인")
            print("   - interval-component 관련 요청 확인")

            input("\n포트 연결 테스트 완료 후 Enter를 누르세요...")

            process.terminate()
            return True
        else:
            stdout, stderr = process.communicate()
            print("4. 서버 시작 실패!")
            print(f"STDOUT: {stdout.decode('utf-8', errors='ignore')}")
            print(f"STDERR: {stderr.decode('utf-8', errors='ignore')}")
            return False

    except Exception as e:
        print(f"시작 순서 테스트 실패: {e}")
        return False

def main():
    print("Interval-component 활성화 디버깅 도구")
    print("=" * 50)

    # 1. 직접 연결 테스트
    connection_ok = test_connection_logic()

    # 2. 설정 플래그 테스트
    config_ok = test_config_flags()

    # 3. 대시보드 시작 순서 테스트
    startup_ok = test_dashboard_startup_sequence()

    print("\n" + "=" * 50)
    print("테스트 결과:")
    print(f"1. 직접 연결 테스트: {'✅ 성공' if connection_ok else '❌ 실패'}")
    print(f"2. 설정 플래그 테스트: {'✅ 성공' if config_ok else '❌ 실패'}")
    print(f"3. 대시보드 시작 테스트: {'✅ 성공' if startup_ok else '❌ 실패'}")

if __name__ == "__main__":
    main()

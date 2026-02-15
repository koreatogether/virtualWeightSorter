#!/usr/bin/env python3
"""대시보드 UI 실시간 모니터링 도구
브라우저에서 대시보드가 실제로 업데이트되는지 확인하는 도구
"""

import json
import os
import subprocess
import sys
import threading
import time
from datetime import datetime

import requests

# dashboard 모듈 경로 추가
sys.path.append('src/python/dashboard')

def start_dashboard():
    """대시보드 서버 시작"""
    print("=== 대시보드 서버 시작 ===")

    # 대시보드 서버를 백그라운드에서 시작
    process = subprocess.Popen([
        sys.executable, '-m', 'dashboard.app'
    ], cwd='src/python', stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    creationflags=subprocess.CREATE_NEW_CONSOLE if os.name == 'nt' else 0)

    # 서버 시작 대기
    print("서버 시작 대기 중...")
    for i in range(20):  # 20초 대기
        try:
            response = requests.get('http://127.0.0.1:8050', timeout=1)
            if response.status_code == 200:
                print(f"OK 대시보드 서버 시작 완료 ({i+1}초)")
                return process
        except Exception:
            pass
        time.sleep(1)
        print(f"대기 중... ({i+1}/20)")

    print("ERROR 대시보드 서버 시작 실패")
    return None

def monitor_dashboard_data():
    """대시보드 내부 데이터 상태 모니터링"""
    try:
        from data_manager import data_manager

        print("\n=== 대시보드 내부 데이터 모니터링 ===")

        for _i in range(30):  # 30초 모니터링
            # 최신 데이터 확인
            latest = data_manager.get_latest_data()
            stats = data_manager.get_statistics()
            graph_data = data_manager.get_graph_data(limit=5)

            timestamp = datetime.now().strftime("%H:%M:%S")

            if latest:
                print(f"[{timestamp}] TEMP: {latest['temperature']}°C | "
                      f"COUNT: {stats['total_data_count']} | "
                      f"GRAPH: {len(graph_data['temperatures'])} points")
            else:
                print(f"[{timestamp}] NO DATA | COUNT: {stats['total_data_count']}")

            time.sleep(1)

    except Exception as e:
        print(f"ERROR 데이터 모니터링 실패: {e}")

def check_interval_component_status():
    """interval-component 상태 확인"""
    print("\n=== interval-component 상태 확인 ===")

    try:
        # app.py 코드에서 interval-component 관련 부분 확인
        with open('src/python/dashboard/app.py', encoding='utf-8') as f:
            content = f.read()

        # interval-component disabled 상태 찾기
        if 'disabled=True' in content:
            print("FOUND interval-component disabled=True 구문 발견")

        if 'interval-component' in content and 'disabled' in content:
            print("FOUND interval-component disabled 관련 코드 존재")

        # 특정 콜백 함수 확인
        if 'update_connection_from_port' in content:
            print("FOUND update_connection_from_port 콜백 함수 존재")

            # 해당 함수에서 interval-component 출력 확인
            lines = content.split('\n')
            in_function = False
            for i, line in enumerate(lines):
                if 'def update_connection_from_port' in line:
                    in_function = True
                    print(f"FOUND 함수 시작: 라인 {i+1}")

                    # 다음 20줄 확인
                    for j in range(i, min(i+30, len(lines))):
                        if 'interval-component' in lines[j]:
                            print(f"LINE {j+1}: {lines[j].strip()}")
                        if 'return' in lines[j] and in_function:
                            print(f"RETURN {j+1}: {lines[j].strip()}")
                            break
                    break

    except Exception as e:
        print(f"ERROR 코드 분석 실패: {e}")

def test_serial_connection():
    """시리얼 연결 상태 테스트"""
    print("\n=== 시리얼 연결 상태 테스트 ===")

    try:
        import serial

        # COM4 직접 테스트
        with serial.Serial('COM4', 115200, timeout=2) as ser:
            print("OK COM4 연결 성공")

            # 몇 개 데이터 수신
            for i in range(5):
                if ser.in_waiting > 0:
                    data = ser.read(ser.in_waiting)
                    text = data.decode('utf-8', errors='ignore').strip()
                    if text:
                        print(f"DATA[{i}]: {text}")

                        # JSON 파싱 시도
                        lines = text.split('\n')
                        for line in lines:
                            line = line.strip()
                            if line.startswith('{') and line.endswith('}'):
                                try:
                                    json_data = json.loads(line)
                                    if json_data.get('type') == 'sensor_data':
                                        print(f"SENSOR: {json_data.get('temperature')}°C")
                                        return True
                                except json.JSONDecodeError:
                                    # Ignore malformed JSON fragments
                                    pass
                                except Exception:
                                    # Any other parsing issue ignored in monitoring context
                                    pass
                time.sleep(1)

        print("ERROR 5초 동안 센서 데이터 수신 없음")
        return False

    except Exception as e:
        print(f"ERROR 시리얼 연결 실패: {e}")
        return False

def wait_for_user_action():
    """사용자 액션 대기"""
    print("\n" + "="*50)
    print("🎯 이제 브라우저에서 다음 작업을 수행하세요:")
    print("1. http://127.0.0.1:8050 접속")
    print("2. 우측 상단 '포트 관리' 버튼 클릭")
    print("3. COM4 포트의 녹색 '연결' 버튼 클릭")
    print("4. 모달이 닫히고 대시보드로 돌아가는지 확인")
    print("5. 온도 값과 그래프가 업데이트되는지 확인")
    print("="*50)

    input("\n포트 연결을 완료한 후 Enter를 누르세요...")

def monitor_after_connection():
    """연결 후 대시보드 상태 모니터링"""
    print("\n=== 연결 후 대시보드 상태 모니터링 ===")

    try:
        from data_manager import data_manager

        prev_count = data_manager.get_statistics().get('total_data_count', 0)
        print(f"연결 전 데이터 카운트: {prev_count}")

        print("30초 동안 데이터 변화 모니터링...")

        for _i in range(30):
            stats = data_manager.get_statistics()
            latest = data_manager.get_latest_data()
            current_count = stats.get('total_data_count', 0)

            timestamp = datetime.now().strftime("%H:%M:%S")

            if latest:
                temp = latest.get('temperature', 'N/A')
                received_time = latest.get('received_time', 'N/A')
                print(f"[{timestamp}] 온도: {temp}°C | 카운트: {current_count} | 수신시간: {received_time[-8:]}")
            else:
                print(f"[{timestamp}] 데이터 없음 | 카운트: {current_count}")

            # 데이터 증가 확인
            if current_count > prev_count:
                print(f"SUCCESS 데이터 증가 감지! {prev_count} → {current_count}")
                prev_count = current_count

            time.sleep(1)

        final_count = data_manager.get_statistics().get('total_data_count', 0)
        if final_count > prev_count:
            print(f"\nSUCCESS 총 {final_count - prev_count}개 새 데이터 수신됨")
            return True
        else:
            print("\nERROR 30초 동안 새 데이터 수신 없음")
            return False

    except Exception as e:
        print(f"ERROR 모니터링 실패: {e}")
        return False

def main():
    print("대시보드 UI 실시간 모니터링 도구")
    print("=" * 50)

    # 1. 시리얼 연결 확인
    if not test_serial_connection():
        print("ERROR Arduino 연결 문제 - 하드웨어 확인 필요")
        return

    # 2. 코드 상태 확인
    check_interval_component_status()

    # 3. 대시보드 서버 시작
    dashboard_process = start_dashboard()
    if not dashboard_process:
        print("ERROR 대시보드 서버 시작 실패")
        return

    try:
        # 4. 내부 데이터 모니터링 시작 (백그라운드)
        monitor_thread = threading.Thread(target=monitor_dashboard_data, daemon=True)
        monitor_thread.start()

        # 5. 사용자 액션 대기
        wait_for_user_action()

        # 6. 연결 후 모니터링
        success = monitor_after_connection()

        if success:
            print("\n✅ SUCCESS: 대시보드가 정상적으로 업데이트됨")
        else:
            print("\n❌ FAILED: 대시보드 업데이트 실패 - 추가 디버깅 필요")

    finally:
        # 서버 종료
        if dashboard_process:
            dashboard_process.terminate()
            print("\n대시보드 서버 종료")

if __name__ == "__main__":
    main()

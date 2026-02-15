#!/usr/bin/env python3
"""간단한 대시보드 테스트 도구"""

import subprocess
import sys
import time
from datetime import datetime

# dashboard 모듈 경로 추가
sys.path.append('src/python/dashboard')

def start_dashboard():
    """대시보드 서버 시작"""
    print("=== 대시보드 서버 시작 ===")

    # 현재 디렉토리에서 대시보드 실행
    process = subprocess.Popen([
        sys.executable, '-m', 'dashboard.app'
    ], cwd='src/python')

    print("서버 시작 대기 중...")
    time.sleep(5)  # 5초 대기
    print("대시보드 서버 시작 완료")
    print("브라우저에서 http://127.0.0.1:8050 접속하세요")

    return process

def monitor_data_manager():
    """data_manager 상태 모니터링"""
    try:
        from data_manager import data_manager

        print("\n=== 데이터 매니저 모니터링 시작 ===")
        print("포트 연결 후 이 창에서 데이터 변화를 확인하세요\n")

        prev_count = 0

        while True:
            try:
                # 최신 데이터 확인
                latest = data_manager.get_latest_data()
                stats = data_manager.get_statistics()
                current_count = stats.get('total_data_count', 0)

                timestamp = datetime.now().strftime("%H:%M:%S")

                if latest:
                    temp = latest.get('temperature', 'N/A')
                    print(f"[{timestamp}] 온도: {temp}°C | 총 데이터: {current_count}개")
                else:
                    print(f"[{timestamp}] 데이터 없음 | 총 데이터: {current_count}개")

                # 새 데이터 감지 시 알림
                if current_count > prev_count:
                    new_data_count = current_count - prev_count
                    print(f"*** 새 데이터 {new_data_count}개 수신! ***")
                    prev_count = current_count

                time.sleep(2)  # 2초마다 확인

            except KeyboardInterrupt:
                print("\n모니터링 중지")
                break
            except Exception as e:
                print(f"ERROR: {e}")
                time.sleep(2)

    except Exception as e:
        print(f"데이터 매니저 로드 실패: {e}")

def main():
    print("간단한 대시보드 테스트 도구")
    print("=" * 40)

    try:
        # 대시보드 서버 시작
        dashboard_process = start_dashboard()

        print("\n지금 다음 작업을 수행하세요:")
        print("1. 브라우저에서 http://127.0.0.1:8050 접속")
        print("2. 우측 상단 '포트 관리' 버튼 클릭")
        print("3. COM4 포트의 녹색 '연결' 버튼 클릭")
        print("4. 아래 모니터링 창에서 데이터 변화 확인")
        print("5. 브라우저에서 온도 값과 그래프가 업데이트되는지 확인")
        print("\n" + "=" * 40)

        # 데이터 모니터링 시작
        monitor_data_manager()

    except KeyboardInterrupt:
        print("\n테스트 종료")
    finally:
        if 'dashboard_process' in locals():
            dashboard_process.terminate()
            print("대시보드 서버 종료")

if __name__ == "__main__":
    main()

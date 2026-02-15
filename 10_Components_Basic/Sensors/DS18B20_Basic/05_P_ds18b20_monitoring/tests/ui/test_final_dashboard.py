#!/usr/bin/env python3
"""최종 대시보드 테스트 - interval-component 직접 활성화 확인"""

import subprocess
import sys
import time
from datetime import datetime

sys.path.append('src/python/dashboard')

def start_dashboard():
    """대시보드 서버 시작"""
    print("=== 대시보드 서버 시작 ===")

    process = subprocess.Popen([
        sys.executable, '-m', 'dashboard.app'
    ], cwd='src/python')

    print("서버 시작 대기 중...")
    time.sleep(8)
    print("대시보드 서버 시작 완료")
    print("브라우저에서 http://127.0.0.1:8050 접속하세요")

    return process

def monitor_data_changes():
    """데이터 변화 실시간 모니터링"""
    try:
        from data_manager import data_manager

        print("\n=== 실시간 데이터 모니터링 ===")
        print("포트 연결 후 데이터 변화를 추적합니다\n")

        prev_count = 0
        no_change_count = 0

        for _i in range(60):  # 60초 모니터링
            try:
                latest = data_manager.get_latest_data()
                stats = data_manager.get_statistics()
                current_count = stats.get('total_data_count', 0)

                timestamp = datetime.now().strftime("%H:%M:%S")

                if latest:
                    temp = latest.get('temperature', 'N/A')
                    print(f"[{timestamp}] 온도: {temp}°C | 데이터: {current_count}개")
                else:
                    print(f"[{timestamp}] 데이터 대기 중... | 총 데이터: {current_count}개")

                # 새 데이터 감지
                if current_count > prev_count:
                    diff = current_count - prev_count
                    print(f"*** 새 데이터 {diff}개 수신! interval-component 활성화됨! ***")
                    prev_count = current_count
                    no_change_count = 0
                else:
                    no_change_count += 1

                # 연속으로 변화 없으면 알림
                if no_change_count == 10:
                    print("WARNING: 10초간 새 데이터 없음 - interval-component 비활성 상태일 수 있음")
                elif no_change_count == 20:
                    print("ERROR: 20초간 새 데이터 없음 - 포트 연결 후에도 interval-component가 활성화되지 않음")

                time.sleep(1)

            except KeyboardInterrupt:
                print("\n모니터링 중지")
                break
            except Exception as e:
                print(f"ERROR: {e}")
                time.sleep(1)

    except Exception as e:
        print(f"데이터 매니저 로드 실패: {e}")

def main():
    print("최종 대시보드 테스트 - interval-component 직접 활성화")
    print("=" * 60)
    print("수정 사항:")
    print("1. port_callbacks.py에서 포트 연결 시 interval-component 직접 활성화")
    print("2. port-scan-interval 의존성 제거")
    print("3. 모달 닫힌 후에도 interval-component 정상 동작")
    print("=" * 60)

    try:
        # 대시보드 서버 시작
        dashboard_process = start_dashboard()

        print("\n지금 다음 작업을 수행하세요:")
        print("1. 브라우저에서 http://127.0.0.1:8050 접속")
        print("2. 우측 상단 '포트 관리' 버튼 클릭")
        print("3. COM4 포트의 녹색 '연결' 버튼 클릭")
        print("4. 모달이 닫히는 것을 확인")
        print("5. 아래 모니터링에서 '새 데이터 수신' 메시지 확인")
        print("6. 대시보드에서 온도값과 그래프 업데이트 확인")
        print()
        print("예상 결과: 포트 연결 즉시 interval-component가 활성화되어 데이터 표시")
        print("=" * 60)

        # 데이터 모니터링 시작
        monitor_data_changes()

    except KeyboardInterrupt:
        print("\n테스트 종료")
    finally:
        if 'dashboard_process' in locals():
            dashboard_process.terminate()
            print("대시보드 서버 종료")

if __name__ == "__main__":
    main()

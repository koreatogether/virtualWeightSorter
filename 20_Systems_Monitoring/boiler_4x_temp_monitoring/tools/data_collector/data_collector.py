import requests
import json
import os
import time
import shutil
from datetime import datetime

# 설정
ARDUINO_IP = "192.168.0.29:80"  # 아두이노 접속 IP
DASHBOARD_PASS = "REDACTED"     # 아두이노 인증 비밀번호
BASE_LOG_DIR = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "..", "docs", "logs"))
FETCH_INTERVAL = 10  # 10초마다 기록 (필요시 조정)
MAX_DAYS = 7  # 최대 7일간 저장

def cleanup_old_logs():
    """7일이 지난 로그 폴더를 삭제하여 순환 저장 구조 유지"""
    if not os.path.exists(BASE_LOG_DIR):
        return
        
    # 모든 폴더 목록 가져오기 (날짜 형식 YYYY-MM-DD 폴더만 대상)
    folders = [d for d in os.listdir(BASE_LOG_DIR) if os.path.isdir(os.path.join(BASE_LOG_DIR, d))]
    
    # 정렬하여 가장 오래된 폴더부터 삭제
    folders.sort()
    
    while len(folders) > MAX_DAYS:
        oldest_folder = folders.pop(0)
        oldest_path = os.path.join(BASE_LOG_DIR, oldest_folder)
        try:
            shutil.rmtree(oldest_path)
            print(f"순환 저장 관리: 오래된 로그 삭제 완료 ({oldest_folder})")
        except Exception as e:
            print(f"로그 삭제 에러 ({oldest_folder}): {e}")

is_waiting = False

def save_data():
    global is_waiting
    try:
        # 아두이노에서 데이터 가져오기 (인증 쿠키 포함)
        headers = {"Cookie": f"auth={DASHBOARD_PASS}"}
        response = requests.get(f"http://{ARDUINO_IP}/data", timeout=3, headers=headers)
        
        if response.status_code == 200:
            if is_waiting:
                print(f"\n[트리거 감지] 아두이노 부팅 확인. 로그 수집을 시작합니다. ({ARDUINO_IP})")
                is_waiting = False

            data = response.json()
            
            # 현재 날짜 및 시간 정보
            now = datetime.now()
            date_folder = now.strftime("%Y-%m-%d")
            time_filename = now.strftime("%Y-%m-%d_%H-%M-%S") + ".json"
            
            # 경로 생성 (폴더가 없으면 생성)
            target_dir = os.path.join(BASE_LOG_DIR, date_folder)
            if not os.path.exists(target_dir):
                cleanup_old_logs()
                os.makedirs(target_dir)
            
            # JSON 저장
            file_path = os.path.join(target_dir, time_filename)
            with open(file_path, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=4)
                
            print(f"[{now.strftime('%H:%M:%S')}] 저장 완료: {file_path}")
            return True
        else:
            print(f"에러: 아두이노 응답 코드 {response.status_code}")
            return False
    except (requests.exceptions.ConnectionError, requests.exceptions.Timeout):
        if not is_waiting:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] 대기 모드: 아두이노 연결 대기 중... ({ARDUINO_IP})")
            is_waiting = True
        return False
    except Exception as e:
        print(f"기타 에러: {e}")
        return False

if __name__ == "__main__":
    print("=" * 50)
    print(f" 보일러 로그 수집기 (7일 순환 모드)")
    print(f" 대상 IP: {ARDUINO_IP}")
    print(f" 저장 위치: {BASE_LOG_DIR}")
    print("=" * 50)
    
    if not os.path.exists(BASE_LOG_DIR):
        os.makedirs(BASE_LOG_DIR)

    while True:
        save_data()
        time.sleep(FETCH_INTERVAL)

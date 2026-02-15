import requests
import json
import os
import time
import shutil
import socket
from datetime import datetime

# --- ANSI Colors ---
class Colors:
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    END = '\033[0m'

# 환경 및 설정 로드
CWD = os.path.dirname(os.path.abspath(__file__))
CONFIG_PATH = os.path.join(CWD, "config_r4.json")

def load_config():
    try:
        with open(CONFIG_PATH, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        print(f"[{Colors.RED}ERROR{Colors.END}] 설정을 불러올 수 없습니다: {e}")
        exit(1)

config = load_config()

# 전역 상수
BASE_LOG_DIR = os.path.normpath(os.path.join(CWD, config["logging"]["base_path"]))
FETCH_INTERVAL = config["logging"]["interval_seconds"]
MAX_DAYS = config["logging"]["retention_days"]
USE_COLORS = config["ui"]["use_colors"]

# 전역 상태
is_online = False

def get_color(color):
    return color if USE_COLORS else ""

def cleanup_old_logs():
    """로그 보관 기간 관리"""
    if not os.path.exists(BASE_LOG_DIR):
        return
        
    folders = [d for d in os.listdir(BASE_LOG_DIR) if os.path.isdir(os.path.join(BASE_LOG_DIR, d))]
    # 날짜 폴더 필터링 (YYYY-MM-DD 형식)
    folders = [f for f in folders if len(f.split('-')) == 3]
    folders.sort()
    
    deleted_count = 0
    while len(folders) > MAX_DAYS:
        oldest_folder = folders.pop(0)
        oldest_path = os.path.join(BASE_LOG_DIR, oldest_folder)
        try:
            shutil.rmtree(oldest_path)
            deleted_count += 1
        except Exception as e:
            print(f"[{get_color(Colors.RED)}ERROR{get_color(Colors.END)}] 삭제 실패 ({oldest_folder}): {e}")
    
    if deleted_count > 0:
        print(f"[{get_color(Colors.BLUE)}INFO{get_color(Colors.END)}] 관리: {deleted_count}개 보관 기간 만료 폴더 정리 완료.")

def save_data():
    global is_online
    
    device = config["device"]
    # 1순위: mDNS, 2순위: Static IP
    targets = [device["hostname"], device["ip_address"]]
    
    last_error = ""
    for target in targets:
        try:
            url = f"http://{target}:{device['port']}/data"
            headers = {"Cookie": f"auth={device['password']}"}
            
            response = requests.get(url, timeout=3, headers=headers)
            
            if response.status_code == 200:
                if not is_online:
                    print(f"\n[{get_color(Colors.GREEN)}LINKED{get_color(Colors.END)}] 장치 연결됨: {target}")
                    is_online = True
                
                payload = response.json()
                now = datetime.now()
                
                # 파일 경로 준비
                date_folder = now.strftime("%Y-%m-%d")
                time_filename = now.strftime("%Y-%m-%d_%H-%M-%S") + ".json"
                target_dir = os.path.join(BASE_LOG_DIR, date_folder)
                
                if not os.path.exists(target_dir):
                    cleanup_old_logs()
                    os.makedirs(target_dir)
                
                file_path = os.path.join(target_dir, time_filename)
                with open(file_path, "w", encoding="utf-8") as f:
                    json.dump(payload, f, indent=4)
                    
                print(f"[{get_color(Colors.GREEN)}{now.strftime('%H:%M:%S')}{get_color(Colors.END)}] 데이터 저장: {time_filename}")
                return True
            
            elif response.status_code == 403:
                last_error = "인증 실패 (비밀번호 불일치)"
            else:
                last_error = f"응답 코드 {response.status_code}"
                
        except (requests.exceptions.ConnectionError, requests.exceptions.Timeout, socket.gaierror):
            last_error = "연결할 수 없음"
            continue
        except Exception as e:
            last_error = str(e)
            continue

    # 모든 타겟 실패 시
    if is_online:
        print(f"[{get_color(Colors.RED)}LOST{get_color(Colors.END)}] 장치 연결 끊김. ({last_error})")
        is_online = False
    else:
        # 조용히 대기 (일정 주기마다 상태 출력)
        curr_time = datetime.now().strftime('%H:%M:%S')
        print(f"\r[{curr_time}] 장치 검색 중... ({device['hostname']} 또는 {device['ip_address']})", end="", flush=True)
    
    return False

if __name__ == "__main__":
    # 윈도우 콘솔 ANSI 컬러 지원 활성화
    os.system('') 
    
    print(f"{get_color(Colors.BOLD)}{get_color(Colors.BLUE)}="*60)
    print(f" Boiler Data Collector for R4 WiFi")
    print(f" 주소: {config['device']['hostname']} / {config['device']['ip_address']}")
    print(f" 저장: {BASE_LOG_DIR} ({MAX_DAYS}일 보관)")
    print(f"="*60 + f"{get_color(Colors.END)}")
    
    if not os.path.exists(BASE_LOG_DIR):
        os.makedirs(BASE_LOG_DIR)

    try:
        while True:
            save_data()
            time.sleep(FETCH_INTERVAL)
    except KeyboardInterrupt:
        print(f"\n[{get_color(Colors.YELLOW)}STOP{get_color(Colors.END)}] 수집을 종료합니다.")

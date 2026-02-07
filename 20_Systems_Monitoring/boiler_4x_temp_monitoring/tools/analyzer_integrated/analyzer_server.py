import os
import json
import time
import threading
import http.server
import socketserver
import webbrowser
import shutil
import sys
from datetime import datetime, timedelta

# --- 패키징된 환경에서 리소스 경로 찾기 (PyInstaller 지원) ---
def get_resource_path(relative_path):
    """PyInstaller의 임시 폴더(_MEIPASS) 또는 현재 디렉터리에서 리소스 경로 반환"""
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath("."), relative_path)

# --- 설정 ---
ARDUINO_IP = "192.168.0.150"    # ESP32C3 Xiao 실제 IP (config_esp32c3.json 기준)
ARDUINO_PORT = 80
DASHBOARD_PASS = "1234"
FETCH_INTERVAL = 2              # 수집 간격 (초)
SERVER_PORT = 8080
MAX_DAYS = 14                   # 로그 보관 일수

# 경로
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
# EXE 실행 시 로그 폴더는 현재 실행 위치 기준으로 설정
LOG_DIR = os.path.abspath(os.path.join(os.getcwd(), "docs", "logs"))
HTML_FILE = get_resource_path("analyzer_ui.html")

# 전역 상태
latest_data = {"s1": 0, "s2": 0, "s3": 0, "s4": 0, "ts": 0}
system_status = {"connected": False, "error_count": 0, "mode": "STBY"}

# ============================================================
#  데이터 수집 (data_collector.py 로직 기반)
# ============================================================
try:
    import requests as req_lib
    USE_REQUESTS = True
except ImportError:
    import urllib.request
    USE_REQUESTS = False

def fetch_data_from_device():
    """아두이노에서 JSON 데이터를 가져옵니다. requests 우선, 없으면 urllib 폴백."""
    url = f"http://{ARDUINO_IP}:{ARDUINO_PORT}/data"
    
    if USE_REQUESTS:
        headers = {"Cookie": f"auth={DASHBOARD_PASS}"}
        resp = req_lib.get(url, timeout=3, headers=headers)
        if resp.status_code == 200:
            return resp.json()
    else:
        import urllib.request
        req = urllib.request.Request(url)
        req.add_header("Cookie", f"auth={DASHBOARD_PASS}")
        with urllib.request.urlopen(req, timeout=3) as resp:
            if resp.getcode() == 200:
                return json.loads(resp.read().decode())
    return None

def cleanup_old_logs():
    """MAX_DAYS 이상 된 로그 폴더 삭제"""
    if not os.path.exists(LOG_DIR): return
    folders = sorted([d for d in os.listdir(LOG_DIR)
                      if os.path.isdir(os.path.join(LOG_DIR, d)) and len(d.split('-')) == 3])
    while len(folders) > MAX_DAYS:
        oldest = folders.pop(0)
        try:
            shutil.rmtree(os.path.join(LOG_DIR, oldest))
            print(f"[Cleanup] Deleted old log: {oldest}")
        except: pass

is_waiting = False

def fetch_and_save():
    """수집 루프 — data_collector.py와 동일한 패턴"""
    global latest_data, system_status, is_waiting
    print(f"[Collector] Started. Target: {ARDUINO_IP}:{ARDUINO_PORT}")
    
    while True:
        try:
            if system_status["error_count"] >= 7:
                system_status["mode"] = "ANALYSIS"
                system_status["connected"] = False
                if not is_waiting:
                    print(f"[Collector] Switching to Analysis mode (7 consecutive failures)")
                    is_waiting = True
                time.sleep(10)
                # 10초마다 한번씩 재시도
                try:
                    data = fetch_data_from_device()
                    if data:
                        system_status["error_count"] = 0
                        is_waiting = False
                        print(f"[Collector] Device reconnected!")
                except:
                    pass
                continue
            
            system_status["mode"] = "COLLECT"
            data = fetch_data_from_device()
            
            if data:
                if is_waiting:
                    print(f"\n[Collector] Device online! Logging resumed.")
                    is_waiting = False
                
                now = datetime.now()
                
                # latest_data 갱신 (UI용, ts 포함)
                latest_data = {
                    "s1": data.get("s1", 0),
                    "s2": data.get("s2", 0),
                    "s3": data.get("s3", 0),
                    "s4": data.get("s4", 0),
                    "ts": int(now.timestamp() * 1000)
                }
                
                system_status["error_count"] = 0
                system_status["connected"] = True
                
                # 파일 저장 (기존 collector와 동일한 형식: s1~s4만)
                date_str = now.strftime("%Y-%m-%d")
                time_str = now.strftime("%Y-%m-%d_%H-%M-%S")
                target_dir = os.path.join(LOG_DIR, date_str)
                
                if not os.path.exists(target_dir):
                    cleanup_old_logs()
                    os.makedirs(target_dir)
                
                file_path = os.path.join(target_dir, f"{time_str}.json")
                with open(file_path, "w", encoding="utf-8") as f:
                    json.dump(data, f, indent=4)
                
                print(f"[{now.strftime('%H:%M:%S')}] Saved: {time_str}.json  "
                      f"s1={data['s1']:.1f} s2={data['s2']:.1f} "
                      f"s3={data['s3']:.1f} s4={data['s4']:.1f}")
            
        except Exception as e:
            system_status["error_count"] += 1
            system_status["connected"] = False
            if not is_waiting:
                print(f"[Collector] Connection failed ({system_status['error_count']}/7): {type(e).__name__}")
                if system_status["error_count"] >= 7:
                    is_waiting = True
        
        time.sleep(FETCH_INTERVAL)


# ============================================================
#  파일명에서 타임스탬프 추출 (log_player.html과 동일한 방식)
# ============================================================
def filename_to_ts(filename):
    """'2026-02-05_21-52-54.json' -> epoch ms"""
    try:
        name = filename.replace('.json', '')
        dt = datetime.strptime(name, "%Y-%m-%d_%H-%M-%S")
        return int(dt.timestamp() * 1000)
    except:
        return 0

def load_json_with_ts(filepath, filename):
    """JSON 파일을 읽고, ts 필드가 없으면 파일명에서 추출하여 추가"""
    with open(filepath, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    # ts가 없으면 파일명에서 추출 (기존 collector 파일 호환)
    if 'ts' not in data or data.get('ts', 0) == 0:
        data['ts'] = filename_to_ts(filename)
    
    return data


# ============================================================
#  HTTP 서버 핸들러
# ============================================================
class AnalyzerHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        # API 호출은 조용히, 파일 서빙만 로그
        if '/api/' not in str(args[0]):
            super().log_message(format, *args)

    def do_GET(self):
        parsed = self.path.split('?')
        path = parsed[0]
        query = parsed[1] if len(parsed) > 1 else ""
        params = {}
        if query:
            for q in query.split('&'):
                if '=' in q:
                    k, v = q.split('=', 1)
                    params[k] = v

        if path == '/api/now':
            self._send_json({"latest": latest_data, "status": system_status})

        elif path == '/api/dates':
            self._send_json(self._get_dates())

        elif path == '/api/history':
            if 'date' in params:
                self._send_json(self._get_by_date(params['date']))
            else:
                hours = int(params.get('hours', 6))
                self._send_json(self._get_recent(hours))
        else:
            if self.path == '/' or self.path == '/index.html':
                # HTML 파일을 읽어서 서빙
                self.send_response(200)
                self.send_header('Content-type', 'text/html; charset=utf-8')
                self.end_headers()
                with open(HTML_FILE, 'rb') as f:
                    self.wfile.write(f.read())
                return
            super().do_GET()

    def _send_json(self, data):
        body = json.dumps(data).encode()
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(body)))
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(body)

    def _get_dates(self):
        if not os.path.exists(LOG_DIR): return []
        dates = []
        for d in os.listdir(LOG_DIR):
            full = os.path.join(LOG_DIR, d)
            if os.path.isdir(full) and len(d.split('-')) == 3:
                # 폴더 안에 .json 파일이 1개 이상 있는지 확인
                jsons = [f for f in os.listdir(full) if f.endswith('.json')]
                if len(jsons) > 0:
                    dates.append(d)
        return sorted(dates, reverse=True)

    def _get_by_date(self, date_str):
        target_dir = os.path.join(LOG_DIR, date_str)
        results = []
        if not os.path.exists(target_dir): return results
        
        for f in sorted(os.listdir(target_dir)):
            if not f.endswith('.json'): continue
            try:
                data = load_json_with_ts(os.path.join(target_dir, f), f)
                results.append(data)
            except: continue
        return results

    def _get_recent(self, hours=6):
        now = datetime.now()
        start = now - timedelta(hours=hours)
        start_ts = int(start.timestamp() * 1000)
        results = []
        
        dates_to_check = set()
        dates_to_check.add(start.strftime("%Y-%m-%d"))
        dates_to_check.add(now.strftime("%Y-%m-%d"))
        
        for date_str in sorted(dates_to_check):
            target_dir = os.path.join(LOG_DIR, date_str)
            if not os.path.exists(target_dir): continue
            
            for f in sorted(os.listdir(target_dir)):
                if not f.endswith('.json'): continue
                ts = filename_to_ts(f)
                if ts >= start_ts:
                    try:
                        data = load_json_with_ts(os.path.join(target_dir, f), f)
                        results.append(data)
                    except: continue
        return results


class ThreadingHTTPServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True
    allow_reuse_address = True


# ============================================================
#  메인
# ============================================================
if __name__ == "__main__":
    os.chdir(BASE_DIR)
    
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)
    
    # 오늘 폴더 미리 생성
    today_dir = os.path.join(LOG_DIR, datetime.now().strftime("%Y-%m-%d"))
    if not os.path.exists(today_dir):
        os.makedirs(today_dir)
    
    print("=" * 60)
    print(" Boiler Integrated Analyzer v2.0")
    print(f" Target Device : {ARDUINO_IP}:{ARDUINO_PORT}")
    print(f" Collect Interval: {FETCH_INTERVAL}s")
    print(f" Log Storage   : {LOG_DIR}")
    print(f" HTTP Library  : {'requests' if USE_REQUESTS else 'urllib (fallback)'}")
    print("=" * 60)
    
    # 1) 수집 스레드
    t = threading.Thread(target=fetch_and_save, daemon=True)
    t.start()
    
    # 2) 웹 서버
    httpd = ThreadingHTTPServer(("", SERVER_PORT), AnalyzerHandler)
    print(f"[Server] Dashboard: http://localhost:{SERVER_PORT}")
    threading.Timer(1.5, lambda: webbrowser.open(f"http://localhost:{SERVER_PORT}")).start()
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[Server] Shutdown.")
        httpd.shutdown()

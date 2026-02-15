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

# --- 설정 ---
ARDUINO_IP = "192.168.0.150"    # ESP32C3 Xiao 실제 IP
ARDUINO_PORT = 80
DASHBOARD_PASS = "REDACTED"
FETCH_INTERVAL = 2              # 수집 간격 (초)
SERVER_PORT = 8080
MAX_DAYS = 14                   # 로그 보관 일수
TARGET_POINTS_DISPLAY = 1000    # 브라우저에 표시할 최대 포인트 수 (다운샘플링 기준)

# 경로
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

def get_resource_path(relative_path):
    """PyInstaller의 임시 폴더(_MEIPASS) 또는 스크립트 디렉터리에서 리소스 경로 반환"""
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(BASE_DIR, relative_path)

# 로그 폴더를 툴 내부 경로로 설정 (EXE 패키징 및 테스트 기준)
LOG_DIR = os.path.join(BASE_DIR, "docs", "logs")
HTML_FILE = get_resource_path("analyzer_ui.html")

# 전역 상태
latest_data = {"s1": 0, "s2": 0, "s3": 0, "s4": 0, "ts": 0}
system_status = {"connected": False, "error_count": 0, "mode": "STBY"}

# ============================================================
#  데이터 수집
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
    """MAX_DAYS 이상 된 로그 파일 삭제"""
    if not os.path.exists(LOG_DIR): return
    # .jsonl 파일과 폴더 모두 확인
    items = sorted([f for f in os.listdir(LOG_DIR) if f.endswith('.jsonl') or (os.path.isdir(os.path.join(LOG_DIR, f)) and len(f.split('-')) == 3)])
    
    while len(items) > MAX_DAYS:
        oldest = items.pop(0)
        path = os.path.join(LOG_DIR, oldest)
        try:
            if os.path.isdir(path):
                shutil.rmtree(path)
            else:
                os.remove(path)
            print(f"[Cleanup] Deleted old log: {oldest}")
        except: pass

is_waiting = False

def fetch_and_save():
    """수집 루프 — JSONL (Single File) 방식 적용"""
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
                timestamp = int(now.timestamp() * 1000)
                
                # latest_data 갱신 (UI용, ts 포함)
                latest_data = {
                    "s1": data.get("s1", 0),
                    "s2": data.get("s2", 0),
                    "s3": data.get("s3", 0),
                    "s4": data.get("s4", 0),
                    "ts": timestamp
                }
                
                system_status["error_count"] = 0
                system_status["connected"] = True
                
                # 파일 저장 (JSONL 방식: 날짜별 하나의 파일에 Append)
                date_str = now.strftime("%Y-%m-%d")
                log_file = os.path.join(LOG_DIR, f"{date_str}.jsonl")
                
                # 저장할 데이터에 타임스탬프 명시적 포함
                save_data = latest_data.copy()
                
                with open(log_file, "a", encoding="utf-8") as f:
                    f.write(json.dumps(save_data) + "\n")
                
                # 30초마다 한 번씩만 콘솔 출력 (로그 과다 방지)
                if now.second % 30 == 0:
                     print(f"[{now.strftime('%H:%M:%S')}] Logged to {date_str}.jsonl")
            
        except Exception as e:
            system_status["error_count"] += 1
            system_status["connected"] = False
            if not is_waiting:
                print(f"[Collector] Connection failed ({system_status['error_count']}/7): {type(e).__name__}")
                if system_status["error_count"] >= 7:
                    is_waiting = True
        
        time.sleep(FETCH_INTERVAL)


# ============================================================
#  데이터 읽기 및 다운샘플링
# ============================================================
def load_data_from_jsonl(filepath):
    """JSONL 파일 전체 로드"""
    data_list = []
    if not os.path.exists(filepath):
        return data_list
    
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line: continue
            try:
                data_list.append(json.loads(line))
            except: continue
    return data_list

def downsample_data(data_list, target_count=TARGET_POINTS_DISPLAY):
    """데이터 개수가 너무 많으면 일정 간격으로 샘플링"""
    count = len(data_list)
    if count <= target_count:
        return data_list
    
    step = count // target_count
    return data_list[::step]


# ============================================================
#  HTTP 서버 핸들러
# ============================================================
class AnalyzerHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        # API 호출은 조용히
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
                # HTML 서빙
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
        # .jsonl 파일명에서 날짜 추출
        dates = []
        for f in os.listdir(LOG_DIR):
            if f.endswith('.jsonl'):
                dates.append(f.replace('.jsonl', ''))
            # 하위 호환성: 폴더 방식도 체크
            elif os.path.isdir(os.path.join(LOG_DIR, f)) and len(f.split('-')) == 3:
                 dates.append(f)
        return sorted(list(set(dates)), reverse=True)

    def _get_by_date(self, date_str):
        # 1. JSONL 확인
        jsonl_path = os.path.join(LOG_DIR, f"{date_str}.jsonl")
        if os.path.exists(jsonl_path):
            raw_data = load_data_from_jsonl(jsonl_path)
            return downsample_data(raw_data)
            
        # 2. 기존 폴더 방식 확인 (Fallback)
        target_dir = os.path.join(LOG_DIR, date_str)
        results = []
        if os.path.exists(target_dir):
            for f in sorted(os.listdir(target_dir)):
                if not f.endswith('.json'): continue
                try:
                    # 기존 방식 로드 (파일명 파싱 등은 복잡하므로 여기선 간단히)
                    full_path = os.path.join(target_dir, f)
                    with open(full_path, 'r', encoding='utf-8') as jf:
                        d = json.load(jf)
                        if 'ts' not in d:
                            # 파일명에서 ts 추출 필요 시 구현 (여기선 생략)
                            pass
                        results.append(d)
                except: continue
        return downsample_data(results)

    def _get_recent(self, hours=6):
        now = datetime.now()
        start = now - timedelta(hours=hours)
        start_ts = int(start.timestamp() * 1000)
        results = []
        
        # 오늘, 어제 (최대 2일치) 확인
        dates_to_check = set()
        dates_to_check.add(start.strftime("%Y-%m-%d"))
        dates_to_check.add(now.strftime("%Y-%m-%d"))
        
        for date_str in sorted(dates_to_check):
            # JSONL 우선
            jsonl_path = os.path.join(LOG_DIR, f"{date_str}.jsonl")
            if os.path.exists(jsonl_path):
                file_data = load_data_from_jsonl(jsonl_path)
                # 시간 필터링
                filtered = [d for d in file_data if d.get('ts', 0) >= start_ts]
                results.extend(filtered)
            else:
                # 폴더 방식 Fallback (필요 시 구현, 여기선 생략하여 성능 확보)
                pass
                
        return downsample_data(results)


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
        
    print("=" * 60)
    print(" Boiler Integrated Analyzer v2.0 (Performance Optimized)")
    print(f" Target Device : {ARDUINO_IP}:{ARDUINO_PORT}")
    print(f" Collect Interval: {FETCH_INTERVAL}s")
    print(f" Log Storage   : {LOG_DIR} (Mode: JSONL)")
    print("=" * 60)
    
    # 1) 수집 스레드
    t = threading.Thread(target=fetch_and_save, daemon=True)
    t.start()
    
    # 2) 웹 서버
    httpd = ThreadingHTTPServer(("", SERVER_PORT), AnalyzerHandler)
    print(f"[Server] Dashboard: http://localhost:{SERVER_PORT}")
    
    # 자동 브라우저 실행
    threading.Timer(1.5, lambda: webbrowser.open(f"http://localhost:{SERVER_PORT}")).start()
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[Server] Shutdown.")
        httpd.shutdown()
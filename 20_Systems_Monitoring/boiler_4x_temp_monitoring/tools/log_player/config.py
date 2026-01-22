import os

# --- 설정 (기본값) ---
DEFAULT_LOG_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), 'docs', 'logs')
WINDOW_TITLE = "Boiler Monitor Log Player (Dual Jog Edition)"

# 뷰 간격 옵션 (초 단위)
VIEW_INTERVALS = {
    "10초": 10,
    "30초": 30,
    "1분": 60,
    "3분": 180,
    "5분": 300,
    "10분": 600,
    "30분": 1800,
    "1시간": 3600,
    "2시간": 7200,
    "3시간": 10800,
    "5시간": 18000,
    "10시간": 36000
}

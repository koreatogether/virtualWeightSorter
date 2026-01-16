import serial
import datetime

# COM 포트와 보드레이트는 환경에 맞게 수정하세요
SERIAL_PORT = 'COM4'
BAUD_RATE = 9600


# 로그 파일 저장 폴더
import os
LOG_DIR = 'logFiles'
os.makedirs(LOG_DIR, exist_ok=True)
now = datetime.datetime.now()
timestamp = now.strftime('%Y%m%d_%H%M%S')
LOG_FILE = os.path.join(LOG_DIR, f'memory_log_{timestamp}.txt')

with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser, open(LOG_FILE, 'w') as f:
    print(f"Logging serial output from {SERIAL_PORT} to {LOG_FILE}...")
    print("Filtering for memory-related logs only...")
    try:
        while True:
            line = ser.readline().decode('utf-8', errors='ignore')
            if line:
                # 메모리 관련 로그만 필터링
                if any(keyword in line for keyword in [
                    'Memory', 'memory', 'Free Memory', 'malloc', 'free', 'MEMORY', 
                    'Basic Test', 'Stress Test', 'Data Structure', 'Serial OK', 
                    '===', 'freeMemory', 'supported', 'UNO R4', 'Renesas',
                    'allocation', 'deallocation', 'Runtime memory', 'PERIODIC MEMORY',
                    'CSV_LOG', 'MANUAL', 'INITIAL', 'System ready'
                ]):
                    f.write(line)
                    f.flush()
                    print(f"[Memory Log Saved] {line.strip()}")
                else:
                    # 메모리 관련이 아닌 로그는 터미널에만 표시 (파일에는 저장하지 않음)
                    print(f"[Filtered Out] {line.strip()}")
    except KeyboardInterrupt:
        print(f"\nLogging stopped. File saved: {LOG_FILE}")

import os
import json
import glob
from datetime import datetime, timedelta, timezone

# 대상 경로 수정 (툴 내부 로그 폴더)
LOG_DIR = os.path.join("tools", "analyzer_integrated", "docs", "logs")
KST = timezone(timedelta(hours=9))

def get_ts_from_filename(filename):
    """'2026-02-05_21-52-54.json' -> epoch ms (KST 기준)"""
    try:
        name = os.path.basename(filename).replace('.json', '')
        # 문자열을 파싱하여 naive datetime 객체 생성
        dt_naive = datetime.strptime(name, "%Y-%m-%d_%H-%M-%S")
        # 이 시간이 한국 시간임을 명시하고 UTC 타임스탬프로 변환
        dt_kst = dt_naive.replace(tzinfo=KST)
        return int(dt_kst.timestamp() * 1000)
    except Exception as e:
        print(f"Error parsing timestamp from {filename}: {e}")
        return 0

def migrate():
    if not os.path.exists(LOG_DIR):
        print(f"Log directory not found: {LOG_DIR}")
        return

    # Get all date folders
    date_folders = [d for d in os.listdir(LOG_DIR) if os.path.isdir(os.path.join(LOG_DIR, d))]
    
    for date_str in date_folders:
        folder_path = os.path.join(LOG_DIR, date_str)
        json_files = sorted(glob.glob(os.path.join(folder_path, "*.json")))
        
        if not json_files:
            continue
            
        output_file = os.path.join(LOG_DIR, f"{date_str}.jsonl")
        print(f"Migrating {date_str} ({len(json_files)} files) -> {output_file}")
        
        with open(output_file, 'w', encoding='utf-8') as outfile:
            for json_file in json_files:
                try:
                    with open(json_file, 'r', encoding='utf-8') as infile:
                        data = json.load(infile)
                        
                    # Inject timestamp if missing
                    if 'ts' not in data:
                        data['ts'] = get_ts_from_filename(json_file)
                        
                    # Write as single line
                    outfile.write(json.dumps(data) + "\n")
                except Exception as e:
                    print(f"Error reading {json_file}: {e}")

        print(f"Done. You can now verify {output_file} and delete the folder '{date_str}' manually if satisfied.")

if __name__ == "__main__":
    migrate()

#!/usr/bin/env python3
"""웹 대시보드 실행 스크립트

간단해진 센서 로그 카드가 포함된 웹 대시보드를 실행합니다.
"""

import sys
import os

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.app import app

if __name__ == '__main__':
    print("🚀 DS18B20 온도 센서 웹 대시보드 시작")
    print("📊 간단한 센서 로그 카드가 적용되었습니다")
    print("브라우저에서 http://127.0.0.1:8050 을 열어주세요")
    print("Ctrl+C로 종료")
    print("=" * 50)
    
    try:
        app.run(debug=True, port=8050, host='127.0.0.1')
    except KeyboardInterrupt:
        print("\n⏹️ 대시보드가 종료되었습니다.")
    except Exception as e:
        print(f"❌ 오류 발생: {e}")
        import traceback
        traceback.print_exc()
#!/usr/bin/env python3
"""
웹 대시보드 실행 스크립트 (moved to tools/scripts)
"""

from pathlib import Path
import sys

# Set project root to repository root (three levels up from tools/scripts)
project_root = Path(__file__).parents[2]
sys.path.insert(0, str(project_root))

from src.python.app import app

if __name__ == '__main__':
    print("🚀 DS18B20 온도 센서 웹 대시보드 시작")
    print("📊 환경에서 사용 가능한 센서 로그 카드가 적용된 웹 대시보드를 실행합니다")
    print("브라우저에서 http://127.0.0.1:8050 를 열어주세요")
    print("Ctrl+C로 종료")
    print("=" * 50)

    try:
        app.run(debug=True, port=8050, host='127.0.0.1')
    except KeyboardInterrupt:
        print("\n⏹️ 대시보드가 정상적으로 종료되었습니다.")
    except Exception as e:
        print(f"❌ 오류 발생: {e}")
        import traceback
        traceback.print_exc()

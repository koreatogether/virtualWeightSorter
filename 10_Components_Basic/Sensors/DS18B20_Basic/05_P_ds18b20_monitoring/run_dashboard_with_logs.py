#!/usr/bin/env python3
"""
센서 로그가 통합된 대시보드 실행 스크립트
"""

import sys
import os

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

def main():
    """메인 함수"""
    print("🚀 DS18B20 온도 센서 모니터링 시스템 시작")
    print("🔍 COM 포트 스캔 중...")
    
    # 포트 매니저 초기화
    from src.python.services.port_manager import port_manager
    
    # 자동 연결 시도
    success = port_manager.auto_connect()
    
    if success:
        current_port = port_manager.get_current_port()
        print(f"✅ {current_port} 연결 성공!")
        print("✅ Arduino 통신 테스트 성공!")
    else:
        print("⚠️ Arduino 자동 연결 실패 - 수동으로 연결해주세요")
    
    print("🌐 대시보드 서버 시작 중...")
    print("📊 센서 로그 기능이 활성화되었습니다!")
    print("\nDash is running on http://127.0.0.1:8050/")
    print("Press CTRL+C to quit")
    
    # Dash 앱 실행
    try:
        from src.python.app import app
        app.run(host="127.0.0.1", port=8050, debug=False)
    except KeyboardInterrupt:
        print("\n⏹️ 사용자가 대시보드를 중단했습니다.")
    except Exception as e:
        print(f"❌ 대시보드 실행 오류: {e}")
        import traceback
        traceback.print_exc()
    finally:
        # 정리 작업
        port_manager.disconnect()
        print("✅ 대시보드 종료 완료")

if __name__ == "__main__":
    main()
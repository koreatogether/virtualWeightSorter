#!/usr/bin/env python3
"""
Arduino Multi-Implementation Dashboard Launcher
대시보드 실행을 위한 편의 스크립트
"""

import sys
import os

# 프로젝트 루트 경로 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation'))

def main():
    """메인 실행 함수"""
    print("Arduino Multi-Implementation Dashboard Launcher")
    print("=" * 60)
    
    print("\n사용 가능한 대시보드:")
    print("1. 자동 실제 Arduino 대시보드 (추천)")
    print("2. 기본 다중 구현 대시보드")
    print("3. 통계 분석 실행")
    
    choice = input("\n선택하세요 (1-3, 기본값: 1): ").strip() or "1"
    
    try:
        if choice == "1":
            print("\n자동 실제 Arduino 대시보드 시작...")
            print("URL: http://localhost:8053")
            print("10초 카운트다운 후 자동 시작")
            
            # 시뮬레이션 모듈 경로 추가
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation'))
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation', 'dashboards'))
            from auto_real_arduino_dashboard import AutoRealArduinoDashboard
            dashboard = AutoRealArduinoDashboard(port=8053, debug=False)
            dashboard.run_server()
            
        elif choice == "2":
            print("\n기본 다중 구현 대시보드 시작...")
            print("URL: http://localhost:8052")
            
            # 시뮬레이션 모듈 경로 추가
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation'))
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation', 'dashboards'))
            from auto_multi_dashboard import AutoMultiDashboard
            dashboard = AutoMultiDashboard(port=8052, debug=False)
            dashboard.run_server()
            
        elif choice == "3":
            print("\n통계 분석 실행...")
            
            # 시뮬레이션 모듈 경로 추가
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation'))
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation', 'analysis'))
            from statistical_analysis import main as analysis_main
            analysis_main()
            
        else:
            print("잘못된 선택입니다.")
            
    except KeyboardInterrupt:
        print("\n사용자에 의해 중단되었습니다.")
    except Exception as e:
        print(f"오류 발생: {e}")
        print("\n문제 해결:")
        print("1. 의존성 설치: pip install -r requirements.txt")
        print("2. 설정 파일 확인: config/arduino_implementations_real.yaml")
        print("3. 포트 충돌 확인: 다른 포트 사용")

if __name__ == "__main__":
    main()
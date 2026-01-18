#!/usr/bin/env python3
"""
Arduino Statistical Analysis Launcher
통계 분석 실행을 위한 편의 스크립트
"""

import sys
import os

# 프로젝트 루트 경로 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation'))

def main():
    """메인 실행 함수"""
    print("📊 Arduino Statistical Analysis Launcher")
    print("=" * 60)
    
    print("\n사용 가능한 분석:")
    print("1. 상세 통계 분석 (추천)")
    print("2. 기본 성능 테스트")
    print("3. 커스텀 분석")
    
    choice = input("\n선택하세요 (1-3, 기본값: 1): ").strip() or "1"
    
    try:
        if choice == "1":
            print("\n📈 상세 통계 분석 시작...")
            print("- 전체 빈도 분석")
            print("- 조건부 확률 분석")
            print("- 편향성 검증")
            print("- 시각화 생성")
            
            # 시뮬레이션 모듈 경로 추가
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation'))
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation', 'analysis'))
            from statistical_analysis import main as analysis_main
            analysis_main()
            
        elif choice == "2":
            print("\n⚡ 기본 성능 테스트 시작...")
            
            from real_arduino_sim import test_real_arduino_implementations
            test_real_arduino_implementations()
            
        elif choice == "3":
            print("\n🔧 커스텀 분석 설정...")
            
            iterations = input("반복 횟수 (기본값: 10000): ").strip()
            iterations = int(iterations) if iterations.isdigit() else 10000
            
            seed = input("시드 값 (기본값: 12345): ").strip()
            seed = int(seed) if seed.isdigit() else 12345
            
            print(f"\n실행 설정: {iterations:,}회 반복, 시드 {seed}")
            
            # 시뮬레이션 모듈 경로 추가
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation'))
            sys.path.insert(0, os.path.join(project_root, 'src', 'arduino_simulation', 'analysis'))
            from statistical_analysis import StatisticalAnalyzer
            analyzer = StatisticalAnalyzer()
            results = analyzer.analyze_all_implementations(iterations=iterations, seed=seed)
            
            if results:
                analyzer.generate_visualization(results)
                analyzer.export_detailed_report(results, f"custom_analysis_{seed}_{iterations}.txt")
                print(f"\n✅ 커스텀 분석 완료!")
                print(f"📄 보고서: custom_analysis_{seed}_{iterations}.txt")
            
        else:
            print("❌ 잘못된 선택입니다.")
            
    except KeyboardInterrupt:
        print("\n🛑 사용자에 의해 중단되었습니다.")
    except Exception as e:
        print(f"❌ 오류 발생: {e}")
        print("\n문제 해결:")
        print("1. 의존성 설치: pip install -r requirements.txt")
        print("2. 설정 파일 확인: config/arduino_implementations_real.yaml")
        print("3. Python 경로 확인")

if __name__ == "__main__":
    main()
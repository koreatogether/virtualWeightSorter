#!/usr/bin/env python3
"""ID 변경 시스템 전체 테스트"""

import sys
import os
import time
import threading
import subprocess
from datetime import datetime

# 프로젝트 루트를 Python 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

def run_simulator():
    """Arduino 시뮬레이터 실행"""
    print("🤖 Arduino 시뮬레이터 시작 중...")
    try:
        result = subprocess.run([
            sys.executable, "test_arduino_simulator.py"
        ], capture_output=False, text=True)
    except KeyboardInterrupt:
        print("🛑 시뮬레이터 종료")
    except Exception as e:
        print(f"❌ 시뮬레이터 오류: {e}")

def run_dashboard():
    """대시보드 실행"""
    print("📊 대시보드 시작 중...")
    try:
        # 가상환경 활성화 후 대시보드 실행
        result = subprocess.run([
            ".venv\\Scripts\\python.exe", "-m", "src.python.run"
        ], capture_output=False, text=True, cwd=".")
    except KeyboardInterrupt:
        print("🛑 대시보드 종료")
    except Exception as e:
        print(f"❌ 대시보드 오류: {e}")

def test_manual_steps():
    """수동 테스트 단계 안내"""
    print("\n" + "="*60)
    print("🧪 ID 변경 시스템 테스트 가이드")
    print("="*60)
    
    print("\n📋 테스트 시나리오:")
    print("1️⃣  기본 연결 확인")
    print("   - 브라우저에서 http://127.0.0.1:8050 접속")
    print("   - 센서 데이터가 실시간으로 표시되는지 확인")
    print("   - 3개 센서 (ID: 01, 02, 03) 데이터 확인")
    
    print("\n2️⃣  ID 변경 모달 테스트")
    print("   - 아무 센서의 'ID 변경' 버튼 클릭")
    print("   - 모달 창이 열리는지 확인")
    print("   - 현재 센서 정보가 올바르게 표시되는지 확인")
    
    print("\n3️⃣  ID 선택 및 유효성 검사")
    print("   - ID 버튼 (01~08) 클릭해보기")
    print("   - 현재 사용 중인 ID 선택 시 경고 메시지 확인")
    print("   - 사용 가능한 ID 선택 시 확인 버튼 활성화 확인")
    
    print("\n4️⃣  ID 변경 실행")
    print("   - 사용 가능한 ID (예: 04, 05) 선택")
    print("   - '변경 실행' 버튼 클릭")
    print("   - Arduino 시뮬레이터에서 명령 수신 확인")
    print("   - 성공 메시지 표시 확인")
    print("   - 센서 목록에서 변경된 ID 확인")
    
    print("\n5️⃣  오류 상황 테스트")
    print("   - 이미 사용 중인 ID로 변경 시도")
    print("   - 오류 메시지가 올바르게 표시되는지 확인")
    
    print("\n" + "="*60)
    print("💡 팁:")
    print("   - 시뮬레이터 터미널에서 실시간 로그 확인")
    print("   - 브라우저 개발자 도구에서 네트워크 요청 확인")
    print("   - Ctrl+C로 각 프로세스 종료")
    print("="*60)

def main():
    """메인 테스트 실행"""
    print("🚀 ID 변경 시스템 전체 테스트")
    print(f"⏰ 시작 시간: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    # 테스트 가이드 출력
    test_manual_steps()
    
    print("\n🔄 실행 옵션:")
    print("1. 시뮬레이터만 실행")
    print("2. 대시보드만 실행") 
    print("3. 둘 다 순서대로 실행")
    print("4. 테스트 가이드만 보기")
    
    try:
        choice = input("\n선택하세요 (1-4): ").strip()
        
        if choice == "1":
            print("\n🤖 Arduino 시뮬레이터 실행...")
            run_simulator()
            
        elif choice == "2":
            print("\n📊 대시보드 실행...")
            run_dashboard()
            
        elif choice == "3":
            print("\n🔄 시뮬레이터와 대시보드 순서대로 실행")
            print("1. 먼저 시뮬레이터를 실행합니다")
            print("2. 시뮬레이터가 준비되면 다른 터미널에서 대시보드를 실행하세요")
            print("   명령: .venv\\Scripts\\activate && python -m src.python.run")
            print("\n시뮬레이터 시작...")
            run_simulator()
            
        elif choice == "4":
            print("\n✅ 테스트 가이드를 참고하여 수동으로 테스트하세요")
            
        else:
            print("❌ 잘못된 선택입니다")
            
    except KeyboardInterrupt:
        print("\n⏹️  테스트 중단됨")
    except Exception as e:
        print(f"\n❌ 테스트 오류: {e}")

if __name__ == "__main__":
    main()
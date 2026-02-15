#!/usr/bin/env python3
"""임시 대시보드 실행 파일 - 인코딩 문제 해결"""

import sys
import os

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

def main():
    """메인 함수"""
    print("DS18B20 온도 센서 모니터링 시스템 시작")
    print("COM 포트 스캔 중...")
    
    try:
        # 포트 매니저 초기화
        from src.python.services.port_manager import port_manager
        
        # 포트 스캔 및 자동 연결 시도
        available_ports = port_manager.scan_ports()
        print(f"사용 가능한 포트: {available_ports}")
        
        success = False
        if available_ports:
            # 첫 번째 포트로 연결 시도
            success = port_manager.connect(available_ports[0])
            
            if success:
                current_port = port_manager.get_current_port()
                print(f"Arduino {current_port} 연결 성공!")
                
                # 통신 테스트
                if port_manager.test_communication():
                    print("Arduino 통신 테스트 성공!")
                else:
                    print("Arduino 통신 테스트 실패")
            else:
                print(f"Arduino {available_ports[0]} 연결 실패")
        else:
            print("사용 가능한 Arduino 포트가 없습니다")
        
        if not success:
            print("Arduino 연결 실패 - 대시보드는 시뮬레이션 모드로 시작됩니다")
        
        print("대시보드 서버 시작 중...")
        print("센서 로그 기능이 활성화되었습니다!")
        print("\nDash is running on http://127.0.0.1:8050/")
        print("Press CTRL+C to quit")
        
        # Dash 앱 실행
        from src.python.app import app
        app.run(host="127.0.0.1", port=8050, debug=False)
        
    except KeyboardInterrupt:
        print("\n사용자가 대시보드를 중단했습니다.")
    except Exception as e:
        print(f"대시보드 실행 오류: {e}")
        import traceback
        traceback.print_exc()
    finally:
        # 정리 작업
        try:
            port_manager.disconnect()
        except:
            pass
        print("대시보드 종료 완료")

if __name__ == "__main__":
    main()
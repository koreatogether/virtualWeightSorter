#!/usr/bin/env python3
"""
로컬 대시보드 실행 도우미 (임시)
"""
import sys
import os

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

def main():
    print("DS18B20 온도 센서 모니터 시스템 시작")
    print("COM 포트 스캔 중...")
    
    port_manager = None
    try:
        from src.python.services.port_manager import port_manager as _pm
        port_manager = _pm
        
        available_ports = port_manager.scan_ports()
        print(f"사용 가능한 포트: {available_ports}")
        
        success = False
        if available_ports:
            success = port_manager.connect(available_ports[0])
            
            if success:
                current_port = port_manager.get_current_port()
                print(f"Arduino {current_port} 연결 성공!")
                
                if port_manager.test_communication():
                    print("Arduino 통신 테스트 성공!")
                else:
                    print("Arduino 통신 테스트 실패")
            else:
                print(f"Arduino {available_ports[0]} 연결 실패")
        else:
            print("사용 가능한 Arduino 포트가 없습니다")
        
        if not success:
            print("Arduino 연결 실패 - 대시보드를 바로 실행합니다")
        
        print("대시보드 실행 중... (127.0.0.1:8050)")
        from src.python.app import app
        app.run(host="127.0.0.1", port=8050, debug=False)
    except KeyboardInterrupt:
        print("\n사용자가 중단했습니다.")
    except Exception as e:
        print(f"대시보드 실행 오류: {e}")
        import traceback
        traceback.print_exc()
    finally:
        try:
            if port_manager:
                port_manager.disconnect()
        except Exception:
            pass
        print("대시보드 종료 완료")

if __name__ == "__main__":
    main()

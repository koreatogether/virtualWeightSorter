#!/usr/bin/env python3
"""
센서 로거가 통합된 대시보드
실시간 센서 데이터와 함께 상세 로그 정보를 제공합니다.
"""

import sys
import os
import time
import threading
from datetime import datetime

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager
from src.python.services.sensor_logger import sensor_logger

class DashboardWithLogger:
    """로거가 통합된 대시보드"""
    
    def __init__(self):
        self.running = False
        self.data_thread = None
        self.log_thread = None
        
    def start(self):
        """대시보드 시작"""
        print("🚀 DS18B20 온도 센서 모니터링 시스템 시작")
        print("=" * 50)
        
        # Arduino 연결
        print("🔍 COM 포트 스캔 중...")
        success = port_manager.auto_connect()
        
        if not success:
            print("❌ Arduino 연결 실패!")
            return False
        
        current_port = port_manager.get_current_port()
        print(f"✅ {current_port} 연결 성공!")
        print("✅ Arduino 통신 테스트 성공!")
        
        # 로그 설정
        sensor_logger.set_log_interval(10)  # 10개 측정마다 로그 출력
        
        # 스레드 시작
        self.running = True
        self.data_thread = threading.Thread(target=self._data_collection_loop, daemon=True)
        self.log_thread = threading.Thread(target=self._periodic_summary_loop, daemon=True)
        
        self.data_thread.start()
        self.log_thread.start()
        
        print("🌐 대시보드 서버 시작 중...")
        print("📊 센서 로거 활성화됨 (10개 측정마다 로그 출력)")
        print("\n" + "=" * 50)
        print("실시간 센서 데이터:")
        print("=" * 50)
        
        return True
    
    def _data_collection_loop(self):
        """데이터 수집 루프"""
        while self.running:
            try:
                # 센서 데이터 읽기
                data = port_manager.read_sensor_data(timeout=2.0)
                
                if data:
                    # 실시간 데이터 출력 (간단한 형태)
                    current_time = datetime.now().strftime("%H:%M:%S")
                    temp = data.get('temperature', 0)
                    sensor_id = data.get('sensor_id', '00')
                    sensor_addr = data.get('sensor_addr', 'UNKNOWN')
                    
                    print(f"[{current_time}] 센서 {sensor_id}: {temp:.1f}°C ({sensor_addr[-8:]})")
                    
                    # 센서 로거에 데이터 전달
                    sensor_logger.update_sensor_data(data)
                
                time.sleep(0.8)  # 약간의 지연
                
            except Exception as e:
                print(f"❌ 데이터 수집 오류: {e}")
                time.sleep(1)
    
    def _periodic_summary_loop(self):
        """주기적 요약 출력 루프"""
        while self.running:
            try:
                time.sleep(60)  # 1분마다
                
                if sensor_logger.sensors:
                    print("\n" + "🔄" * 20 + " 1분 요약 " + "🔄" * 20)
                    sensor_logger.print_system_summary()
                    print("=" * 50)
                    print("실시간 센서 데이터:")
                    print("=" * 50)
                
            except Exception as e:
                print(f"❌ 요약 출력 오류: {e}")
    
    def stop(self):
        """대시보드 중지"""
        print("\n🛑 대시보드 중지 중...")
        self.running = False
        
        if self.data_thread:
            self.data_thread.join(timeout=2)
        if self.log_thread:
            self.log_thread.join(timeout=2)
        
        port_manager.disconnect()
        print("✅ 대시보드 중지 완료")
    
    def run_interactive(self):
        """인터랙티브 모드 실행"""
        if not self.start():
            return
        
        print("\n📋 명령어:")
        print("  's' - 시스템 요약 출력")
        print("  'l' - 로그 간격 변경")
        print("  'r' - 센서 통계 리셋")
        print("  'q' - 종료")
        print("\nPress Enter for commands...")
        
        try:
            while self.running:
                command = input().strip().lower()
                
                if command == 'q':
                    break
                elif command == 's':
                    print("\n📊 현재 시스템 요약:")
                    sensor_logger.print_system_summary()
                elif command == 'l':
                    try:
                        interval = int(input("새로운 로그 간격 (측정 횟수): "))
                        sensor_logger.set_log_interval(interval)
                    except ValueError:
                        print("❌ 숫자를 입력해주세요.")
                elif command == 'r':
                    # 통계 리셋 (새로운 로거 인스턴스 생성)
                    from src.python.services.sensor_logger import SensorLogger
                    global sensor_logger
                    sensor_logger = SensorLogger()
                    sensor_logger.set_log_interval(10)
                    print("✅ 센서 통계가 리셋되었습니다.")
                elif command == '':
                    continue
                else:
                    print("❌ 알 수 없는 명령어입니다.")
        
        except KeyboardInterrupt:
            pass
        
        finally:
            self.stop()

def main():
    """메인 함수"""
    dashboard = DashboardWithLogger()
    
    try:
        dashboard.run_interactive()
    except KeyboardInterrupt:
        print("\n⏹️ 사용자가 프로그램을 중단했습니다.")
    except Exception as e:
        print(f"❌ 예상치 못한 오류: {e}")
        import traceback
        traceback.print_exc()
    finally:
        dashboard.stop()

if __name__ == "__main__":
    main()
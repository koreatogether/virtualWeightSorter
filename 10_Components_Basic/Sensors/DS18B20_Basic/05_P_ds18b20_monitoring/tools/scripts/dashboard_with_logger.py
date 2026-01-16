#!/usr/bin/env python3
"""
Dashboard runner with periodic logging helper (moved to tools/scripts)
"""
from pathlib import Path
import sys

project_root = Path(__file__).parents[2]
sys.path.insert(0, str(project_root))

from src.python.services.port_manager import port_manager
from src.python.services.sensor_logger import sensor_logger
from datetime import datetime
import time
import threading

class DashboardWithLogger:
    def __init__(self):
        self.running = False
        self.data_thread = None
        self.log_thread = None

    def start(self):
        print("🚀 DS18B20 대시보드 + 로거 시작")
        # Use scan + connect because PortManager may not implement auto_connect()
        def _auto_connect(pm):
            ports = pm.scan_ports()
            if not ports:
                return False
            return pm.connect(ports[0])

        success = _auto_connect(port_manager)
        if not success:
            print("❌ Arduino 연결 실패")
            return False

        current_port = port_manager.get_current_port()
        print(f"✅ {current_port} 연결 성공!")

        sensor_logger.set_log_interval(10)

        self.running = True
        self.data_thread = threading.Thread(target=self._data_collection_loop, daemon=True)
        self.log_thread = threading.Thread(target=self._periodic_summary_loop, daemon=True)

        self.data_thread.start()
        self.log_thread.start()

        return True

    def _data_collection_loop(self):
        while self.running:
            try:
                data = port_manager.read_sensor_data(timeout=2.0)
                if data:
                    current_time = datetime.now().strftime("%H:%M:%S")
                    temp = data.get('temperature', 0)
                    sensor_id = data.get('sensor_id', '00')
                    print(f"[{current_time}] 센서 {sensor_id}: {temp:.1f}°C")
                    sensor_logger.update_sensor_data(data)
                time.sleep(0.8)
            except Exception as e:
                print(f"❌ 데이터 수집 오류: {e}")
                time.sleep(1)

    def _periodic_summary_loop(self):
        while self.running:
            try:
                time.sleep(60)
                if sensor_logger.sensors:
                    print("\n🔄 1분 요약")
                    sensor_logger.print_system_summary()
            except Exception as e:
                print(f"❌ 요약 루프 오류: {e}")

    def stop(self):
        self.running = False
        if self.data_thread:
            self.data_thread.join(timeout=2)
        if self.log_thread:
            self.log_thread.join(timeout=2)
        try:
            port_manager.disconnect()
        except Exception:
            pass

    def run_interactive(self):
        if not self.start():
            return
        try:
            while self.running:
                command = input().strip().lower()
                if command == 'q':
                    break
                elif command == 's':
                    sensor_logger.print_system_summary()
                elif command == 'l':
                    try:
                        interval = int(input("새로운 로그 간격 (초): "))
                        sensor_logger.set_log_interval(interval)
                    except ValueError:
                        print("숫자를 입력하세요")
        finally:
            self.stop()

def main():
    dashboard = DashboardWithLogger()
    try:
        dashboard.run_interactive()
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    main()

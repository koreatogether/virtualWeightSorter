#!/usr/bin/env python3
"""간단한 Arduino 시뮬레이터 - ID 변경 테스트용"""

import threading
import time
import sys
import os

# 프로젝트 루트를 Python 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

class SimpleArduinoSimulator:
    def __init__(self):
        self.running = False
        
        # 가상 센서 데이터
        self.sensors = {
            "285882840000000E": {"id": "01", "temp": 23.5},
            "28E6AA830000005A": {"id": "02", "temp": 24.2}, 
            "28FF123456789ABC": {"id": "03", "temp": 22.8},
        }
        
    def start(self):
        """시뮬레이터 시작"""
        self.running = True
        print("🤖 간단한 Arduino 시뮬레이터 시작")
        print("📋 가상 센서 목록:")
        for addr, data in self.sensors.items():
            print(f"  - ID {data['id']}: {addr} ({data['temp']:.1f}°C)")
        
        print("\n💡 테스트 명령 (직접 입력):")
        print("  - SET_SENSOR_ID:285882840000000E:04")
        print("  - GET_SENSORS")
        print("  - quit")
        print("-" * 50)
        
        # 데이터 전송 스레드
        data_thread = threading.Thread(target=self.send_sensor_data)
        data_thread.daemon = True
        data_thread.start()
        
        # 사용자 입력 처리
        self.handle_user_input()
    
    def send_sensor_data(self):
        """센서 데이터 주기적 출력"""
        while self.running:
            try:
                for addr, data in self.sensors.items():
                    # 온도 약간 변동
                    base_temp = data["temp"]
                    variation = (time.time() % 20 - 10) * 0.1
                    current_temp = base_temp + variation
                    
                    # Arduino 형식으로 데이터 출력
                    message = f"SENSOR_DATA:{data['id']}:{addr}:{current_temp:.2f}"
                    print(f"📡 {message}")
                    
                time.sleep(3)  # 3초마다 전송
                
            except Exception as e:
                print(f"❌ 데이터 전송 오류: {e}")
                break
    
    def handle_command(self, command):
        """개별 명령 처리"""
        print(f"📨 수신된 명령: {command}")
        
        if command.startswith("SET_SENSOR_ID:"):
            self.handle_set_sensor_id(command)
        elif command == "GET_SENSORS":
            self.handle_get_sensors()
        else:
            print(f"❓ 알 수 없는 명령: {command}")
    
    def handle_set_sensor_id(self, command):
        """센서 ID 변경 명령 처리"""
        try:
            # SET_SENSOR_ID:주소:새ID 형식 파싱
            parts = command.split(":")
            if len(parts) != 3:
                response = "ERROR:Invalid command format"
                print(f"📤 {response}")
                return
                
            _, sensor_addr, new_id = parts
            
            # 센서 주소 확인
            if sensor_addr not in self.sensors:
                response = f"ERROR:Sensor not found: {sensor_addr}"
                print(f"📤 {response}")
                return
            
            # ID 중복 체크
            for addr, data in self.sensors.items():
                if addr != sensor_addr and data["id"] == new_id:
                    response = f"ERROR:ID {new_id} already in use"
                    print(f"📤 {response}")
                    return
            
            # ID 변경 실행
            old_id = self.sensors[sensor_addr]["id"]
            self.sensors[sensor_addr]["id"] = new_id
            
            response = f"SUCCESS:ID changed from {old_id} to {new_id}"
            print(f"✅ ID 변경 성공: {sensor_addr} ({old_id} → {new_id})")
            print(f"📤 {response}")
            
            # 변경된 센서 데이터 즉시 전송
            current_temp = self.sensors[sensor_addr]["temp"]
            new_data = f"SENSOR_DATA:{new_id}:{sensor_addr}:{current_temp:.2f}"
            print(f"📡 {new_data}")
            
        except Exception as e:
            response = f"ERROR:Failed to change ID: {str(e)}"
            print(f"❌ ID 변경 오류: {e}")
            print(f"📤 {response}")
    
    def handle_get_sensors(self):
        """센서 목록 요청 처리"""
        sensor_list = []
        for addr, data in self.sensors.items():
            sensor_list.append(f"{data['id']}:{addr}:{data['temp']:.2f}")
        
        response = "SENSORS:" + ",".join(sensor_list)
        print(f"📤 {response}")
    
    def handle_user_input(self):
        """사용자 입력 처리"""
        while self.running:
            try:
                user_input = input("시뮬레이터> ").strip()
                
                if user_input.lower() == 'quit':
                    self.stop()
                    break
                elif user_input:
                    self.handle_command(user_input)
                    
            except KeyboardInterrupt:
                self.stop()
                break
            except EOFError:
                self.stop()
                break
            except Exception as e:
                print(f"❌ 입력 처리 오류: {e}")
    
    def stop(self):
        """시뮬레이터 중지"""
        print("\n🛑 시뮬레이터 종료 중...")
        self.running = False

def main():
    """시뮬레이터 실행"""
    simulator = SimpleArduinoSimulator()
    
    try:
        simulator.start()
    except KeyboardInterrupt:
        print("\n⏹️  사용자가 중지했습니다")
    except Exception as e:
        print(f"❌ 시뮬레이터 오류: {e}")
    finally:
        simulator.stop()

if __name__ == "__main__":
    main()
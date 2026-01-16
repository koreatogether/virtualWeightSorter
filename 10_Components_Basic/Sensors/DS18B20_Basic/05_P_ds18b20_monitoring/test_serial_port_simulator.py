#!/usr/bin/env python3
"""실제 시리얼 포트를 시뮬레이션하는 Arduino 시뮬레이터"""

import threading
import time
import sys
import os
import socket
import select

# 프로젝트 루트를 Python 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

class SerialPortSimulator:
    def __init__(self, port=9999):
        self.port = port
        self.running = False
        self.server_socket = None
        self.client_socket = None
        
        # 가상 센서 데이터
        self.sensors = {
            "285882840000000E": {"id": "01", "temp": 23.5},
            "28E6AA830000005A": {"id": "02", "temp": 24.2}, 
            "28FF123456789ABC": {"id": "03", "temp": 22.8},
        }
        
    def start(self):
        """시뮬레이터 시작"""
        self.running = True
        
        # TCP 소켓으로 시리얼 포트 시뮬레이션
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.server_socket.bind(('localhost', self.port))
            self.server_socket.listen(1)
            
            print(f"🤖 시리얼 포트 시뮬레이터 시작 (포트: {self.port})")
            print("📋 가상 센서 목록:")
            for addr, data in self.sensors.items():
                print(f"  - ID {data['id']}: {addr} ({data['temp']:.1f}°C)")
            
            print(f"\n💡 대시보드 연결 방법:")
            print(f"  1. 대시보드 실행: python -m src.python.run")
            print(f"  2. COM 포트에서 'localhost:{self.port}' 선택")
            print("  3. 연결 버튼 클릭")
            print("-" * 50)
            
            while self.running:
                try:
                    print("📡 클라이언트 연결 대기 중...")
                    self.client_socket, addr = self.server_socket.accept()
                    print(f"✅ 클라이언트 연결됨: {addr}")
                    
                    # 데이터 전송 스레드 시작
                    data_thread = threading.Thread(target=self.send_sensor_data)
                    data_thread.daemon = True
                    data_thread.start()
                    
                    # 명령 수신 처리
                    self.handle_commands()
                    
                except socket.error as e:
                    if self.running:
                        print(f"❌ 연결 오류: {e}")
                        time.sleep(1)
                        
        except Exception as e:
            print(f"❌ 시뮬레이터 오류: {e}")
        finally:
            self.cleanup()
    
    def send_sensor_data(self):
        """센서 데이터 주기적 전송"""
        while self.running and self.client_socket:
            try:
                for addr, data in self.sensors.items():
                    # 온도 약간 변동
                    base_temp = data["temp"]
                    variation = (time.time() % 20 - 10) * 0.1
                    current_temp = base_temp + variation
                    
                    # Arduino 형식으로 데이터 전송
                    message = f"SENSOR_DATA:{data['id']}:{addr}:{current_temp:.2f}\\n"
                    self.client_socket.send(message.encode())
                    print(f"📡 전송: SENSOR_DATA:{data['id']}:{addr}:{current_temp:.2f}")
                    
                time.sleep(3)  # 3초마다 전송
                
            except socket.error:
                print("📡 클라이언트 연결 끊어짐")
                break
            except Exception as e:
                print(f"❌ 데이터 전송 오류: {e}")
                break
    
    def handle_commands(self):
        """Arduino 명령 처리"""
        buffer = ""
        
        while self.running and self.client_socket:
            try:
                # 논블로킹 소켓으로 데이터 수신 확인
                ready = select.select([self.client_socket], [], [], 0.1)
                if ready[0]:
                    data = self.client_socket.recv(1024).decode()
                    if not data:
                        break
                        
                    buffer += data
                    print(f"📨 수신된 데이터: {repr(data)}")
                    
                    # 완전한 명령 처리
                    while '\\n' in buffer:
                        line, buffer = buffer.split('\\n', 1)
                        if line.strip():
                            self.process_command(line.strip())
                            
            except socket.error:
                break
            except Exception as e:
                print(f"❌ 명령 처리 오류: {e}")
                break
    
    def process_command(self, command):
        """개별 명령 처리"""
        print(f"🔧 처리할 명령: {command}")
        
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
                response = "ERROR:Invalid command format\\n"
                self.send_response(response)
                return
                
            _, sensor_addr, new_id = parts
            
            # 센서 주소 확인
            if sensor_addr not in self.sensors:
                response = f"ERROR:Sensor not found: {sensor_addr}\\n"
                self.send_response(response)
                return
            
            # ID 중복 체크
            for addr, data in self.sensors.items():
                if addr != sensor_addr and data["id"] == new_id:
                    response = f"ERROR:ID {new_id} already in use\\n"
                    self.send_response(response)
                    return
            
            # ID 변경 실행
            old_id = self.sensors[sensor_addr]["id"]
            self.sensors[sensor_addr]["id"] = new_id
            
            response = f"SUCCESS:ID changed from {old_id} to {new_id}\\n"
            print(f"✅ ID 변경 성공: {sensor_addr} ({old_id} → {new_id})")
            self.send_response(response)
            
            # 변경된 센서 데이터 즉시 전송
            current_temp = self.sensors[sensor_addr]["temp"]
            new_data = f"SENSOR_DATA:{new_id}:{sensor_addr}:{current_temp:.2f}\\n"
            self.client_socket.send(new_data.encode())
            print(f"📡 즉시 전송: SENSOR_DATA:{new_id}:{sensor_addr}:{current_temp:.2f}")
            
        except Exception as e:
            response = f"ERROR:Failed to change ID: {str(e)}\\n"
            print(f"❌ ID 변경 오류: {e}")
            self.send_response(response)
    
    def handle_get_sensors(self):
        """센서 목록 요청 처리"""
        sensor_list = []
        for addr, data in self.sensors.items():
            sensor_list.append(f"{data['id']}:{addr}:{data['temp']:.2f}")
        
        response = f"SENSORS:{','.join(sensor_list)}\\n"
        self.send_response(response)
    
    def send_response(self, message):
        """응답 전송"""
        try:
            if self.client_socket:
                self.client_socket.send(message.encode())
                print(f"📤 응답 전송: {message.strip()}")
        except Exception as e:
            print(f"❌ 응답 전송 오류: {e}")
    
    def cleanup(self):
        """리소스 정리"""
        print("🛑 시뮬레이터 종료 중...")
        self.running = False
        
        if self.client_socket:
            try:
                self.client_socket.close()
            except:
                pass
                
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass

def main():
    """시뮬레이터 실행"""
    simulator = SerialPortSimulator()
    
    try:
        simulator.start()
    except KeyboardInterrupt:
        print("\\n⏹️  사용자가 중지했습니다")
    except Exception as e:
        print(f"❌ 시뮬레이터 오류: {e}")
    finally:
        simulator.cleanup()

if __name__ == "__main__":
    main()
"""다양한 통신 속도로 Arduino 테스트"""
import serial
import time

def test_different_baudrates():
    """여러 통신 속도로 테스트"""
    port = "COM4"
    baudrates = [9600, 115200, 57600, 38400, 19200, 4800]
    
    for baudrate in baudrates:
        print(f"\\n=== {baudrate} bps 테스트 ===")
        try:
            # 연결
            ser = serial.Serial(port, baudrate, timeout=2)
            time.sleep(1)  # 안정화
            
            # 기존 데이터 클리어
            ser.reset_input_buffer()
            
            # 데이터 읽기 시도
            print("초기 데이터 확인...")
            if ser.in_waiting > 0:
                data = ser.read(min(100, ser.in_waiting))
                print(f"원시 데이터: {data[:50]}...")  # 처음 50바이트만
                try:
                    decoded = data.decode('utf-8', errors='replace')
                    print(f"디코딩 결과: '{decoded[:100]}'")
                except:
                    print("디코딩 불가")
            
            # ping 테스트
            print("ping 테스트...")
            ser.write(b'ping\\n')
            time.sleep(1)
            
            if ser.in_waiting > 0:
                response = ser.readline()
                try:
                    decoded_response = response.decode('utf-8').strip()
                    print(f"응답: '{decoded_response}'")
                    if 'pong' in decoded_response.lower():
                        print("✅ PING-PONG 성공!")
                        ser.close()
                        return baudrate
                except:
                    print(f"응답 디코딩 실패: {response}")
            else:
                print("응답 없음")
            
            ser.close()
            
        except Exception as e:
            print(f"오류: {e}")
    
    print("\\n❌ 모든 통신 속도에서 실패")
    return None

if __name__ == "__main__":
    working_baudrate = test_different_baudrates()
    if working_baudrate:
        print(f"\\n🎯 권장 설정: {working_baudrate} bps")
    else:
        print("\\n💡 Arduino 펌웨어를 새로 업로드하세요.")
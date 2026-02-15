import serial
import time
import sys
from serial.tools import list_ports

# 설정: 아두이노와 PC 연결 속도 (Bridge 코드의 Serial.begin 값과 일치해야 함)
USB_BAUD_RATE = 115200 
TIMEOUT = 2.0

def test_bridge_system():
    # 1. 포트 선택
    ports = list_ports.comports()
    if not ports:
        print("❌ 연결된 시리얼 포트가 없습니다. 아두이노를 연결해주세요.")
        return

    print("\n=== 연결된 포트 목록 ===")
    for i, port in enumerate(ports):
        print(f"[{i}] {port.device} - {port.description}")
    
    try:
        idx = int(input("\n아두이노가 연결된 포트 번호를 입력하세요 [0]: ") or 0)
        target_port = ports[idx].device
    except:
        print("잘못된 입력입니다.")
        return

    try:
        ser = serial.Serial(target_port, USB_BAUD_RATE, timeout=TIMEOUT)
        print(f"\n✅ {target_port} 포트 열림 (속도: {USB_BAUD_RATE})")
    except Exception as e:
        print(f"❌ 포트 열기 실패: {e}")
        print("VS Code의 시리얼 모니터가 켜져있다면 꺼주세요!")
        return

    # -----------------------------------------------------------------
    # 테스트 1: Nextion 연결 확인 (Ping)
    # -----------------------------------------------------------------
    print("\n[단계 1] Nextion 연결 테스트 (Connect Ping)")
    print("아두이노가 Bridge Mode인지 확인하세요 (LED 켜짐).")
    print("Nextion에 'connect' 명령을 보냅니다...")

    # Nextion Editor가 보내는 표준 연결 시퀀스
    # 1. 버퍼 클리어 (0xFF 0xFF 0xFF)
    ser.write(b'\xff\xff\xff')
    time.sleep(0.05)
    
    # 2. connect 명령
    # Nextion은 연결되면 'comok ...' 같은 응답을 보냅니다.
    # 단, Nextion 속도가 9600이 아닐 경우 응답이 깨질 수 있습니다.
    cmd = b'connect\xff\xff\xff'
    ser.write(cmd)
    
    print(">> 전송함: connect")
    
    start_time = time.time()
    received_data = b''
    
    while time.time() - start_time < 3.0: # 3초간 대기
        if ser.in_waiting:
            chunk = ser.read(ser.in_waiting)
            received_data += chunk
            # 'comok' 문자열이 포함되어 있으면 성공
            if b'comok' in received_data or b'\x00\x00\x00\xff\xff\xff' in received_data:
                break
        time.sleep(0.1)

    print(f">> 수신됨({len(received_data)} bytes): {received_data}")

    if b'comok' in received_data:
        print("\n⭐⭐⭐ [TEST PASS] Nextion 연결 성공! ⭐⭐⭐")
        print("브릿지가 정상 작동 중이며, Nextion도 응답하고 있습니다.")
        print("Nextion Editor에서 업로드하시면 됩니다.")
        ser.close()
        return
    elif len(received_data) > 0:
        print("\n⚠️ [데이터 수신됨] 응답은 왔지만 'comok'이 아닙니다.")
        print("가능성:")
        print("1. Nextion Baudrate가 맞지 않음 (9600 vs 115200)")
        print("   -> Nextion 전원을 뺐다 꽂아서 재부팅 해보세요.")
        print("2. 이미 연결된 상태")
    else:
        print("\n❌ [무응답] 아무런 데이터도 오지 않았습니다.")

    # -----------------------------------------------------------------
    # 테스트 2: 아두이노 브릿지 자체 점검 (Loopback)
    # -----------------------------------------------------------------
    print("\n-----------------------------------------------------------")
    print("[단계 2] 아두이노 브릿지 기능 점검 (Loopback Test)")
    print("아두이노가 PC의 데이터를 0/1번 핀으로 잘 전달하는지 확인합니다.")
    print("👉 아두이노의 0번(RX)과 1번(TX) 핀을 점퍼선으로 서로 연결해주세요 (쇼트).")
    print("   (Nextion 선은 잠시 빼두셔도 됩니다)")
    
    input("준비되었으면 Enter를 누르세요...")

    test_msg = b'BridgeTest_12345'
    ser.write(test_msg)
    print(f">> 전송함: {test_msg}")
    
    time.sleep(1.0)
    
    loopback_data = b''
    if ser.in_waiting:
        loopback_data = ser.read(ser.in_waiting)
    
    print(f">> 수신됨: {loopback_data}")

    if test_msg in loopback_data:
        print("\n⭐⭐⭐ [TEST PASS] 아두이노 브릿지 정상! ⭐⭐⭐")
        print("PC -> 아두이노(USB) -> 핀(TX) -> 핀(RX) -> 아두이노(USB) -> PC 확인됨.")
        print(">> 결론: 아두이노 코드는 완벽합니다. 문제는 'Nextion 배선'이나 'Nextion 전원'입니다.")
    else:
        print("\n❌ [TEST FAIL] 루프백 실패.")
        print("가능성:")
        print("1. 아두이노가 Bridge Mode가 아님 (LED 꺼짐?)")
        print("2. 점퍼선 접촉 불량")
        print("3. 코드상에서 Serial/Serial1 포트 설정 오류")

    ser.close()

if __name__ == "__main__":
    # pyserial 설치 안내
    try:
        import serial
        test_bridge_system()
    except ImportError:
        print("Error: 'pyserial' 라이브러리가 필요합니다.")
        print("터미널에서 'pip install pyserial'을 실행해주세요.")

# rev01 – PMS7003 Passive Mode 예제 스케치 설명

## 개요
`rev01.ino` 파일은 **PMS7003** 레이저 미세먼지 센서를 **Passive Mode** 로 동작시켜 데이터를 요청하고, 32 바이트 패킷을 검증한 뒤 `PM1.0`, `PM2.5`, `PM10` 농도를 시리얼 모니터에 출력하는 Arduino 예제입니다.

이 문서는 **Arduino Uno R4 WiFi** 보드를 기준으로 작성되었으며, **HardwareSerial(Serial1)** 사용의 중요성과 안정적인 통신을 위한 **타이밍 및 버퍼 처리** 기법을 중점적으로 설명합니다.

## 파일 구조
```
rev01.ino
├─ 상수 정의 (핀 번호, 시리얼 보드레이트, 패킷 크기)
├─ 명령 프레임 (Passive Mode 전환, 데이터 요청)
├─ Helper 함수
│   ├─ calculateChecksum()   // 체크섬 계산
│   ├─ readPacket()          // 패킷 수신 및 검증 (타임아웃 강화)
│   └─ printConcentrations() // PM 농도 출력
└─ Arduino lifecycle
    ├─ setup()   // 시리얼 초기화, 센서 Passive Mode 전환
    └─ loop()    // 버퍼 비우기 → 데이터 요청 → readPacket → printConcentrations
```

## 주요 기술적 포인트 (Troubleshooting 결과 반영)

### 1. HardwareSerial vs SoftwareSerial
PMS7003 센서는 **3.3V 로직 레벨**을 사용합니다. 5V 기반의 아두이노 보드에서 이를 안정적으로 수신하기 위해서는 **HardwareSerial (Serial1)** 사용이 필수적입니다.

| 구분 | SoftwareSerial (일반 핀) | HardwareSerial (0, 1번 핀) |
| :--- | :--- | :--- |
| **동작 방식** | CPU가 직접 시간을 재며 비트를 읽음 (Bit-banging) | 전용 UART 칩이 데이터를 수신하고 버퍼에 저장 |
| **안정성** | CPU 부하가 높거나 인터럽트 발생 시 데이터 깨짐 발생 | CPU 상태와 무관하게 안정적인 수신 가능 |
| **전압 감도** | 3.3V 신호를 HIGH(1)로 인식하지 못할 수 있음 (불안정) | **입력 감도가 우수**하여 3.3V 신호도 정확히 인식 |
| **결론** | **비권장** (특히 Uno R4 WiFi 등 고성능 보드에서) | **강력 권장** (안정적인 통신 보장) |

> **[참고]** Arduino Uno R4 WiFi는 0번(RX), 1번(TX) 핀이 USB 통신과 분리된 독립적인 `Serial1` 입니다.

### 2. 타이밍 및 버퍼 처리 강화
통신 오류(Waiting for valid data...)를 방지하기 위해 다음과 같은 로직이 추가되었습니다.

#### A. 버퍼 비우기 (Flush Buffer)
```cpp
// loop() 시작 부분
while(Serial1.available()) Serial1.read();
```
- **이유:** 이전에 요청했거나 노이즈로 인해 들어온 **오래된 데이터(Garbage)**가 버퍼에 남아있을 수 있습니다.
- **해결:** 명령을 보내기 직전에 버퍼를 깨끗이 비워, 내가 요청한 데이터만 정확히 받도록 합니다.

#### B. 타임아웃(Timeout) 증가
```cpp
// readPacket() 내부
if (millis() - start > 1000) return false; // 1000ms (1초)
```
- **이유:** 센서 상태나 케이블 길이에 따라 응답이 0.5초보다 늦게 올 수 있습니다.
- **해결:** 타임아웃을 500ms에서 **1000ms**로 늘려 수신 성공률을 높였습니다.

## 주요 코드 설명

### 1. 상수 정의 (Uno R4 WiFi 기준)
```cpp
// Arduino Uno R4 WiFi uses Serial1 on pins 0 and 1
const uint32_t SERIAL_BAUD = 115200; // PC 통신 (USB)
const uint32_t PMS_BAUD    = 9600;   // 센서 통신 (Serial1)
const uint16_t PACKET_SIZE = 32;
```

### 2. `setup()`
```cpp
void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(PMS_BAUD); // Hardware Serial1 초기화
  delay(1000);
  
  // 초기 버퍼 비우기
  while(Serial1.available()) Serial1.read();

  // Passive Mode 전환
  Serial1.write(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
  delay(100);
}
```

### 3. `loop()`
```cpp
void loop() {
  // 1. 버퍼 비우기 (중요!)
  while(Serial1.available()) Serial1.read();

  // 2. 데이터 요청
  Serial1.write(CMD_READ_DATA, sizeof(CMD_READ_DATA));
  
  // 3. 응답 대기 (센서 처리 시간 확보)
  delay(200);

  // 4. 패킷 읽기 및 출력
  uint8_t packet[PACKET_SIZE];
  if (readPacket(packet)) {
    printConcentrations(packet);
  } else {
    Serial.println(F("Waiting for valid data..."));
  }
  delay(2000);
}
```

## 사용 방법 (Arduino Uno R4 WiFi)
1. **배선 연결**:
   - **PMS TX** → **Arduino Pin 0 (RX)**
   - **PMS RX** → **Arduino Pin 1 (TX)**
   - **VCC** → 5V
   - **GND** → GND
2. **업로드**: `rev01.ino` 를 업로드합니다.
3. **시리얼 모니터**: **115200 bps** 로 설정하여 결과를 확인합니다.

## 참고 문서
- `docs/pms7003/pms7003_communication.md` – 통신 프로토콜 상세 설명
- `docs/pms7003/pms7003_attentions.md` – 회로·설치·기타 주의사항

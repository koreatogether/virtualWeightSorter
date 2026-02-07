# Seeed Studio XIAO ESP32-C3 Hardware Serial Example

이 예제는 Seeed Studio XIAO ESP32-C3 보드에서 추가 하드웨어 시리얼(Serial1)을 사용하여 외부 장치(Nextion LCD, 센서 등)와 통신하는 방법을 보여줍니다.

## 1. 하드웨어 시리얼 핀 정보

ESP32-C3 칩셋은 유연한 GPIO 매핑을 지원합니다. XIAO ESP32-C3에서 권장되는 매핑은 다음과 같습니다:

| 기능 | 핀 번호 (XIAO Label) | GPIO 번호 | 용도 |
| :--- | :--- | :--- | :--- |
| **Serial** | Built-in USB | - | 디버깅 및 업로드용 |
| **Serial1 TX** | **D4** | **GPIO 6** | 외부 장치로 전송 |
| **Serial1 RX** | **D5** | **GPIO 7** | 외부 장치로부터 수신 |

### 하드웨어 권장 핀 (U0TXD / U0RXD)
칩셋 레벨에서 기본 UART0 핀은 다음과 같으며, `Serial`을 USB 대신 물리 핀으로 사용할 때 참조하십시오:
- **TX**: D6 (GPIO 21)
- **RX**: D7 (GPIO 20)

## 2. 사용 방법

1. `main.cpp` 코드를 작성합니다.
2. `platformio.ini` 설정에서 `board = seeed_xiao_esp32c3`를 선택합니다.
3. 코드를 업로드합니다.
4. 시리얼 모니터(115200 baud)를 열면 Serial1로 데이터를 송수신할 수 있습니다.

## 3. 코드 포인트

```cpp
// Serial1 초기화 (Baud rate, Protocol, RX, TX)
Serial1.begin(9600, SERIAL_8N1, 7, 6); 
```
- `7`은 XIAO D5 (RX) 핀입니다.
- `6`은 XIAO D4 (TX) 핀입니다.

## 4. 연결 예시 (Nextion LCD 등)
- XIAO **D4 (TX)** <---> 장치 **RX**
- XIAO **D5 (RX)** <---> 장치 **TX**
- XIAO **GND** <---> 장치 **GND**
- XIAO **3V3/5V** <---> 장치 **VCC**

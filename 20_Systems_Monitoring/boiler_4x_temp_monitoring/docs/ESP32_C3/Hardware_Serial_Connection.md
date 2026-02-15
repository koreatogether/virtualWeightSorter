# XIAO ESP32-C3 Hardware Serial (UART) 연결 가이드

Seeed Studio XIAO ESP32-C3 보드에는 총 3개의 UART 장치가 있으며, 이를 통해 다양한 시리얼 통신을 수행할 수 있습니다.

## 1. UART 장치 구성
*   **USB CDC (Serial over USB)**: 기본 PC 연결용 시리얼.
*   **UART0 (Hardware UART)**: 기본 하드웨어 시리얼 0.
*   **UART1 (Hardware UART)**: 기본 하드웨어 시리얼 1.

> [!NOTE]
> ESP32-C3의 하드웨어 UART 장치는 사용 가능한 모든 GPIO 핀에 할당할 수 있습니다.

## 2. 핀 맵핑 (Default)

### UART0 (기본값)
*   **TX**: D6 (GPIO 21)
*   **RX**: D7 (GPIO 20)

### UART1 (권장 핀)
*   **TX**: D10 (GPIO 10)
*   **RX**: D9 (GPIO 9)

## 3. 코드 구현 방법

Arduino 프레임워크에서 `HardwareSerial` 라이브러리를 사용하여 시리얼 포트를 정의하고 초기화할 수 있습니다.

```cpp
#include <HardwareSerial.h>

// UART1 인스턴스 생성 (1번 장치 사용)
HardwareSerial MySerial1(1);

void setup() {
    // USB 시리얼 초기화 (PC 모니터링용)
    Serial.begin(115200);

    // UART1 초기화
    // MySerial1.begin(baudrate, config, rxPin, txPin);
    MySerial1.begin(115200, SERIAL_8N1, 9, 10); 
    
    MySerial1.println("UART1 Initialized");
}

void loop() {
    // 사용 예시
    if (MySerial1.available()) {
        Serial.write(MySerial1.read());
    }
}
```

## 4. 주의사항 (Important)
> [!IMPORTANT]
> **UART0** 포트는 부팅 시 또는 시스템 크래시 발생 시 진단(Diagnostic) 메시지가 자동으로 출력됩니다. 
> 만약 외부 장치(예: LoRa 모뎀, 센서 등)가 부팅 시 메시지에 민감하게 반응한다면, UART1 사용을 권장하거나 퓨즈 비트(Fuse bits) 설정을 통해 진단 메시지 출력을 비활성화해야 할 수도 있습니다.

---
**출처**: [Seeed Studio Forum - How to use Serial1 with XIAO ESP32C3](https://forum.seeedstudio.com/t/how-to-use-serial1-with-xiao-esp32c3/266306/5)

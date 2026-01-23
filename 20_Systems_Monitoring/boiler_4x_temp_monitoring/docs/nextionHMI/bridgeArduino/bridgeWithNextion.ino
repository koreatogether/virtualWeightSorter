/**
 * Arduino R4 WiFi - Nextion Serial Bridge
 *
 * Hardware Connection (SoftwareSerial 예시):
 * Nextion TX -> Arduino R4 Pin 2 (RX)
 * Nextion RX -> Arduino R4 Pin 3 (TX)
 * Nextion GND -> Arduino R4 GND
 * Nextion +5V -> Arduino R4 5V (3.5인치 이상 모델은 외부 전원 권장)
 */

#include <SoftwareSerial.h>

// SoftwareSerial 핀 (필요 시 변경)
constexpr uint8_t NEXTION_RX_PIN = 2; // Arduino RX (Nextion TX)
constexpr uint8_t NEXTION_TX_PIN = 3; // Arduino TX (Nextion RX)
constexpr uint32_t NEXTION_BAUD = 115200;

SoftwareSerial nextionSerial(NEXTION_RX_PIN, NEXTION_TX_PIN);

void setup()
{
    // PC(USB) Baudrate: 115200 (R4 WiFi의 USB는 가상 포트라 속도가 매우 빠릅니다)
    Serial.begin(115200);

    // Nextion SoftwareSerial
    // Nextion 기본값은 9600입니다.
    nextionSerial.begin(NEXTION_BAUD);

    // [추가된 부분] offcircuit/Nextion 라이브러리 기반 강제 연결 시퀀스
    // 디스플레이가 대기 모드이거나 통신이 꼬인 경우를 대비해 '깨우기' 커맨드를 보냅니다.
    delay(500);
    nextionSerial.print("DRAKJHSUYDGBNCJHGJKSHBDN"); // 일부 모델용 범용 웨이크업
    nextionSerial.write(0xFF);
    nextionSerial.write(0xFF);
    nextionSerial.write(0xFF);
    nextionSerial.print("connect");
    nextionSerial.write(0xFF);
    nextionSerial.write(0xFF);
    nextionSerial.write(0xFF);

    // 시리얼 연결 대기
    while (!Serial && millis() < 3000)
        ;
}

void loop()
{
    // [수정] Arduino R4 WiFi의 Serial 객체에는 baud() 메서드가 상 표준 API에 포함되어 있지 않아 삭제했습니다.
    // Nextion Editor에서 업로드 시 속도가 자동으로 변경되기를 기다리는 대신,
    // 기본적으로 9600으로 동작하며 필요 시 아래 속도를 직접 수정하여 업로드해야 합니다.

    // PC -> Nextion (Fast Passthrough)
    while (Serial.available())
    {
        nextionSerial.write(Serial.read());
    }

    // Nextion -> PC (Fast Passthrough)
    while (nextionSerial.available())
    {
        Serial.write(nextionSerial.read());
    }
}
/*
 * [Arduino R4 WiFi] Nextion Hardware Connection Confirmation Test
 *
 * 목적: 아두이노 R4 WiFi와 Nextion 디스플레이 간의 "하드웨어 연결(배선)"이 정상적인지 확인하는 가장 기초적인 코드입니다.
 *
 * 동작:
 * 1. 1초 간격으로 화면 밝기를 100% (최대) -> 10% (어두움) 으로 반복 변경합니다.
 * 2. 화면이 깜빡거린다면 하드웨어 배선(RX/TX)은 100% 정상입니다.
 *
 * [필수 배선 - Cross Connection]
 * - Arduino Pin 0 (RX) <---연결---> Nextion Blue   (TX)
 * - Arduino Pin 1 (TX) <---연결---> Nextion Yellow (RX)
 * - Arduino 5V         <---연결---> Nextion Red    (5V)
 * - Arduino GND        <---연결---> Nextion Black  (GND)
 *
 * [주의]
 * - 업로드 시 Nextion 선을 뺄 필요 없습니다 (R4는 USB와 Serial1이 분리됨).
 * - 테스트 후에는 다시 원래 프로젝트 코드를 업로드해야 기능이 복구됩니다.
 */

void setup()
{
    // 1. 디버깅용 USB 시리얼 (PC 시리얼 모니터 보기용)
    Serial.begin(115200);

    // 2. Nextion용 하드웨어 시리얼 (R4 WiFi는 Serial1을 사용해야 함)
    // Nextion의 기본 통신 속도는 9600입니다.
    Serial1.begin(9600);

    Serial.println("=============================================");
    Serial.println("   Nextion Hardware Connection Test Script   ");
    Serial.println("=============================================");
    Serial.println("Attempting to command Nextion...");

    // 안정화를 위해 2초 대기
    delay(2000);
}

void loop()
{
    // 테스트 1: 밝기 100% (dim=100)
    Serial.println("[Command] dim=100 (Bright)");
    nextionCommand("dim=100");
    delay(1000);

    // 테스트 2: 밝기 10% (dim=10)
    Serial.println("[Command] dim=10 (Dark)");
    nextionCommand("dim=10");
    delay(1000);
}

// Nextion 명령어 전송 헬퍼 함수
// 명령어 끝에 0xFF 0xFF 0xFF를 3번 보내야 Nextion이 실행합니다.
void nextionCommand(String cmd)
{
    Serial1.print(cmd);
    Serial1.write(0xFF);
    Serial1.write(0xFF);
    Serial1.write(0xFF);
}

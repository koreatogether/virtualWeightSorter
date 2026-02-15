/*
Mega 보드용 Nextion TFT 업로드 브리지 모드 (Serial Pass-through)

조건:
- PC와 연결된 USB: Serial (핀 0/1)
- Nextion 디스플레이 연결: Serial1 (핀 18 RX1, 19 TX1) 또는 Serial2 (핀 16 RX2, 17 TX2)
  (이 예제는 Serial2 기준)

동작:
- PC(USB)에서 들어오는 데이터를 Nextion(Serial2)으로 전달
- Nextion에서 들어오는 데이터를 PC(USB)로 전달
- Nextion Editor에서 "Upload" 버튼을 누르면 이 경로를 통해 TFT 파일이 전송됨

주의:
- Nextion Editor 업로드 시 Baudrate가 자동으로 변경되므로,
  Serial 포트 속도는 상황에 맞게 유동적으로 처리하거나 고속(115200 등)으로 고정할 수도 있으나,
  가장 확실한 방법은 단순 바이트 패스스루 루프를 돌리는 것임.
*/

/*
현재의 코드로 해서 테스트 해본 결과 9600 속도까지는 지원되나 115200처럼 고속으로 하려고하면 실패를 하게됨
*/

// Nextion이 연결된 포트 지정 (Serial1, Serial2, Serial3 중 선택)
#define NEXTION_SERIAL Serial2

void setup()
{
    // PC 연결 (USB)
    Serial.begin(9600);

    // Nextion 연결
    // Nextion Editor 기본 업로드 속도는 9600에서 시작하여 협상 후 변경됨
    NEXTION_SERIAL.begin(9600);
}

void loop()
{
    // PC -> Nextion
    if (Serial.available())
    {
        NEXTION_SERIAL.write(Serial.read());
    }

    // Nextion -> PC
    if (NEXTION_SERIAL.available())
    {
        Serial.write(NEXTION_SERIAL.read());
    }
}

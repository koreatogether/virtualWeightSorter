// pms7003_raw_sniffer.ino
// 가장 단순한 형태의 수신 테스트 프로그램입니다.
// 센서에 아무런 명령도 보내지 않고, 들어오는 데이터가 있는지 확인만 합니다.
//
// [업데이트] Arduino Uno R4 WiFi (Serial1) 대응
// 배선: PMS TX -> Arduino 0 (RX), PMS RX -> Arduino 1 (TX)

// -----------------------------------------------------------------------------
// 설정
// -----------------------------------------------------------------------------
// Arduino Uno R4 WiFi는 0, 1번 핀이 Serial1 입니다.
const uint32_t SERIAL_BAUD = 115200;
const uint32_t PMS_BAUD = 9600;

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(PMS_BAUD); // Hardware Serial1 사용
  
  Serial.println(F("============================================="));
  Serial.println(F("   PMS7003 RAW SNIFFER (Serial1) - START"));
  Serial.println(F("============================================="));
  Serial.println(F("Target Board: Arduino Uno R4 WiFi"));
  Serial.println(F("Wiring: PMS TX->Pin 0, PMS RX->Pin 1"));
  Serial.println(F("1. Please POWER CYCLE (Unplug/Replug) the PMS7003 sensor now."));
  Serial.println(F("   (This resets it to default Active Mode)"));
  Serial.println(F("============================================="));
}

void loop() {
  if (Serial1.available()) {
    Serial.print(F("[DATA] "));
    while (Serial1.available()) {
      uint8_t c = Serial1.read();
      if (c < 0x10) Serial.print("0");
      Serial.print(c, HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

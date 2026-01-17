// ------------------------------------------------------------
// Advanced PMS7003 Serial Control Example (Arduino R4 WiFi)
// ------------------------------------------------------------
// - Board : Arduino R4 WiFi (5 V)
// - PC ↔ Arduino : Serial  (115200 bps)   <-- 사용자와 대화
// - Arduino ↔ PMS7003 : Serial1 (9600 bps) <-- 센서와 통신 (Passive/Active Switching)
// - 데이터 저장 : RAM (3 × 150 uint16_t)  <-- 5 분(2 s 간격) 기록
// ------------------------------------------------------------

#include <Arduino.h>

// ---------------------------
// 1️⃣  설정 상수
// ---------------------------
const uint32_t PC_BAUD      = 115200;   // PC와의 시리얼 속도 (유지)
const uint32_t PMS_BAUD     = 9600;     // PMS7003 UART 속도
const uint16_t SAMPLE_COUNT = 150;      // 5 분 / 2 s = 150 샘플
const uint16_t PACKET_SIZE  = 32;       // PMS7003 고정 패킷 길이

// ---------------------------
// 2️⃣  명령 프레임
// ---------------------------
const uint8_t CMD_PASSIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
const uint8_t CMD_ACTIVE_MODE[]  = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71};
const uint8_t CMD_READ_DATA[]    = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};

// ---------------------------
// 3️⃣  데이터 저장소 및 상태 변수
// ---------------------------
uint16_t pmHistory[3][SAMPLE_COUNT] = {0};
uint16_t sampleIdx = 0;          // 현재 저장 위치 (순환 버퍼)
bool isPassiveMode = true;       // 현재 모드 상태 (기본: Passive)

// ---------------------------
// 4️⃣  헬퍼 함수
// ---------------------------
uint16_t calculateChecksum(const uint8_t *buf) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < 30; ++i) sum += buf[i];
  return sum;
}

// 센서 패킷을 읽고 검증한다.
// Passive 모드일 때는 타임아웃을 체크하고, Active 모드일 때는 데이터가 있을 때만 읽는다.
bool readPacket(uint8_t *packet) {
  // 1000 ms 안에 전체 패킷이 들어올 때까지 대기
  unsigned long start = millis();
  while (Serial1.available() < PACKET_SIZE) {
    if (millis() - start > 1000) return false;   // 타임아웃
  }

  for (uint8_t i = 0; i < PACKET_SIZE; ++i) packet[i] = Serial1.read();

  // 헤더 확인
  if (packet[0] != 0x42 || packet[1] != 0x4D) return false;

  // 체크섬 검증
  uint16_t received = (packet[30] << 8) | packet[31];
  return calculateChecksum(packet) == received;
}

// 현재 샘플을 순환 버퍼에 저장
void storeSample(uint16_t pm1, uint16_t pm25, uint16_t pm10) {
  pmHistory[0][sampleIdx] = pm1;
  pmHistory[1][sampleIdx] = pm25;
  pmHistory[2][sampleIdx] = pm10;
  sampleIdx = (sampleIdx + 1) % SAMPLE_COUNT;
}

// 저장된 데이터를 CSV‑like 표 형태로 출력
void printTable() {
  Serial.println(F("Idx,PM1.0,PM2.5,PM10"));
  for (uint16_t i = 0; i < SAMPLE_COUNT; ++i) {
    Serial.print(i);
    Serial.print(',');
    Serial.print(pmHistory[0][i]);
    Serial.print(',');
    Serial.print(pmHistory[1][i]);
    Serial.print(',');
    Serial.println(pmHistory[2][i]);
  }
}

// 5 분(150 샘플) 평균을 구하고 전·후 변화 판단
void analyzeTrend() {
  // 현재 구간 평균
  uint32_t sumNow[3] = {0, 0, 0};
  for (uint16_t i = 0; i < SAMPLE_COUNT; ++i) {
    sumNow[0] += pmHistory[0][i];
    sumNow[1] += pmHistory[1][i];
    sumNow[2] += pmHistory[2][i];
  }
  float avgNow[3] = {
    sumNow[0] / (float)SAMPLE_COUNT,
    sumNow[1] / (float)SAMPLE_COUNT,
    sumNow[2] / (float)SAMPLE_COUNT
  };

  Serial.println(F("--- 5‑min Trend Analysis ---"));
  const char *labels[3] = {"PM1.0", "PM2.5", "PM10"};
  for (uint8_t k = 0; k < 3; ++k) {
    Serial.print(labels[k]);
    Serial.print(F(": "));
    Serial.print(avgNow[k], 1);
    Serial.println(F(" µg/m³"));
  }
}

// 저장된 모든 데이터를 0 으로 초기화
void resetData() {
  for (uint8_t k = 0; k < 3; ++k)
    for (uint16_t i = 0; i < SAMPLE_COUNT; ++i)
      pmHistory[k][i] = 0;
  sampleIdx = 0;
  Serial.println(F("DATA CLEARED"));
}

// 메뉴 출력
void printMenu() {
  Serial.println(F("\n---------------------------------------------"));
  Serial.println(F(" PMS7003 Control Menu"));
  Serial.println(F("---------------------------------------------"));
  Serial.println(F(" 1. Show PM 1.0"));
  Serial.println(F(" 2. Show PM 2.5"));
  Serial.println(F(" 3. Show PM 10"));
  Serial.println(F(" 4. Show All (PM 1.0 ~ 10)"));
  Serial.println(F(" 5. Recent 20s Data Table (Last 10 samples)"));
  Serial.println(F(" 6. Switch Mode (Active <-> Passive)"));
  Serial.println(F("---------------------------------------------"));
  Serial.println(F(" Enter your choice (1-6) or other commands:"));
}

// 최근 10개 샘플 (약 20초 데이터) 출력
void printRecentTable() {
  Serial.println(F("\n--- Recent 20s Data (Last 10 Samples) ---"));
  Serial.println(F("Idx\tPM1.0\tPM2.5\tPM10"));
  
  uint16_t startIdx = (sampleIdx + SAMPLE_COUNT - 10) % SAMPLE_COUNT;
  
  for (uint16_t i = 0; i < 10; ++i) {
    uint16_t current = (startIdx + i) % SAMPLE_COUNT;
    Serial.print(i + 1); // 1~10 표시
    Serial.print(F("\t"));
    Serial.print(pmHistory[0][current]);
    Serial.print(F("\t"));
    Serial.print(pmHistory[1][current]);
    Serial.print(F("\t"));
    Serial.println(pmHistory[2][current]);
  }
  Serial.println(F("---------------------------------------------"));
}

// 현재 모드 상태 출력
void printModeStatus() {
  if (isPassiveMode) {
    Serial.println(F(" [Mode: Passive]"));
  } else {
    Serial.println(F(" [Mode: Active]"));
  }
}

// 모드 전환 함수
void toggleMode() {
  if (isPassiveMode) {
    // Passive -> Active
    Serial1.write(CMD_ACTIVE_MODE, sizeof(CMD_ACTIVE_MODE));
    isPassiveMode = false;
    Serial.println(F("\n>> Switched to ACTIVE Mode."));
    Serial.println(F("   (Sensor sends data automatically every ~1s)"));
  } else {
    // Active -> Passive
    Serial1.write(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
    isPassiveMode = true;
    Serial.println(F("\n>> Switched to PASSIVE Mode."));
    Serial.println(F("   (Arduino requests data every 2s)"));
  }
  delay(100);
  
  // 모드 전환 시 버퍼 비우기 (이전 모드의 잔여 데이터 제거)
  while(Serial1.available()) Serial1.read();
  
  // 모드 전환 후 메뉴 다시 출력 (사용자가 바로 명령을 내릴 수 있도록)
  printMenu();
}

// 명령 파싱 및 실행
void handleCommand(const String &cmd) {
  String upperCmd = cmd;
  upperCmd.toUpperCase();

  if (upperCmd == "1") {
    Serial.print(F("[Current] PM 1.0 : "));
    Serial.print(pmHistory[0][(sampleIdx + SAMPLE_COUNT - 1) % SAMPLE_COUNT]);
    Serial.print(F(" µg/m³"));
    printModeStatus();
  } else if (upperCmd == "2") {
    Serial.print(F("[Current] PM 2.5 : "));
    Serial.print(pmHistory[1][(sampleIdx + SAMPLE_COUNT - 1) % SAMPLE_COUNT]);
    Serial.print(F(" µg/m³"));
    printModeStatus();
  } else if (upperCmd == "3") {
    Serial.print(F("[Current] PM 10  : "));
    Serial.print(pmHistory[2][(sampleIdx + SAMPLE_COUNT - 1) % SAMPLE_COUNT]);
    Serial.print(F(" µg/m³"));
    printModeStatus();
  } else if (upperCmd == "4") {
    uint16_t lastIdx = (sampleIdx + SAMPLE_COUNT - 1) % SAMPLE_COUNT;
    Serial.print(F("[Current] PM 1.0: ")); Serial.print(pmHistory[0][lastIdx]);
    Serial.print(F(" | PM 2.5: "));      Serial.print(pmHistory[1][lastIdx]);
    Serial.print(F(" | PM 10: "));       Serial.print(pmHistory[2][lastIdx]);
    printModeStatus();
  } else if (upperCmd == "5") {
    printRecentTable();
    printModeStatus();
  } else if (upperCmd == "6") {
    toggleMode();
  } else if (upperCmd == "GET TABLE") {
    printTable(); // 전체 데이터
  } else if (upperCmd == "ANALYZE") {
    analyzeTrend();
  } else if (upperCmd == "RESET") {
    resetData();
  } else if (upperCmd == "MENU" || upperCmd == "HELP") {
    printMenu();
  } else {
    Serial.println(F("UNKNOWN COMMAND. Type 'MENU' for options."));
  }
}

// ---------------------------
// 5️⃣  Arduino lifecycle
// ---------------------------
void setup() {
  // PC와의 시리얼
  Serial.begin(PC_BAUD);
  while (!Serial) ;               // USB 연결 대기

  // 센서와의 시리얼 (Hardware Serial1)
  Serial1.begin(PMS_BAUD);
  delay(1000);

  // 초기 버퍼 비우기
  while(Serial1.available()) Serial1.read();

  Serial.println(F("\n--- PMS7003 Advanced Serial Control (R4 WiFi) ---"));
  // 초기 상태: Passive Mode 로 설정
  Serial1.write(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
  isPassiveMode = true;
  delay(100);
  
  printMenu(); // 초기 메뉴 출력
}

void loop() {
  // 1) 데이터 수신 로직 (모드에 따라 다름)
  bool dataReceived = false;
  uint8_t packet[PACKET_SIZE];

  if (isPassiveMode) {
    // --- Passive Mode ---
    // 요청 -> 대기 -> 수신
    
    // 버퍼 비우기 (중요!)
    while(Serial1.available()) Serial1.read();
    
    // 데이터 요청
    Serial1.write(CMD_READ_DATA, sizeof(CMD_READ_DATA));
    delay(200); // 응답 대기
    
    if (readPacket(packet)) {
      dataReceived = true;
    } else {
       // Passive 모드에서 실패 시 로그
       Serial.println(F("Waiting for valid data..."));
    }
    
  } else {
    // --- Active Mode ---
    // 센서가 스스로 보냄 -> 버퍼에 쌓인 것 읽기
    
    // Active 모드에서는 데이터가 계속 들어오므로,
    // 패킷 크기만큼 쌓였는지 확인
    if (Serial1.available() >= PACKET_SIZE) {
      // 패킷 헤더(0x42, 0x4D)를 찾을 때까지 읽어버림 (동기화)
      if (Serial1.peek() != 0x42) {
        Serial1.read(); // 버림
      } else {
        if (readPacket(packet)) {
          dataReceived = true;
        }
      }
    }
  }

  // 2) 데이터 저장
  if (dataReceived) {
    uint16_t pm1  = (packet[10] << 8) | packet[11];
    uint16_t pm25 = (packet[12] << 8) | packet[13];
    uint16_t pm10 = (packet[14] << 8) | packet[15];
    storeSample(pm1, pm25, pm10);
    
    // Active 모드일 때만 데이터 출력 및 메뉴 주기적 표시
    if (!isPassiveMode) {
      Serial.print(F("[Active] PM1.0: "));
      Serial.print(pm1);
      Serial.print(F(" | PM2.5: "));
      Serial.print(pm25);
      Serial.print(F(" | PM10: "));
      Serial.print(pm10);
      Serial.println(F(" µg/m³"));

      static uint16_t activeDataCount = 0;
      activeDataCount++;
      
      // 10번 출력마다 메뉴 다시 보여주기
      if (activeDataCount % 10 == 0) {
        printMenu();
      }
    } else {
      // Passive 모드는 저장되었다는 점만 간단히 (또는 생략)
      // Serial.println(F("[Saved]"));
    } 
  }

  // 3) PC에서 들어온 명령 처리
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();                  // 앞·뒤 공백 제거
    handleCommand(cmd);
  }

  // 4) 루프 지연
  if (isPassiveMode) {
    delay(2000); // Passive 모드는 2초 간격으로 요청
  } else {
    // Active 모드는 센서가 약 0.8~1초마다 보내므로, 
    // 너무 길게 쉬면 버퍼가 넘칠 수 있음. 적당히 짧게 대기.
    delay(100); 
  }
}


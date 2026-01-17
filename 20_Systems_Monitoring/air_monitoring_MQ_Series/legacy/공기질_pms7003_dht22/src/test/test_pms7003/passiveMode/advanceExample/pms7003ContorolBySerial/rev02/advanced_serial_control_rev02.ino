// ------------------------------------------------------------
// Advanced PMS7003 Serial Control Example (Arduino R4 WiFi) - Rev02
// ------------------------------------------------------------
// - Board : Arduino R4 WiFi (5 V)
// - PC ↔ Arduino : Serial  (115200 bps)   <-- User Interface
// - Arduino ↔ PMS7003 : Serial1 (9600 bps) <-- Sensor Comm
// - Features:
//    1. Passive/Active Mode Switching
//    2. 5-min Data Logging (RAM)
//    3. RX/TX Real-time Monitoring (Hex/Dec/Bin)
// ------------------------------------------------------------

#include <Arduino.h>

// ---------------------------
// 1️⃣  Configuration Constants
// ---------------------------
const uint32_t PC_BAUD      = 115200;
const uint32_t PMS_BAUD     = 9600;
const uint16_t SAMPLE_COUNT = 150;      // 5 min / 2s = 150 samples
const uint16_t PACKET_SIZE  = 32;

// ---------------------------
// 2️⃣  Command Frames
// ---------------------------
const uint8_t CMD_PASSIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
const uint8_t CMD_ACTIVE_MODE[]  = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71};
const uint8_t CMD_READ_DATA[]    = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};

// ---------------------------
// 3️⃣  Global State & Data
// ---------------------------
uint16_t pmHistory[3][SAMPLE_COUNT] = {0};
uint16_t sampleIdx = 0;
bool isPassiveMode = true;

// --- Monitoring Settings ---
enum MonitorMode { MON_OFF, MON_RX, MON_TX, MON_BOTH };
enum DataFormat  { FMT_BIN, FMT_DEC, FMT_HEX };

struct {
  MonitorMode mode = MON_OFF;
  DataFormat format = FMT_HEX;
} monitor;

void printMenu(); // Forward declaration

// ---------------------------
// 4️⃣  Helper Functions
// ---------------------------

// Global line counter for monitor
uint16_t monitorLineCount = 0;

// Check if we need to pause
void checkMonitorPause() {
  if (monitorLineCount >= 10) {
    Serial.println(F("\n--- [Paused] 1. Stop Print  2. Menu  (Any other key to continue) ---"));
    while (!Serial.available()); // Wait for input
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "1") {
      monitor.mode = MON_OFF;
      Serial.println(F(">> Monitoring Stopped."));
    } else if (input == "2") {
      monitor.mode = MON_OFF;
      printMenu();
    } else {
      Serial.println(F(">> Continuing..."));
    }
    monitorLineCount = 0; // Reset counter
  }
}

// Print a single byte in the selected format
void printFormatted(uint8_t b) {
  if (monitor.format == FMT_BIN) {
    for (int i = 7; i >= 0; i--) Serial.print((b >> i) & 1);
    Serial.print(' ');
  } else if (monitor.format == FMT_DEC) {
    if (b < 100) Serial.print(' ');
    if (b < 10)  Serial.print(' ');
    Serial.print(b);
    Serial.print(' ');
  } else { // FMT_HEX
    if (b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
}

// Wrapper for Serial1.write to support TX monitoring
size_t pmsWrite(const uint8_t *buf, size_t size) {
  if (monitor.mode == MON_TX || monitor.mode == MON_BOTH) {
    Serial.print(F("[TX] "));
    for (size_t i = 0; i < size; i++) {
      printFormatted(buf[i]);
    }
    Serial.println();
    monitorLineCount++;
    checkMonitorPause();
  }
  return Serial1.write(buf, size);
}

// Wrapper for Serial1.read to support RX monitoring
int pmsRead() {
  int c = Serial1.read();
  if (c >= 0 && (monitor.mode == MON_RX || monitor.mode == MON_BOTH)) {
    static unsigned long lastRxTime = 0;
    // If new burst or just starting, print newline and prefix
    if (millis() - lastRxTime > 50) { 
      Serial.println(); 
      monitorLineCount++;
      checkMonitorPause();
      if (monitor.mode != MON_OFF) Serial.print(F("[RX] "));
    }
    lastRxTime = millis();
    
    if (monitor.mode != MON_OFF) printFormatted((uint8_t)c);
  }
  return c;
}

uint16_t calculateChecksum(const uint8_t *buf) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < 30; ++i) sum += buf[i];
  return sum;
}

bool readPacket(uint8_t *packet) {
  unsigned long start = millis();
  // Wait for enough data
  while (Serial1.available() < PACKET_SIZE) {
    if (millis() - start > 1000) return false;
  }

  // Read packet using wrapper
  for (uint8_t i = 0; i < PACKET_SIZE; ++i) {
    int c = pmsRead(); 
    if (c < 0) return false; // Should not happen due to check above
    packet[i] = (uint8_t)c;
  }

  // Validate Header
  if (packet[0] != 0x42 || packet[1] != 0x4D) return false;

  // Validate Checksum
  uint16_t received = (packet[30] << 8) | packet[31];
  return calculateChecksum(packet) == received;
}

void storeSample(uint16_t pm1, uint16_t pm25, uint16_t pm10) {
  pmHistory[0][sampleIdx] = pm1;
  pmHistory[1][sampleIdx] = pm25;
  pmHistory[2][sampleIdx] = pm10;
  sampleIdx = (sampleIdx + 1) % SAMPLE_COUNT;
}

void printTable() {
  Serial.println(F("Idx,PM1.0,PM2.5,PM10"));
  for (uint16_t i = 0; i < SAMPLE_COUNT; ++i) {
    Serial.print(i); Serial.print(',');
    Serial.print(pmHistory[0][i]); Serial.print(',');
    Serial.print(pmHistory[1][i]); Serial.print(',');
    Serial.println(pmHistory[2][i]);
  }
}

void analyzeTrend() {
  uint32_t sum[3] = {0};
  for (uint16_t i = 0; i < SAMPLE_COUNT; ++i) {
    sum[0] += pmHistory[0][i];
    sum[1] += pmHistory[1][i];
    sum[2] += pmHistory[2][i];
  }
  Serial.println(F("--- 5-min Trend Analysis ---"));
  const char *labels[3] = {"PM1.0", "PM2.5", "PM10"};
  for (int k = 0; k < 3; ++k) {
    Serial.print(labels[k]); Serial.print(F(": "));
    Serial.print(sum[k] / (float)SAMPLE_COUNT, 1);
    Serial.println(F(" µg/m³"));
  }
}

void resetData() {
  for (int k = 0; k < 3; ++k)
    for (int i = 0; i < SAMPLE_COUNT; ++i) pmHistory[k][i] = 0;
  sampleIdx = 0;
  Serial.println(F("DATA CLEARED"));
}

void printMonitorStatus() {
  Serial.print(F(" [Monitor: "));
  switch(monitor.mode) {
    case MON_OFF: Serial.print(F("OFF")); break;
    case MON_RX:  Serial.print(F("RX Only")); break;
    case MON_TX:  Serial.print(F("TX Only")); break;
    case MON_BOTH:Serial.print(F("RX & TX")); break;
  }
  Serial.print(F(" | Fmt: "));
  switch(monitor.format) {
    case FMT_BIN: Serial.print(F("BIN")); break;
    case FMT_DEC: Serial.print(F("DEC")); break;
    case FMT_HEX: Serial.print(F("HEX")); break;
  }
  Serial.println(F("]"));
}

void printMenu() {
  Serial.println(F("\n---------------------------------------------"));
  Serial.println(F(" PMS7003 Control Menu (Rev02)"));
  Serial.println(F("---------------------------------------------"));
  Serial.println(F(" 1. Show PM 1.0"));
  Serial.println(F(" 2. Show PM 2.5"));
  Serial.println(F(" 3. Show PM 10"));
  Serial.println(F(" 4. Show All (PM 1.0 ~ 10)"));
  Serial.println(F(" 5. Recent 20s Data Table"));
  Serial.println(F(" 6. Switch Mode (Active <-> Passive)"));
  Serial.println(F(" 7. Monitor Settings (RX/TX View)"));
  Serial.println(F("---------------------------------------------"));
  printMonitorStatus();
  Serial.println(F(" Enter choice (1-7) or commands (MENU, RESET)"));
}

void printRecentTable() {
  Serial.println(F("\n--- Recent 20s Data (Last 10 Samples) ---"));
  Serial.println(F("Idx\tPM1.0\tPM2.5\tPM10"));
  uint16_t startIdx = (sampleIdx + SAMPLE_COUNT - 10) % SAMPLE_COUNT;
  for (uint16_t i = 0; i < 10; ++i) {
    uint16_t curr = (startIdx + i) % SAMPLE_COUNT;
    Serial.print(i + 1); Serial.print(F("\t"));
    Serial.print(pmHistory[0][curr]); Serial.print(F("\t"));
    Serial.print(pmHistory[1][curr]); Serial.print(F("\t"));
    Serial.println(pmHistory[2][curr]);
  }
}

void toggleMode() {
  if (isPassiveMode) {
    pmsWrite(CMD_ACTIVE_MODE, sizeof(CMD_ACTIVE_MODE));
    isPassiveMode = false;
    Serial.println(F("\n>> Switched to ACTIVE Mode."));
  } else {
    pmsWrite(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
    isPassiveMode = true;
    Serial.println(F("\n>> Switched to PASSIVE Mode."));
  }
  delay(100);
  while(Serial1.available()) Serial1.read(); // Clear buffer
  printMenu();
}

void configureMonitor() {
  Serial.println(F("\n--- Monitor Configuration ---"));
  Serial.println(F(" [1] View: OFF"));
  Serial.println(F(" [2] View: RX Only"));
  Serial.println(F(" [3] View: TX Only"));
  Serial.println(F(" [4] View: BOTH"));
  Serial.println(F(" [5] Format: BIN"));
  Serial.println(F(" [6] Format: DEC"));
  Serial.println(F(" [7] Format: HEX"));
  Serial.println(F(" Enter choice: "));

  // Simple blocking wait for input to keep it simple
  while(!Serial.available());
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input == "1") monitor.mode = MON_OFF;
  else if (input == "2") monitor.mode = MON_RX;
  else if (input == "3") monitor.mode = MON_TX;
  else if (input == "4") monitor.mode = MON_BOTH;
  else if (input == "5") monitor.format = FMT_BIN;
  else if (input == "6") monitor.format = FMT_DEC;
  else if (input == "7") monitor.format = FMT_HEX;
  
  printMonitorStatus();
}

void handleCommand(const String &cmd) {
  String upperCmd = cmd;
  upperCmd.toUpperCase();

  if (upperCmd == "1") {
    Serial.print(F("PM 1.0: ")); Serial.println(pmHistory[0][(sampleIdx+SAMPLE_COUNT-1)%SAMPLE_COUNT]);
  } else if (upperCmd == "2") {
    Serial.print(F("PM 2.5: ")); Serial.println(pmHistory[1][(sampleIdx+SAMPLE_COUNT-1)%SAMPLE_COUNT]);
  } else if (upperCmd == "3") {
    Serial.print(F("PM 10:  ")); Serial.println(pmHistory[2][(sampleIdx+SAMPLE_COUNT-1)%SAMPLE_COUNT]);
  } else if (upperCmd == "4") {
    uint16_t last = (sampleIdx + SAMPLE_COUNT - 1) % SAMPLE_COUNT;
    Serial.print(F("PM1.0: ")); Serial.print(pmHistory[0][last]);
    Serial.print(F(" | PM2.5: ")); Serial.print(pmHistory[1][last]);
    Serial.print(F(" | PM10: ")); Serial.println(pmHistory[2][last]);
  } else if (upperCmd == "5") {
    printRecentTable();
  } else if (upperCmd == "6") {
    toggleMode();
  } else if (upperCmd == "7") {
    configureMonitor();
  } else if (upperCmd == "GET TABLE") {
    printTable();
  } else if (upperCmd == "ANALYZE") {
    analyzeTrend();
  } else if (upperCmd == "RESET") {
    resetData();
  } else if (upperCmd == "MENU" || upperCmd == "HELP") {
    printMenu();
  } else {
    Serial.println(F("UNKNOWN COMMAND"));
  }
}

// ---------------------------
// 5️⃣  Arduino Lifecycle
// ---------------------------
void setup() {
  Serial.begin(PC_BAUD);
  while (!Serial);

  Serial1.begin(PMS_BAUD);
  delay(1000);
  while(Serial1.available()) Serial1.read();

  Serial.println(F("\n--- PMS7003 Advanced Control Rev02 ---"));
  
  // Init Passive Mode
  pmsWrite(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
  isPassiveMode = true;
  delay(100);

  printMenu();
}

void loop() {
  bool dataReceived = false;
  uint8_t packet[PACKET_SIZE];

  if (isPassiveMode) {
    // Passive: Request -> Wait -> Read
    while(Serial1.available()) Serial1.read(); // Flush before request
    
    pmsWrite(CMD_READ_DATA, sizeof(CMD_READ_DATA));
    delay(200); 
    
    if (readPacket(packet)) {
      dataReceived = true;
    } else {
      // Serial.println(F("Wait...")); // Quiet in passive mode unless error
    }
  } else {
    // Active: Read if available
    if (Serial1.available() >= PACKET_SIZE) {
      if (Serial1.peek() != 0x42) {
        pmsRead(); // Consume garbage byte via wrapper
      } else {
        if (readPacket(packet)) dataReceived = true;
      }
    }
  }

  if (dataReceived) {
    uint16_t pm1  = (packet[10] << 8) | packet[11];
    uint16_t pm25 = (packet[12] << 8) | packet[13];
    uint16_t pm10 = (packet[14] << 8) | packet[15];
    storeSample(pm1, pm25, pm10);

    if (!isPassiveMode) {
      // Only print data if NOT monitoring raw stream to avoid clutter
      if (monitor.mode == MON_OFF) {
        Serial.print(F("[Active] PM2.5: ")); Serial.println(pm25);
      }
      
      static int count = 0;
      if (++count % 10 == 0 && monitor.mode == MON_OFF) printMenu();
    }
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    handleCommand(cmd);
  }

  if (isPassiveMode) delay(2000);
  else delay(100);
}

// passiveMode_debug.ino
// Debugging version of PMS7003 Passive Mode Example
// Created to diagnose communication issues (e.g., no data after 30s)
//
// [Updated] for Arduino Uno R4 WiFi (Hardware Serial1)
// Wiring: PMS TX -> Pin 0, PMS RX -> Pin 1

// -----------------------------------------------------------------------------
// Configuration constants
// -----------------------------------------------------------------------------
// Arduino Uno R4 WiFi uses Serial1 on pins 0 and 1
const uint32_t SERIAL_BAUD = 115200;
const uint32_t PMS_BAUD = 9600;
const uint16_t PACKET_SIZE = 32;

// -----------------------------------------------------------------------------
// Command frames
// -----------------------------------------------------------------------------
const uint8_t CMD_PASSIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
const uint8_t CMD_READ_DATA[]    = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};

// No SoftwareSerial needed for Uno R4 WiFi

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

uint16_t calculateChecksum(const uint8_t *buf) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < 30; ++i) sum += buf[i];
  return sum;
}

// Debug helper to print hex values
void printHex(uint8_t val) {
  if (val < 0x10) Serial.print("0");
  Serial.print(val, HEX);
  Serial.print(" ");
}

bool readPacket(uint8_t *packet) {
  // Clear buffer first to ensure we get fresh data response
  // But for debugging, let's see what's there first? 
  // Actually, standard practice is to clear before command, but here we just wait.
  
  unsigned long start = millis();
  Serial.print(F("[DEBUG] Waiting for data... Available: "));
  Serial.println(Serial1.available());

  // Wait up to 1000ms (increased from 500ms) for full packet
  while (Serial1.available() < PACKET_SIZE) {
    if (millis() - start > 1000) {
      Serial.print(F("[DEBUG] Timeout! Bytes available: "));
      int avail = Serial1.available();
      Serial.println(avail);
      
      if (avail > 0) {
        Serial.print(F("[DEBUG] Partial data received: "));
        while (Serial1.available()) {
          printHex(Serial1.read());
        }
        Serial.println();
      } else {
        Serial.println(F("[DEBUG] No data received. Check wiring (TX/RX swapped?) or power."));
      }
      return false;
    }
  }

  // Read packet
  Serial.println(F("[DEBUG] Reading packet..."));
  for (uint8_t i = 0; i < PACKET_SIZE; ++i) {
    packet[i] = Serial1.read();
    // Print raw bytes for debugging
    // printHex(packet[i]); 
  }
  // Serial.println(); // End raw dump

  // Validate Header
  if (packet[0] != 0x42 || packet[1] != 0x4D) {
    Serial.print(F("[DEBUG] Invalid Header: "));
    printHex(packet[0]); printHex(packet[1]);
    Serial.println();
    return false;
  }

  // Validate Checksum
  uint16_t received = (packet[30] << 8) | packet[31];
  uint16_t calculated = calculateChecksum(packet);
  
  if (received != calculated) {
    Serial.print(F("[DEBUG] Checksum Fail. Calc: "));
    Serial.print(calculated, HEX);
    Serial.print(F(", Recv: "));
    Serial.println(received, HEX);
    return false;
  }

  Serial.println(F("[DEBUG] Packet Valid!"));
  return true;
}

void printConcentrations(const uint8_t *packet) {
  uint16_t pm1_0 = (packet[10] << 8) | packet[11];
  uint16_t pm2_5 = (packet[12] << 8) | packet[13];
  uint16_t pm10  = (packet[14] << 8) | packet[15];
  Serial.print(F("PM1.0: ")); Serial.print(pm1_0); Serial.println(F(" ug/m3"));
  Serial.print(F("PM2.5: ")); Serial.print(pm2_5); Serial.println(F(" ug/m3"));
  Serial.print(F("PM10 : ")); Serial.print(pm10);  Serial.println(F(" ug/m3"));
}

// -----------------------------------------------------------------------------
// Arduino lifecycle
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(PMS_BAUD);
  delay(1000);
  
  Serial.println(F("============================================="));
  Serial.println(F("   PMS7003 DEBUG MODE - START"));
  Serial.println(F("============================================="));
  Serial.println(F("Target: Arduino Uno R4 WiFi"));
  Serial.println(F("RX Pin: 0 (Hardware Serial1)"));
  Serial.println(F("TX Pin: 1 (Hardware Serial1)"));
  Serial.println(F("Sending Passive Mode Command..."));
  
  // Clear any garbage
  while(Serial1.available()) Serial1.read();
  
  Serial1.write(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
  delay(100);
  Serial.println(F("Passive Mode Command Sent."));
}

void loop() {
  static unsigned long loopCount = 0;
  loopCount++;
  
  Serial.print(F("\n--- Loop #")); Serial.print(loopCount); Serial.println(F(" ---"));
  
  // Clear buffer before requesting new data to avoid sync issues
  int garbage = 0;
  while (Serial1.available()) {
    Serial1.read();
    garbage++;
  }
  if (garbage > 0) {
    Serial.print(F("[DEBUG] Cleared ")); Serial.print(garbage); Serial.println(F(" bytes of garbage/old data."));
  }

  Serial.println(F("[DEBUG] Sending Read Request..."));
  Serial1.write(CMD_READ_DATA, sizeof(CMD_READ_DATA));
  
  // Give sensor time to respond (datasheet says response is immediate, but SoftwareSerial needs care)
  // Wait a bit before checking
  delay(200); 

  uint8_t packet[PACKET_SIZE];
  if (readPacket(packet)) {
    printConcentrations(packet);
  } else {
    Serial.println(F("[DEBUG] Failed to read valid packet."));
  }
  
  delay(2000);
}

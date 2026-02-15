// rev01 – Passive Mode Example for PMS7003 Sensor
// This sketch is a refactored version of the original passiveMode_example.ino.
// It demonstrates switching the PMS7003 to passive mode, requesting a data frame,
// validating the checksum, and printing PM1.0, PM2.5, and PM10 concentrations.

// [Updated] for Arduino Uno R4 WiFi (Hardware Serial1)
// Wiring: PMS TX -> Pin 0, PMS RX -> Pin 1

// -----------------------------------------------------------------------------
// Configuration constants
// -----------------------------------------------------------------------------
// Arduino Uno R4 WiFi uses Serial1 on pins 0 and 1
const uint32_t SERIAL_BAUD = 115200;
const uint32_t PMS_BAUD = 9600;
const uint16_t PACKET_SIZE = 32; // Fixed PMS7003 packet length

// -----------------------------------------------------------------------------
// Command frames (hex) – see communication documentation
// -----------------------------------------------------------------------------
const uint8_t CMD_PASSIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
const uint8_t CMD_READ_DATA[]   = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71};

// No SoftwareSerial needed for Uno R4 WiFi

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
/** Calculate checksum for a PMS7003 packet (sum of bytes 0‑29). */
uint16_t calculateChecksum(const uint8_t *buf) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < 30; ++i) sum += buf[i];
  return sum;
}

/** Read a full packet; returns true if header & checksum are valid. */
bool readPacket(uint8_t *packet) {
  unsigned long start = millis();
  // Increased timeout to 1000ms for stability
  while (Serial1.available() < PACKET_SIZE) {
    if (millis() - start > 1000) return false; 
  }
  for (uint8_t i = 0; i < PACKET_SIZE; ++i) packet[i] = Serial1.read();
  if (packet[0] != 0x42 || packet[1] != 0x4D) return false;
  uint16_t received = (packet[30] << 8) | packet[31];
  return calculateChecksum(packet) == received;
}

/** Print atmospheric PM concentrations (µg/m³). */
void printConcentrations(const uint8_t *packet) {
  uint16_t pm1_0 = (packet[10] << 8) | packet[11];
  uint16_t pm2_5 = (packet[12] << 8) | packet[13];
  uint16_t pm10  = (packet[14] << 8) | packet[15];
  Serial.print(F("PM1.0: ")); Serial.print(pm1_0); Serial.println(F(" µg/m³"));
  Serial.print(F("PM2.5: ")); Serial.print(pm2_5); Serial.println(F(" µg/m³"));
  Serial.print(F("PM10 : ")); Serial.print(pm10);  Serial.println(F(" µg/m³"));
}

// -----------------------------------------------------------------------------
// Arduino lifecycle
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(PMS_BAUD);
  delay(1000);
  Serial.println(F("--- PMS7003 Passive Mode rev01 (Uno R4 WiFi) ---"));
  Serial.println(F("Using Hardware Serial1 (Pin 0 RX, Pin 1 TX)"));
  
  // Clear buffer
  while(Serial1.available()) Serial1.read();

  // Switch sensor to passive mode
  Serial1.write(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
  delay(100);
}

void loop() {
  // Clear buffer before requesting to avoid reading old/garbage data
  while(Serial1.available()) Serial1.read();

  // Request a data frame
  Serial1.write(CMD_READ_DATA, sizeof(CMD_READ_DATA));
  
  // Wait a bit for sensor to respond
  delay(200); 

  uint8_t packet[PACKET_SIZE];
  if (readPacket(packet)) {
    printConcentrations(packet);
  } else {
    Serial.println(F("Waiting for valid data..."));
  }
  delay(2000); // repeat every 2 seconds
}

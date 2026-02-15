// rev01 – Active Mode Example for PMS7003 Sensor
// This sketch demonstrates using the PMS7003 in Active Mode.
// In Active Mode, the sensor automatically sends a 32-byte data packet
// approximately every 200~800ms (depending on the sensor version/state).
// The Arduino simply listens for the data stream, validates the packet,
// and prints the PM concentrations.

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
// Command frames (hex)
// -----------------------------------------------------------------------------
// Switch to Active Mode: 42 4D E1 00 01 01 71
const uint8_t CMD_ACTIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71};

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
/** Calculate checksum for a PMS7003 packet (sum of bytes 0‑29). */
uint16_t calculateChecksum(const uint8_t *buf)
{
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 30; ++i)
        sum += buf[i];
    return sum;
}

/** Print atmospheric PM concentrations (µg/m³). */
void printConcentrations(const uint8_t *packet)
{
    uint16_t pm1_0 = (packet[10] << 8) | packet[11];
    uint16_t pm2_5 = (packet[12] << 8) | packet[13];
    uint16_t pm10 = (packet[14] << 8) | packet[15];
    Serial.print(F("PM1.0: "));
    Serial.print(pm1_0);
    Serial.println(F(" µg/m³"));
    Serial.print(F("PM2.5: "));
    Serial.print(pm2_5);
    Serial.println(F(" µg/m³"));
    Serial.print(F("PM10 : "));
    Serial.print(pm10);
    Serial.println(F(" µg/m³"));
    Serial.println(F("-----------------------------"));
}

// -----------------------------------------------------------------------------
// Arduino lifecycle
// -----------------------------------------------------------------------------
void setup()
{
    Serial.begin(SERIAL_BAUD);
    Serial1.begin(PMS_BAUD);
    delay(1000);
    Serial.println(F("--- PMS7003 Active Mode rev01 (Uno R4 WiFi) ---"));
    Serial.println(F("Using Hardware Serial1 (Pin 0 RX, Pin 1 TX)"));

    // Switch sensor to Active mode (just in case it was in Passive)
    Serial1.write(CMD_ACTIVE_MODE, sizeof(CMD_ACTIVE_MODE));
    delay(100);
}

void loop()
{
    // In Active Mode, we constantly check for incoming data.
    // We need at least 32 bytes to form a packet.
    if (Serial1.available() >= PACKET_SIZE)
    {

        // Check for the first header byte (0x42)
        if (Serial1.peek() != 0x42)
        {
            Serial1.read(); // Discard invalid byte to realign
            return;
        }

        // We have 0x42. Now let's check the second byte.
        // We need to read the first byte to peek the second one reliably
        // (or just read it and put it back if we could, but we can't).
        // So we read the first byte.
        Serial1.read(); // Consume 0x42

        if (Serial1.peek() != 0x4D)
        {
            // Second byte is not 0x4D. This wasn't a valid header.
            // We already consumed the 0x42. The next loop will check the byte that was just peeked.
            return;
        }

        // If we are here, we found 0x42 followed by 0x4D.
        // Let's read the full packet.
        uint8_t packet[PACKET_SIZE];
        packet[0] = 0x42;
        packet[1] = Serial1.read(); // Consume 0x4D

        // Read the remaining 30 bytes
        for (uint8_t i = 2; i < PACKET_SIZE; ++i)
        {
            packet[i] = Serial1.read();
        }

        // Validate Checksum
        uint16_t receivedChecksum = (packet[30] << 8) | packet[31];
        if (calculateChecksum(packet) == receivedChecksum)
        {
            printConcentrations(packet);
        }
        else
        {
            Serial.println(F("Checksum mismatch!"));
        }
    }
}

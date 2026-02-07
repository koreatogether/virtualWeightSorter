#include <Arduino.h>
#include <HardwareSerial.h>

// XIAO ESP32-C3 Hardware Serial 1 Pins
#define RX1_PIN 7 // D5
#define TX1_PIN 6 // D4

HardwareSerial MySerial1(1);

void setup() {
    // USB Serial for Debugging
    Serial.begin(115200);
    while (!Serial && millis() < 5000);
    
    Serial.println("\n--- XIAO ESP32-C3 Hardware Serial Test ---");
    Serial.printf("Target: Nextion Display (UART1)\n");
    Serial.printf("Pins: TX=D4(GPIO6), RX=D5(GPIO7)\n");

    // Initialize UART1
    MySerial1.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
    
    Serial.println("System Ready. Sending test commands to Nextion...");
}

void loop() {
    static unsigned long lastMsg = 0;
    if (millis() - lastMsg > 3000) {
        lastMsg = millis();
        
        // Nextion Command Example: Change page or update text
        // Note: 0xFF 0xFF 0xFF is required at the end of every command
        Serial.println("Sending: t0.txt=\"ESP32 Link Ok\"");
        
        MySerial1.print("t0.txt=\"ESP32 Link Ok\"");
        MySerial1.write(0xff);
        MySerial1.write(0xff);
        MySerial1.write(0xff);
    }

    // Check for incoming data from Nextion (e.g., touch events)
    if (MySerial1.available()) {
        Serial.print("Nextion Recv [HEX]: ");
        while (MySerial1.available()) {
            uint8_t b = MySerial1.read();
            Serial.printf("%02X ", b);
        }
        Serial.println();
    }
}

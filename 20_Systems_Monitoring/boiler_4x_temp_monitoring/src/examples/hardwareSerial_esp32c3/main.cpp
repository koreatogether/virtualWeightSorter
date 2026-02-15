#include <Arduino.h>
#include <HardwareSerial.h>

/**
 * Seeed Studio XIAO ESP32-C3 Hardware Serial Example
 * 
 * ESP32-C3 has three UART controllers:
 * 1. Serial (USB CDC / built-in JTAG) - Used for Debugging/Upload
 * 2. Serial0 - Traditional UART0 (Default: TX GPIO21, RX GPIO20)
 * 3. Serial1 - Traditional UART1 (Must be mapped to GPIOs)
 * 
 * This example demonstrates how to use UART1 (HardwareSerial 1) with specific pins.
 */

// Define pins for UART1
// XIAO ESP32-C3 Pin Mapping:
// D4 -> GPIO6 (Available)
// D5 -> GPIO7 (Available)
#define RX1_PIN 7 // XIAO D5
#define TX1_PIN 6 // XIAO D4

// Explicitly define HardwareSerial instance for UART1
HardwareSerial MySerial1(1);

void setup() {
  // Initialize USB Serial for monitoring (Baud rate 115200)
  Serial.begin(115200);
  while (!Serial); // Wait for Serial to be ready
  
  Serial.println("Seeed Studio XIAO ESP32-C3 Serial Example Started");
  Serial.println("Monitoring UART1 (D4=TX, D5=RX) at 9600 baud...");

  // Initialize HardwareSerial 1
  // Parameters: Baud rate, Config, RX Pin, TX Pin
  MySerial1.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
}

void loop() {
  // 1. Read from USB Serial and send to UART1
  if (Serial.available()) {
    char c = Serial.read();
    MySerial1.write(c);
  }

  // 2. Read from UART1 and send to USB Serial (Terminal)
  if (MySerial1.available()) {
    char c = MySerial1.read();
    Serial.print("[UART1 Receive]: ");
    Serial.println(c);
  }

  // Periodic heartbeat message to UART1
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    lastTime = millis();
    MySerial1.println("Hello from XIAO ESP32-C3 UART1!");
    Serial.println("Sent heartbeat to UART1");
  }
}

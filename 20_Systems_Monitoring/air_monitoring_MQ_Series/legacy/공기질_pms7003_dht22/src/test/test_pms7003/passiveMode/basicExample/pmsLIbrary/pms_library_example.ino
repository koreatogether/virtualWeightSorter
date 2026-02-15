// pms_library_example.ino
// PMS7003 Example using the fu-hsi/pms library
// Library URL: https://github.com/fu-hsi/pms

#include "PMS.h"

// -----------------------------------------------------------------------------
// Pin Configuration for Arduino Uno R4 WiFi
// -----------------------------------------------------------------------------
// Arduino Uno R4 WiFi has a dedicated Hardware Serial port (Serial1) on pins 0 and 1.
// This is much more stable than SoftwareSerial and handles 3.3V logic better.
//
// Wiring:
// PMS TX  ---> Arduino RX (Pin 0)
// PMS RX  ---> Arduino TX (Pin 1)
// PMS VCC ---> 5V
// PMS GND ---> GND

PMS pms(Serial1);
PMS::DATA data;

void setup() {
  Serial.begin(115200);   // For Serial Monitor (USB)
  Serial1.begin(9600);    // For PMS7003 Sensor
  
  Serial.println(F("--- PMS7003 Library Example (Arduino Uno R4 WiFi) ---"));
  Serial.println(F("Using Hardware Serial1 (Pin 0 RX, Pin 1 TX)"));
  Serial.println(F("Please ensure PMS TX is connected to Pin 0, and PMS RX to Pin 1."));

  // Switch to passive mode
  pms.passiveMode();
  Serial.println(F("Sent Passive Mode command."));
  
  // Wake up (just in case)
  pms.wakeUp();
}

void loop() {
  Serial.println(F("Requesting data..."));
  pms.requestRead();

  // Wait for data (with timeout handling inside library, but we check return)
  if (pms.readUntil(data)) {
    Serial.println(F("Data received:"));
    
    Serial.print(F("PM 1.0 (ug/m3): "));
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print(F("PM 2.5 (ug/m3): "));
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print(F("PM 10.0 (ug/m3): "));
    Serial.println(data.PM_AE_UG_10_0);

    Serial.println(F("---------------------------------------"));
  } else {
    Serial.println(F("No data received (timeout)."));
  }

  // Wait 2 seconds before next read
  delay(2000);
}

/*
 * DHT22 Environmental Monitoring Sensor
 *
 * This Arduino sketch reads temperature and humidity data from a DHT22 sensor
 * and sends it via serial communication to a Python Dash application.
 *
 * Connections:
 * - DHT22 VCC -> Arduino 5V
 * - DHT22 GND -> Arduino GND
 * - DHT22 DATA -> Arduino Pin 2
 *
 * Required Libraries:
 * - DHT sensor library by Adafruit
 * - Adafruit Unified Sensor
 */

#include <DHT.h>
#include <ArduinoJson.h>

#define DHT_PIN 2
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Wait for serial port to connect
  while (!Serial) {
    delay(10);
  }

  Serial.println("DHT22 Environmental Monitor Started");
  Serial.println("Sending JSON data every 2 seconds...");
}

void loop() {
  // Read sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("{\"error\":\"Failed to read from DHT sensor!\"}");
    delay(2000);
    return;
  }

  // Calculate heat index
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  // Create JSON object
  StaticJsonDocument<200> doc;
  doc["timestamp"] = millis();
  doc["temperature"] = round(temperature * 100.0) / 100.0;  // Round to 2 decimal places
  doc["humidity"] = round(humidity * 100.0) / 100.0;
  doc["heat_index"] = round(heatIndex * 100.0) / 100.0;
  doc["sensor"] = "DHT22";
  doc["status"] = "OK";

  // Send JSON to serial
  serializeJson(doc, Serial);
  Serial.println(); // New line for easier parsing

  delay(2000); // Send data every 2 seconds
}

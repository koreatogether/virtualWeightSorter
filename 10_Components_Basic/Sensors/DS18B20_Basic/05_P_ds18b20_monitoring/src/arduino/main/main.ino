/**
 * DS18B20 온도 센서 모니터링 시스템 (Arduino Uno R4 WiFi)
 *
 * 2025-08-19: 실제 하드웨어 방식으로 완전 재작성
 * - EEPROM 기반 설정 관리
 * - 통신 프로토콜 개선
 * - 모듈화된 코드 구조
 *
 * Hardware Requirements:
 * - Arduino Uno R4 WiFi
 * - DS18B20 temperature sensor (waterproof recommended)
 * - 4.7k ohm pull-up resistor
 * - Jumper wires and breadboard
 *
 * Wiring:
 * - DS18B20 VDD -> Arduino 3.3V or 5V
 * - DS18B20 GND -> Arduino GND
 * - DS18B20 DATA -> Arduino Digital Pin 2 (with 4.7k ohm pull-up to VDD)
 *
 * Libraries Required:
 * - OneWire by Paul Stoffregen
 * - DallasTemperature by Miles Burton
 * - ArduinoJson by Benoit Blanchon (version 7.x)
 */

#include "sensor_manager.h"
#include "communication.h"
#include "config_manager.h"

void setup()
{
  // Initialize components
  initializeSerial();
  Serial.println("=== SYSTEM BOOT START ===");
  // 시리얼 안정화: 짧은 스핀 대기 (1초 → 100ms 내)
  {
    unsigned long t0 = millis();
    while (millis() - t0 < 100)
    {
      // spin
    }
  }

  // 1) 먼저 EEPROM 기반 설정을 불러온다 (센서 스캔 전에 로드해야 저장된 주소/ID를 반영 가능)
  Serial.println("[SETUP DEBUG] About to call initializeConfig()...");
  initializeConfig();
  Serial.println("[SETUP DEBUG] initializeConfig() completed");
  // 2) 센서를 스캔한다
  Serial.println("[SETUP DEBUG] About to call initializeSensor()...");
  initializeSensor();
  Serial.println("[SETUP DEBUG] initializeSensor() completed");

  // 3) EEPROM에 저장된 센서 ID가 있으면 해당 센서를 선택 시도 (스캔 후에만 가능)
  if (current_config.selected_sensor_id > 0)
  {
    Serial.println("[SETUP DEBUG] Selecting sensor ID: " + String(current_config.selected_sensor_id));
    // 선택 실패 시(센서가 교체되었거나 ID 불일치) 기본 선택 유지
    setSelectedSensor(current_config.selected_sensor_id);
  }

  // Send startup message
  Serial.println("[SETUP DEBUG] Sending startup message...");
  sendResponse("success", "DS18B20 monitoring system started");

  // Send initial system status
  Serial.println("[SETUP DEBUG] Sending system status...");
  sendSystemStatus();
  Serial.println("=== SYSTEM BOOT COMPLETE ===");
}

void loop()
{
  static unsigned long lastMeasurement = 0;
  static unsigned long last_loop_start_debug = 0;
  static unsigned long loop_counter = 0;

  loop_counter++;

  if (millis() - last_loop_start_debug > 2000)
  {
    Serial.println("[LOOP START DEBUG] Loop iteration #" + String(loop_counter) + " started at " + String(millis()));
    last_loop_start_debug = millis();
  }

  // Handle incoming serial commands
  if (Serial.available() > 0)
  {
    Serial.println("[LOOP DEBUG] Processing serial command at " + String(millis()));
    Serial.println("[LOOP DEBUG] Available bytes: " + String(Serial.available()));
    processSerialCommand();
    Serial.println("[LOOP DEBUG] Serial command processed at " + String(millis()));
  }

  // Perform periodic temperature measurement
  unsigned long currentTime = millis();
  if (currentTime - lastMeasurement >= getMeasurementInterval())
  {
    Serial.println("[LOOP DEBUG] Starting measurement");
    performMeasurement();
    Serial.println("[LOOP DEBUG] Measurement completed");
    lastMeasurement = currentTime;
  }

  // 처리 지연된 EEPROM 저장 (디바운스)
  static unsigned long last_autosave_debug = 0;
  static unsigned long autosave_call_counter = 0;

  autosave_call_counter++;

  if (millis() - last_autosave_debug > 3000)
  {
    Serial.println("[MAIN LOOP DEBUG] About to call processConfigAutosave() #" + String(autosave_call_counter) + " at " + String(millis()));
    last_autosave_debug = millis();
  }

  unsigned long before_autosave = millis();
  processConfigAutosave();
  unsigned long after_autosave = millis();

  if (after_autosave - before_autosave > 100)
  {
    Serial.println("[MAIN LOOP DEBUG] processConfigAutosave() took " + String(after_autosave - before_autosave) + "ms");
  }

  if (millis() - last_autosave_debug < 500)
  {
    Serial.println("[MAIN LOOP DEBUG] processConfigAutosave() #" + String(autosave_call_counter) + " completed at " + String(millis()));
  }

  // Small cooperative pause without blocking: no hard delay to avoid stutters
}
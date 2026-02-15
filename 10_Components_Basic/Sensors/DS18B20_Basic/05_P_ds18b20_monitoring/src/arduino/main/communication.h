/**
 * Communication Management Header
 * 
 * Handles JSON serial communication with Python dashboard,
 * including command processing and data transmission.
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <ArduinoJson.h>

// ==================== Constants ====================
#define SERIAL_BAUDRATE 115200      // Serial communication speed
#define JSON_BUFFER_SIZE 1024       // JSON document buffer size (8개 센서 대응)

// ==================== Function Declarations ====================
void initializeSerial();
void sendSensorData(float temperature);
void sendMultiSensorData(float temperature, uint8_t original_index, uint8_t sorted_index);
void sendResponse(const String& status, const String& message);
void sendSystemStatus();
void processSerialCommand();
void handleConfigCommand(JsonDocument& doc);
void handleSetThresholdCommand(JsonDocument& doc);
void handleGetSensorConfigCommand(JsonDocument& doc);

#endif // COMMUNICATION_H
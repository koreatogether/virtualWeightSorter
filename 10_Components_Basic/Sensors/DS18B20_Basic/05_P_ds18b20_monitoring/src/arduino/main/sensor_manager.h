/**
 * DS18B20 Sensor Management Header
 *
 * Handles DS18B20 temperature sensor initialization, reading,
 * and validation functions.
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <OneWire.h>
#include <DallasTemperature.h>

// ==================== Constants ====================
#define ONE_WIRE_BUS 2                    // Digital pin for DS18B20 data line
#define DEFAULT_MEASUREMENT_INTERVAL 1000 // Default measurement interval in ms

// ==================== Structures ====================
struct SensorInfo {
  uint8_t original_index;
  int16_t user_id;
  String address_str;
  String display_id;
};

// ==================== Function Declarations ====================
void initializeSensor();
void performMeasurement();
bool validateTemperature(float temp);
bool isSensorConnected();
String getSensorId();
float getLastTemperature();

// Sensor resolution tracking (9..12)
uint8_t getSensorResolution();

// Sensor selection and management
uint8_t getSensorCount();
bool selectSensorByIndex(uint8_t index);
bool selectSensorByAddress(String address);
String getAllSensorAddresses();

// Sensor sorting (for multi-sensor display order)
void sortSensors(struct SensorInfo sensor_info[], uint8_t count);

// ==================== Global Sensor Objects ====================
extern OneWire oneWire;
extern DallasTemperature sensors;

// ==================== Global Sensor State ====================
extern bool sensor_connected;
extern String sensor_id;
extern float last_temperature;
extern uint8_t sensor_resolution;

#endif // SENSOR_MANAGER_H
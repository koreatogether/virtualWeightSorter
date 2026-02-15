/**
 * DS18B20 Sensor Management Implementation
 */

#include "sensor_manager.h"
#include "communication.h"

// ==================== Global Objects ====================
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ==================== Global Variables ====================
bool sensor_connected = false;
uint8_t sensor_count = 0;          // Number of detected sensors
DeviceAddress sensor_addresses[8]; // Support up to 8 sensors on one bus
uint8_t sorted_sensor_indices[8];  // Sorted indices: user ID sensors first, then address-sorted
String sensor_id = "";             // Selected sensor address string (HEX16)
float last_temperature = -127.0;   // Invalid temperature indicator
uint8_t current_sensor_index = 0;  // Currently selected sensor (0-based)
uint8_t sensor_resolution = 10;    // 8개 센서 최적화: 10비트 (188ms/센서)

// Sensor info structure is defined in sensor_manager.h

// ==================== Sensor Initialization ====================
void initializeSensor()
{
  sensors.begin();

  // Scan for all DS18B20 sensors on the bus
  sensor_count = sensors.getDeviceCount();
  sensor_connected = (sensor_count > 0);

  if (sensor_connected)
  {
    // Configure all sensors with 12-bit resolution for highest precision
    sensors.setResolution(sensor_resolution);

    // Collect sensor information for sorting
    SensorInfo sensor_info[8];

    for (uint8_t i = 0; i < sensor_count && i < 8; i++)
    {
      if (sensors.getAddress(sensor_addresses[i], i))
      {
        // Convert address to hex string for sensor i
        String addr_str = "";
        for (uint8_t j = 0; j < 8; j++)
        {
          if (sensor_addresses[i][j] < 16)
            addr_str += "0";
          addr_str += String(sensor_addresses[i][j], HEX);
        }
        addr_str.toUpperCase();

        // Read user ID from EEPROM (using TH/TL registers as user data)
        int16_t user_id = sensors.getUserDataByIndex(i);
        String display_id;

        // Check if user ID is valid (1-8 range)
        if (user_id >= 1 && user_id <= 8)
        {
          display_id = String(user_id, DEC);
          if (display_id.length() == 1)
          {
            display_id = "0" + display_id; // Format as "01", "02", etc.
          }
        }
        else
        {
          display_id = "00"; // Default for sensors without valid user ID
        }

        // Store sensor info for sorting
        sensor_info[i].original_index = i;
        sensor_info[i].user_id = user_id;
        sensor_info[i].address_str = addr_str;
        sensor_info[i].display_id = display_id;

        sendResponse("info", "Found sensor " + String(i) + ": " + addr_str + " (ID: " + display_id + ")");
      }
    }

    // Sort sensors: user ID sensors (1-8) first by ID, then address-sorted sensors (00)
    sortSensors(sensor_info, sensor_count);

    // Set first sorted sensor as default
    if (sensor_count > 0)
    {
      current_sensor_index = 0; // Index in sorted array order
      uint8_t first_original_idx = sorted_sensor_indices[0];
      // Use physical address (HEX16) as the selected sensor identifier string
      sensor_id = sensor_info[first_original_idx].address_str;
    }

    sendResponse("success", "Initialized " + String(sensor_count) + " DS18B20 sensor(s), selected: " + sensor_id);
  }
  else
  {
    sendResponse("error", "No DS18B20 sensors detected on pin " + String(ONE_WIRE_BUS));
  }
}

// ==================== Temperature Measurement ====================
void performMeasurement()
{
  if (!sensor_connected || sensor_count == 0)
  {
    // Retry sensor detection
    initializeSensor();
    return;
  }

  // Request temperature readings from all sensors
  sensors.requestTemperatures();

  // 8개 센서 과부하 방지: 배치 전송으로 최적화
  static uint8_t batch_start = 0;
  const uint8_t BATCH_SIZE = 2; // 한 번에 2개씩만 처리

  uint8_t batch_end = min(batch_start + BATCH_SIZE, sensor_count);

  // 배치 단위로 센서 데이터 처리
  for (uint8_t i = batch_start; i < batch_end && i < 8; i++)
  {
    uint8_t original_index = sorted_sensor_indices[i];
    float temperature = sensors.getTempC(sensor_addresses[original_index]);

    // Validate temperature reading
    if (validateTemperature(temperature))
    {
      // Update last_temperature for the first (primary) sensor
      if (i == 0)
      {
        last_temperature = temperature;
      }

      // Send data for this specific sensor
      sendMultiSensorData(temperature, original_index, i);

      // 전송 간 블로킹 지연 제거 (stutter 방지)
      // 시리얼 버퍼 상태는 하위 레벨에서 처리되며, 다음 루프에서 자연스럽게 분산 처리됨
    }
    else
    {
      // 에러 메시지도 배치 처리 시에만 (과도한 에러 메시지 방지)
      if (i == batch_start)
      {
        String addr_str = "";
        for (uint8_t j = 0; j < 8; j++)
        {
          if (sensor_addresses[original_index][j] < 16)
            addr_str += "0";
          addr_str += String(sensor_addresses[original_index][j], HEX);
        }
        addr_str.toUpperCase();

        sendResponse("error", "Invalid reading from sensor " + String(i) + " (" + addr_str + ")");
      }
    }
  }

  // 다음 배치로 순환
  batch_start = (batch_end >= sensor_count) ? 0 : batch_end;
}

// ==================== Utility Functions ====================
bool validateTemperature(float temp)
{
  // DS18B20 valid temperature range is -55°C to +125°C
  // Invalid readings typically return -127°C or 85°C (default power-on state)
  return (temp > -55.0 && temp < 125.0 && temp != -127.0 && temp != 85.0);
}

bool isSensorConnected()
{
  return sensor_connected;
}

String getSensorId()
{
  return sensor_id;
}

float getLastTemperature()
{
  return last_temperature;
}

// ==================== Sensor Selection Functions ====================
uint8_t getSensorCount()
{
  return sensor_count;
}

bool selectSensorByIndex(uint8_t index)
{
  if (index < sensor_count)
  {
    current_sensor_index = index;

    // Update sensor_id string
    sensor_id = "";
    for (uint8_t i = 0; i < 8; i++)
    {
      if (sensor_addresses[index][i] < 16)
        sensor_id += "0";
      sensor_id += String(sensor_addresses[index][i], HEX);
    }
    sensor_id.toUpperCase();

    return true;
  }
  return false;
}

bool selectSensorByAddress(String address)
{
  address.toUpperCase();

  // Find sensor with matching address
  for (uint8_t i = 0; i < sensor_count; i++)
  {
    String addr_str = "";
    for (uint8_t j = 0; j < 8; j++)
    {
      if (sensor_addresses[i][j] < 16)
        addr_str += "0";
      addr_str += String(sensor_addresses[i][j], HEX);
    }
    addr_str.toUpperCase();

    if (addr_str == address)
    {
      current_sensor_index = i;
      sensor_id = address;
      return true;
    }
  }
  return false;
}

String getAllSensorAddresses()
{
  String result = "[";
  for (uint8_t i = 0; i < sensor_count; i++)
  {
    if (i > 0)
      result += ",";

    String addr_str = "";
    for (uint8_t j = 0; j < 8; j++)
    {
      if (sensor_addresses[i][j] < 16)
        addr_str += "0";
      addr_str += String(sensor_addresses[i][j], HEX);
    }
    addr_str.toUpperCase();

    result += "\"" + addr_str + "\"";
  }
  result += "]";
  return result;
}

// ==================== Sensor Sorting ====================
void sortSensors(SensorInfo sensor_info[], uint8_t count)
{
  // Bubble sort implementation for sensor ordering:
  // 1. Sensors with valid user_id (1-8) first, sorted by user_id ascending
  // 2. Sensors with invalid user_id (00) second, sorted by address ascending

  for (uint8_t i = 0; i < count - 1; i++)
  {
    for (uint8_t j = 0; j < count - i - 1; j++)
    {
      bool should_swap = false;

      // Comparison logic
      bool j_has_valid_id = (sensor_info[j].user_id >= 1 && sensor_info[j].user_id <= 8);
      bool j_plus_1_has_valid_id = (sensor_info[j + 1].user_id >= 1 && sensor_info[j + 1].user_id <= 8);

      if (j_has_valid_id && j_plus_1_has_valid_id)
      {
        // Both have valid IDs: sort by user_id ascending
        if (sensor_info[j].user_id > sensor_info[j + 1].user_id)
        {
          should_swap = true;
        }
      }
      else if (!j_has_valid_id && !j_plus_1_has_valid_id)
      {
        // Both have invalid IDs: sort by address ascending
        if (sensor_info[j].address_str > sensor_info[j + 1].address_str)
        {
          should_swap = true;
        }
      }
      else if (!j_has_valid_id && j_plus_1_has_valid_id)
      {
        // j has invalid ID, j+1 has valid ID: swap (valid IDs come first)
        should_swap = true;
      }
      // If j has valid ID and j+1 has invalid ID: no swap needed

      if (should_swap)
      {
        // Swap sensor info
        SensorInfo temp = sensor_info[j];
        sensor_info[j] = sensor_info[j + 1];
        sensor_info[j + 1] = temp;
      }
    }
  }

  // Build sorted indices array
  for (uint8_t i = 0; i < count; i++)
  {
    sorted_sensor_indices[i] = sensor_info[i].original_index;
    sendResponse("debug", "Sorted position " + String(i) +
                              ": original_index=" + String(sensor_info[i].original_index) +
                              ", ID=" + sensor_info[i].display_id +
                              ", addr=" + sensor_info[i].address_str);
  }
}

uint8_t getSensorResolution()
{
  return sensor_resolution;
}
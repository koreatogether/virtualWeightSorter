/**
 * Configuration Management Implementation
 */

#include "config_manager.h"
#include "sensor_manager.h"

// ==================== Global Configuration Variables ====================
float th_value = DEFAULT_TH_VALUE; // High temperature threshold
float tl_value = DEFAULT_TL_VALUE; // Low temperature threshold
unsigned long measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
ConfigData current_config;
static bool config_dirty = false;
static unsigned long last_config_change_ms = 0;
static unsigned long runtime_save_debounce_ms = CONFIG_SAVE_DEBOUNCE_MS;
static uint32_t eeprom_write_count = 0; // increment when saveConfigToEEPROM writes

// ==================== Configuration Initialization ====================
void initializeConfig()
{
  // Initialize EEPROM (Arduino Uno R4 WiFi doesn't need size parameter)
  EEPROM.begin();

  // Try to load configuration from EEPROM
  if (loadConfigFromEEPROM())
  {
    // Successfully loaded from EEPROM
    Serial.println("[EEPROM] EEPROM data loaded successfully!");
    Serial.println("[EEPROM] Sensor count in config: " + String(current_config.sensor_count));

    // 전역 변수는 더 이상 사용하지 않음 - EEPROM에서 직접 조회
    // 호환성을 위해 기본값으로만 설정
    th_value = DEFAULT_TH_VALUE;
    tl_value = DEFAULT_TL_VALUE;
    measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;

    Serial.println("[EEPROM] Loaded configuration from EEPROM:");
    printConfigData(current_config);
  }
  else
  {
    // Use default values and save to EEPROM
    th_value = DEFAULT_TH_VALUE;
    tl_value = DEFAULT_TL_VALUE;
    measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;

    // 새로운 설정 구조체 초기화 (ID 기반)
    current_config.magic_number = CONFIG_MAGIC_NUMBER;
    current_config.version = CONFIG_VERSION;
    current_config.selected_sensor_id = 0; // 선택된 센서 없음
    current_config.sensor_count = 0;
    current_config.last_write_time = millis();
    current_config.write_count = 0;

    // 센서 배열 초기화 (인덱스 0-7 = ID 1-8)
    for (uint8_t i = 0; i < MAX_SENSORS_IN_CONFIG; i++)
    {
      memset(&current_config.sensors[i], 0, sizeof(SensorConfig));
      current_config.sensors[i].sensor_id = 0; // 비활성
      strcpy(current_config.sensors[i].sensor_address, "");
      current_config.sensors[i].th_value = DEFAULT_TH_VALUE;
      current_config.sensors[i].tl_value = DEFAULT_TL_VALUE;
      current_config.sensors[i].measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
      current_config.sensors[i].is_active = 0;
    }

    // Save defaults to EEPROM
    bool ok = saveConfigToEEPROM();
    if (ok)
    {
      Serial.println("[EEPROM] Saved default configuration to EEPROM:");
      printConfigData(current_config);
    }
    else
    {
      Serial.println("[EEPROM] Failed to save default configuration to EEPROM");
      printConfigData(current_config);
    }
  }
}

// ==================== Temperature Threshold Functions ====================
float getThValue()
{
  return th_value;
}

float getTlValue()
{
  return tl_value;
}

void setThValue(float value)
{
  if (value >= -55.0 && value <= 125.0)
  {
    if (th_value != value)
    {
      th_value = value;
      markConfigDirty();
    }
  }
}

void setTlValue(float value)
{
  if (value >= -55.0 && value <= 125.0)
  {
    if (tl_value != value)
    {
      tl_value = value;
      markConfigDirty();
    }
  }
}

// ==================== Measurement Interval Functions ====================
unsigned long getMeasurementInterval()
{
  return measurement_interval;
}

void setMeasurementInterval(unsigned long interval)
{
  Serial.println("[INTERVAL DEBUG] Requested measurement interval: " + String(interval) + "ms");
  if (interval >= MIN_MEASUREMENT_INTERVAL_MS && interval <= 60000)
  { // MIN to 60s
    if (measurement_interval != interval)
    {
      measurement_interval = interval;
      Serial.println("[INTERVAL DEBUG] Global interval updated to " + String(measurement_interval) + "ms");
      // 현재 선택된 센서 슬롯에도 반영하여 EEPROM 저장 대상에 포함
      if (current_config.selected_sensor_id >= 1 && current_config.selected_sensor_id <= 8)
      {
        uint8_t idx = current_config.selected_sensor_id - 1;
        if (current_config.sensors[idx].is_active && current_config.sensors[idx].sensor_id == current_config.selected_sensor_id)
        {
          current_config.sensors[idx].measurement_interval = interval;
          Serial.println("[INTERVAL DEBUG] Reflected to selected sensor ID " + String(current_config.selected_sensor_id) +
                         ": sensor slot interval=" + String(current_config.sensors[idx].measurement_interval) + "ms");
        }
      }
      markConfigDirty();
      Serial.println("[INTERVAL DEBUG] Config marked dirty for autosave (interval)");
    }
  }
}

unsigned long getEffectiveMeasurementInterval()
{
  // Effective interval is the max of requested interval and sensor conversion time for current resolution.
  // Map resolution to approximate max conversion time (ms)
  unsigned long conv_ms = 750; // default for 12-bit
  // sensor_resolution is defined in sensor_manager.cpp
  extern uint8_t sensor_resolution;
  if (sensor_resolution == 9)
    conv_ms = 94;
  else if (sensor_resolution == 10)
    conv_ms = 188;
  else if (sensor_resolution == 11)
    conv_ms = 375;
  else
    conv_ms = 750;

  unsigned long requested = measurement_interval;
  if (requested < conv_ms)
    return conv_ms;
  return requested;
}

// ==================== Sensor Configuration Functions ====================

// ID 기반 센서 설정 조회 (새로운 주요 함수)
bool getSensorConfig(uint8_t sensor_id, SensorConfig &sensor_config)
{
  Serial.println("[EEPROM DEBUG] Looking for sensor ID: " + String(sensor_id));

  // ID 범위 검증 (1-8)
  if (sensor_id < 1 || sensor_id > 8)
  {
    Serial.println("[EEPROM DEBUG] Invalid sensor ID: " + String(sensor_id));
    return false;
  }

  // 배열 인덱스는 ID-1 (ID 1 = 인덱스 0)
  uint8_t index = sensor_id - 1;

  if (current_config.sensors[index].is_active && current_config.sensors[index].sensor_id == sensor_id)
  {
    Serial.println("[EEPROM DEBUG] Found active sensor ID " + String(sensor_id) + " at index " + String(index));
    Serial.println("[EEPROM DEBUG] Address: " + String(current_config.sensors[index].sensor_address));
    Serial.println("[EEPROM DEBUG] TH=" + String(current_config.sensors[index].th_value, 1) +
                   " TL=" + String(current_config.sensors[index].tl_value, 1));

    // Copy sensor config
    memcpy(&sensor_config, &current_config.sensors[index], sizeof(SensorConfig));
    return true;
  }

  Serial.println("[EEPROM DEBUG] Sensor ID " + String(sensor_id) + " not active or not found");
  return false;
}

// 주소 기반 센서 설정 조회 (호환성용 헬퍼 함수)
bool getSensorConfigByAddress(const String &sensor_address, SensorConfig &sensor_config)
{
  Serial.println("[EEPROM DEBUG] Looking for sensor by address: " + sensor_address);

  // 모든 활성 센서에서 주소로 검색
  for (uint8_t i = 0; i < MAX_SENSORS_IN_CONFIG; i++)
  {
    if (current_config.sensors[i].is_active && current_config.sensors[i].sensor_id > 0)
    {
      String stored_addr = String(current_config.sensors[i].sensor_address);
      Serial.println("[EEPROM DEBUG] Checking sensor ID " + String(current_config.sensors[i].sensor_id) +
                     " with address: " + stored_addr);

      // 대소문자 구분없이 비교
      if (stored_addr.equalsIgnoreCase(sensor_address))
      {
        Serial.println("[EEPROM DEBUG] MATCH FOUND! Address matches sensor ID " + String(current_config.sensors[i].sensor_id));
        // Copy sensor config
        memcpy(&sensor_config, &current_config.sensors[i], sizeof(SensorConfig));
        return true;
      }
    }
  }
  Serial.println("[EEPROM DEBUG] No match found for address: " + sensor_address);
  return false;
}

// 주소로 센서 ID 찾기 (헬퍼 함수)
uint8_t findSensorIdByAddress(const String &sensor_address)
{
  Serial.println("[ID LOOKUP] Looking for sensor ID by address: " + sensor_address);

  for (uint8_t i = 0; i < MAX_SENSORS_IN_CONFIG; i++)
  {
    if (current_config.sensors[i].is_active && current_config.sensors[i].sensor_id > 0)
    {
      String stored_addr = String(current_config.sensors[i].sensor_address);
      if (stored_addr.equalsIgnoreCase(sensor_address))
      {
        Serial.println("[ID LOOKUP] Found sensor ID " + String(current_config.sensors[i].sensor_id) + " for address: " + sensor_address);
        return current_config.sensors[i].sensor_id;
      }
    }
  }
  Serial.println("[ID LOOKUP] No sensor ID found for address: " + sensor_address);
  return 0; // ID 없음
}

// ID 기반 센서 설정 추가/업데이트 (새로운 주요 함수)
bool addOrUpdateSensorConfig(uint8_t sensor_id, const String &sensor_address, const SensorConfig &config)
{
  Serial.println("[SAVE DEBUG] addOrUpdateSensorConfig called for ID: " + String(sensor_id) + ", Address: " + sensor_address);
  Serial.println("[SAVE DEBUG] TH=" + String(config.th_value, 1) + ", TL=" + String(config.tl_value, 1));

  // Input validation
  if (sensor_id < 1 || sensor_id > 8)
  {
    Serial.println("[SAVE DEBUG] ERROR: Invalid sensor ID: " + String(sensor_id));
    return false;
  }
  if (sensor_address.length() != 16)
  {
    Serial.println("[SAVE DEBUG] ERROR: Invalid address length");
    return false;
  }
  if (config.th_value < -55.0 || config.th_value > 125.0)
  {
    Serial.println("[SAVE DEBUG] ERROR: TH value out of range");
    return false;
  }
  if (config.tl_value < -55.0 || config.tl_value > 125.0)
  {
    Serial.println("[SAVE DEBUG] ERROR: TL value out of range");
    return false;
  }
  if (config.measurement_interval < MIN_MEASUREMENT_INTERVAL_MS || config.measurement_interval > 60000)
  {
    Serial.println("[SAVE DEBUG] ERROR: Interval out of range");
    return false;
  }

  Serial.println("[SAVE DEBUG] Input validation passed");

  // 배열 인덱스는 ID-1
  uint8_t index = sensor_id - 1;

  Serial.println("[SAVE DEBUG] Using index " + String(index) + " for sensor ID " + String(sensor_id));

  // 변경사항 유무 확인 후 업데이트
  const SensorConfig &prev = current_config.sensors[index];
  bool changed = false;
  if (!prev.is_active || prev.sensor_id != sensor_id)
    changed = true;
  if (strncmp(prev.sensor_address, sensor_address.c_str(), 16) != 0)
    changed = true;
  if (prev.th_value != config.th_value)
    changed = true;
  if (prev.tl_value != config.tl_value)
    changed = true;
  if (prev.measurement_interval != config.measurement_interval)
    changed = true;

  if (!changed)
  {
    Serial.println("[SAVE DEBUG] No changes detected for sensor ID " + String(sensor_id) + ", skipping update");
    return true; // 아무 변화 없음
  }

  // Update sensor config at specific index
  memcpy(&current_config.sensors[index], &config, sizeof(SensorConfig));
  current_config.sensors[index].sensor_id = sensor_id; // ID 설정
  strncpy(current_config.sensors[index].sensor_address, sensor_address.c_str(), 16);
  current_config.sensors[index].sensor_address[16] = '\0';
  current_config.sensors[index].is_active = 1;

  Serial.println("[SAVE DEBUG] Config saved to index " + String(index) + " with ID " + String(sensor_id));

  // Update sensor count if needed (최대 활성 센서 ID까지)
  uint8_t max_active_id = 0;
  for (uint8_t i = 0; i < MAX_SENSORS_IN_CONFIG; i++)
  {
    if (current_config.sensors[i].is_active && current_config.sensors[i].sensor_id > max_active_id)
    {
      max_active_id = current_config.sensors[i].sensor_id;
    }
  }
  current_config.sensor_count = max_active_id;

  Serial.println("[SAVE DEBUG] Final sensor count: " + String(current_config.sensor_count));
  Serial.println("[SAVE DEBUG] About to call markConfigDirty()...");

  markConfigDirty();

  Serial.println("[SAVE DEBUG] markConfigDirty() call completed!");
  return true;
}

// 주소 기반 설정 추가/업데이트 (호환성용 - 주소를 통해 ID 찾거나 새 ID 할당)
bool addOrUpdateSensorConfigByAddress(const String &sensor_address, const SensorConfig &config)
{
  Serial.println("[SAVE DEBUG] addOrUpdateSensorConfigByAddress called for: " + sensor_address);

  // 먼저 이미 등록된 센서인지 확인
  uint8_t existing_id = findSensorIdByAddress(sensor_address);

  if (existing_id > 0)
  {
    // 기존 센서 업데이트
    Serial.println("[SAVE DEBUG] Updating existing sensor ID " + String(existing_id));
    return addOrUpdateSensorConfig(existing_id, sensor_address, config);
  }
  else
  {
    // 새 센서 - 빈 ID 찾아서 할당
    for (uint8_t id = 1; id <= 8; id++)
    {
      uint8_t index = id - 1;
      if (!current_config.sensors[index].is_active || current_config.sensors[index].sensor_id == 0)
      {
        Serial.println("[SAVE DEBUG] Assigning new sensor ID " + String(id) + " to address " + sensor_address);
        return addOrUpdateSensorConfig(id, sensor_address, config);
      }
    }
    Serial.println("[SAVE DEBUG] ERROR: No available sensor IDs");
    return false;
  }
}

// ID 기반 센서 설정 추가/업데이트 (간단한 오버로드)
bool addOrUpdateSensorConfig(uint8_t sensor_id, const String &sensor_address, float th_value, float tl_value, unsigned long measurement_interval)
{
  // Create SensorConfig structure
  SensorConfig config;
  config.sensor_id = sensor_id;
  strncpy(config.sensor_address, sensor_address.c_str(), 16);
  config.sensor_address[16] = '\0';
  config.th_value = th_value;
  config.tl_value = tl_value;
  config.measurement_interval = measurement_interval;
  config.is_active = 1;

  // Call main function
  return addOrUpdateSensorConfig(sensor_id, sensor_address, config);
}

// 주소 기반 센서 설정 추가/업데이트 (호환성용 간단한 오버로드)
bool addOrUpdateSensorConfigByAddress(const String &sensor_address, float th_value, float tl_value, unsigned long measurement_interval)
{
  // Create SensorConfig structure
  SensorConfig config;
  config.sensor_id = 0; // Will be set by the main function
  strncpy(config.sensor_address, sensor_address.c_str(), 16);
  config.sensor_address[16] = '\0';
  config.th_value = th_value;
  config.tl_value = tl_value;
  config.measurement_interval = measurement_interval;
  config.is_active = 1;

  // Call main function
  return addOrUpdateSensorConfigByAddress(sensor_address, config);
}

// ID 기반 센서 임계값 설정 (새로운 주요 함수)
bool setSensorThreshold(uint8_t sensor_id, float th_value, float tl_value)
{
  Serial.println("[THRESHOLD DEBUG] Setting thresholds for sensor ID " + String(sensor_id) + " TH=" + String(th_value, 1) + " TL=" + String(tl_value, 1));

  // Input validation
  if (sensor_id < 1 || sensor_id > 8)
  {
    Serial.println("[THRESHOLD DEBUG] ERROR: Invalid sensor ID: " + String(sensor_id));
    return false;
  }
  if (th_value < -55.0 || th_value > 125.0)
  {
    Serial.println("[THRESHOLD DEBUG] ERROR: TH value out of range");
    return false;
  }
  if (tl_value < -55.0 || tl_value > 125.0)
  {
    Serial.println("[THRESHOLD DEBUG] ERROR: TL value out of range");
    return false;
  }

  // 배열 인덱스는 ID-1
  uint8_t index = sensor_id - 1;

  // Check if sensor is active
  if (!current_config.sensors[index].is_active || current_config.sensors[index].sensor_id != sensor_id)
  {
    Serial.println("[THRESHOLD DEBUG] ERROR: Sensor ID " + String(sensor_id) + " is not active");
    return false;
  }

  // Update thresholds if changed
  bool changed = false;
  if (current_config.sensors[index].th_value != th_value)
  {
    current_config.sensors[index].th_value = th_value;
    changed = true;
  }
  if (current_config.sensors[index].tl_value != tl_value)
  {
    current_config.sensors[index].tl_value = tl_value;
    changed = true;
  }

  if (changed)
  {
    Serial.println("[THRESHOLD DEBUG] Thresholds updated for sensor ID " + String(sensor_id));

    // Update global values if this is the selected sensor
    if (current_config.selected_sensor_id == sensor_id)
    {
      th_value = current_config.sensors[index].th_value;
      tl_value = current_config.sensors[index].tl_value;
      Serial.println("[THRESHOLD DEBUG] Updated global values for selected sensor");
    }
    markConfigDirty();
  }
  else
  {
    Serial.println("[THRESHOLD DEBUG] No changes needed for sensor ID " + String(sensor_id));
  }
  return true;
}

bool setSensorThresholdTH(uint8_t sensor_id, float th_value)
{
  // Get current TL value and set both
  SensorConfig config;
  if (getSensorConfig(sensor_id, config))
  {
    return setSensorThreshold(sensor_id, th_value, config.tl_value);
  }
  return false;
}

bool setSensorThresholdTL(uint8_t sensor_id, float tl_value)
{
  // Get current TH value and set both
  SensorConfig config;
  if (getSensorConfig(sensor_id, config))
  {
    return setSensorThreshold(sensor_id, config.th_value, tl_value);
  }
  return false;
}

// 주소 기반 센서 임계값 설정 (호환성용)
bool setSensorThresholdsByAddress(const String &sensor_address, float th_value, float tl_value)
{
  uint8_t sensor_id = findSensorIdByAddress(sensor_address);
  if (sensor_id > 0)
  {
    return setSensorThreshold(sensor_id, th_value, tl_value);
  }
  Serial.println("[THRESHOLD DEBUG] ERROR: No sensor ID found for address: " + sensor_address);
  return false;
}

// ID 기반 센서 측정 간격 설정 (새로운 주요 함수)
bool setSensorMeasurementInterval(uint8_t sensor_id, unsigned long interval)
{
  Serial.println("[INTERVAL DEBUG] Setting interval for sensor ID " + String(sensor_id) + " to " + String(interval) + "ms");

  // Input validation
  if (sensor_id < 1 || sensor_id > 8)
  {
    Serial.println("[INTERVAL DEBUG] ERROR: Invalid sensor ID: " + String(sensor_id));
    return false;
  }
  if (interval < MIN_MEASUREMENT_INTERVAL_MS || interval > 60000)
  {
    Serial.println("[INTERVAL DEBUG] ERROR: Interval out of range");
    return false;
  }

  // 배열 인덱스는 ID-1
  uint8_t index = sensor_id - 1;

  // Check if sensor is active
  if (!current_config.sensors[index].is_active || current_config.sensors[index].sensor_id != sensor_id)
  {
    Serial.println("[INTERVAL DEBUG] ERROR: Sensor ID " + String(sensor_id) + " is not active");
    return false;
  }

  // Update interval if changed
  if (current_config.sensors[index].measurement_interval != interval)
  {
    current_config.sensors[index].measurement_interval = interval;
    Serial.println("[INTERVAL DEBUG] Interval updated for sensor ID " + String(sensor_id));

    // Update global value if this is the selected sensor
    if (current_config.selected_sensor_id == sensor_id)
    {
      measurement_interval = interval;
      Serial.println("[INTERVAL DEBUG] Updated global interval for selected sensor");
    }
    markConfigDirty();
  }
  else
  {
    Serial.println("[INTERVAL DEBUG] No change needed for sensor ID " + String(sensor_id));
  }
  return true;
}

// 주소 기반 센서 측정 간격 설정 (호환성용)
bool setSensorMeasurementIntervalByAddress(const String &sensor_address, unsigned long interval)
{
  uint8_t sensor_id = findSensorIdByAddress(sensor_address);
  if (sensor_id > 0)
  {
    return setSensorMeasurementInterval(sensor_id, interval);
  }
  Serial.println("[INTERVAL DEBUG] ERROR: No sensor ID found for address: " + sensor_address);
  return false;
}

// ID 기반 센서 선택 설정 (새로운 주요 함수)
bool setSelectedSensor(uint8_t sensor_id)
{
  Serial.println("[SELECTED DEBUG] Setting selected sensor to ID: " + String(sensor_id));

  // ID 0은 선택 해제를 의미
  if (sensor_id == 0)
  {
    current_config.selected_sensor_id = 0;
    th_value = DEFAULT_TH_VALUE;
    tl_value = DEFAULT_TL_VALUE;
    measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
    Serial.println("[SELECTED DEBUG] No sensor selected - using defaults");
    markConfigDirty();
    return true;
  }

  // ID 범위 검증 (1-8)
  if (sensor_id < 1 || sensor_id > 8)
  {
    Serial.println("[SELECTED DEBUG] ERROR: Invalid sensor ID: " + String(sensor_id));
    return false;
  }

  // 센서가 활성 상태인지 확인
  uint8_t index = sensor_id - 1;
  if (!current_config.sensors[index].is_active || current_config.sensors[index].sensor_id != sensor_id)
  {
    Serial.println("[SELECTED DEBUG] ERROR: Sensor ID " + String(sensor_id) + " is not active");
    return false;
  }

  // 변경사항 확인 (같은 센서를 재선택하는 경우 불필요 저장 방지)
  bool selection_changed = (current_config.selected_sensor_id != sensor_id);
  bool values_changed = false;
  if (selection_changed)
  {
    values_changed = (th_value != current_config.sensors[index].th_value) ||
                     (tl_value != current_config.sensors[index].tl_value) ||
                     (measurement_interval != current_config.sensors[index].measurement_interval);
  }

  // Update selected sensor ID & globals
  current_config.selected_sensor_id = sensor_id;
  th_value = current_config.sensors[index].th_value;
  tl_value = current_config.sensors[index].tl_value;
  measurement_interval = current_config.sensors[index].measurement_interval;

  Serial.println("[SELECTED DEBUG] Selected sensor ID " + String(sensor_id) +
                 " TH=" + String(th_value, 1) + " TL=" + String(tl_value, 1));

  if (selection_changed || values_changed)
  {
    markConfigDirty();
  }
  return true;
}

// 주소 기반 센서 선택 설정 (호환성용)
bool setSelectedSensorByAddress(const String &sensor_address)
{
  if (sensor_address.length() == 0)
  {
    return setSelectedSensor(0); // 선택 해제
  }

  uint8_t sensor_id = findSensorIdByAddress(sensor_address);
  if (sensor_id > 0)
  {
    return setSelectedSensor(sensor_id);
  }
  Serial.println("[SELECTED DEBUG] ERROR: No sensor ID found for address: " + sensor_address);
  return false;
}

uint8_t getSelectedSensorId()
{
  return current_config.selected_sensor_id;
}

String getSelectedSensorAddress()
{
  if (current_config.selected_sensor_id == 0)
  {
    return String("");
  }

  uint8_t index = current_config.selected_sensor_id - 1;
  if (index < MAX_SENSORS_IN_CONFIG && current_config.sensors[index].is_active)
  {
    return String(current_config.sensors[index].sensor_address);
  }
  return String("");
}

uint8_t getActiveSensorCount()
{
  uint8_t count = 0;
  for (uint8_t i = 0; i < current_config.sensor_count && i < MAX_SENSORS_IN_CONFIG; i++)
  {
    if (current_config.sensors[i].is_active)
    {
      count++;
    }
  }
  return count;
}

// ==================== Sensor ID Functions ====================

// ==================== EEPROM Functions ====================
bool loadConfigFromEEPROM()
{
  Serial.println("[EEPROM LOAD DEBUG] Starting EEPROM load...");

  // Read configuration structure from EEPROM
  EEPROM.get(EEPROM_START_ADDR, current_config);

  Serial.println("[EEPROM LOAD DEBUG] Data read from EEPROM");
  Serial.println("[EEPROM LOAD DEBUG] Magic number: 0x" + String(current_config.magic_number, HEX) + " (expected: 0x" + String(CONFIG_MAGIC_NUMBER, HEX) + ")");
  Serial.println("[EEPROM LOAD DEBUG] Version: " + String(current_config.version) + " (expected: " + String(CONFIG_VERSION) + ")");
  Serial.println("[EEPROM LOAD DEBUG] Sensor count: " + String(current_config.sensor_count));

  // Validate magic number and checksum
  if (current_config.magic_number != CONFIG_MAGIC_NUMBER)
  {
    Serial.println("[EEPROM LOAD DEBUG] FAILED: Invalid magic number!");
    return false; // Invalid EEPROM data
  }

  // Version check (allow future migration logic)
  if (current_config.version == 0 || current_config.version > CONFIG_VERSION)
  {
    Serial.println("[EEPROM LOAD DEBUG] FAILED: Unsupported version " + String(current_config.version));
    return false; // Unsupported / uninitialized version
  }

  uint8_t calculated_checksum = calculateChecksum(current_config);
  Serial.println("[EEPROM LOAD DEBUG] Checksum: stored=0x" + String(current_config.checksum, HEX) + ", calculated=0x" + String(calculated_checksum, HEX));
  if (current_config.checksum != calculated_checksum)
  {
    Serial.println("[EEPROM LOAD DEBUG] FAILED: Checksum mismatch!");
    return false; // Checksum mismatch
  }

  // Validate sensor count
  if (current_config.sensor_count > MAX_SENSORS_IN_CONFIG)
  {
    return false; // Invalid sensor count
  }

  // Validate sensor configs (only active ones)
  for (uint8_t i = 0; i < current_config.sensor_count; i++)
  {
    if (current_config.sensors[i].is_active)
    {
      if (current_config.sensors[i].th_value < -55.0 || current_config.sensors[i].th_value > 125.0 ||
          current_config.sensors[i].tl_value < -55.0 || current_config.sensors[i].tl_value > 125.0 ||
          current_config.sensors[i].measurement_interval < MIN_MEASUREMENT_INTERVAL_MS ||
          current_config.sensors[i].measurement_interval > 60000)
      {
        return false; // Out of valid range
      }
    }
  }

  Serial.println("[EEPROM LOAD DEBUG] SUCCESS: All validations passed!");
  Serial.println("[EEPROM LOAD DEBUG] Final sensor count: " + String(current_config.sensor_count));
  return true; // Successfully loaded and validated
}

bool saveConfigToEEPROM()
{
  Serial.println("[EEPROM SAVE DEBUG] Starting EEPROM save...");
  // Interval summary for quick trace
  Serial.println("[INTERVAL DEBUG] Saving measurement_interval=" + String(measurement_interval) + "ms, selected_sensor_id=" + String(current_config.selected_sensor_id));
  // Prepare a candidate config for save (meta fields will be filled later if we actually write)
  Serial.println("[EEPROM SAVE DEBUG] Sensor count to save: " + String(current_config.sensor_count));

  // EEPROM 수명 보호: 이전 쓰기와의 시간 간격 체크
  static unsigned long last_actual_write = 0;
  unsigned long time_since_last = millis() - last_actual_write;

  if (time_since_last < CONFIG_SAVE_DEBOUNCE_MS && last_actual_write > 0)
  {
    // 너무 자주 쓰려고 함 - 디바운스 적용
    Serial.println("[EEPROM SAVE DEBUG] Write skipped - debounce protection (" + String(time_since_last) + "ms < " + String(CONFIG_SAVE_DEBOUNCE_MS) + "ms)");
    return true; // 성공으로 처리하지만 실제로는 쓰지 않음
  }

  // 불필요한 쓰기 방지: EEPROM에 저장된 내용과 실질적인 차이가 없으면 쓰기 생략
  ConfigData prev_config;
  EEPROM.get(EEPROM_START_ADDR, prev_config);

  // 이전 구성이 유효한 경우에만 비교 (매직/체크섬 확인 생략 시 false-양성 가능)
  bool prev_valid = (prev_config.magic_number == CONFIG_MAGIC_NUMBER);
  auto equalIgnoringMeta = [](const ConfigData &a, const ConfigData &b)
  {
    if (a.version != b.version)
      return false;
    if (a.selected_sensor_id != b.selected_sensor_id)
      return false;
    if (a.sensor_count != b.sensor_count)
      return false;
    for (uint8_t i = 0; i < MAX_SENSORS_IN_CONFIG; i++)
    {
      const SensorConfig &sa = a.sensors[i];
      const SensorConfig &sb = b.sensors[i];
      if (sa.is_active != sb.is_active)
        return false;
      if (sa.sensor_id != sb.sensor_id)
        return false;
      if (strncmp(sa.sensor_address, sb.sensor_address, 16) != 0)
        return false;
      if (sa.th_value != sb.th_value)
        return false;
      if (sa.tl_value != sb.tl_value)
        return false;
      if (sa.measurement_interval != sb.measurement_interval)
        return false;
    }
    return true;
  };

  if (prev_valid && equalIgnoringMeta(prev_config, current_config))
  {
    Serial.println("[EEPROM SAVE DEBUG] No significant changes detected - skipping write");
    config_dirty = false; // 더 이상 저장할 변경사항 없음
    return true;
  }

  // Prepare meta fields prior to checksum calculation
  current_config.magic_number = CONFIG_MAGIC_NUMBER;
  current_config.version = CONFIG_VERSION;
  last_actual_write = millis();
  current_config.last_write_time = last_actual_write;
  current_config.write_count++;
  Serial.println("[EEPROM SAVE DEBUG] Write count: " + String(current_config.write_count));

  // Now calculate checksum after all fields (including meta) are finalized
  current_config.checksum = calculateChecksum(current_config);
  Serial.println("[EEPROM SAVE DEBUG] Calculated checksum: 0x" + String(current_config.checksum, HEX));

  Serial.println("[EEPROM SAVE DEBUG] Writing to EEPROM address " + String(EEPROM_START_ADDR));

  // Write to EEPROM
  EEPROM.put(EEPROM_START_ADDR, current_config);

  Serial.println("[EEPROM SAVE DEBUG] EEPROM write completed, verifying...");

  // Read back and verify to detect write failures on platforms that require commit or have transient write issues.
  ConfigData verify_config;
  EEPROM.get(EEPROM_START_ADDR, verify_config);

  Serial.println("[EEPROM SAVE DEBUG] Verification read completed");
  Serial.println("[EEPROM SAVE DEBUG] Verify magic: 0x" + String(verify_config.magic_number, HEX) + " (expected: 0x" + String(current_config.magic_number, HEX) + ")");
  Serial.println("[EEPROM SAVE DEBUG] Verify version: " + String(verify_config.version) + " (expected: " + String(current_config.version) + ")");
  Serial.println("[EEPROM SAVE DEBUG] Verify checksum: 0x" + String(verify_config.checksum, HEX) + " (expected: 0x" + String(current_config.checksum, HEX) + ")");
  Serial.println("[EEPROM SAVE DEBUG] Verify sensor count: " + String(verify_config.sensor_count) + " (expected: " + String(current_config.sensor_count) + ")");

  // Compare critical fields (including checksum and magic number)
  bool match = true;
  if (verify_config.magic_number != current_config.magic_number)
    match = false;
  if (verify_config.version != current_config.version)
    match = false;
  if (verify_config.checksum != current_config.checksum)
    match = false;
  if (verify_config.sensor_count != current_config.sensor_count)
    match = false;
  if (verify_config.selected_sensor_id != current_config.selected_sensor_id)
    match = false;
  if (verify_config.write_count != current_config.write_count)
    match = false;

  if (!match)
  {
    // Indicate write didn't take; keep config_dirty true so autosave may retry later
    config_dirty = true;
    Serial.println("[EEPROM SAVE DEBUG] VERIFICATION FAILED - write did not persist");
    return false;
  }

  // Successful write
  config_dirty = false;
  eeprom_write_count++;
  Serial.println("[EEPROM SAVE DEBUG] EEPROM save SUCCESS! Config is no longer dirty.");
  return true;
}

void markConfigDirty()
{
  Serial.println("[DIRTY DEBUG] *** MARKCONFIG DIRTY FUNCTION CALLED ***");
  config_dirty = true;
  last_config_change_ms = millis();
  Serial.println("[DIRTY DEBUG] Config marked as DIRTY at time: " + String(last_config_change_ms));
  Serial.println("[DIRTY DEBUG] config_dirty = " + String(config_dirty));
  Serial.println("[DIRTY DEBUG] *** MARKCONFIG DIRTY FUNCTION FINISHED ***");
}

void markConfigDirtyDebug()
{
  Serial.println("[DEBUG DIRTY] *** NEW MARKCONFIG DIRTY DEBUG FUNCTION CALLED ***");
  config_dirty = true;
  last_config_change_ms = millis();
  Serial.println("[DEBUG DIRTY] Config marked as DIRTY at time: " + String(last_config_change_ms));
  Serial.println("[DEBUG DIRTY] config_dirty = " + String(config_dirty));
  Serial.println("[DEBUG DIRTY] *** NEW MARKCONFIG DIRTY DEBUG FUNCTION FINISHED ***");
}

bool isConfigDirty() { return config_dirty; }

void processConfigAutosave()
{
  static unsigned long last_debug_log = 0;
  unsigned long now = millis();

  // 5초마다 상태 출력 (너무 많은 로그 방지)
  if (now - last_debug_log > 5000)
  {
    Serial.println("[AUTOSAVE HEARTBEAT] config_dirty = " + String(config_dirty) + ", millis = " + String(now));
    last_debug_log = now;
  }

  if (config_dirty)
  {
    unsigned long time_since_change = now - last_config_change_ms;

    Serial.println("[AUTOSAVE DEBUG] Config is dirty, checking autosave...");
    Serial.println("[AUTOSAVE DEBUG] Time since change: " + String(time_since_change) + "ms");
    Serial.println("[AUTOSAVE DEBUG] Debounce threshold: " + String(runtime_save_debounce_ms) + "ms");

    if (time_since_change >= runtime_save_debounce_ms)
    {
      Serial.println("[AUTOSAVE DEBUG] Executing EEPROM save...");
      bool save_result = saveConfigToEEPROM();
      Serial.println("[AUTOSAVE DEBUG] Save result: " + String(save_result ? "SUCCESS" : "FAILED"));
    }
    else
    {
      Serial.println("[AUTOSAVE DEBUG] Still waiting for debounce period");
    }
  }
}

void setConfigSaveDebounceMs(unsigned long ms)
{
  // enforce reasonable bounds
  if (ms < 100)
    ms = 100;
  if (ms > 60000)
    ms = 60000;
  runtime_save_debounce_ms = ms;
}

unsigned long getConfigSaveDebounceMs()
{
  return runtime_save_debounce_ms;
}

uint32_t getEEPROMWriteCount()
{
  return eeprom_write_count;
}

bool isEEPROMValid()
{
  ConfigData temp_config;
  EEPROM.get(EEPROM_START_ADDR, temp_config);

  return (temp_config.magic_number == CONFIG_MAGIC_NUMBER &&
          temp_config.checksum == calculateChecksum(temp_config));
}

uint8_t calculateChecksum(const ConfigData &config)
{
  uint8_t checksum = 0;
  const uint8_t *data = (const uint8_t *)&config;

  // 체크섬 필드의 정확한 오프셋 계산
  size_t checksum_offset = offsetof(ConfigData, checksum);
  size_t total_size = sizeof(ConfigData);

  Serial.println("[CHECKSUM DEBUG] Total size: " + String(total_size));
  Serial.println("[CHECKSUM DEBUG] Checksum offset: " + String(checksum_offset));
  Serial.println("[CHECKSUM DEBUG] Bytes to process: " + String(checksum_offset));

  // Calculate checksum for all fields except checksum itself
  for (size_t i = 0; i < checksum_offset; i++)
  {
    checksum ^= data[i];
  }

  return checksum;
}

void printConfigData(const ConfigData &config)
{
  Serial.println("=== Configuration Data (ID-based) ===");
  Serial.print("Magic Number: 0x");
  Serial.println(config.magic_number, HEX);
  Serial.print("Version: ");
  Serial.println(config.version);
  Serial.print("Selected Sensor ID: ");
  Serial.println(config.selected_sensor_id);
  Serial.print("Sensor Count: ");
  Serial.println(config.sensor_count);
  Serial.print("Write Count: ");
  Serial.println(config.write_count);
  Serial.print("Checksum: 0x");
  Serial.println(config.checksum, HEX);

  // Print active sensors (search all slots, not just up to sensor_count)
  for (uint8_t i = 0; i < MAX_SENSORS_IN_CONFIG; i++)
  {
    if (config.sensors[i].is_active && config.sensors[i].sensor_id > 0)
    {
      Serial.print("Sensor[");
      Serial.print(i);
      Serial.print("] ID=");
      Serial.print(config.sensors[i].sensor_id);
      Serial.print(": ");
      Serial.print(config.sensors[i].sensor_address);
      Serial.print(" TH=");
      Serial.print(config.sensors[i].th_value, 1);
      Serial.print(" TL=");
      Serial.print(config.sensors[i].tl_value, 1);
      Serial.print(" Interval=");
      Serial.println(config.sensors[i].measurement_interval);
    }
  }
  Serial.println("=============================");
}
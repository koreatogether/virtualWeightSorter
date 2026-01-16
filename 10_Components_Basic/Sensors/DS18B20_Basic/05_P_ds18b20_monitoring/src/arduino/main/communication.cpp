/**
 * Communication Management Implementation
 */

#include "communication.h"
#include "sensor_manager.h"
#include "config_manager.h"

// ==================== Serial Initialization ====================
void initializeSerial()
{
  Serial.begin(SERIAL_BAUDRATE);

  // Arduino Uno R4 WiFi 연결 안정성 개선
  // 무한 대기 방지를 위해 타임아웃 추가
  unsigned long start_time = millis();
  while (!Serial && (millis() - start_time < 3000)) // 최대 3초 대기
  {
    // non-blocking friendly wait
    // tiny yield without long blocking
    // (keep loop spinning)
  }
  // 추가 안정화 시간 최소화 (non-blocking)
  unsigned long after_start = millis();
  while (millis() - after_start < 20)
  {
    // short spin to settle Serial without long delay
  }
}

// ==================== Data Transmission ====================
void sendSensorData(float temperature)
{
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;

  // Create sensor data JSON - 파이썬 프로토콜 호환 형식
  doc["type"] = "sensor_data";
  doc["temperature"] = round(temperature * 10) / 10.0; // Round to 1 decimal place
  doc["sensor_addr"] = getSensorId();                  // 물리(고유) 센서 주소
  // DS18B20 센서에서 직접 사용자 ID 읽기
  extern uint8_t current_sensor_index;
  extern DallasTemperature sensors;
  int16_t user_id = sensors.getUserDataByIndex(current_sensor_index);
  doc["user_sensor_id"] = (user_id >= 1 && user_id <= 8) ? user_id : 0;

  // ID 기반으로 센서 설정 가져오기 (새로운 방식)
  SensorConfig sensor_config;
  bool config_found = false;

  // 먼저 사용자 ID로 설정을 찾아보기 (사용자 ID가 1-8 범위인 경우)
  if (user_id >= 1 && user_id <= 8)
  {
    Serial.println("[TH/TL DEBUG] Looking for config by sensor ID: " + String(user_id));
    config_found = getSensorConfig(user_id, sensor_config);
  }

  // ID로 찾지 못했으면 주소로 찾기 (호환성)
  if (!config_found)
  {
    String current_sensor_addr = getSensorId();
    Serial.println("[TH/TL DEBUG] Looking for config by sensor addr: " + current_sensor_addr);
    config_found = getSensorConfigByAddress(current_sensor_addr, sensor_config);
  }

  if (config_found)
  {
    Serial.println("[TH/TL DEBUG] Found config - TH: " + String(sensor_config.th_value, 1) + ", TL: " + String(sensor_config.tl_value, 1));
    doc["th_value"] = sensor_config.th_value;
    doc["tl_value"] = sensor_config.tl_value;
  }
  else
  {
    Serial.println("[TH/TL DEBUG] No config found, using defaults");
    // 설정이 없으면 하드코딩 기본값 사용
    doc["th_value"] = DEFAULT_TH_VALUE;
    doc["tl_value"] = DEFAULT_TL_VALUE;
  }

  doc["measurement_interval"] = getMeasurementInterval(); // 추가된 필드

  // ISO 형식 타임스탬프 대신 밀리초 사용 (단순화)
  doc["timestamp"] = millis();

  // Send JSON data
  serializeJson(doc, Serial);
  Serial.println();
}

void sendMultiSensorData(float temperature, uint8_t original_index, uint8_t sorted_index)
{
  extern uint8_t sensor_count;
  extern DeviceAddress sensor_addresses[8];
  extern DallasTemperature sensors;

  StaticJsonDocument<JSON_BUFFER_SIZE> doc;

  // Create sensor data JSON for multi-sensor system
  doc["type"] = "sensor_data";
  doc["temperature"] = round(temperature * 10) / 10.0;

  // Get sensor hardware address
  String addr_str = "";
  for (uint8_t i = 0; i < 8; i++)
  {
    if (sensor_addresses[original_index][i] < 16)
      addr_str += "0";
    addr_str += String(sensor_addresses[original_index][i], HEX);
  }
  addr_str.toUpperCase();
  doc["sensor_addr"] = addr_str;

  // Get user ID from EEPROM (using TH/TL registers as user data)
  int16_t user_id = sensors.getUserDataByIndex(original_index);
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

  doc["sensor_id"] = display_id;   // Python expects this field name
  doc["user_sensor_id"] = user_id; // Raw user ID for reference

  // ID 기반으로 센서 설정 가져오기 (새로운 방식)
  SensorConfig sensor_config;
  bool config_found = false;

  // 먼저 사용자 ID로 설정을 찾아보기 (사용자 ID가 1-8 범위인 경우)
  if (user_id >= 1 && user_id <= 8)
  {
    Serial.println("[TH/TL DEBUG] Multi-sensor looking for config by sensor ID: " + String(user_id));
    config_found = getSensorConfig(user_id, sensor_config);
  }

  // ID로 찾지 못했으면 주소로 찾기 (호환성)
  if (!config_found)
  {
    Serial.println("[TH/TL DEBUG] Multi-sensor looking for config by addr: " + addr_str);
    config_found = getSensorConfigByAddress(addr_str, sensor_config);
  }

  if (config_found)
  {
    Serial.println("[TH/TL DEBUG] Found multi-config - TH: " + String(sensor_config.th_value, 1) + ", TL: " + String(sensor_config.tl_value, 1));
    doc["th_value"] = sensor_config.th_value;
    doc["tl_value"] = sensor_config.tl_value;
  }
  else
  {
    Serial.println("[TH/TL DEBUG] No multi-config found, using defaults");
    // 설정이 없으면 하드코딩 기본값 사용
    doc["th_value"] = DEFAULT_TH_VALUE;
    doc["tl_value"] = DEFAULT_TL_VALUE;
  }

  doc["measurement_interval"] = getMeasurementInterval();
  doc["timestamp"] = millis();

  // Send JSON data
  serializeJson(doc, Serial);
  Serial.println();
}

void sendResponse(const String &status, const String &message)
{
  Serial.println("[RESPONSE DEBUG] Sending response: " + status + " - " + message);

  StaticJsonDocument<JSON_BUFFER_SIZE> doc;

  doc["type"] = "response";
  doc["status"] = status;
  doc["message"] = message;
  doc["timestamp"] = millis();

  serializeJson(doc, Serial);
  Serial.println();
  Serial.flush(); // 중요: 버퍼 강제 플러시

  Serial.println("[RESPONSE DEBUG] Response sent and flushed");
}

void sendSystemStatus()
{
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;

  doc["type"] = "system_status";
  doc["sensor_connected"] = isSensorConnected();
  // Report selected sensor physical address explicitly (and keep legacy field for compatibility)
  doc["selected_sensor_addr"] = getSensorId();
  doc["sensor_addr"] = getSensorId();
  // DS18B20 센서에서 직접 사용자 ID 읽기
  extern uint8_t current_sensor_index;
  extern DallasTemperature sensors;
  int16_t user_id = sensors.getUserDataByIndex(current_sensor_index);
  doc["user_sensor_id"] = (user_id >= 1 && user_id <= 8) ? user_id : 0;
  doc["th_value"] = getThValue();
  doc["tl_value"] = getTlValue();
  doc["measurement_interval"] = getMeasurementInterval();
  doc["last_temperature"] = getLastTemperature();
  doc["uptime"] = millis();
  doc["config_version"] = current_config.version;
  doc["config_dirty"] = isConfigDirty();
  doc["eeprom_writes"] = getEEPROMWriteCount();
  doc["debounce_ms"] = getConfigSaveDebounceMs();
  // expose effective measurement interval and sensor resolution
  doc["effective_interval_ms"] = getEffectiveMeasurementInterval();
  Serial.println("[INTERVAL DEBUG] System status: interval_ms=" + String(doc["measurement_interval"].as<unsigned long>()) +
                 ", effective_ms=" + String(doc["effective_interval_ms"].as<unsigned long>()));
  extern uint8_t sensor_resolution;
  doc["sensor_resolution"] = sensor_resolution;

  serializeJson(doc, Serial);
  Serial.println();
}

// ==================== Command Processing ====================
void processSerialCommand()
{
  Serial.println("[COMM DEBUG] processSerialCommand() started");

  // Read incoming JSON command with timeout protection
  String command_line = Serial.readStringUntil('\n');
  command_line.trim();

  Serial.println("[COMM DEBUG] Received command: " + command_line);

  if (command_line.length() == 0)
  {
    Serial.println("[COMM DEBUG] Empty command, returning");
    return;
  }

  // 연결 안정성을 위한 추가 체크
  if (!Serial)
  {
    Serial.println("[COMM DEBUG] Serial not connected, returning");
    return; // 시리얼 연결이 끊어진 경우 즉시 리턴
  }

  // 시리얼 버퍼 정리 (중요: 응답 전에 입력 버퍼 클리어)
  while (Serial.available() > 0)
  {
    Serial.read(); // 남은 데이터 클리어
  }

  // Parse JSON command
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;
  DeserializationError error = deserializeJson(doc, command_line);

  if (error)
  {
    Serial.println("[COMM DEBUG] JSON parse error: " + String(error.c_str()));
    sendResponse("error", "Invalid JSON command: " + String(error.c_str()));
    Serial.println("[COMM DEBUG] Error response sent");
    return;
  }

  Serial.println("[COMM DEBUG] JSON parsed successfully");

  // Handle different command types
  String command_type = doc["type"].as<String>();

  // 간단한 텍스트 명령어 처리 (SET_SENSOR_ID:주소:ID 형식)
  if (command_line.startsWith("SET_SENSOR_ID:"))
  {
    // SET_SENSOR_ID:285C82850000005D:02 형식 파싱
    int first_colon = command_line.indexOf(':', 14);
    int second_colon = command_line.indexOf(':', first_colon + 1);

    if (first_colon > 0 && second_colon > 0)
    {
      String sensor_addr = command_line.substring(14, first_colon);
      String sensor_id_str = command_line.substring(first_colon + 1, second_colon);

      if (sensor_addr.length() == 16 && sensor_id_str.length() <= 2)
      {
        int new_id = sensor_id_str.toInt();
        if (new_id >= 1 && new_id <= 8)
        {
          // 센서 주소로 센서 찾기 및 ID 설정 + EEPROM 매핑 갱신
          extern uint8_t sensor_count;
          extern DeviceAddress sensor_addresses[8];

          bool found = false;
          bool conflict = false;
          uint8_t conflict_id = 0;

          // 충돌 검사: 이 주소가 이미 다른 ID에 매핑되어 있는지
          uint8_t existing_id = findSensorIdByAddress(sensor_addr);
          if (existing_id > 0 && existing_id != (uint8_t)new_id)
          {
            conflict = true;
            conflict_id = existing_id;
          }

          if (conflict)
          {
            sendResponse("error", "Address already mapped to ID " + String(conflict_id));
            return;
          }

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

            if (addr_str == sensor_addr)
            {
              // 센서 발견, ID 설정
              sensors.setUserDataByIndex(i, (int16_t)new_id);
              int16_t verify_id = sensors.getUserDataByIndex(i);

              if (verify_id == (int16_t)new_id)
              {
                // EEPROM 구성 갱신: 기존 설정 보존하며 주소/ID 바인딩
                SensorConfig cfg;
                bool have_cfg = getSensorConfig((uint8_t)new_id, cfg);
                if (!have_cfg)
                {
                  // 기본값 생성
                  cfg.sensor_id = (uint8_t)new_id;
                  strncpy(cfg.sensor_address, sensor_addr.c_str(), 16);
                  cfg.sensor_address[16] = '\0';
                  cfg.th_value = DEFAULT_TH_VALUE;
                  cfg.tl_value = DEFAULT_TL_VALUE;
                  cfg.measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
                  cfg.is_active = 1;
                }
                else
                {
                  // 주소만 갱신하여 교체 시나리오 반영(TH/TL 등 유지)
                  strncpy(cfg.sensor_address, sensor_addr.c_str(), 16);
                  cfg.sensor_address[16] = '\0';
                  cfg.sensor_id = (uint8_t)new_id;
                  cfg.is_active = 1;
                }
                addOrUpdateSensorConfig((uint8_t)new_id, sensor_addr, cfg);
                sendResponse("success", "Sensor ID and mapping updated successfully");
                found = true;
              }
              else
              {
                sendResponse("error", "Failed to set sensor ID");
                found = true;
              }
              break;
            }
          }

          if (!found)
          {
            sendResponse("error", "Sensor address not found");
          }
        }
        else
        {
          sendResponse("error", "Invalid sensor ID (must be 1-8)");
        }
      }
      else
      {
        sendResponse("error", "Invalid command format");
      }
    }
    else
    {
      sendResponse("error", "Invalid command format");
    }
    return;
  }

  if (command_type == "ping")
  {
    sendResponse("success", "pong");
  }
  else if (command_type == "command")
  {
    String command = doc["command"].as<String>();

    if (command == "get_status")
    {
      sendSystemStatus();
    }
    else if (command == "set_config")
    {
      handleConfigCommand(doc);
    }
    else if (command == "list_sensors")
    {
      // Build richer sensor listing with id/address/present/thresholds
      extern uint8_t sensor_count;
      extern DeviceAddress sensor_addresses[8];
      StaticJsonDocument<JSON_BUFFER_SIZE> resp;
      // Keep legacy envelope
      resp["type"] = "response";
      resp["status"] = "success";
      resp["message"] = "sensor_list";
      resp["count"] = getSensorCount();
      resp["addresses"] = getAllSensorAddresses();
      // New rich array
      JsonArray items = resp.createNestedArray("items");
      for (uint8_t i = 0; i < sensor_count; i++)
      {
        // Compose address
        String addr_str = "";
        for (uint8_t j = 0; j < 8; j++)
        {
          if (sensor_addresses[i][j] < 16)
            addr_str += "0";
          addr_str += String(sensor_addresses[i][j], HEX);
        }
        addr_str.toUpperCase();

        // Derive user ID from sensor scratchpad
        int16_t uid = sensors.getUserDataByIndex(i);
        uint8_t id_val = (uid >= 1 && uid <= 8) ? (uint8_t)uid : 0;

        // Lookup config
        SensorConfig cfg;
        bool has_cfg = false;
        if (id_val >= 1 && id_val <= 8)
        {
          has_cfg = getSensorConfig(id_val, cfg);
        }
        if (!has_cfg)
        {
          has_cfg = getSensorConfigByAddress(addr_str, cfg);
        }

        JsonObject it = items.createNestedObject();
        it["id"] = id_val;
        it["address"] = addr_str;
        it["present"] = true; // on-bus sensors are present
        if (has_cfg)
        {
          it["th"] = cfg.th_value;
          it["tl"] = cfg.tl_value;
        }
        else
        {
          it["th"] = DEFAULT_TH_VALUE;
          it["tl"] = DEFAULT_TL_VALUE;
        }
      }
      serializeJson(resp, Serial);
      Serial.println();
    }
    else if (command == "select_sensor_index")
    {
      if (!doc.containsKey("index"))
      {
        sendResponse("error", "Missing index for select_sensor_index");
      }
      else
      {
        int idx = doc["index"].as<int>();
        if (idx >= 0 && idx < getSensorCount())
        {
          if (selectSensorByIndex((uint8_t)idx))
          {
            markConfigDirty(); // 선택 주소를 유지하려면 저장 반영
            sendResponse("success", "Selected sensor index " + String(idx));
            sendSystemStatus();
          }
          else
          {
            sendResponse("error", "Selection failed");
          }
        }
        else
        {
          sendResponse("error", "Index out of range");
        }
      }
    }
    else if (command == "set_sensor_data")
    {
      // 사용자 정의 센서 ID 설정 (1~8)
      if (!doc.containsKey("new_value"))
      {
        sendResponse("error", "Missing new_value for set_sensor_data");
      }
      else
      {
        int new_id = doc["new_value"].as<int>();
        if (new_id >= 1 && new_id <= 8)
        {
          // 현재 선택된 센서의 EEPROM에 사용자 ID 저장
          extern uint8_t current_sensor_index;
          extern uint8_t sensor_count;
          extern DeviceAddress sensor_addresses[8];

          if (current_sensor_index < sensor_count)
          {
            // 센서 EEPROM의 TH/TL 레지스터에 사용자 ID 저장
            sensors.setUserDataByIndex(current_sensor_index, (int16_t)new_id);

            // DS18B20 센서의 영구 저장소에 저장 (scratchpad → EEPROM)
            sensors.saveScratchPad(sensor_addresses[current_sensor_index]);

            // 저장 확인을 위해 다시 읽어보기
            int16_t verify_id = sensors.getUserDataByIndex(current_sensor_index);

            if (verify_id == (int16_t)new_id)
            {
              // EEPROM 매핑 갱신 처리
              String addr_str = "";
              for (uint8_t j = 0; j < 8; j++)
              {
                if (sensor_addresses[current_sensor_index][j] < 16)
                  addr_str += "0";
                addr_str += String(sensor_addresses[current_sensor_index][j], HEX);
              }
              addr_str.toUpperCase();

              Serial.println(String("[SET_SENSOR_DATA MAP] Verifying mapping for addr=") + addr_str + ", new_id=" + String(new_id));
              uint8_t existing_id = findSensorIdByAddress(addr_str);
              if (existing_id == (uint8_t)new_id)
              {
                // 이미 같은 ID로 매핑되어 있음 → 설정만 보존 보장
                SensorConfig cfg;
                bool have = getSensorConfig((uint8_t)new_id, cfg);
                if (!have)
                {
                  cfg.sensor_id = (uint8_t)new_id;
                  strncpy(cfg.sensor_address, addr_str.c_str(), 16);
                  cfg.sensor_address[16] = '\0';
                  cfg.th_value = DEFAULT_TH_VALUE;
                  cfg.tl_value = DEFAULT_TL_VALUE;
                  cfg.measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
                  cfg.is_active = 1;
                  addOrUpdateSensorConfig((uint8_t)new_id, addr_str, cfg);
                  Serial.println("[SET_SENSOR_DATA MAP] Created default config for existing mapping");
                }
              }
              else if (existing_id == 0)
              {
                // 신규 매핑 생성
                SensorConfig cfg;
                cfg.sensor_id = (uint8_t)new_id;
                strncpy(cfg.sensor_address, addr_str.c_str(), 16);
                cfg.sensor_address[16] = '\0';
                cfg.th_value = DEFAULT_TH_VALUE;
                cfg.tl_value = DEFAULT_TL_VALUE;
                cfg.measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
                cfg.is_active = 1;
                addOrUpdateSensorConfig((uint8_t)new_id, addr_str, cfg);
                Serial.println("[SET_SENSOR_DATA MAP] Created new mapping ID→ADDR");
              }
              else
              {
                // 충돌: 같은 주소가 다른 ID에 있음 → 재바인딩 수행(교체 시나리오)
                Serial.println(String("[SET_SENSOR_DATA MAP] Rebinding: addr maps to ID ") + String(existing_id) + ", moving to " + String(new_id));
                SensorConfig cfg_old;
                bool have_old = getSensorConfig((uint8_t)existing_id, cfg_old);
                if (!have_old)
                {
                  // 기본값으로 이전 슬롯 생성 후 이동
                  cfg_old.sensor_id = (uint8_t)existing_id;
                  strncpy(cfg_old.sensor_address, addr_str.c_str(), 16);
                  cfg_old.sensor_address[16] = '\0';
                  cfg_old.th_value = DEFAULT_TH_VALUE;
                  cfg_old.tl_value = DEFAULT_TL_VALUE;
                  cfg_old.measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
                  cfg_old.is_active = 1;
                }
                // 새 ID 슬롯으로 복사(주소는 동일, ID만 변경)
                cfg_old.sensor_id = (uint8_t)new_id;
                strncpy(cfg_old.sensor_address, addr_str.c_str(), 16);
                cfg_old.sensor_address[16] = '\0';
                cfg_old.is_active = 1;
                addOrUpdateSensorConfig((uint8_t)new_id, addr_str, cfg_old);
                // 이전 ID 슬롯 비활성화
                if (existing_id >= 1 && existing_id <= 8)
                {
                  uint8_t idx = (uint8_t)existing_id - 1;
                  current_config.sensors[idx].is_active = 0;
                  current_config.sensors[idx].sensor_id = 0;
                  current_config.sensors[idx].sensor_address[0] = '\0';
                  markConfigDirty();
                }
              }

              sendResponse("success", "Sensor ID " + String(new_id) + " saved & mapping updated for sensor " + addr_str + " (DS18B20 + EEPROM)");
            }
            else
            {
              sendResponse("error", "Failed to write user ID to sensor EEPROM (wrote " + String(verify_id) + " instead of " + String(new_id) + ")");
            }
          }
          else
          {
            sendResponse("error", "No sensor selected or sensor index out of range");
          }
        }
        else
        {
          sendResponse("error", "User sensor ID must be 1-8");
        }
      }
    }
    else if (command == "force_measurement")
    {
      performMeasurement();
    }
    else if (command == "set_threshold")
    {
      // 센서별 임계값 설정
      Serial.println("[COMM DEBUG] Processing set_threshold command");
      handleSetThresholdCommand(doc);
      Serial.println("[COMM DEBUG] set_threshold command completed");
    }
    else if (command == "get_sensor_config")
    {
      // 센서별 설정 조회
      Serial.println("[COMM DEBUG] Processing get_sensor_config command");
      handleGetSensorConfigCommand(doc);
      Serial.println("[COMM DEBUG] get_sensor_config command completed");
    }
    else if (command == "commit_config")
    {
      if (isConfigDirty())
      {
        bool ok = saveConfigToEEPROM();
        if (ok)
        {
          sendResponse("success", "Config committed to EEPROM");
          sendSystemStatus();
        }
        else
        {
          sendResponse("error", "Failed to write config to EEPROM");
        }
      }
      else
      {
        sendResponse("success", "Config not dirty; no write needed");
      }
    }
    else
    {
      sendResponse("error", "Unknown command: " + command);
    }
  }
  else
  {
    sendResponse("error", "Unknown command type: " + command_type);
  }
}

void handleConfigCommand(JsonDocument &doc)
{
  String config_type = doc["config_type"].as<String>();
  // Backward compatibility / alias handling: allow shorter 'interval'
  if (config_type == "interval")
  {
    config_type = "measurement_interval";
  }
  bool config_changed = false;

  if (config_type == "th")
  {
    float new_th = doc["new_value"].as<float>();
    if (new_th >= -55.0 && new_th <= 125.0)
    {
      setThValue(new_th);
      config_changed = true;
      // 즉시 저장하지 않고 디바운스 사용 (EEPROM 수명 보호)
      markConfigDirty();
      sendResponse("success", "TH value set to " + String(getThValue(), 1) + "°C (will be saved automatically)");
    }
    else
    {
      sendResponse("error", "TH value out of range (-55°C to 125°C)");
    }
  }
  else if (config_type == "tl")
  {
    float new_tl = doc["new_value"].as<float>();
    if (new_tl >= -55.0 && new_tl <= 125.0)
    {
      setTlValue(new_tl);
      config_changed = true;
      // 즉시 저장하지 않고 디바운스 사용 (EEPROM 수명 보호)
      markConfigDirty();
      sendResponse("success", "TL value set to " + String(getTlValue(), 1) + "°C (will be saved automatically)");
    }
    else
    {
      sendResponse("error", "TL value out of range (-55°C to 125°C)");
    }
  }
  else if (config_type == "measurement_interval")
  {                                                                           // also accepts alias 'interval'
    unsigned long new_interval = doc["new_value"].as<unsigned long>() * 1000; // Convert to ms
    Serial.println("[INTERVAL DEBUG] set_config(measurement_interval) new_value(sec)=" + String(doc["new_value"].as<unsigned long>()) +
                   ", new_interval_ms=" + String(new_interval));
    if (new_interval >= MIN_MEASUREMENT_INTERVAL_MS && new_interval <= 60000)
    { // MIN to 60s
      setMeasurementInterval(new_interval);
      config_changed = true;
      // 즉시 저장하지 않고 디바운스 사용 (EEPROM 수명 보호)
      markConfigDirty();
      Serial.println("[INTERVAL DEBUG] Interval accepted; dirty flagged for autosave");
      sendResponse("success", "Measurement interval set to " + String(new_interval / 1000) + "s (will be saved automatically)");
    }
    else
    {
      sendResponse("error", "Measurement interval out of range (MIN to 60s)");
    }
  }
  else if (config_type == "sensor_addr" || config_type == "addr") // sensor_id -> sensor_addr로 변경, addr 별칭 지원
  {
    String new_addr = doc["new_value"].as<String>();
    if (new_addr.length() == 16)
    {
      bool ok = selectSensorByAddress(new_addr);
      if (ok)
      {
        // 센서 주소 설정 성공 시 config에도 저장 (주소를 ID로 변환)
        uint8_t sensor_id = findSensorIdByAddress(new_addr);
        if (sensor_id > 0 && current_config.selected_sensor_id != sensor_id)
        {
          current_config.selected_sensor_id = sensor_id;
          config_changed = true;
          // 즉시 저장하지 않고 디바운스 사용 (EEPROM 수명 보호)
          markConfigDirty();
        }
        sendResponse("success", "Sensor ADDR set to " + getSensorId() + " (will be saved automatically)");
      }
      else
      {
        sendResponse("error", "Sensor address not found or not selectable");
      }
    }
    else
    {
      sendResponse("error", "Sensor ADDR must be 16 characters long");
    }
  }
  else
  {
    sendResponse("error", "Unknown config type: " + config_type);
  }

  // Allow runtime tweak of debounce
  if (config_type == "debounce_ms")
  {
    unsigned long new_ms = doc["new_value"].as<unsigned long>();
    if (new_ms >= 100 && new_ms <= 60000)
    {
      setConfigSaveDebounceMs(new_ms);
      sendResponse("success", "Debounce set to " + String(getConfigSaveDebounceMs()) + " ms");
      config_changed = true;
    }
    else
    {
      sendResponse("error", "debounce_ms out of range (100..60000)");
    }
  }

  // Send updated system status if config changed
  if (config_changed)
  {
    // Avoid blocking delay before sending status
    sendSystemStatus();
  }
}

void handleSetThresholdCommand(JsonDocument &doc)
{
  Serial.println("[THRESHOLD COMM DEBUG] handleSetThresholdCommand called");

  // 센서 식별 방법 확인 (sensor_id 우선, 없으면 sensor_addr)
  bool has_sensor_id = doc.containsKey("sensor_id");
  bool has_sensor_addr = doc.containsKey("sensor_addr");

  if (!has_sensor_id && !has_sensor_addr)
  {
    sendResponse("error", "Missing sensor_id or sensor_addr for set_threshold");
    return;
  }

  uint8_t sensor_id = 0;
  String sensor_address = "";

  if (has_sensor_id)
  {
    // ID 기반 방식 (새로운 주요 방식)
    sensor_id = doc["sensor_id"].as<uint8_t>();
    Serial.println("[THRESHOLD COMM DEBUG] Using sensor ID: " + String(sensor_id));

    if (sensor_id < 1 || sensor_id > 8)
    {
      sendResponse("error", "Invalid sensor_id (must be 1-8)");
      return;
    }
  }
  else
  {
    // 주소 기반 방식 (호환성)
    sensor_address = doc["sensor_addr"].as<String>();
    Serial.println("[THRESHOLD COMM DEBUG] Using sensor address: " + sensor_address);

    if (sensor_address.length() != 16)
    {
      sendResponse("error", "Invalid sensor address format (must be 16 characters)");
      return;
    }

    // 주소를 ID로 변환 시도
    sensor_id = findSensorIdByAddress(sensor_address);
    if (sensor_id == 0)
    {
      sendResponse("error", "Sensor address not found or not assigned an ID (1-8)");
      return;
    }
    Serial.println("[THRESHOLD COMM DEBUG] Found sensor ID " + String(sensor_id) + " for address " + sensor_address);
  }

  // 임계값 파라미터 확인 (최소 하나는 있어야 함)
  bool has_th = doc.containsKey("th_value");
  bool has_tl = doc.containsKey("tl_value");

  if (!has_th && !has_tl)
  {
    sendResponse("error", "Missing th_value or tl_value (at least one required)");
    return;
  }

  float th_value = has_th ? doc["th_value"].as<float>() : NAN;
  float tl_value = has_tl ? doc["tl_value"].as<float>() : NAN;

  // 값 유효성 검사
  if (has_th && (th_value < -55.0 || th_value > 125.0))
  {
    sendResponse("error", "TH value out of range (-55°C to 125°C)");
    return;
  }

  if (has_tl && (tl_value < -55.0 || tl_value > 125.0))
  {
    sendResponse("error", "TL value out of range (-55°C to 125°C)");
    return;
  }

  // TH > TL 검사 (둘 다 설정된 경우)
  if (has_th && has_tl && th_value <= tl_value)
  {
    sendResponse("error", "TH value must be greater than TL value");
    return;
  }

  // 기존 센서 설정 가져오기
  SensorConfig sensor_config;
  bool sensor_exists = getSensorConfig(sensor_id, sensor_config);

  Serial.println("[THRESHOLD COMM DEBUG] Sensor exists: " + String(sensor_exists ? "true" : "false"));

  if (!sensor_exists)
  {
    // 매핑이 없으면 DS18B20 사용자 ID 스캔을 통해 자동 바인딩 시도
    Serial.println("[THRESHOLD COMM DEBUG] No config for ID; attempting auto-bind via DS18B20 user_id scan");
    extern uint8_t sensor_count;
    extern DeviceAddress sensor_addresses[8];
    extern DallasTemperature sensors;

    bool bound = false;
    String bound_addr = "";
    for (uint8_t i = 0; i < sensor_count; i++)
    {
      int16_t uid = sensors.getUserDataByIndex(i);
      if (uid == (int16_t)sensor_id)
      {
        // 주소 문자열 생성
        String addr_str = "";
        for (uint8_t j = 0; j < 8; j++)
        {
          if (sensor_addresses[i][j] < 16)
            addr_str += "0";
          addr_str += String(sensor_addresses[i][j], HEX);
        }
        addr_str.toUpperCase();

        SensorConfig cfg;
        cfg.sensor_id = sensor_id;
        strncpy(cfg.sensor_address, addr_str.c_str(), 16);
        cfg.sensor_address[16] = '\0';
        cfg.th_value = DEFAULT_TH_VALUE;
        cfg.tl_value = DEFAULT_TL_VALUE;
        cfg.measurement_interval = DEFAULT_MEASUREMENT_INTERVAL;
        cfg.is_active = 1;

        addOrUpdateSensorConfig(sensor_id, addr_str, cfg);
        markConfigDirty();

        sensor_config = cfg;
        sensor_exists = true;
        bound = true;
        bound_addr = addr_str;
        Serial.println("[THRESHOLD COMM DEBUG] Auto-bound sensor ID " + String(sensor_id) + " to addr " + addr_str);
        break;
      }
    }

    if (!bound)
    {
      sendResponse("error", "Sensor ID " + String(sensor_id) + " not found or not active");
      return;
    }
  }

  // ID 기반으로 임계값 설정
  bool save_success = false;

  if (has_th && has_tl)
  {
    save_success = setSensorThreshold(sensor_id, th_value, tl_value);
  }
  else if (has_th)
  {
    save_success = setSensorThresholdTH(sensor_id, th_value);
  }
  else if (has_tl)
  {
    save_success = setSensorThresholdTL(sensor_id, tl_value);
  }

  Serial.println("[THRESHOLD COMM DEBUG] Save success: " + String(save_success ? "true" : "false"));

  if (save_success)
  {
    String response_msg = "Threshold settings saved for sensor ID " + String(sensor_id);
    if (has_th && has_tl)
    {
      response_msg += " (TH=" + String(th_value, 1) + "°C, TL=" + String(tl_value, 1) + "°C)";
    }
    else if (has_th)
    {
      response_msg += " (TH=" + String(th_value, 1) + "°C)";
    }
    else if (has_tl)
    {
      response_msg += " (TL=" + String(tl_value, 1) + "°C)";
    }
    response_msg += " - will be saved to EEPROM automatically";

    sendResponse("success", response_msg);

    // 시스템 상태 업데이트
    delay(100);
    sendSystemStatus();
  }
  else
  {
    sendResponse("error", "Failed to save threshold settings for sensor ID " + String(sensor_id));
  }

  Serial.println("[THRESHOLD COMM DEBUG] handleSetThresholdCommand completed");
}

void handleGetSensorConfigCommand(JsonDocument &doc)
{
  Serial.println("[GET CONFIG DEBUG] handleGetSensorConfigCommand called");

  // 센서 식별 방법 확인 (sensor_id 우선, 없으면 sensor_addr)
  bool has_sensor_id = doc.containsKey("sensor_id");
  bool has_sensor_addr = doc.containsKey("sensor_addr");

  if (!has_sensor_id && !has_sensor_addr)
  {
    sendResponse("error", "Missing sensor_id or sensor_addr for get_sensor_config");
    return;
  }

  uint8_t sensor_id = 0;
  String sensor_address = "";
  SensorConfig sensor_config;
  bool found = false;

  if (has_sensor_id)
  {
    // ID 기반 방식 (새로운 주요 방식)
    sensor_id = doc["sensor_id"].as<uint8_t>();
    Serial.println("[GET CONFIG DEBUG] Using sensor ID: " + String(sensor_id));

    if (sensor_id < 1 || sensor_id > 8)
    {
      sendResponse("error", "Invalid sensor_id (must be 1-8)");
      return;
    }

    // ID 기반으로 설정 조회
    found = getSensorConfig(sensor_id, sensor_config);
    sensor_address = found ? String(sensor_config.sensor_address) : "Unknown";
  }
  else
  {
    // 주소 기반 방식 (호환성)
    sensor_address = doc["sensor_addr"].as<String>();
    Serial.println("[GET CONFIG DEBUG] Using sensor address: " + sensor_address);

    if (sensor_address.length() != 16)
    {
      sendResponse("error", "Invalid sensor address format (must be 16 characters)");
      return;
    }

    // 주소를 ID로 변환 후 조회
    sensor_id = findSensorIdByAddress(sensor_address);
    if (sensor_id > 0)
    {
      found = getSensorConfig(sensor_id, sensor_config);
    }
    else
    {
      // ID가 없으면 호환성을 위해 기본값 사용
      found = false;
    }
  }

  Serial.println("[GET CONFIG DEBUG] Sensor found: " + String(found ? "true" : "false"));

  // 응답 생성
  StaticJsonDocument<JSON_BUFFER_SIZE> response_doc;
  response_doc["type"] = "response";
  response_doc["status"] = "success";

  if (found)
  {
    // 센서 설정이 있으면 해당 값들 반환
    response_doc["message"] = "Sensor config found";
    response_doc["sensor_addr"] = sensor_address;
    response_doc["sensor_id"] = sensor_id;
    response_doc["th_value"] = sensor_config.th_value;
    response_doc["tl_value"] = sensor_config.tl_value;
    response_doc["measurement_interval"] = sensor_config.measurement_interval;
  }
  else
  {
    // 센서 설정이 없으면 기본값 반환
    response_doc["message"] = "Sensor config not found, returning defaults";
    response_doc["sensor_addr"] = sensor_address;
    response_doc["sensor_id"] = sensor_id;
    response_doc["th_value"] = DEFAULT_TH_VALUE;
    response_doc["tl_value"] = DEFAULT_TL_VALUE;
    response_doc["measurement_interval"] = DEFAULT_MEASUREMENT_INTERVAL;
  }

  response_doc["timestamp"] = millis();

  serializeJson(response_doc, Serial);
  Serial.println();

  Serial.println("[GET CONFIG DEBUG] handleGetSensorConfigCommand completed");
}
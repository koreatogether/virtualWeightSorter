/**
 * Configuration Management Header
 *
 * Handles system configuration parameters including temperature
 * thresholds, measurement intervals, and sensor settings with
 * EEPROM persistence support.
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <EEPROM.h>
#include "sensor_manager.h"

// ==================== Constants ====================
#define DEFAULT_TH_VALUE 30.0             // High temperature threshold (°C)
#define DEFAULT_TL_VALUE 15.0             // Low temperature threshold (°C)
#define DEFAULT_MEASUREMENT_INTERVAL 2000 // Default measurement interval in ms (8개 센서 대응)
#define MIN_MEASUREMENT_INTERVAL_MS 100   // Minimum configurable measurement interval in ms (logical minimum)

#define CONFIG_VERSION 3             // Config structure version (증가: ID 기반 시스템으로 변경)
#define CONFIG_SAVE_DEBOUNCE_MS 3000 // Delay (ms) before committing EEPROM after last change (EEPROM 안전 위해 3초)
#define MAX_SENSORS_IN_CONFIG 8      // 최대 센서 수

// EEPROM Settings
#define EEPROM_SIZE 512            // Total EEPROM size for Uno R4 WiFi
#define EEPROM_START_ADDR 0        // Starting address for configuration
#define CONFIG_MAGIC_NUMBER 0xA5C3 // Magic number to validate EEPROM data

// EEPROM 수명 보호 강화
#define CONFIG_BATCH_UPDATE_DELAY_MS 2000 // 배치 업데이트 대기 시간
#define CONFIG_MAX_PENDING_CHANGES 5      // 최대 대기 변경사항 수

// ==================== Configuration Structure ====================

// 개별 센서 설정 정보 (ID 기반)
struct SensorConfig
{
  uint8_t sensor_id;                  // 센서 ID (1-8, 0=비활성)
  char sensor_address[17];            // 센서 주소 (16자리 + null terminator, 검증용)
  float th_value;                     // 상한 온도 임계값
  float tl_value;                     // 하한 온도 임계값
  unsigned long measurement_interval; // 측정 주기 (ms)
  uint8_t is_active;                  // 활성 상태 (0=비활성, 1=활성)
};

// 메인 설정 구조체 (ID 기반)
struct ConfigData
{
  uint16_t magic_number;                       // 유효성 검사용 매직 넘버
  uint8_t version;                             // 설정 스키마 버전
  uint8_t selected_sensor_id;                  // 현재 선택된 센서 ID (1-8, 0=없음)
  uint8_t sensor_count;                        // 저장된 센서 수
  SensorConfig sensors[MAX_SENSORS_IN_CONFIG]; // 센서별 설정 배열 (인덱스 0-7 = ID 1-8)
  uint32_t last_write_time;                    // 마지막 EEPROM 쓰기 시간 (수명 관리용)
  uint16_t write_count;                        // EEPROM 쓰기 횟수 (수명 관리용)
  uint8_t checksum;                            // 체크섬
};

// ==================== Function Declarations ====================
void initializeConfig();

// Temperature threshold functions (현재 선택된 센서)
float getThValue();
float getTlValue();
void setThValue(float value);
void setTlValue(float value);

// Measurement interval functions (현재 선택된 센서)
unsigned long getMeasurementInterval();
void setMeasurementInterval(unsigned long interval);

// 센서별 설정 관리 함수들 (ID 기반)
bool setSensorThreshold(uint8_t sensor_id, float th_value, float tl_value);
bool setSensorThresholdTH(uint8_t sensor_id, float th_value);
bool setSensorThresholdTL(uint8_t sensor_id, float tl_value);
bool setSensorMeasurementInterval(uint8_t sensor_id, unsigned long interval);
bool getSensorConfig(uint8_t sensor_id, SensorConfig &config);
bool addOrUpdateSensorConfig(uint8_t sensor_id, const String &sensor_address, const SensorConfig &config);
bool addOrUpdateSensorConfig(uint8_t sensor_id, const String &sensor_address, float th_value, float tl_value, unsigned long measurement_interval);

// 주소 기반 헬퍼 함수들 (호환성용)
bool getSensorConfigByAddress(const String &sensor_address, SensorConfig &config);
uint8_t findSensorIdByAddress(const String &sensor_address);
bool addOrUpdateSensorConfigByAddress(const String &sensor_address, const SensorConfig &config);
bool addOrUpdateSensorConfigByAddress(const String &sensor_address, float th_value, float tl_value, unsigned long measurement_interval);
bool setSensorThresholdsByAddress(const String &sensor_address, float th_value, float tl_value);
bool setSensorMeasurementIntervalByAddress(const String &sensor_address, unsigned long interval);

// 센서 선택 관리 함수들
bool setSelectedSensor(uint8_t sensor_id);
bool setSelectedSensorByAddress(const String &sensor_address);
uint8_t getSelectedSensorId();
String getSelectedSensorAddress();

// EEPROM 수명 보호 강화 함수들
bool shouldSkipWrite(const ConfigData &new_config);
void recordWriteOperation();
bool isBatchUpdateReady();
void markConfigForBatchUpdate();
uint32_t getTimeSinceLastWrite();
bool isConfigChangeSignificant(const ConfigData &old_config, const ConfigData &new_config);

// EEPROM persistence functions
bool loadConfigFromEEPROM();
bool saveConfigToEEPROM();
bool isEEPROMValid();
uint8_t calculateChecksum(const ConfigData &config);
void printConfigData(const ConfigData &config);

// Runtime-configurable debounce (ms) and EEPROM diagnostics
void setConfigSaveDebounceMs(unsigned long ms);
unsigned long getConfigSaveDebounceMs();
unsigned long getEffectiveMeasurementInterval();
uint32_t getEEPROMWriteCount();

// Debounced persistence helpers
void markConfigDirty();
void markConfigDirtyDebug();
void processConfigAutosave();
bool isConfigDirty();

// ==================== Global Configuration Variables ====================
extern float th_value;
extern float tl_value;
extern unsigned long measurement_interval;
extern ConfigData current_config;

#endif // CONFIG_MANAGER_H
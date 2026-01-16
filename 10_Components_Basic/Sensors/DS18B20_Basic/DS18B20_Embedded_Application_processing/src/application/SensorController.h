#pragma once
#include <Arduino.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <vector>
#include "../domain/ITemperatureSensor.h"
#include "../domain/SensorStatus.h"

constexpr int SENSOR_MAX_COUNT = 8;

// DS18B20 온도 범위 상수
constexpr float DS18B20_MIN_TEMP = -55.0f;
constexpr float DS18B20_MAX_TEMP = 125.0f;
constexpr float DEFAULT_UPPER_THRESHOLD = 30.0f;
constexpr float DEFAULT_LOWER_THRESHOLD = 20.0f;

// 측정 주기 관련 상수
constexpr unsigned long MIN_MEASUREMENT_INTERVAL = 10000;      // 10초 (밀리초)
constexpr unsigned long MAX_MEASUREMENT_INTERVAL = 2592000000; // 30일 (밀리초)
constexpr unsigned long DEFAULT_MEASUREMENT_INTERVAL = 15000;  // 15초 (밀리초)

// EEPROM 주소 할당
constexpr int EEPROM_BASE_ADDR = 0;
constexpr int EEPROM_SIZE_PER_SENSOR = 8; // float(4) + float(4) = 8 bytes
constexpr int EEPROM_INTERVAL_ADDR = 64;  // 측정 주기 저장 주소 (unsigned long 4 bytes)

struct SensorThresholds {
    float upperThreshold = DEFAULT_UPPER_THRESHOLD;  // TH (상한)
    float lowerThreshold = DEFAULT_LOWER_THRESHOLD;  // TL (하한)
    bool isCustomSet = false;                        // 사용자 설정 여부
};

struct SensorRowInfo
{
    int idx;
    int logicalId;
    DeviceAddress addr;
    float temp;
    bool connected;
};

class SensorController
{
public:
    SensorController();

    // 센서 논리 ID 관리
    uint8_t getSensorLogicalId(int idx);
    void setSensorLogicalId(int idx, uint8_t newId);
    bool isIdDuplicated(int newId, int exceptIdx = -1);
    void assignIDsByAddress();
    void resetAllSensorIds(); // 전체 ID 초기화
    
    // 센서 임계값 관리 (sensorIdx는 표시 행 번호 기반 0-7 인덱스)
    void initializeThresholds(); // EEPROM에서 임계값 로드
    float getUpperThreshold(int sensorIdx);
    float getLowerThreshold(int sensorIdx);
    void setThresholds(int sensorIdx, float upperTemp, float lowerTemp);
    bool isValidTemperature(float temp);
    void resetSensorThresholds(int sensorIdx); // 개별 센서 임계값 초기화
    void resetAllThresholds(); // 모든 센서 임계값 초기화
    
    // 측정 주기 관리
    void initializeMeasurementInterval(); // EEPROM에서 측정 주기 로드
    unsigned long getMeasurementInterval(); // 현재 측정 주기 조회
    void setMeasurementInterval(unsigned long intervalMs); // 측정 주기 설정
    bool isValidMeasurementInterval(unsigned long intervalMs); // 측정 주기 유효성 검증
    String formatInterval(unsigned long intervalMs); // 측정 주기를 읽기 쉬운 형태로 변환

    // 센서 상태 테이블 관리
    void printSensorStatusTable();
    void updateSensorRows();
    const SensorRowInfo *getSortedSensorRows() const { return g_sortedSensorRows; }

    // 임계값 관리 (기존 - 전역 임계값)
    const char *getUpperState(float temp);
    const char *getLowerState(float temp);
    const char *getSensorStatus(float temp);
    
    // 새로운 임계값 관리 (센서별 임계값, sensorIdx는 표시 행 번호 기반 0-7 인덱스)
    const char *getUpperState(int sensorIdx, float temp);
    const char *getLowerState(int sensorIdx, float temp);
    const char *getSensorStatus(int sensorIdx, float temp);

private:
    static SensorRowInfo g_sortedSensorRows[SENSOR_MAX_COUNT];
    SensorThresholds sensorThresholds[SENSOR_MAX_COUNT]; // 센서별 임계값 저장
    unsigned long measurementInterval; // 현재 측정 주기 (밀리초)
    
    void printSensorAddress(const DeviceAddress &addr);
    void printSensorRow(int idx, int id, const DeviceAddress &addr, float temp);
    
    // EEPROM 관련 private 메서드
    void loadSensorThresholds(int sensorIdx);
    void saveSensorThresholds(int sensorIdx);
    void saveSensorThresholds(int sensorIdx, bool verbose);
    int getEEPROMAddress(int sensorIdx);
    
    // 측정 주기 EEPROM 관련 메서드
    void loadMeasurementInterval();
    void saveMeasurementInterval();
    
    // Helper methods for updateSensorRows
    void collectSensorData(std::vector<SensorRowInfo>& sensorRows);
    SensorRowInfo createSensorRowInfo(int idx, int deviceCount);
    void sortSensorRows(std::vector<SensorRowInfo>& sensorRows);
    void storeSortedResults(const std::vector<SensorRowInfo>& sensorRows);
};

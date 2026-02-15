#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "Config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

extern float sensorTemps[MAX_SENSORS];
extern uint8_t foundSensors;
extern DeviceAddress sensorAddress[MAX_SENSORS];

/**
 * @brief 센서 시스템 초기화
 */
void initSensors();

/**
 * @brief 센서 데이터 비차단 업데이트 루프
 * 
 * requestTemperatures()를 호출한 뒤 실제 결과가 나올 때까지 기다리지 않고
 * 다음 루프에서 결과를 확인하여 업데이트합니다.
 */
void updateSensors();

/**
 * @brief 온도 데이터 즉시 읽기 (초기화 시 사용)
 */
void readTemperatures();

/**
 * @brief DS18B20 주소 출력 (디버깅용)
 */
void printAddress(DeviceAddress deviceAddress);

/**
 * @brief 새로운 센서 데이터가 준비되었는지 확인
 * @return true 준비됨, false 대기 중
 */
bool isNewDataAvailable();

#endif // SENSOR_MANAGER_H

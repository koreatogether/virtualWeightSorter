#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include "DS18B20_Sensor.h"
#include "Communicator.h"
#include "config.h"

// 센서 정보 구조체
struct SensorInfo
{
    int deviceId;
    DeviceAddress addr;
    float temp;
    uint8_t resolution; // 추가
    bool valid;
};

class DataProcessor
{
public:
    DataProcessor(DS18B20_Sensor *sensorManager, Communicator *comm);
    virtual void printSensorTable();
    virtual void printInvalidSensorTable();
    virtual void printAvailableIds();
    virtual bool hasInvalidSensors();

    // 소프트웨어 임계값 관리 함수
    virtual void setThreshold(int sensorId, int8_t thValue, int8_t tlValue);
    virtual int8_t getTh(int sensorId);
    virtual int8_t getTl(int sensorId);
    virtual void clearAllThresholds();

private:
    DS18B20_Sensor *_sensorManager;
    Communicator *_comm;
    bool _hasInvalidSensors;

    // 소프트웨어 임계값 저장을 위한 배열 (th: 상한, tl: 하한)
    int8_t th[SENSOR_COUNT + 1];
    int8_t tl[SENSOR_COUNT + 1];
};

#endif // DATA_PROCESSOR_H
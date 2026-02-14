#include "DataProcessor.h"
#include "Utils.h"

// PROGMEM에 저장할 상수 문자열들
const char PROGMEM TABLE_BORDER[] = "---------------------------------------------------------------------------------------------";
const char PROGMEM TABLE_HEADER[] = "|  No |   Device ID   |        ADDRESS      |  Temp(℃) |    th    | 초과온도 |    tl    | 초과온도 | 상태         |";
const char PROGMEM SENSOR_NONE_ROW[] = "| %3d |     NONE      |        NONE         |   NONE   |   NONE  |   NONE  |   NONE  |   NONE  | 센서없음     |";
const char PROGMEM SENSOR_DATA_ROW[] = "| %3d | %13d | %s | %8.2f | %8d | %7s | %8d | %7s | %-10s |";
const char PROGMEM STATUS_OK[] = "이상없음";
const char PROGMEM STATUS_CHECK[] = "점검 필요";
const char PROGMEM OVER_MARK[] = "초과";
const char PROGMEM PLUS_MARK[] = "+";
const char PROGMEM MINUS_MARK[] = "-";

const char PROGMEM INVALID_TABLE_BORDER[] = "-------------------------------";
const char PROGMEM INVALID_TABLE_HEADER[] = "|  Device ID  |    ADDRESS    |";
const char PROGMEM INVALID_SENSOR_ROW[] = "| %10d | %s |";

const char PROGMEM USED_IDS_MSG[] = "현재 사용 중인 ID: ";
const char PROGMEM AVAILABLE_IDS_MSG[] = "사용 가능한 ID: ";
const char PROGMEM SPACE_STR[] = " ";
#include <Arduino.h>

DataProcessor::DataProcessor(DS18B20_Sensor *sensorManager, Communicator *comm)
    : _sensorManager(sensorManager), _comm(comm), _hasInvalidSensors(false)
{
    // 임계값 배열 초기화
    clearAllThresholds();
}

void DataProcessor::printSensorTable()
{
    SensorInfo sensorsTable[SENSOR_COUNT];
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        sensorsTable[i].valid = false;
        sensorsTable[i].deviceId = -1;
    }

    int found = _sensorManager->getDeviceCount();
    _sensorManager->requestTemperatures();

    for (int i = 0; i < found && i < SENSOR_COUNT; i++)
    {
        DeviceAddress addr;
        if (!_sensorManager->getAddress(addr, i))
        {
            continue;
        }
        int deviceId = _sensorManager->getUserDataByIndex(i);
        float temp = _sensorManager->getTempC(addr);
        if (deviceId >= 1 && deviceId <= SENSOR_COUNT)
        {
            int slotIndex = deviceId - 1;
            memcpy(sensorsTable[slotIndex].addr, addr, 8);
            sensorsTable[slotIndex].deviceId = deviceId;
            sensorsTable[slotIndex].temp = temp;
            sensorsTable[slotIndex].valid = true;
        }
    }

    char buf[128];
    Serial.println((__FlashStringHelper *)TABLE_BORDER);
    Serial.println((__FlashStringHelper *)TABLE_HEADER);
    Serial.println((__FlashStringHelper *)TABLE_BORDER);

    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        if (!sensorsTable[i].valid)
        {
            snprintf_P(buf, sizeof(buf), SENSOR_NONE_ROW, i + 1);
            Serial.println(buf);
            continue;
        }
        char formattedAddr[32] = "";
        addrToString(sensorsTable[i].addr, formattedAddr);

        float temp = sensorsTable[i].temp;
        bool overTh = temp > th[i + 1];
        bool underTl = temp < tl[i + 1];
        const char *status = (overTh || underTl) ? STATUS_CHECK : STATUS_OK;
        snprintf_P(buf, sizeof(buf), SENSOR_DATA_ROW,
                   i + 1,
                   sensorsTable[i].deviceId,
                   formattedAddr,
                   temp,
                   th[i + 1],
                   overTh ? OVER_MARK : PLUS_MARK,
                   tl[i + 1],
                   underTl ? OVER_MARK : MINUS_MARK,
                   status);
        Serial.println(buf);
    }
    Serial.println((__FlashStringHelper *)TABLE_BORDER);
}

void DataProcessor::printInvalidSensorTable()
{
    struct InvalidSensorInfo
    {
        int deviceId;
        DeviceAddress addr;
    };
    InvalidSensorInfo invalidList[SENSOR_COUNT];
    int found = _sensorManager->getDeviceCount();
    int invalidCount = 0;
    for (int i = 0; i < found; i++)
    {
        DeviceAddress addr;
        if (!_sensorManager->getAddress(addr, i))
            continue;
        int deviceId = _sensorManager->getUserDataByIndex(i);
        if (deviceId < 1 || deviceId > SENSOR_COUNT)
        {
            memcpy(invalidList[invalidCount].addr, addr, 8);
            invalidList[invalidCount].deviceId = deviceId;
            invalidCount++;
        }
    }

    for (int i = 0; i < invalidCount - 1; i++)
    {
        for (int j = i + 1; j < invalidCount; j++)
        {
            if (memcmp(invalidList[i].addr, invalidList[j].addr, 8) > 0)
            {
                InvalidSensorInfo tmp = invalidList[i];
                invalidList[i] = invalidList[j];
                invalidList[j] = tmp;
            }
        }
    }

    char buf[64];
    Serial.println((__FlashStringHelper *)INVALID_TABLE_BORDER);
    Serial.println((__FlashStringHelper *)INVALID_TABLE_HEADER);
    Serial.println((__FlashStringHelper *)INVALID_TABLE_BORDER);
    for (int i = 0; i < invalidCount; i++)
    {
        char addrStr[24] = "";
        addrToString(invalidList[i].addr, addrStr);
        snprintf_P(buf, sizeof(buf), INVALID_SENSOR_ROW, invalidList[i].deviceId, addrStr);
        Serial.println(buf);
    }
    Serial.println((__FlashStringHelper *)INVALID_TABLE_BORDER);
}

void DataProcessor::printAvailableIds()
{
    bool usedIds[SENSOR_COUNT + 1] = {false};
    int found = _sensorManager->getDeviceCount();
    for (int i = 0; i < found; i++)
    {
        DeviceAddress addr;
        _sensorManager->getAddress(addr, i);
        int id = _sensorManager->getUserData(addr);
        if (id >= 1 && id <= SENSOR_COUNT)
        {
            usedIds[id] = true;
        }
    }

    Serial.print((__FlashStringHelper *)USED_IDS_MSG);
    for (int i = 1; i <= SENSOR_COUNT; i++)
    {
        if (usedIds[i])
        {
            Serial.print(i);
            Serial.print((__FlashStringHelper *)SPACE_STR);
        }
    }
    Serial.println();
    Serial.print((__FlashStringHelper *)AVAILABLE_IDS_MSG);
    for (int i = 1; i <= SENSOR_COUNT; i++)
    {
        if (!usedIds[i])
        {
            Serial.print(i);
            Serial.print((__FlashStringHelper *)SPACE_STR);
        }
    }
    Serial.println();
}

bool DataProcessor::hasInvalidSensors()
{
    int found = _sensorManager->getDeviceCount();
    for (int i = 0; i < found; i++)
    {
        DeviceAddress addr;
        if (!_sensorManager->getAddress(addr, i))
            continue;
        int deviceId = _sensorManager->getUserDataByIndex(i);
        if (deviceId < 1 || deviceId > SENSOR_COUNT)
        {
            return true;
        }
    }
    return false;
}

void DataProcessor::setThreshold(int sensorId, int8_t thValue, int8_t tlValue)
{
    if (sensorId > 0 && sensorId <= SENSOR_COUNT)
    {
        th[sensorId] = thValue;
        tl[sensorId] = tlValue;
    }
}

int8_t DataProcessor::getTh(int sensorId)
{
    if (sensorId > 0 && sensorId <= SENSOR_COUNT)
    {
        return th[sensorId];
    }
    return 125; // 기본값
}

int8_t DataProcessor::getTl(int sensorId)
{
    if (sensorId > 0 && sensorId <= SENSOR_COUNT)
    {
        return tl[sensorId];
    }
    return -55; // 기본값
}

void DataProcessor::clearAllThresholds()
{
    for (int i = 0; i <= SENSOR_COUNT; i++)
    {
        th[i] = 125; // DS18B20 기본 상한값
        tl[i] = -55; // DS18B20 기본 하한값
    }
}

/**
 * @file SensorManager.cpp
 * @brief DS18B20 온도 센서 관리 및 데이터 수집 구현
 */

#include "SensorManager.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress sensorAddress[MAX_SENSORS];
uint8_t foundSensors = 0;
float sensorTemps[MAX_SENSORS];

/**
 * @brief 센서 주소 시리얼 출력 (디버깅용)
 */
void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16)
        {
            Serial.print(F("0"));
        }
        Serial.print(deviceAddress[i], HEX);
    }
}

/**
 * @brief 1-Wire 버스 스캔 및 센서 초기화
 */
void initSensors()
{
    sensors.begin();
    uint8_t count = sensors.getDeviceCount();

    if (count > MAX_SENSORS)
    {
        count = MAX_SENSORS;
    }

    uint8_t stored = 0;
    for (uint8_t i = 0; i < sensors.getDeviceCount() && stored < MAX_SENSORS; i++)
    {
        if (sensors.getAddress(sensorAddress[stored], i))
        {
            Serial.print(F("Sensor #"));
            Serial.print(stored);
            Serial.print(F(" addr: "));
            printAddress(sensorAddress[stored]);
            Serial.println();
            stored++;
        }
        else
        {
            Serial.print(F("Failed to read address for device index "));
            Serial.println(i);
        }
    }

    foundSensors = stored;
    for (uint8_t i = 0; i < foundSensors; i++)
    {
        sensorTemps[i] = DEVICE_DISCONNECTED_C;
    }

    Serial.print(F("Found sensors: "));
    Serial.println(foundSensors);

    if (foundSensors == 0)
    {
        Serial.println(F("Please check sensor connections!"));
    }
}

/**
 * @brief 모든 등록된 센서로부터 온도 읽기
 */
void readTemperatures()
{
    sensors.requestTemperatures();

    for (uint8_t i = 0; i < foundSensors; i++)
    {
        float tempC = sensors.getTempC(sensorAddress[i]);
        sensorTemps[i] = tempC;

        if (tempC == DEVICE_DISCONNECTED_C)
        {
            Serial.print(F("Sensor "));
            Serial.print(i);
            Serial.println(F(" DISCONNECTED"));
        }
        else
        {
            Serial.print(F("Sensor "));
            Serial.print(i);
            Serial.print(F(": "));
            Serial.print(tempC, 1);
            Serial.println(F(" C"));
        }
    }
}

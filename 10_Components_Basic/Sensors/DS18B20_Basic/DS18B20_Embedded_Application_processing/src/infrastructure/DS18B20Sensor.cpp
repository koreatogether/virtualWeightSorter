#include "DS18B20Sensor.h"

DS18B20Sensor::DS18B20Sensor(uint8_t pin) : _pin(pin) {}

void DS18B20Sensor::begin()
{
    // 센서 초기화 코드
}

float DS18B20Sensor::readTemperature()
{
    // 온도 읽기 코드
    return 0.0f;
}

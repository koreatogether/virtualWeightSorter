#include "DS18B20_Sensor.h"

DS18B20_Sensor::DS18B20_Sensor(OneWire *oneWire) : _sensors(oneWire) {}

void DS18B20_Sensor::begin()
{
    _sensors.begin();
}

int DS18B20_Sensor::getDeviceCount()
{
    return _sensors.getDeviceCount();
}

bool DS18B20_Sensor::getAddress(DeviceAddress address, uint8_t index)
{
    return _sensors.getAddress(address, index);
}

void DS18B20_Sensor::requestTemperatures()
{
    _sensors.requestTemperatures();
}

float DS18B20_Sensor::getTempC(DeviceAddress address)
{
    return _sensors.getTempC(address);
}

float DS18B20_Sensor::getTempCByIndex(uint8_t index)
{
    DeviceAddress address;
    if (getAddress(address, index))
    {
        return getTempC(address);
    }
    return -127.0f; // 연결 안된 센서의 기본값
}

int DS18B20_Sensor::getUserData(DeviceAddress address)
{
    // DallasTemperature 라이브러리의 getUserData 함수 사용
    int16_t userData = _sensors.getUserData(address);

    // 유효한 범위 확인 (1~SENSOR_COUNT)
    if (userData >= 1 && userData <= SENSOR_COUNT)
    {
        return userData;
    }

    // 유효하지 않은 값이면 0 반환 (미설정 상태)
    return 0;
}

void DS18B20_Sensor::setUserData(DeviceAddress address, int userData)
{
    // 유효한 범위 확인
    if (userData >= 0 && userData <= SENSOR_COUNT)
    {
        // DallasTemperature 라이브러리의 setUserData 함수 사용
        _sensors.setUserData(address, userData);
        delay(100); // 설정 완료 대기

        // 설정 후 센서 재초기화 방지를 위해 온도 요청
        _sensors.requestTemperatures();
        delay(100);
    }
}

int DS18B20_Sensor::getUserDataByIndex(uint8_t index)
{
    DeviceAddress addr;
    if (getAddress(addr, index))
    {
        return getUserData(addr);
    }
    return -1;
}

bool DS18B20_Sensor::isIdUsed(int id)
{
    if (id < 1 || id > SENSOR_COUNT)
        return true;

    int found = getDeviceCount();
    for (int i = 0; i < found; i++)
    {
        DeviceAddress addr;
        if (getAddress(addr, i))
        {
            if (getUserData(addr) == id)
            {
                return true;
            }
        }
    }
    return false;
}

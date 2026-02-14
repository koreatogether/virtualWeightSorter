#ifndef DS18B20_SENSOR_H
#define DS18B20_SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#ifdef UNIT_TEST
#include "FakeArduino.h" // For DeviceAddress
#else
#include <Arduino.h> // For DeviceAddress
#endif

class DS18B20_Sensor
{
public:
    DS18B20_Sensor(OneWire *oneWire);
    virtual void begin();
    virtual int getDeviceCount();
    virtual bool getAddress(DeviceAddress address, uint8_t index);
    virtual void requestTemperatures();
    virtual float getTempC(DeviceAddress address);
    virtual float getTempCByIndex(uint8_t index);
    virtual int getUserData(DeviceAddress address);
    virtual void setUserData(DeviceAddress address, int userData);
    virtual int getUserDataByIndex(uint8_t index);
    virtual bool isIdUsed(int id);
    virtual uint8_t getResolution(DeviceAddress address);
    virtual void setResolution(DeviceAddress address, uint8_t resolution);
    virtual void setResolution(uint8_t resolution); // Set for all devices

private:
    DallasTemperature _sensors;
};

#endif // DS18B20_SENSOR_H
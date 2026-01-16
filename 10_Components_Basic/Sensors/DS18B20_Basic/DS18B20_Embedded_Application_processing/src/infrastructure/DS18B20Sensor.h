#pragma once
#include <Arduino.h>
#include "../domain/ITemperatureSensor.h"

class DS18B20Sensor : public ITemperatureSensor
{
public:
    DS18B20Sensor(uint8_t pin);
    void begin() override;
    float readTemperature() override;

private:
    uint8_t _pin;
};

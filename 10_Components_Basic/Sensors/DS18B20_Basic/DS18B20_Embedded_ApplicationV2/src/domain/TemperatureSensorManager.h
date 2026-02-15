#pragma once
#include "ITemperatureSensor.h"

class TemperatureSensorManager
{
public:
    TemperatureSensorManager(ITemperatureSensor *sensor);
    void update();

private:
    ITemperatureSensor *_sensor;
};

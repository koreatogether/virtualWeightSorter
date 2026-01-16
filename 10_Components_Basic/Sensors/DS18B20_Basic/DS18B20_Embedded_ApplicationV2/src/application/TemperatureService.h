#pragma once
#include "../domain/TemperatureSensorManager.h"

class TemperatureService
{
public:
    TemperatureService(TemperatureSensorManager *manager);
    void update();
    void printStatus();

private:
    TemperatureSensorManager *_manager;
};

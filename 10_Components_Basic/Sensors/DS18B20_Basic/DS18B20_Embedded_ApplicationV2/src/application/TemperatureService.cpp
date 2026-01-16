#include "TemperatureService.h"
#include <Arduino.h>

TemperatureService::TemperatureService(TemperatureSensorManager *manager) : _manager(manager) {}

void TemperatureService::update()
{
    if (_manager)
    {
        _manager->update();
    }
}

void TemperatureService::printStatus()
{
    Serial.println("[TemperatureService] Status print stub");
}

#include "TemperatureSensorManager.h"

TemperatureSensorManager::TemperatureSensorManager(ITemperatureSensor *sensor) : _sensor(sensor) {}

void TemperatureSensorManager::update()
{
    if (_sensor)
    {
        _sensor->readTemperature();
    }
}

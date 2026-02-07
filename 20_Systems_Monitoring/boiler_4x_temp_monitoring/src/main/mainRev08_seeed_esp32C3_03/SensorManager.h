#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "Config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

extern float sensorTemps[MAX_SENSORS];
extern uint8_t foundSensors;
extern DeviceAddress sensorAddress[MAX_SENSORS];

void initSensors();
void readTemperatures();
void printAddress(DeviceAddress deviceAddress);

#endif // SENSOR_MANAGER_H

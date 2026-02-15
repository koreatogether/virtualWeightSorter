#include "SensorManager.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress sensorAddress[MAX_SENSORS];
uint8_t foundSensors = 0;
float sensorTemps[MAX_SENSORS];

void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16)
            Serial.print(F("0"));
        Serial.print(deviceAddress[i], HEX);
    }
}

void initSensors()
{
    sensors.begin();
    delay(150); // Sensor stabilization
    uint8_t count = sensors.getDeviceCount();
    if (count > MAX_SENSORS)
        count = MAX_SENSORS;

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
    }
    foundSensors = stored;
    for (uint8_t i = 0; i < MAX_SENSORS; i++)
    {
        sensorTemps[i] = (i < foundSensors) ? 0.0 : DEVICE_DISCONNECTED_C;
    }
}

void readTemperatures()
{
    sensors.requestTemperatures();
    for (uint8_t i = 0; i < foundSensors; i++)
    {
        float tempC = sensors.getTempC(sensorAddress[i]);
        sensorTemps[i] = tempC;
    }
}

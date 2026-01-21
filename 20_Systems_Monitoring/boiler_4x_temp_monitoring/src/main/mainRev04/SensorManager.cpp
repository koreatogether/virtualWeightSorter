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
        {
            Serial.print("0");
        }
        Serial.print(deviceAddress[i], HEX);
    }
}

void initSensors()
{
    sensors.begin();
    uint8_t count = sensors.getDeviceCount();
    if (count > MAX_SENSORS)
    {
        count = MAX_SENSORS;
    }

    uint8_t stored = 0;
    for (uint8_t i = 0; i < sensors.getDeviceCount() && stored < MAX_SENSORS; i++)
    {
        if (sensors.getAddress(sensorAddress[stored], i))
        {
            Serial.print("Sensor #");
            Serial.print(stored);
            Serial.print(" addr: ");
            printAddress(sensorAddress[stored]);
            Serial.println();
            stored++;
        }
        else
        {
            Serial.print("Failed to read address for device index ");
            Serial.println(i);
        }
    }

    foundSensors = stored;
    for (uint8_t i = 0; i < foundSensors; i++)
    {
        sensorTemps[i] = DEVICE_DISCONNECTED_C;
    }

    Serial.print("Found sensors: ");
    Serial.println(foundSensors);
    if (foundSensors == 0)
    {
        Serial.println("add sensor please");
    }
}

void readTemperatures()
{
    sensors.requestTemperatures();
    for (uint8_t i = 0; i < foundSensors; i++)
    {
        float tempC = sensors.getTempC(sensorAddress[i]);
        sensorTemps[i] = tempC;
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.println(" 연결 끊김");
        }
        else
        {
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(tempC, 1);
            Serial.println(" C");
        }
    }
}

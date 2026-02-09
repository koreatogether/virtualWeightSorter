/*

*/

#include <OneWire.h>
#include <DallasTemperature.h>

const int DS18B20_DATA_PIN = 2;

oneWire oneWire(DS18B20_DATA_PIN);
DallaTemperature tempSensor(&oneWire);

void setup()
{
    Serial.begin(9600);
    Serial.println("DS18B20 Example");

    tempSensor.begin();
}

void loop()
{
    Serial.println("Requesting temperatures...");
    tempSensor.requestTemperatures();
    Serial.println("DONE");

    flat tempC = tempSensor.getTempCByIndex(0);

    if (tempC != DEVICE_DISCONNECTED_C)
    {
        Serial.print("Temperature for the device 1 (index 0) is: ");
        Serial.println(tempC);
    }
    else
    {
        Serial.println("Error: Could not read temperature data");
    }
}

/*
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
    Serial.begin(9600);
    sensors.begin();
}

void loop() {
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");
    delay(1000);
}
    */
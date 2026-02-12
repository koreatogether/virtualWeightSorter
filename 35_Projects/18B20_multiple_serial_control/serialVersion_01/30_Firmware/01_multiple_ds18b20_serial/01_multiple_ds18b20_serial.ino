/*
예제: 여러 DS18B20 센서를 읽어 시리얼로 출력
핀: DATA -> D2 (ONE_WIRE_BUS)
라이브러리: OneWire, DallasTemperature
*/

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup()
{
    Serial.begin(9600);
    sensors.begin();
    Serial.println("DS18B20 multiple sensor example");
}

void loop()
{
    sensors.requestTemperatures();
    int count = sensors.getDeviceCount();
    Serial.print("Found ");
    Serial.print(count);
    Serial.println(" sensor(s)");
    for (int i = 0; i < count; i++)
    {
        DeviceAddress addr;
        if (sensors.getAddress(addr, i))
        {
            float t = sensors.getTempC(addr);
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.print(" : ");
            Serial.print(t);
            Serial.println(" C");
        }
    }
    Serial.println("---");
    delay(2000);
}

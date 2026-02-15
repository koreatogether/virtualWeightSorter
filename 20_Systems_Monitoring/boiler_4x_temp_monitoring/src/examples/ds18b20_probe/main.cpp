/*
  ds18b20_probe/main.cpp
  DS18B20 센서 탐색 및 온도 측정 테스트 예제
  - Arduino R4 WiFi 보드용
  - 최대 4개의 DS18B20 센서 지원
  - 시리얼 모니터로 결과 출력

  Wiring:
   - DS18B20 DATA -> D2
   - DS18B20 VCC -> 5V
   - DS18B20 GND -> GND
   - DATA와 VCC 사이에 4.7K 풀업 저항 필수
*/

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// 상수
const uint8_t ONE_WIRE_BUS = 2;                // DS18B20 데이터 핀
const uint8_t MAX_SENSORS = 4;                 // 최대 센서 수
const unsigned long SAMPLE_INTERVAL_MS = 2000; // 측정 주기

// 전역 객체 및 상태
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorAddress[MAX_SENSORS];
uint8_t foundSensors = 0;

unsigned long lastSampleMs = 0;

// 헬퍼: 주소 출력
void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16)
            Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("DS18B20 Probe Test Start");

    sensors.begin();
    foundSensors = sensors.getDeviceCount();

    if (foundSensors > MAX_SENSORS)
        foundSensors = MAX_SENSORS;

    Serial.print("Found ");
    Serial.print(foundSensors);
    Serial.println(" sensors.");

    for (uint8_t i = 0; i < foundSensors; i++)
    {
        if (sensors.getAddress(sensorAddress[i], i))
        {
            Serial.print("Sensor #");
            Serial.print(i);
            Serial.print(" Address: ");
            printAddress(sensorAddress[i]);
            Serial.println();
        }
    }
}

void loop()
{
    unsigned long now = millis();
    if (now - lastSampleMs >= SAMPLE_INTERVAL_MS)
    {
        lastSampleMs = now;

        if (foundSensors == 0)
        {
            Serial.println("No sensors found! Check wiring.");
            return;
        }

        sensors.requestTemperatures();
        for (uint8_t i = 0; i < foundSensors; i++)
        {
            float tempC = sensors.getTempC(sensorAddress[i]);
            Serial.print("Sensor #");
            Serial.print(i);
            Serial.print(": ");
            if (tempC == DEVICE_DISCONNECTED_C)
                Serial.println("Disconnected!");
            else
            {
                Serial.print(tempC, 2);
                Serial.println(" C");
            }
        }
        Serial.println("--------------------");
    }
}

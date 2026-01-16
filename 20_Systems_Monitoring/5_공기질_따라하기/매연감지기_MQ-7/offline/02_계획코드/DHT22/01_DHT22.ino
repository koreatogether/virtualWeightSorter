/*
adafruit DHT22 library
*/

#include "DHT.h"

const int DHTPIN = 2;      // DHT22 센서의 데이터 핀 (아두이노 2번 핀)
const int DHTTYPE = DHT22; // DHT 센서 타입 (DHT22)
DHT dht(DHTPIN, DHTTYPE);

float humidity, temperature;
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000;

void setup()
{
    Serial.begin(9600);
    Serial.println("DHT22 Test!");
    dht.begin();
}

void readSensorData()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    else
    {
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C");
    }
}

void loop()
{
    if (millis() - lastReadTime >= readInterval)
    {
        lastReadTime = millis();
        readSensorData();
    }
}
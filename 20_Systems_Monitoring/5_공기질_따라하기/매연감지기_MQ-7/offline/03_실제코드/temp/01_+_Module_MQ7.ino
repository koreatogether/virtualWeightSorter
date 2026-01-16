#include <math.h>

const int analogPin = A0;
const float RL = 10.0;
const float R0 = 10.0;

unsigned long warmupStart;
const unsigned long warmupDuration = 20000;
bool isWarmedUp = false;

void setup()
{
    Serial.begin(9600);
    warmupStart = millis();
}

void loop()
{
    if (!isWarmedUp)
    {
        if (millis() - warmupStart >= warmupDuration)
        {
            isWarmedUp = true;
            Serial.println("Sensor warmed up.");
        }
        return;
    }
    measureCO();
    delay(1000);
}

void measureCO()
{
    int analogValue = analogRead(analogPin);
    float voltage = (analogValue * 5.0) / 1023.0;

    if (voltage <= 0)
    {
        Serial.println("Error: Voltage reading is zero or invalid.");
        return;
    }

    float Rs = ((5.0 * RL) / voltage) - RL;
    float ratio = Rs / R0;
    float ppm = calculateCO(ratio);

    Serial.print("Analog Value: ");
    Serial.print(analogValue);
    Serial.print(", Voltage: ");
    Serial.print(voltage, 2);
    Serial.print(" V, Rs: ");
    Serial.print(Rs, 2);
    Serial.print(" kΩ, Rs/R0: ");
    Serial.print(ratio, 2);
    Serial.print(", CO Concentration: ");
    Serial.print(ppm, 2);
    Serial.println(" ppm");
}

float calculateCO(float ratio)
{
    return pow(10, (2.8 - log10(ratio)));
}

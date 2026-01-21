#include "DisplayManager.h"
#include "SensorManager.h"

LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);

void initDisplay()
{
    lcd.begin(20, 4);
    lcd.setBacklight(255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Boiler Temp Monitor");
}

void showSensorShortAddresses(unsigned long durationMs)
{
    if (foundSensors == 0)
    {
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor addrs (last4):");

    for (uint8_t i = 0; i < foundSensors && i < MAX_SENSORS; i++)
    {
        char buf[5] = {0};
        uint8_t b1 = sensorAddress[i][6];
        uint8_t b2 = sensorAddress[i][7];
        sprintf(buf, "%02X%02X", b1, b2);

        lcd.setCursor(0, 1 + i);
        lcd.print("S");
        lcd.print(i + 1);
        lcd.print(": ");
        lcd.print(buf);
    }

    unsigned long startMs = millis();
    while (millis() - startMs < durationMs)
    {
        delay(50);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Boiler Temp Monitor");
}

// 개별 온도 표시 모드
void updateDisplayIndividual()
{
    for (uint8_t i = 0; i < MAX_SENSORS; i++)
    {
        lcd.setCursor(0, i);
        if (i < foundSensors)
        {
            if (sensorTemps[i] == DEVICE_DISCONNECTED_C)
            {
                lcd.print("S");
                lcd.print(i + 1);
                lcd.print(": err           ");
            }
            else
            {
                lcd.print("S");
                lcd.print(i + 1);
                lcd.print(": ");
                lcd.print(sensorTemps[i], 1);
                lcd.print(" C          ");
            }
        }
        else
        {
            lcd.print("S");
            lcd.print(i + 1);
            lcd.print(": -             ");
        }
    }
}

// 온도차이 표시 모드
void updateDisplayDiff()
{
    // S1~S2 센서 차이
    lcd.setCursor(0, 0);
    if (foundSensors >= 2 && sensorTemps[0] != DEVICE_DISCONNECTED_C && sensorTemps[1] != DEVICE_DISCONNECTED_C)
    {
        lcd.print("S1: ");
        lcd.print(sensorTemps[0], 1);
        lcd.print("C ");
        lcd.setCursor(10, 0);
        lcd.print("S2: ");
        lcd.print(sensorTemps[1], 1);
        lcd.print("C");

        lcd.setCursor(0, 1);
        lcd.print("S1-S2 : ");
        lcd.print(sensorTemps[0] - sensorTemps[1], 1);
        lcd.print(" C      ");
    }
    else
    {
        lcd.print("S1-S2: Check Sensors ");
    }

    // S3~S4 센서 차이
    lcd.setCursor(0, 2);
    if (foundSensors >= 4 && sensorTemps[2] != DEVICE_DISCONNECTED_C && sensorTemps[3] != DEVICE_DISCONNECTED_C)
    {
        lcd.print("S3: ");
        lcd.print(sensorTemps[2], 1);
        lcd.print("C ");
        lcd.setCursor(10, 2);
        lcd.print("S4: ");
        lcd.print(sensorTemps[3], 1);
        lcd.print("C");

        lcd.setCursor(0, 3);
        lcd.print("S3-S4 : ");
        lcd.print(sensorTemps[2] - sensorTemps[3], 1);
        lcd.print(" C      ");
    }
    else if (foundSensors >= 2)
    {
        lcd.print("S3-S4: Need 4 Sens  ");
    }
    else
    {
        lcd.print("S3-S4: Check Sensors ");
    }
}

void updateDisplay(uint8_t mode)
{
    if (mode == 0)
    {
        updateDisplayDiff();
    }
    else
    {
        updateDisplayIndividual();
    }
}

/**
 * @file DisplayManager.cpp
 * @brief LCD 시각화 및 페이지 관리 구현
 */

#include "DisplayManager.h"
#include "SensorManager.h"

LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);

/**
 * @brief LCD 초기 설정
 */
void initDisplay()
{
    lcd.begin(20, 4);
    lcd.setBacklight(255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Boiler Temp Monitor"));
}

/**
 * @brief 센서의 고유 ID(마지막 4자리)를 LCD에 표시
 */
void showSensorShortAddresses(unsigned long durationMs)
{
    if (foundSensors == 0)
    {
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Sensor addrs(last4):"));

    for (uint8_t i = 0; i < foundSensors && i < MAX_SENSORS; i++)
    {
        char buf[5] = {0};
        uint8_t b1 = sensorAddress[i][6];
        uint8_t b2 = sensorAddress[i][7];
        sprintf(buf, "%02X%02X", b1, b2);

        lcd.setCursor(0, 1 + i);
        lcd.print(F("S"));
        lcd.print(i + 1);
        lcd.print(F(": "));
        lcd.print(buf);
    }

    unsigned long startMs = millis();
    while (millis() - startMs < durationMs)
    {
        delay(50);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Boiler Temp Monitor"));
}

/**
 * @brief 개별 온도 표시 모드 (페이지 2)
 */
static void updateDisplayIndividual()
{
    for (uint8_t i = 0; i < MAX_SENSORS; i++)
    {
        lcd.setCursor(0, i);
        if (i < foundSensors)
        {
            if (sensorTemps[i] == DEVICE_DISCONNECTED_C)
            {
                lcd.print(F("S"));
                lcd.print(i + 1);
                lcd.print(F(": err           "));
            }
            else
            {
                lcd.print(F("S"));
                lcd.print(i + 1);
                lcd.print(F(": "));
                lcd.print(sensorTemps[i], 1);
                lcd.print(F(" C          "));
            }
        }
        else
        {
            lcd.print(F("S"));
            lcd.print(i + 1);
            lcd.print(F(": -             "));
        }
    }
}

/**
 * @brief 온도차이 표시 모드 (페이지 1)
 */
static void updateDisplayDiff()
{
    // S1~S2 센서 차이
    lcd.setCursor(0, 0);
    if (foundSensors >= 2 && sensorTemps[0] != DEVICE_DISCONNECTED_C && sensorTemps[1] != DEVICE_DISCONNECTED_C)
    {
        lcd.print(F("S1: "));
        lcd.print(sensorTemps[0], 1);
        lcd.print(F("C "));
        lcd.setCursor(10, 0);
        lcd.print(F("S2: "));
        lcd.print(sensorTemps[1], 1);
        lcd.print(F("C"));

        lcd.setCursor(0, 1);
        lcd.print(F("S1-S2 : "));
        lcd.print(sensorTemps[0] - sensorTemps[1], 1);
        lcd.print(F(" C      "));
    }
    else
    {
        lcd.print(F("S1-S2: Check Sensors "));
    }

    // S3~S4 센서 차이
    lcd.setCursor(0, 2);
    if (foundSensors >= 4 && sensorTemps[2] != DEVICE_DISCONNECTED_C && sensorTemps[3] != DEVICE_DISCONNECTED_C)
    {
        lcd.print(F("S3: "));
        lcd.print(sensorTemps[2], 1);
        lcd.print(F("C "));
        lcd.setCursor(10, 2);
        lcd.print(F("S4: "));
        lcd.print(sensorTemps[3], 1);
        lcd.print(F("C"));

        lcd.setCursor(0, 3);
        lcd.print(F("S3-S4 : "));
        lcd.print(sensorTemps[2] - sensorTemps[3], 1);
        lcd.print(F(" C      "));
    }
    else if (foundSensors >= 2)
    {
        lcd.print(F("S3-S4: Need 4 Sens  "));
    }
    else
    {
        lcd.print(F("S3-S4: Check Sensors "));
    }
}

/**
 * @brief 빈 페이지 표시 (페이지 3, 4)
 */
static void updateDisplayEmpty(uint8_t pageNum)
{
    lcd.setCursor(5, 1);
    lcd.print(F("Empty Page "));
    lcd.print(pageNum);
    lcd.setCursor(4, 2);
    lcd.print(F("(Coming Soon)"));
}

/**
 * @brief 전체 화면 업데이트 (공통 요소 포함)
 */
void updateDisplay(uint8_t mode)
{
    if (mode == 0)
    {
        updateDisplayDiff();
    }
    else if (mode == 1)
    {
        updateDisplayIndividual();
    }
    else
    {
        updateDisplayEmpty(mode + 1);
    }

    // 로깅 상태 표시 (우측 하단)
    extern unsigned long lastHeartbeatMs;
    lcd.setCursor(15, 3);
    if (millis() - lastHeartbeatMs < 15000)
    {
        lcd.print(F("[LOG]"));
    }
    else
    {
        lcd.print(F("     "));
    }
}

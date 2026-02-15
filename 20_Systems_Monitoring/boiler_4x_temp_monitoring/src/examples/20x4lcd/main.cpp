/*
  20x4lcd/main.cpp
  20x4 I2C LCD (PCF8574) 테스트 예제
  - Arduino R4 WiFi 보드용
  - LCD I2C 주소: 0x27 (또는 0x3F)

  Wiring:
   - LCD SDA -> A4 (또는 전용 SDA)
   - LCD SCL -> A5 (또는 전용 SCL)
   - LCD VCC -> 5V
   - LCD GND -> GND
*/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// 상수
const uint8_t LCD_I2C_ADDRESS = 0x27;
const unsigned long REFRESH_INTERVAL_MS = 1000;

// 전역 객체
LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);

unsigned long lastRefreshMs = 0;
uint32_t counter = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("LCD 20x4 Test Start");

    lcd.begin(20, 4);
    lcd.setBacklight(255);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("LCD 20x4 Test");
    lcd.setCursor(0, 1);
    lcd.print("I2C Addr: 0x27");
    lcd.setCursor(0, 2);
    lcd.print("Status: Running");
}

void loop()
{
    unsigned long now = millis();
    if (now - lastRefreshMs >= REFRESH_INTERVAL_MS)
    {
        lastRefreshMs = now;
        counter++;

        lcd.setCursor(0, 3);
        lcd.print("Counter: ");
        lcd.print(counter);

        Serial.print("Update LCD, counter: ");
        Serial.println(counter);
    }
}

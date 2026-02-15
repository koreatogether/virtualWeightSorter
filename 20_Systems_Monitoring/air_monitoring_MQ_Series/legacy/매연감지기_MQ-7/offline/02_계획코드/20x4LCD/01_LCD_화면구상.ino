/*
AI에게 받은 추천 LCD 화면 구성
CO:123ppm  T:25.5°C
Vh:5.0V    Vout:2.3V
R0:10.2k   Rs:5.6k
Mode: Heating   t:45s
*/

// 대충 자리 잡은 WOKWI 시뮬레이션 링크
// https://wokwi.com/projects/419933492961796097

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 20 chars and 4 line display

void setup()
{
  lcd.begin(20, 4);      // initialize the lcd for 20 chars 4 lines
  lcd.setBacklight(255); // turn on the backlight
}

void loop()
{
  // Example values, replace these with actual sensor readings
  int co_ppm = 123;
  float temperature = 25.5;
  float vh = 5.0;
  float vout = 2.3;
  float r0 = 10.2;
  float rs = 5.6;
  String mode = "Heating";
  int time_remaining = 45;

  // Clear the display
  lcd.clear();

  // First row
  lcd.setCursor(0, 0);
  lcd.print("CO:");
  lcd.print(co_ppm);
  lcd.print("ppm  T:");
  lcd.print(temperature);
  lcd.print((char)223); // Degree symbol
  lcd.print("C");

  // Second row
  lcd.setCursor(0, 1);
  lcd.print("Vh:");
  lcd.print(vh);
  lcd.print("V    Vout:");
  lcd.print(vout);
  lcd.print("V");

  // Third row
  lcd.setCursor(0, 2);
  lcd.print("R0:");
  lcd.print(r0);
  lcd.print("k   Rs:");
  lcd.print(rs);
  lcd.print("k");

  // Fourth row
  lcd.setCursor(0, 3);
  lcd.print("Mode: ");
  lcd.print(mode);
  lcd.print("   t:");
  lcd.print(time_remaining);
  lcd.print("s");

  // Update the display every second
  delay(1000);
}

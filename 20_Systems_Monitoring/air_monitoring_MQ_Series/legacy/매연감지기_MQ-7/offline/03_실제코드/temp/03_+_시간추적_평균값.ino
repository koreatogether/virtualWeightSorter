#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

#include "MQ7.h"

#define A_PIN A0
#define VOLTAGE 5

// init MQ7 device
MQ7 mq7(A_PIN, VOLTAGE);

LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 20 chars and 4 line display

void setup_MQ7()
{
  Serial.println(""); // blank new line

  Serial.println("Calibrating MQ7");
  mq7.calibrate(); // calculates R0
  Serial.println("Calibration done!");
}

void setup_LCD()
{
  lcd.begin(20, 4);      // initialize the lcd for 20 chars 4 lines
  lcd.setBacklight(255); // turn on the backlight
}

void loop_LCD()
{
  // Clear the LCD
  lcd.clear();

  // First row
  lcd.setCursor(0, 0);
  lcd.print("CO:");
  lcd.print(mq7.readPpm());

  // first row  , 30초마다 30초간 측정된 값의 평균을 출력하는 코드
  lcd.setCursor(9, 0);
  lcd.print("Avg.PPM:");
  

  // Second row , mills()시작부터 경과된 시간을 출력하는 코드,형식 시간 , 분 , 초
  lcd.setCursor(0, 1);
  lcd.print("Time:");
  lcd.print(millis() / 1000 / 60 / 60);
  lcd.print(":");
  lcd.print(millis() / 1000 / 60 % 60);
  lcd.print(":");
  lcd.print(millis() / 1000 % 60);

  // Third row , 30초마다 30초간 측정된 값의 평균을 출력하는 코드
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial connection
  }

  setup_MQ7();
  setup_LCD();
}

void loop()
{
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis >= 1000)
  {
    lastMillis = millis();
    loop_LCD();
  }
  // ...existing code...
}
/**
 * @file waterPumpFuntion_ver06.ino
 * @author iot발걸음
 * @brief  2개의 버튼 모터 의 동작 제어 , 가변저항을 통한 모터 on시간 , off 시간제어 , 상황 디스플레이
 * @version 0.5
 * @date 2022-09-22
 * @link : https://wokwi.com/projects/343503342073283156
 * @copyright Copyright (c) 2022
 * @lisence : MIT
 */

// library ezButton , u8glib
#include <U8glib.h>
#include <ezButton.h>

// u8glib 클래스 객체 생성
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

// ezButton 클래스 객체 생성
ezButton start_button(3); // 시작 버튼
ezButton stop_button(4);  // 멈춤 버튼

// 가변저항 관련 변수들
const int pot1Pin = A0;
const int pot2Pin = A1;
int pot1Val, pot2Val;
int pot1Hour, pot1Min, pot2Hour, pot2Min;

// 릴레이 관련 변수들
const int relayPin = 2;
bool relayState = LOW;
int intervalOn , intervalOff ;
long currentMillis, previousMillis;

// 스위치 함수 관련 변수들
int switchOnNumber;

// 시리얼통신 115200 ,  ezButton and u8glib 초기화 없음
void setup()
{
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // ezButton debounce time 45ms
  start_button.setDebounceTime(45);
  stop_button.setDebounceTime(45);
}

void loop()
{
  // ezButton 루프 함수 선언 , 라이브러리 안내에 따르면 맨처음 와야 한다고함
  start_button.loop();
  stop_button.loop();

  // 시작 버튼 눌렸을때 동작(한번만 실행됨)
  if (start_button.isPressed())
  {
    relayState = HIGH;
    digitalWrite(relayPin, relayState);
    // 스위치 함수에서 11번으로 이동하라는 변수
    switchOnNumber = 11;

    // previousMillis 에 millis() 값을 넣어줌 , 이유 : 시간이 축적되어서 부정확한 디스플레이됨
    previousMillis = millis();
    debuggingSerialPrint();
  }
  // 버튼2 상태에 따른 동작 , { } 안에 동작 내용 없음
  if (stop_button.isPressed())
  {
    debuggingSerialPrint();
    switchOnNumber = 22;
  }
  readAnalogPins();  // 아날로그 핀 신호 읽기
  pot1ValToTime();   // pot1Val 값을 시간으로 변환
  pot2ValToTime();   // pot2Val 값을 시간으로 변환
  calculateInterval(); // intervalOn , intervalOff 계산
  switchfuntion();   // 시작 버튼과 멈춤 버튼에 따른 동작

}

void readAnalogPins()
{
  pot1Val = analogRead(pot1Pin);
  pot2Val = analogRead(pot2Pin);
}

void pot1ValToTime() // intervalON
{
  pot1Val = analogRead(pot1Pin);
  pot1Val = map(pot1Val, 0, 1023, 1, 180);
  pot1Hour = pot1Val / 60;
  pot1Min = pot1Val % 60;
}

void pot2ValToTime() // intervalOff
{
  pot2Val = analogRead(pot2Pin);
  pot2Val = map(pot2Val, 0, 1023, 1, 180);
  pot2Hour = pot2Val / 60;
  pot2Min = pot2Val % 60;
}

void calculateInterval()
{
  intervalOn = pot1Val * 50;     // 테스트용
  intervalOff = pot2Val * 50;    // 테스트용
  //intervalOn = pot1Val * 1000; // 실제 사용용
  //intervalOff = pot2Val * 1000;// 실제 사용용
}

void switchfuntion()
{
  switch (switchOnNumber)
  {
  case 11:

    relayStateSwitchFuntion();
    break;
  case 22:
    oledDisplayClear();
    break;
    
  default:
    break;
  }
}

void relayStateSwitchFuntion()  // 켜지는 시간 과 꺼지는 시간에 다른 동작 함수
{
  
  long currentMillis = millis();
  long cMinusP_time = currentMillis - previousMillis;
  if ((relayState == HIGH && cMinusP_time >= intervalOn) ||
      (relayState == LOW && cMinusP_time >= intervalOff))
  {
    previousMillis = currentMillis;
    if (relayState == HIGH)  // 릴레이 NO(normally Open) + COM
    {
      relayState = LOW;
      Serial.print("on cMinusP_time =");
      Serial.println(cMinusP_time);
      digitalWrite(relayPin, relayState);
    }
    else                    // 릴레이 NC(normally Close) + COM
    {
      relayState = HIGH;
      Serial.print("off cMinusP_time =");
      Serial.println(cMinusP_time);
      digitalWrite(relayPin, relayState);

    }
  }  
  oledDisplay();
}

void oledDisplay()
{
  u8g.firstPage();
  do
  {
    u8g.setRot180(); //화면 반전
    u8g.setFont(u8g_font_unifont);
    u8g.drawStr(15, 10, "Motor Contol");
    u8g.setPrintPos(5, 28);
    u8g.print(pot1Hour);
    u8g.drawStr(15, 28, "H");
    u8g.setPrintPos(26, 28);
    u8g.print(pot1Min);
    u8g.drawStr(42, 28, "M");
    u8g.setPrintPos(78, 28);
    u8g.print(pot2Hour);
    u8g.drawStr(88, 28, "H");
    u8g.setPrintPos(98, 28);
    u8g.print(pot2Min);
    u8g.drawStr(114, 28, "M");
    u8g.drawStr(10, 48, "ON");
    u8g.drawStr(90, 48, "OFF");
    u8g.setFont(u8g_font_unifont);
    if (relayState == HIGH) // 릴레이에 전류가 흐르면
    {
      u8g.drawFrame(5, 31, 25, 25);
    }
    else                    // 릴레이에 전류가 안 흐르면
    {
      u8g.drawFrame(85, 31, 35, 25);
    }
  } while (u8g.nextPage());
}

// OLED display clear funtion
void oledDisplayClear()
{
  u8g.firstPage();
  do
  {
  } while (u8g.nextPage());
}

void debuggingSerialPrint()
{
  if (start_button.isPressed())
  {
    Serial.println("start button");
    Serial.print(pot1Hour);
    Serial.print("   ");
    Serial.println(pot1Min);
  }
  else if (stop_button.isPressed())
  {
    Serial.println("stop button");
    Serial.print(pot2Hour);
    Serial.print("   ");
    Serial.println(pot2Min);
  }
}
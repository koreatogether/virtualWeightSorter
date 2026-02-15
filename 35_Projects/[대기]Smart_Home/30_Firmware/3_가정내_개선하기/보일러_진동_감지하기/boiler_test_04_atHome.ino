/*
출처 : https://cafe.naver.com/dgarduino/946
- 인터럽트 등록 함수
void attachInterrupt(interrupt, ISR, mode)
interrupt: 인터럽트 번호, 핀 번호 아님!
ISR: 인터럽트가 발생할 때 수행할 callback 함수 (Interrupt Service Routine)
mode: 인터럽트가 수행될 조건
        LOW: pin 이 LOW 상태일 때
        CHANGE: pin 입력 값이 변경 될 때
        RISING: LOW -> HIGH 로 변경 될 때
        FALLING: HIGH -> LOW 로 변경 될 때
        (HIGH: HIGH 상태일 때 – 아두이노 DUE 에서만 지원)
*/
// u8glib 라이브러리 선언

#include <U8glib.h>

// u8glib , ssd1306 , i2c type
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI
const int vibrationSensorPin = 2;   // 나노보드의 인터럽트 번호 1에 해당함
boolean shockState = false;         // 진동 감지 상태 저장 변수
unsigned long previousMillis = 0;
long interval = 250;                // 진동 체크 간격 시간

long shockCount = 0;                // 진동 감지 횟수 저장 변수
long noShockCount = 0;              // 진동 감지 안된 횟수 저장 변수
int boilerTotalMoveCount = 0;

unsigned long boilerStartTime = 0;        // 보일러 작동 시작 시간
unsigned long boilerStopTime = 0;         // 보일러 작동 종료 시간
unsigned long boilerWorkedTime = 0;       // 보일러 작동 시간
unsigned long previousBoilerStopTime = 0; // 이전 보일러 작동 종료 시간

boolean boilerStartFlag = false;          // 보일러 작동 시작 플래그
boolean boilerStopFlag = false;           // 보일러 작동 종료 플래그

void setup()
{
    Serial.begin(115200);
    u8g.setColorIndex(1);
    u8g.begin();
    pinMode(vibrationSensorPin, INPUT);                    // 나노 보드의 인터럽트 1번 사용 ( 디지털 핀 2번 사용)
    attachInterrupt(1, sensorChangeStateFuntion, FALLING); // 1 은 나노 보드의 디지털 핀 2번에 해당함
}
void loop()
{
    unsigned long currentMillis = millis();

    // shockStateFuntion() 을 millis() 를 이용 해서 interval 시간마다 호출
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        // shockStateFuntion();
        shockCountFuntion();
        checkBoilerWorkedTime();
    }
    // call oled display funtion
    oledDisplayFuntion();
}

// 인터럽트 함수조건에 의해서 신호가 오면 shockState 변수를 true 로 변경 함수
void sensorChangeStateFuntion()
{
    shockState = true;
}

void shockCountFuntion()
{
    if (shockState)
    {
        shockCount++;
        shockState = false;
        // Serial.print(" shockCount = ");
        // Serial.print(shockCount);
    }
    else
    {
        noShockCount++;
        // Serial.print("   noShockCount = ");
        // Serial.println(noShockCount);
    }
}

void checkBoilerWorkedTime()
{
    if (shockCount >= 16)
    {
        // Serial.println("Boiler Working");
        noShockCount = 0;
        shockCount = 0;
        boilerStartTime = millis();
        boilerStartFlag = true;
    }
    if (noShockCount >= 16)
    {
        // Serial.println("Boiler Stoped");
        shockCount = 0;
        noShockCount = 0;
        previousBoilerStopTime = boilerStopTime;
        boilerStopTime = millis();
        boilerStopFlag = true;
    }
    if (boilerStartFlag == true && boilerStopFlag == true)
    {
        if (boilerStopTime >= boilerStartTime)
        {
            boilerWorkedTime = boilerStartTime - previousBoilerStopTime;
            boilerTotalMoveCount++;
            // Serial.print("   Boiler Worked Time = ");
            // Serial.println(boilerWorkedTime);
            boilerStartFlag = false;
            boilerStopFlag = false;
        }
    }
}

// OLED DISPLAY 출력 함수
void oledDisplayFuntion()
{
    u8g.firstPage();
    do
    {
        u8g.setFont(u8g_font_6x10);
        // u8g.drawStr(3, 10, "Boiler Check System");
        u8g.drawStr(5, 10, "Shock C.T");
        u8g.drawStr(60, 10, "NoShock C.T");
        u8g.setPrintPos(30, 20);
        u8g.print(shockCount);
        u8g.setPrintPos(90, 20);
        u8g.print(noShockCount);
        if (boilerStartFlag == true && boilerStopFlag == true)
        {
            u8g.drawStr(5, 30, "Boiler Working");
        }
        else
        {
            u8g.drawStr(5, 30, "Boiler Stoped");
            u8g.drawStr(5, 40, " BoilerWorkedTime = ");
            u8g.setPrintPos(30, 50);
            long toSecondTime = boilerWorkedTime / 1000;
            u8g.print(toSecondTime);
            u8g.drawStr(50, 50, "Second");
        }
        u8g.drawStr(5, 60, " Total C.T = ");
        u8g.setPrintPos(90, 60);
        u8g.print(boilerTotalMoveCount);
    } while (u8g.nextPage());
}
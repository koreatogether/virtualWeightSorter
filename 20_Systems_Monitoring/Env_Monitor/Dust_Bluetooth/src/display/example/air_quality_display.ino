
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// LCD 설정 상수
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
const int LEFT_COL = 0;
const int RIGHT_COL = 10;

// 타이밍 상수
const unsigned long DISPLAY_INTERVAL = 1000; // 1초
const unsigned long STARTUP_DELAY = 2000;    // 시작 메시지 표시 시간

// 공기질 등급 개수
const int NUM_AIR_QUALITY_LEVELS = 4;

// LCD 클리어용 공백 문자열
const char *CLEAR_LINE = "                    "; // 20칸 공백

// LCD 객체 생성 (I2C 주소는 보드에 따라 다를 수 있음, 일반적으로 0x27 또는 0x3F)
hd44780_I2Cexp lcd;

// 전역 변수
int currentLevel = 0;
unsigned long previousMillis = 0;

// LCD 초기화 함수
void initializeLCD()
{
    lcd.begin(LCD_COLS, LCD_ROWS);
}

// 시작 메시지 표시 함수
void showStartupMessage()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Air Quality");
    lcd.setCursor(0, 1);
    lcd.print("Monitor");
    delay(STARTUP_DELAY); // 시작 메시지 표시 시간
    lcd.clear();
}

// 공기질 표시 업데이트 함수
void updateAirQualityDisplay()
{
    // 1줄: "Air Quality:" 표시
    lcd.setCursor(0, 0);
    lcd.print("Air Quality:");

    // 현재 표시할 등급 결정
    int displayIndex = currentLevel % NUM_AIR_QUALITY_LEVELS;

    // 등급 표시 영역 클리어 (줄 1-3)
    clearDisplayLines();

    // 현재 등급 표시
    displayAirQualityLevel(displayIndex);
}

// 등급 표시 영역 클리어 함수
void clearDisplayLines()
{
    for (int row = 1; row < LCD_ROWS; row++)
    {
        lcd.setCursor(0, row);
        lcd.print(CLEAR_LINE);
    }
}

// 공기질 등급 표시 함수
void displayAirQualityLevel(int level)
{
    switch (level)
    {
    case 0: // Very Poor - 좌측 상단
        lcd.setCursor(LEFT_COL, 1);
        lcd.print("Very Poor");
        break;
    case 1: // Poor - 우측 상단
        lcd.setCursor(RIGHT_COL, 1);
        lcd.print("Poor");
        break;
    case 2: // Good - 좌측 하단
        lcd.setCursor(LEFT_COL, 2);
        lcd.print("Good");
        break;
    case 3: // Excellent - 우측 하단
        lcd.setCursor(RIGHT_COL, 2);
        lcd.print("Excellent");
        break;
    }
}

void setup()
{
    initializeLCD();
    showStartupMessage();
}

void loop()
{
    unsigned long currentMillis = millis();

    // 1초마다 공기질 단계 변경 및 표시
    if (currentMillis - previousMillis >= DISPLAY_INTERVAL)
    {
        previousMillis = currentMillis;
        updateAirQualityDisplay();
        currentLevel = (currentLevel + 1) % NUM_AIR_QUALITY_LEVELS;
    }

    // 여기에 다른 작업들을 추가할 수 있음
    // 예: 센서 데이터 읽기, 버튼 입력 처리 등
}
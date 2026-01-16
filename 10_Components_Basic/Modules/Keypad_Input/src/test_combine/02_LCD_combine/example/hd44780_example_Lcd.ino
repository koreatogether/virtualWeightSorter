// LCD 기본 동작에 대한 여러가지 테스트 하는 코드이며
// ARDUINO R4 WIFI에 어울리는 라이브러리를 선정하는 코드이기도 하다.

/*
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

호환성이 높은 lcd 라이브러리를 고르다가 이걸로 정하게 되었다.
*/

// Arduino IDE에서 hd44780_I2Cexp가 Wire 객체를 사용하므로 반드시 먼저 포함 필요
#include <Wire.h> // I2C 통신을 위한 Wire 라이브러리 추가 (항상 hd44780 이전에 포함)
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

hd44780_I2Cexp lcd; // I2C LCD 객체 생성

// 사용자 정의 문자 데이터 (하트 모양)
byte heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000};

// 사용자 정의 문자 데이터 (화살표)
byte arrow[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00000,
    0b00000};

// 테스트 단계들을 정의하는 열거형
enum TestPhase
{
    PHASE_BEGIN,
    PHASE_BASIC_PRINT,
    PHASE_CURSOR_CONTROL,
    PHASE_DISPLAY_CONTROL,
    PHASE_SCROLLING,
    PHASE_CUSTOM_CHARS,
    PHASE_BACKLIGHT,
    PHASE_AUTOSCROLL,
    PHASE_TEXT_DIRECTION,
    PHASE_BLINK_CURSOR,
    PHASE_END
};

TestPhase currentPhase = PHASE_BEGIN;
unsigned long lastPhaseChange = 0;
const unsigned long PHASE_DURATION = 2000; // 2초마다 변경

// 백라이트 상태 추적을 위한 변수
bool backlightState = true; // 기본적으로 켜져 있다고 가정

void setup()
{
    Serial.begin(9600);
    delay(2000);

    // LCD 초기화
    int status = lcd.begin(16, 2); // 16열 2행 LCD
    if (status)
    {
        Serial.print("LCD 초기화 실패: ");
        Serial.println(status);
        while (1)
            ; // 초기화 실패 시 무한 루프
    }

    Serial.println("LCD 초기화 성공!");
    // 모든 페이즈 기본값: 백라이트 ON
    lcd.setBacklight(255);
    backlightState = true;
    lcd.clear();
    lcd.print("LCD Test Start!");
    delay(1000);
}

void loop()
{
    unsigned long currentTime = millis();

    // 2초마다 다음 단계로 진행
    if (currentTime - lastPhaseChange >= PHASE_DURATION)
    {
        currentPhase = static_cast<TestPhase>((static_cast<int>(currentPhase) + 1) % (PHASE_END + 1));
        lastPhaseChange = currentTime;

        // 페이즈 7(백라이트 테스트) 외에는 항상 백라이트 켬 상태 유지
        if (currentPhase != PHASE_BACKLIGHT)
        {
            lcd.setBacklight(255);
            backlightState = true;
        }

        // 각 단계별 테스트 실행
        switch (currentPhase)
        {
        case PHASE_BEGIN:
            testBegin();
            break;
        case PHASE_BASIC_PRINT:
            testBasicPrint();
            break;
        case PHASE_CURSOR_CONTROL:
            testCursorControl();
            break;
        case PHASE_DISPLAY_CONTROL:
            testDisplayControl();
            break;
        case PHASE_SCROLLING:
            testScrolling();
            break;
        case PHASE_CUSTOM_CHARS:
            testCustomChars();
            break;
        case PHASE_BACKLIGHT:
            testBacklight();
            break;
        case PHASE_AUTOSCROLL:
            testAutoscroll();
            break;
        case PHASE_TEXT_DIRECTION:
            testTextDirection();
            break;
        case PHASE_BLINK_CURSOR:
            testBlinkCursor();
            break;
        case PHASE_END:
            testEnd();
            break;
        }
    }
}

// 1. begin() 메서드 테스트 - LCD 초기화
void testBegin()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 1:");
    lcd.setCursor(0, 1);
    lcd.print("LCD Begin Test");
    Serial.println("Phase 1: LCD Begin Test");
}

// 2. print() 메서드 테스트 - 기본 텍스트 출력
void testBasicPrint()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 2: Print");
    lcd.setCursor(0, 1);
    lcd.print("Hello, Arduino!");
    Serial.println("Phase 2: Basic Print Test");
}

// 3. setCursor() 메서드 테스트 - 커서 위치 제어
void testCursorControl()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 3: Cursor");
    lcd.setCursor(5, 1);
    lcd.print("Position Test");
    Serial.println("Phase 3: Cursor Control Test");
}

// 4. display()/noDisplay() 메서드 테스트 - 화면 표시 제어
void testDisplayControl()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 4: Display");
    lcd.setCursor(0, 1);
    lcd.print("Control Test");

    // 1초 후 화면 끄기
    delay(1000);
    lcd.noDisplay();
    delay(500);
    lcd.display(); // 다시 켜기

    Serial.println("Phase 4: Display Control Test");
}

// 5. scrollDisplayLeft()/scrollDisplayRight() 메서드 테스트 - 화면 스크롤
void testScrolling()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 5: Scroll");
    lcd.setCursor(0, 1);
    lcd.print("Scrolling Test!");

    // 왼쪽으로 스크롤
    for (int i = 0; i < 5; i++)
    {
        lcd.scrollDisplayLeft();
        delay(300);
    }

    // 오른쪽으로 스크롤
    for (int i = 0; i < 10; i++)
    {
        lcd.scrollDisplayRight();
        delay(200);
    }

    Serial.println("Phase 5: Scrolling Test");
}

// 6. createChar() 메서드 테스트 - 사용자 정의 문자
void testCustomChars()
{
    lcd.clear();

    // 사용자 정의 문자 생성
    lcd.createChar(0, heart);
    lcd.createChar(1, arrow);

    lcd.setCursor(0, 0);
    lcd.print("Phase 6: Custom");
    lcd.setCursor(0, 1);
    lcd.print("Chars: ");

    // 사용자 정의 문자 출력
    lcd.write(byte(0)); // 하트
    lcd.print(" ");
    lcd.write(byte(1)); // 화살표

    Serial.println("Phase 6: Custom Characters Test");
}

// 7. setBacklight() 메서드 테스트 - 백라이트 제어
void testBacklight()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 7: Backlight");

    // 백라이트 상태 토글
    lcd.setCursor(0, 1);

    if (backlightState)
    {
        lcd.print("Backlight: ON");
        lcd.setBacklight(0); // 끄기
        backlightState = false;
    }
    else
    {
        lcd.print("Backlight: OFF");
        lcd.setBacklight(255); // 켜기
        backlightState = true;
    }

    Serial.println("Phase 7: Backlight Control Test");
}

// 8. autoscroll() 메서드 테스트 - 자동 스크롤
void testAutoscroll()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 8: AutoScroll");

    lcd.autoscroll();     // 자동 스크롤 활성화
    lcd.setCursor(15, 1); // 두 번째 줄 끝으로 커서 이동

    // 텍스트를 하나씩 추가하면서 자동 스크롤 관찰
    for (char c = 'A'; c <= 'H'; c++)
    {
        lcd.print(c);
        delay(400);
    }

    lcd.noAutoscroll(); // 자동 스크롤 비활성화
    Serial.println("Phase 8: Autoscroll Test");
}

// 9. leftToRight()/rightToLeft() 메서드 테스트 - 텍스트 방향
void testTextDirection()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 9: Direction");

    // 왼쪽에서 오른쪽으로
    lcd.setCursor(0, 1);
    lcd.leftToRight();
    lcd.print("Left->Right");

    delay(1000);

    // 오른쪽에서 왼쪽으로
    lcd.setCursor(15, 1);
    lcd.rightToLeft();
    lcd.print("Right<-Left");

    Serial.println("Phase 9: Text Direction Test");
}

// 10. cursor()/noCursor(), blink()/noBlink() 메서드 테스트 - 커서 및 깜빡임
void testBlinkCursor()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 10: Cursor");

    // 커서 표시
    lcd.cursor();
    lcd.setCursor(0, 1);
    lcd.print("Cursor Visible");
    delay(1000);

    // 커서 깜빡임
    lcd.blink();
    lcd.setCursor(0, 1);
    lcd.print("Cursor Blinking");
    delay(1000);

    // 커서 및 깜빡임 숨기기
    lcd.noBlink();
    lcd.noCursor();

    Serial.println("Phase 10: Cursor & Blink Test");
}

// 11. clear(), home() 메서드 테스트 - 화면 및 커서 초기화
void testEnd()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phase 11: End");
    lcd.setCursor(0, 1);
    lcd.print("Test Complete!");

    // home() 메서드 테스트
    delay(1000);
    lcd.home(); // 커서를 (0,0)으로 이동 (화면은 지우지 않음)
    lcd.print("Home Test!");

    Serial.println("Phase 11: End Test - Restarting...");

    // 다시 처음으로 돌아가기 위해 딜레이
    delay(1000);
}
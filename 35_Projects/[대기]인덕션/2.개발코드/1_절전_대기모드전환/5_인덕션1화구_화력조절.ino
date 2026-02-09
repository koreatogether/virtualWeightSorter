/*
--------------   메뉴얼 내 전원 버튼 동작 ----------------
1. 제품의 전원 플러그를 콘센트에 꽂아 주십시오.
   - 전원 플러그를 꽂으면 알람음과 함께 모든 화면이 점등된 후 소등되며 절전
모드로 진입합니다.
   - 절전 모드는 에너지 세이빙(Energy Saving) 기능으로 모든 화면이 소등되며
[전원], [정지/잠금] 버튼 이외의 모든 버튼이 동작하지 않습니다.

2. 절전 모드에서 [전원] 버튼을 1초 이상 누르면 그림과 같이 대기 상태로
전환됩니다.
   - 낮이 안전 기능 설정 시에는 [전원] 버튼과 [알람/펜] 버튼을 동시에 1초 이상
눌러야 대기 상태로 전환됩니다.
   - 각 학교의 화력단계가 "0", 꺼짐에 시간 표시부에 "--" 이 표시되며, 음성 LED가
점등됩니다.
   - 음성이 OFF로 설정되어 있을 경우 음성 LED가 소등됩니다.
   - 대기 상태에서 각 화구 및 기타기능을 조작할 수 있습니다.
*/

/*
--------------   구성품  ----------------

1. Arduino Uno 보드
2. TM1627 모듈 7-Segment 4-Digit Display 2개
3. 버튼 4개 (전원, 화력 증가, 화력 감소, 인덕션 1화구 선택)
4. LED (음성 표시용)
5. 저항(220옴) 및 배선
*/

/*
--------------   배선도 ----------------
아두이노          TM1637 (인덕션용)
4번 핀 <----> DIO
5번 핀 <----> CLK
5V    <----> VCC
GND   <----> GND

아두이노          TM1637 (시간표시용)
10번 핀 <----> DIO
11번 핀 <----> CLK
5V     <----> VCC
GND    <----> GND

아두이노          전원버튼
2번 핀 <----> 버튼의 한쪽 다릿발
GND   <----> 버튼의 다른쪽 다릿발

아두이노         인덕션 1화구 선택 버튼
8번 핀 <----> 버튼의 한쪽 다릿발
GND    <----> 버튼의 다른쪽 다릿발

아두이노          화력 증가 버튼
13번 핀 <----> 버튼의 한쪽 다릿발
GND    <----> 버튼의 다른쪽 다릿발

아두이노          화력 감소 버튼
12번 핀 <----> 버튼의 한쪽 다릿발
GND    <----> 버튼의 다른쪽 다릿발
*/

/*
------------- 시뮬레이터 1차 테스트 ---------------
https://wokwi.com/projects/422637449959118849 -> 절전 < -- > 대기 모드 전환 테스트
https://wokwi.com/projects/422725698358451201 -> 위 링크 테스트 + 인덕션 1구 화력조절 테스트
*/

/*
------------- 기타 사항 ----------------
시리얼 출력은 메모리 사용량(SRAM)을 줄이기 위해 F() 매크로 사용함. ( SRAM -> 프로그램 메모리 이동 )
*/

#include <TM1637.h>
#include <ezButton.h>

// 시간, 인덕션 화구 표시 7 세그먼트 모듈 핀 및 객체 선언
const int TIME_DIO_PIN = 4;
const int TIME_CLK_PIN = 5;
const int INDUCTION_DIO_PIN = 6;
const int INDUCTION_CLK_PIN = 7;
TM1637 timeSegment(TIME_CLK_PIN, TIME_DIO_PIN);
TM1637 inductionSegment(INDUCTION_CLK_PIN, INDUCTION_DIO_PIN);

// 버튼 핀 및 객체 선언
const int POWER_BUTTON_PIN = 2;
const int POWER_UP_BUTTON_PIN = 13;
const int POWER_DOWN_BUTTON_PIN = 12;
const int SELECT_INDUCTION_ONE_BUTTON_PIN = 8;
ezButton powerButton(POWER_BUTTON_PIN);
ezButton powerUpButton(POWER_UP_BUTTON_PIN);
ezButton powerDownButton(POWER_DOWN_BUTTON_PIN);
ezButton selectInductionOne(SELECT_INDUCTION_ONE_BUTTON_PIN);
const int debounceTime = 100; // 디바운스 시간

// 음성 LED 핀
const int VOICE_PIN = 3;

// 모드 변수
int mode = 0; // 0: 초기상태, 1: 대기 모드, 2: 절전 모드 , 3: 인덕션 화구중 1구 선택 모드

// 화구 세기 변수
int powerLevel = 0; // 0: 꺼짐, 1~10: 화구 세기

/*-----------------  setup ----------------------*/

void setup()
{
    Serial.begin(9600); // 컴퓨터와 시리얼 통신 시작
    Serial.println(F("Setup 시작"));

    // 7 세그먼트 모듈 초기화 및 밝기 설정(라이브러리 상수)
    timeSegment.init();
    timeSegment.set(BRIGHT_TYPICAL);
    // Serial.println(F("시간 세그먼트 초기화 완료"));

    inductionSegment.init();
    inductionSegment.set(BRIGHT_TYPICAL);
    // Serial.println(F("인덕션 세그먼트 초기화 완료"));

    // 버튼 디바운스 시간 설정 , 기본 50ms,  버튼 마다 개별 설정 필요 할수 도 있음.
    powerButton.setDebounceTime(debounceTime);
    powerUpButton.setDebounceTime(debounceTime);
    powerDownButton.setDebounceTime(debounceTime);
    selectInductionOne.setDebounceTime(debounceTime);
    Serial.println(F("버튼 디바운스 시간 설정 완료"));

    // LED 핀 모드 설정 (음성 표시용)
    pinMode(VOICE_PIN, OUTPUT);
    Serial.println(F("LED 핀 모드 설정 완료"));
}

/*-----------------  controlLED ----------------------*/
// 음성 LED 제어함수 : 인자 값이 1이면 켜고, 0이면 끈다.
void controlLED(int turnOnFlag)
{
    if (turnOnFlag)
    {
        digitalWrite(VOICE_PIN, HIGH); // LED 켜기
                                       // Serial.println(F("LED 켜짐"));
    }
    else
    {
        digitalWrite(VOICE_PIN, LOW); // LED 끄기
                                      // Serial.println(F("LED 꺼짐"));
    }
}

/*-----------------  timeSegmentDisplay ----------------------*/
void timeSegmentDisplay()
{
    timeSegment.display("----");
    // Serial.println(F("시간 세그먼트: ----"));
}

/*-----------------  inductionSegmentDisplay ----------------------*/
void inductionSegmentDisplay()
{
    inductionSegment.display("---0");
    // Serial.println(F("인덕션 세그먼트: ---0"));
}

/*-----------------  powerButtonLongPressed ----------------------*/
void powerButtonLongPressed()
{
    // static 변수로 선언하여 함수종료(벗어날때) 값이 유지되도록 한다.
    static unsigned long firstPressedTime = 0;
    static int pressCount = 0; // long press 횟수를 기록하는 변수

    // 버튼이 누르기 시작하면 시간 기록
    if (powerButton.isPressed() && firstPressedTime == 0)
    {
        firstPressedTime = millis();
    }

    // 버튼이 해제되었을 때, 누른 시간이 1초 이상이면 처리
    if (powerButton.isReleased() && firstPressedTime != 0)
    {
        unsigned long pressedDuration = millis() - firstPressedTime;
        if (pressedDuration >= 1000)
        {
            pressCount++; // long press 횟수 증가
            if (pressCount == 1)
            {
                mode = 1;       // 첫 번째 long press 시 대기 모드
                                // Serial.println(F("전원 버튼 첫 long press, 대기 모드: 1"));
                powerLevel = 0; // 화력을 0으로 초기화
            }
            else if (pressCount == 2)
            {
                mode = 2; // 두 번째 long press 시 절전 모드
                // Serial.println(F("전원 버튼 두번째 long press, 절전 모드: 2"));
                powerLevel = 0; // 화력을 0으로 초기화
                pressCount = 0; // 필요에 따라 카운터 초기화
            }
        }
        // 시작 시간 초기화
        firstPressedTime = 0;
    }
}

// ----------------- 화력 올림 버튼 처리 ----------------------
void handlePowerUpButton()
{
    // 화력 증가 버튼이 눌렸을 때
    if (powerUpButton.isPressed())
    {
        Serial.println(F("화력 증가 버튼눌림"));
        powerLevel++;
        if (powerLevel > 10)
        {
            powerLevel = 10;
        }
    }
}

// ----------------- 화력 내림 버튼 처리 ----------------------
void handlePowerDownButton()
{
    if (powerDownButton.isPressed())
    {
        Serial.println(F("화력 감소 버튼 눌림"));
        powerLevel--;
        if (powerLevel <= 0)
        {
            powerLevel = 0;
        }
    }
}

// ----------------- 인덕션 1화구 선택 버튼 처리 ----------------------
void handleInductionOne()
{
    if (selectInductionOne.isPressed())
    {
        Serial.println(F("인덕션 1화구 선택 버튼 눌림"));
        mode = 3;       // 인덕션 화구중 1구 선택 모드
        powerLevel = 7; // 초기 화력은 7로 시작 설정
        Serial.println(F("인덕션 1화구 화력 7로 시작"));
    }
}

/*-----------------  loop ----------------------*/
void loop()
{
    // 버튼 루프
    powerButton.loop();
    powerUpButton.loop();
    powerDownButton.loop();
    selectInductionOne.loop();

    powerButtonLongPressed();

    switch (mode)
    {
    case 1: // 대기 모드 진입

        controlLED(1);             // 음성 LED 점등
        timeSegmentDisplay();      // 시간 표시부에 "--" 표시
        inductionSegmentDisplay(); // 인덕션 1화구 표시부에 "0" 표시 한다.
        handleInductionOne();      // 인덕션 1화구 선택 버튼 눌림 처리
        break;

    case 2: // 절전모드

        controlLED(0);                    // 음성 LED 소등
        timeSegment.display("    ");      // 시간 표시부에 공백표시
        inductionSegment.display("    "); // // 화력 표시구에 공백 표시

        break;

    case 3: // 인덕션 화구중 1구 선택 모드

        // 화력 조절 버튼 함수 불러오기
        handlePowerUpButton();
        handlePowerDownButton();

        // powerLevel이 0 이하일 때 세그먼트에 표시가 안되어서 따로 처리
        if (powerLevel <= 0)
        {
            inductionSegment.display(3, 0); // 0 ~ 3 세그먼트 표시 위치중 3번째에 0 표시
        }
        else
        {
            inductionSegment.displayNum(powerLevel);
        }

        powerButtonLongPressed(); // 전원버튼 조작시 모드2 또는 1로 전환되게 할것

        break;

    default: // (mode == 0 , 초기상태(아무것도 안함))
        // nothing
        // Serial.println(F("DEFAULT"));
        break;
    }
}

/*
날짜: 2024-01-09
시간: 16:00

```mermaid
flowchart TD
    A[시작] --> B[초기화:<br/>- 세그먼트 초기화<br/>- 버튼 디바운스 설정<br/>- LED 핀 설정]
    B --> C[버튼 상태 확인]
    C --> D{전원 버튼<br/>Long Press?}
    D -->|No| E{현재 모드?}
    D -->|Yes, 1초이상| F{Press Count?}
    F -->|1회| G[대기 모드<br/>mode=1<br/>powerLevel=0]
    F -->|2회| H[절전 모드<br/>mode=2<br/>powerLevel=0<br/>pressCount=0]

    E -->|mode=1| I[대기 모드:<br/>1. LED ON<br/>2. 시간: ----<br/>3. 화력: ---0]
    E -->|mode=2| J[절전 모드:<br/>1. LED OFF<br/>2. 시간: 공백<br/>3. 화력: 공백]
    E -->|mode=3| K[화구 선택 모드]

    I --> L{인덕션 1구<br/>버튼?}
    L -->|Yes| M[mode=3<br/>powerLevel=7]

    K --> N{화력 증가?}
    K --> O{화력 감소?}
    N -->|Yes| P[powerLevel + 1<br/>최대 10]
    O -->|Yes| Q[powerLevel - 1<br/>최소 0]
    P --> R{powerLevel = 0?}
    Q --> R
    R -->|Yes| S[표시: 0]
    R -->|No| T[표시: 숫자]

    G --> C
    H --> C
    J --> C
    S --> C
    T --> C
```
*/

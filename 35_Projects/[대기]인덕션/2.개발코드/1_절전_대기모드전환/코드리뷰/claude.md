코드를 자세히 리뷰해드리겠습니다. 초보자 관점에서 중요한 부분들을 설명하겠습니다.

1. 코드 구조와 가독성
- 한글 주석이 잘 되어있어 초보자가 이해하기 좋습니다
- 코드가 논리적으로 setup()과 loop() 부분으로 잘 나뉘어져 있습니다
- 변수와 상수 선언이 상단에 잘 정리되어 있습니다

2. 장점들:
- ezButton 라이브러리를 사용하여 디바운싱을 쉽게 처리했습니다
- 모드 관리를 switch-case로 깔끔하게 구현했습니다
- 함수들이 하나의 역할만 담당하도록 잘 분리되어 있습니다

3. 개선할 만한 부분:



```cpp
// 상수 정의로 매직넘버 제거
const int STANDBY_MODE = 1;
const int POWER_SAVE_MODE = 2;
const int LONG_PRESS_DURATION = 1000; // 1초
const int DEBOUNCE_TIME = 50;

// 7-세그먼트 디스플레이 핀 정의
struct DisplayPins {
    const int DIO;
    const int CLK;
};

const DisplayPins TIME_DISPLAY = {4, 5};
const DisplayPins INDUCTION_DISPLAY = {10, 11};

TM1637 timeSegment(TIME_DISPLAY.CLK, TIME_DISPLAY.DIO);
TM1637 inductionSegment(INDUCTION_DISPLAY.CLK, INDUCTION_DISPLAY.DIO);

// 버튼 핀 정의
const int POWER_BUTTON_PIN = 2;
const int POWER_PLUS_PIN = 13;
const int POWER_MINUS_PIN = 12;
const int VOICE_LED_PIN = 3;

ezButton powerButton(POWER_BUTTON_PIN);
ezButton powerPlusButton(POWER_PLUS_PIN);
ezButton powerMinusButton(POWER_MINUS_PIN);

int currentMode = 0;

void setup() {
    Serial.begin(9600);
    Serial.println(F("Setup 시작")); // F() 매크로로 메모리 최적화

    initializeDisplays();
    initializeButtons();
    pinMode(VOICE_LED_PIN, OUTPUT);
}

void initializeDisplays() {
    // 디스플레이 초기화
    timeSegment.init();
    timeSegment.set(BRIGHT_TYPICAL);
    
    inductionSegment.init();
    inductionSegment.set(BRIGHT_TYPICAL);
}

void initializeButtons() {
    // 버튼 초기화
    powerButton.setDebounceTime(DEBOUNCE_TIME);
    powerPlusButton.setDebounceTime(DEBOUNCE_TIME);
    powerMinusButton.setDebounceTime(DEBOUNCE_TIME);
}

void handleStandbyMode() {
    timeSegment.display("----");
    controlLED(true);
    inductionSegment.display("---0");
}

void handlePowerSaveMode() {
    timeSegment.display("    ");
    inductionSegment.display("    ");
    controlLED(false);
}

void controlLED(bool turnOn) {
    digitalWrite(VOICE_LED_PIN, turnOn ? HIGH : LOW);
}

void powerButtonLongPressed() {
    static unsigned long firstPressedTime = 0;
    static int pressCount = 0;

    if (powerButton.isPressed() && firstPressedTime == 0) {
        firstPressedTime = millis();
    }

    if (powerButton.isReleased() && firstPressedTime != 0) {
        unsigned long pressedDuration = millis() - firstPressedTime;
        
        if (pressedDuration >= LONG_PRESS_DURATION) {
            pressCount++;
            currentMode = (pressCount == 1) ? STANDBY_MODE : POWER_SAVE_MODE;
            
            if (pressCount >= 2) {
                pressCount = 0;
            }
        }
        firstPressedTime = 0;
    }
}

void loop() {
    // 버튼 상태 업데이트
    powerButton.loop();
    powerPlusButton.loop();
    powerMinusButton.loop();

    powerButtonLongPressed();

    // 모드에 따른 동작 처리
    switch (currentMode) {
        case STANDBY_MODE:
            handleStandbyMode();
            break;
        
        case POWER_SAVE_MODE:
            handlePowerSaveMode();
            break;
            
        default:
            break;
    }
}

```

주요 개선사항을 설명드리겠습니다:

1. 상수 정의 추가
- 매직넘버를 제거하고 의미 있는 상수로 정의했습니다
- 모드값(1, 2)도 상수로 정의하여 가독성을 높였습니다

2. 구조체 사용
- 관련된 핀들을 구조체로 그룹화하여 관리가 쉽도록 했습니다

3. 함수 분리
- 초기화 코드를 별도 함수로 분리했습니다
- 각 모드별 처리를 별도 함수로 분리했습니다

4. 변수명 개선
- mode를 currentMode로 변경하여 의미를 더 명확히 했습니다

5. 메모리 최적화
- Serial.println에 F() 매크로를 사용하여 문자열을 프로그램 메모리에 저장하도록 했습니다

6. 불필요한 주석 제거
- 코드 자체로 의미가 명확한 경우 불필요한 주석을 제거했습니다

이런 개선사항들이 적용된 코드는 초보자가 보기에도 더 이해하기 쉽고, 나중에 기능을 추가하거나 수정하기도 더 편할 것입니다. 

추가로 궁금하신 부분이 있으시다면 질문해 주세요!
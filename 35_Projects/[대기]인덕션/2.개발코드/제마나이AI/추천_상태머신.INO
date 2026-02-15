#include <Arduino.h>

// 상태 정의
enum SystemState
{
    IDLE,        // 대기 상태
    INITIALIZING // 초기화 상태
};

// 핀 정의
const int POWER_BUTTON = 2;
const int LED_PIN = 13; // 예시 LED 핀

// 변수
SystemState currentState = IDLE;
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
unsigned long buttonHoldStart = 0;

void setup()
{
    Serial.begin(9600);
    pinMode(POWER_BUTTON, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    // 전원 버튼 입력 처리 및 디바운싱
    int currentReading = digitalRead(POWER_BUTTON);
    debouncePowerButton(currentReading);

    // 상태에 따른 동작 수행
    switch (currentState)
    {
    case IDLE:
        handleIdleState();
        break;
    case INITIALIZING:
        handleInitializingState();
        break;
    }
}

void debouncePowerButton(int reading)
{
    if (reading != stableButtonState)
    {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        stableButtonState = reading;
    }
}

void handleIdleState()
{
    if (stableButtonState == LOW)
    {
        if (isPowerButtonHeld())
        {
            // 초기화 상태로 전환
            currentState = INITIALIZING;
            digitalWrite(LED_PIN, HIGH); // LED 켜기 (초기화 중)
            initializeSystem();
        }
    }
}

void handleInitializingState()
{
    // 초기화 완료 후 IDLE 상태로 전환
    if (/* 초기화 완료 조건 */)
    {
        currentState = IDLE;
        digitalWrite(LED_PIN, LOW); // LED 끄기 (초기화 완료)
    }
}

bool isPowerButtonHeld()
{
    if (stableButtonState == LOW)
    {
        if (buttonHoldStart == 0)
        {
            buttonHoldStart = millis();
        }
        if (millis() - buttonHoldStart >= 1000)
        {
            buttonHoldStart = 0;
            return true;
        }
    }
    else
    {
        buttonHoldStart = 0;
    }
    return false;
}

void initializeSystem()
{
    // 시스템 초기화 코드 작성
    Serial.println("Initializing system...");
    delay(2000); // 예시: 2초 동안 초기화 작업 수행
    Serial.println("System initialized.");
}
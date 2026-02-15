#include <Arduino.h>

const int POWER_ON_BUTTON = 2;
const unsigned long debounceDelay = 50;

// INPUT_PULLUP 모드에 맞게 디바운싱 후 버튼 눌림(LOW) 상태를 반환
bool isPowerOnButtonPressed()
{
    int reading = digitalRead(POWER_ON_BUTTON);
    static int lastReading = HIGH; // 초기값: 안눌린 상태
    static unsigned long lastDebounceTime = 0;

    if (reading != lastReading)
    {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        lastReading = reading;
        return (reading == LOW); // LOW이면 눌린 상태
    }
    return (lastReading == LOW);
}

// 버튼의 눌림 시작과 해제 시점을 기록하여 1초 이상 눌렸을 때 true 반환
bool checkPowerOnButtonLongPress()
{
    static bool buttonPressed = false;
    static unsigned long pressStartTime = 0;
    bool currentState = isPowerOnButtonPressed();

    if (currentState && !buttonPressed)
    {
        pressStartTime = millis();
        buttonPressed = true;
    }
    if (!currentState && buttonPressed)
    {
        unsigned long heldTime = millis() - pressStartTime;
        buttonPressed = false;
        if (heldTime >= 1000)
            return true;
    }
    return false;
}

void setup()
{
    Serial.begin(9600);
    pinMode(POWER_ON_BUTTON, INPUT_PULLUP);
}

void loop()
{
    if (checkPowerOnButtonLongPress())
    {
        Serial.println("Powering On...");
    }

    // ...existing code...
}

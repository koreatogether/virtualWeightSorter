#include <Arduino.h>

/**
 * Simple Button Test Example
 *
 * Connection:
 * - Button Pin 3 to one side of the button
 * - GND to the other side of the button
 *
 * Logic:
 * - Uses Internal Pull-up (pin is HIGH when open, LOW when pressed)
 * - Toggles the Built-in LED on each press.
 * - Prints status to Serial Monitor (115200 bps).
 */

const int BUTTON_PIN = 3;
const int LED_PIN = LED_BUILTIN;

bool lastButtonState = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;
int pressCount = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ; // Wait for serial monitor

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    Serial.println("========================================");
    Serial.println("Button Test Example Initialized");
    Serial.println("Pin: 3 (Internal Pull-up)");
    Serial.println("Monitoring press events...");
    Serial.println("========================================");
}

void loop()
{
    bool reading = digitalRead(BUTTON_PIN);

    // 버튼의 물리적 상태가 변했는지 확인
    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    // 설정된 디바운스 시간보다 오래 상태가 유지되었는지 확인
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY)
    {
        // 필터링된 실제 버튼 상태가 변했는지 확인
        if (reading != buttonState)
        {
            buttonState = reading;

            // 버튼이 눌린 경우 (LOW)
            if (buttonState == LOW)
            {
                pressCount++;
                digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED

                Serial.print("Button Pressed (Debounced)! Count: ");
                Serial.println(pressCount);
            }
        }
    }

    lastButtonState = reading;
}

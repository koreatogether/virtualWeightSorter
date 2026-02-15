#include "KeypadLockController.h"

KeypadLockController::KeypadLockController() : currentMode(Mode::IDLE) {}

void KeypadLockController::begin()
{
    Serial.begin(9600);
    keypadManager.begin();
    // 다른 초기화 코드들
}

void KeypadLockController::update()
{
    char buffer[20];
    if (keypadManager.readCode(buffer, sizeof(buffer)))
    {
        String code = String(buffer);
        Serial.print("Input received: ");
        Serial.println(code);

        switch (currentMode)
        {
        case Mode::IDLE:
            if (code.startsWith("*") && code.length() == 8 && (code.endsWith("#") || code.endsWith("*")))
            {
                String pwd = code.substring(1, 7);
                if (pwd == PASSWORD)
                {
                    Serial.println("Correct password entered");
                    // 잠금 해제 로직 추가 가능
                    currentMode = Mode::ENTER_PASSWORD;
                }
                else if (pwd == ADMIN_PASSWORD)
                {
                    Serial.println("Admin password entered");
                    currentMode = Mode::ENTER_ADMIN_PASSWORD;
                }
                else
                {
                    Serial.println("Wrong password");
                    // 추가로 관리자 비밀번호인지 확인
                    if (pwd.length() == 6)
                    {
                        Serial.println("Note: This was attempted as a password input");
                    }
                }
            }
            else
            {
                Serial.println("Invalid input format - must start with * and be 8 characters (*XXXXXX# or *XXXXXX*)");
            }
            break;

        case Mode::ENTER_PASSWORD:
            // 일반 비밀번호 모드에서 추가 처리
            Serial.println("Password mode active");
            currentMode = Mode::IDLE; // 예시로 리셋
            break;

        case Mode::ENTER_ADMIN_PASSWORD:
            // 관리자 비밀번호 모드에서 추가 처리
            Serial.println("Admin mode active");
            currentMode = Mode::IDLE; // 예시로 리셋
            break;
        }
    }

    // 기타 상황 처리: * 없이 입력된 경우 등은 readCode에서 처리되지 않음
    // 추가로 키패드 이벤트 확인 가능
}
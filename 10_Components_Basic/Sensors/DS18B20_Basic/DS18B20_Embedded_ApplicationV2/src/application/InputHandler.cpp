#include <Arduino.h>
#include <cstring>
#include <cctype>
#include "InputHandler.h"

InputHandler::InputHandler()
{
    // 생성자 - 필요시 초기화 로직 추가
}

// 임시로 Uno R4 WiFi용 스텁 구현
#ifdef ARDUINO_ARCH_RENESAS

bool InputHandler::safeSerialRead(char &outChar, int &attempts)
{
    // 임시 스텁 - 간단한 Serial 읽기
    if (Serial.available())
    {
        int result = Serial.read();
        if (result != -1)
        {
            outChar = (char)result;
            return true;
        }
    }
    return false;
}

bool InputHandler::processSerialInput(String &outputBuffer)
{
    // 임시 스텁 - 한 번에 하나의 문자만 처리하여 중복 실행 방지
    if (Serial.available())
    {
        char c = Serial.read();
        if (c == '\r' || c == '\n')
        {
            if (outputBuffer.length() > 0)
            {
                return true; // 완성된 입력 반환
            }
            // 빈 라인은 무시
        }
        else if (isValidMenuChar(c))
        {
            outputBuffer += c;
        }
    }
    return false; // 아직 완성되지 않은 입력
}

void InputHandler::clearInputBuffer()
{
    // 임시 스텁 - 버퍼 클리어
    while (Serial.available())
    {
        Serial.read();
    }
}

#else

// 안전한 Serial 읽기를 위한 헬퍼 함수
// CWE-120, CWE-20 보안 취약점 대응
bool InputHandler::safeSerialRead(char &outChar, int &attempts)
{
    const int MAX_READ_ATTEMPTS = 3;
    const unsigned long READ_TIMEOUT_MS = 1;

    if (attempts >= MAX_READ_ATTEMPTS)
    {
        return false;
    }

    if (!Serial.available())
    {
        return false;
    }

    unsigned long readStartTime = millis();
    int readResult = -1;

    while ((millis() - readStartTime) < READ_TIMEOUT_MS)
    {
        if (Serial.available())
        {
            readResult = Serial.read();
            break;
        }
    }

    attempts++;

    if (readResult == -1)
    {
        return false;
    }

    // ASCII 범위 검증
    if (readResult < 32 || readResult > 126)
    {
        if (readResult != '\r' && readResult != '\n' && readResult != '\t')
        {
            return false;
        }
    }

    // 위험 문자 필터링
    char tempChar = static_cast<char>(readResult);
    if (tempChar == '\0' || tempChar == '\x1B')
    {
        return false;
    }

    outChar = tempChar;
    return true;
}

bool InputHandler::processSerialInput(String &outputBuffer)
{
    unsigned long startTime = millis();
    int charCount = 0;
    int readAttempts = 0;
    int consecutiveFailures = 0;
    bool hasInput = false;

    while (charCount < MAX_CHARS_PER_CALL &&
           (millis() - startTime) < MAX_PROCESSING_TIME_MS &&
           consecutiveFailures < MAX_CONSECUTIVE_FAILURES)
    {
        char c;
        bool readSuccess = safeSerialRead(c, readAttempts);

        if (!readSuccess)
        {
            consecutiveFailures++;
            if (consecutiveFailures < MAX_CONSECUTIVE_FAILURES)
            {
                delayMicroseconds(100);
                continue;
            }
            else
            {
                break;
            }
        }

        charCount++;
        readAttempts = 0;
        consecutiveFailures = 0;

        if (c == '\r' || c == '\n')
        {
            if (outputBuffer.length() > 0)
            {
                hasInput = true;
                break;
            }
        }
        else if (isValidMenuChar(c))
        {
            if (outputBuffer.length() < MAX_INPUT_LENGTH)
            {
                outputBuffer += c;
            }
            else
            {
                Serial.println("Error: Input exceeds maximum length. Cleared.");
                outputBuffer = "";
                break;
            }
        }
    }

    return hasInput;
}

void InputHandler::clearInputBuffer()
{
    const int MAX_CLEAR_CHARS = 64;
    const unsigned long MAX_CLEAR_TIME_MS = 5;

    unsigned long startTime = millis();
    int clearCount = 0;
    int consecutiveFailures = 0;
    const int MAX_CONSECUTIVE_FAILURES = 3;

    while (clearCount < MAX_CLEAR_CHARS &&
           (millis() - startTime) < MAX_CLEAR_TIME_MS &&
           consecutiveFailures < MAX_CONSECUTIVE_FAILURES)
    {
        if (!Serial.available())
        {
            break;
        }
        int readResult = Serial.read();
        if (readResult == -1)
        {
            consecutiveFailures++;
            continue;
        }
        consecutiveFailures = 0;
        clearCount++;
    }
}

#endif

bool InputHandler::isValidMenuChar(char c)
{
    return isalnum(c) || c == ',' || c == '-';
}

bool InputHandler::isValidSensorIndex(char c)
{
    return c >= '1' && c <= '8';
}
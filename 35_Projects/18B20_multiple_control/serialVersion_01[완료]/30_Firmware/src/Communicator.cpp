#include "Communicator.h"
#ifdef UNIT_TEST
#include "FakeArduino.h"
#endif

void Communicator::begin()
{
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial)
    {
        // 시리얼 포트가 연결될 때까지 대기 (Leonardo 같은 보드에 필요)
    }
}

void Communicator::print(const char *message)
{
    Serial.print(message);
}

void Communicator::println(const char *message)
{
    Serial.println(message);
}

void Communicator::print(String message)
{
    Serial.print(message);
}

void Communicator::println(String message)
{
    Serial.println(message);
}

void Communicator::println()
{
    Serial.println();
}

void Communicator::print(int value)
{
    Serial.print(String(value));
}

void Communicator::println(int value)
{
    Serial.println(String(value));
}

void Communicator::print(float value)
{
    Serial.print(String(value));
}

void Communicator::println(float value)
{
    Serial.println(String(value));
}

bool Communicator::isSerialAvailable()
{
    return Serial.available() > 0;
}

String Communicator::readStringUntil(char terminator)
{
    return Serial.readStringUntil(terminator);
}

int Communicator::readUserInput(unsigned long timeout)
{
    unsigned long startMillis = millis();
    String input = "";
    while (millis() - startMillis < timeout)
    {
        if (Serial.available() > 0)
        {
            char c = Serial.read();
            if (c == '\n' || c == '\r')
            {
                if (input.length() > 0)
                {
                    return input.toInt();
                }
            }
            else if (isDigit(c))
            {
                input += c;
            }
        }
    }
    return -1; // Timeout
}

String Communicator::readStringInput(unsigned long timeout)
{
    unsigned long startMillis = millis();
    String input = "";
    while (millis() - startMillis < timeout)
    {
        if (Serial.available() > 0)
        {
            char c = Serial.read();
            if (c == '\n' || c == '\r')
            {
                input.trim();
                input.toLowerCase();
                return input;
            }
            else
            {
                input += c;
            }
        }
    }
    return ""; // Timeout
}

static String _nonBlockingLineBuffer = "";

String Communicator::readNonBlockingLine()
{
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '\n' || c == '\r')
        {
            if (_nonBlockingLineBuffer.length() > 0)
            {
                String result = _nonBlockingLineBuffer;
                _nonBlockingLineBuffer = ""; // 버퍼 비우기
                result.trim();
                return result;
            }
            // 빈 줄이거나, 이전 문자가 \r이었고 현재 \n인 경우
            _nonBlockingLineBuffer = ""; // 버퍼 비우기
        }
        else
        {
            _nonBlockingLineBuffer += c;
        }
    }
    return ""; // 아직 완전한 줄이 아님
}

void Communicator::clearInputBuffer()
{
    while (Serial.available())
    {
        Serial.read();
    }
    _nonBlockingLineBuffer = ""; // Clear the non-blocking line buffer as well
}

void Communicator::print(const __FlashStringHelper *message)
{
    Serial.print(message);
}

void Communicator::println(const __FlashStringHelper *message)
{
    Serial.println(message);
}

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

void Communicator::println()
{
    Serial.println();
}

void Communicator::print(int value)
{
    Serial.print(value);
}

void Communicator::println(int value)
{
    Serial.println(value);
}

void Communicator::print(float value)
{
    Serial.print(value);
}

void Communicator::println(float value)
{
    Serial.println(value);
}

bool Communicator::isSerialAvailable()
{
    return Serial.available() > 0;
}

bool Communicator::readUntil(char *buffer, size_t maxLength, char terminator)
{
    size_t length = Serial.readBytesUntil(terminator, buffer, maxLength - 1);
    buffer[length] = '\0';
    return length > 0;
}

int Communicator::readUserInput(unsigned long timeout)
{
    unsigned long startMillis = millis();
    int value = 0;
    bool received = false;
    while (millis() - startMillis < timeout)
    {
        if (Serial.available() > 0)
        {
            char c = Serial.read();
            if (c == '\n' || c == '\r')
            {
                if (received) return value;
            }
            else if (isDigit(c))
            {
                value = value * 10 + (c - '0');
                received = true;
            }
        }
    }
    return -1; // Timeout
}

bool Communicator::readInput(char *buffer, size_t maxLength, unsigned long timeout)
{
    unsigned long startMillis = millis();
    size_t pos = 0;
    while (millis() - startMillis < timeout)
    {
        if (Serial.available() > 0)
        {
            char c = Serial.read();
            if (c == '\n' || c == '\r')
            {
                if (pos > 0)
                {
                    buffer[pos] = '\0';
                    return true;
                }
            }
            else if (pos < maxLength - 1)
            {
                buffer[pos++] = c;
            }
        }
    }
    buffer[pos] = '\0';
    return false; // Timeout
}

static char _nonBlockingLineBuffer[128];
static size_t _nonBlockingPos = 0;

const char* Communicator::readNonBlockingLine()
{
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '\n' || c == '\r')
        {
            if (_nonBlockingPos > 0)
            {
                _nonBlockingLineBuffer[_nonBlockingPos] = '\0';
                _nonBlockingPos = 0; // Reset for next line
                return _nonBlockingLineBuffer;
            }
        }
        else if (_nonBlockingPos < sizeof(_nonBlockingLineBuffer) - 1)
        {
            _nonBlockingLineBuffer[_nonBlockingPos++] = c;
        }
    }
    return nullptr; // Still waiting for full line
}

void Communicator::clearInputBuffer()
{
    while (Serial.available())
    {
        Serial.read();
    }
    _nonBlockingPos = 0;
}

void Communicator::print(const __FlashStringHelper *message)
{
    Serial.print(message);
}

void Communicator::println(const __FlashStringHelper *message)
{
    Serial.println(message);
}

#include <Arduino.h>
#include <cstring>
#include <cctype>
#include "CommandManagerService.h"

CommandManagerService::CommandManagerService(ICommandProcessor *cmdProc, ILogger *log)
    : commandProcessor(cmdProc), logger(log), bufferIndex(0)
{
    memset(inputBuffer, 0, sizeof(inputBuffer));
}

void CommandManagerService::initialize()
{
    if (logger)
    {
        logger->info("Command Manager Service initialized");
    }
    bufferIndex = 0;
}

void CommandManagerService::update()
{
    processSerialInput();
}

// 안전한 Serial 읽기를 위한 헬퍼 함수
static bool safeSerialRead(char &outChar, int &attempts)
{
    const int MAX_READ_ATTEMPTS = 3;

    if (attempts >= MAX_READ_ATTEMPTS)
    {
        return false; // 최대 시도 횟수 초과
    }

    if (!Serial.available())
    {
        return false; // 데이터 없음
    }

    int readResult = Serial.read();
    attempts++;

    if (readResult == -1)
    {
        return false; // 읽기 실패
    }

    // 유효한 ASCII 범위 검증 (보안 강화)
    if (readResult < 0 || readResult > 127)
    {
        return false; // 유효하지 않은 문자
    }

    outChar = static_cast<char>(readResult);
    return true;
}

void CommandManagerService::processSerialInput()
{
    // 안전한 입력 처리를 위한 제한값들
    const int MAX_CHARS_PER_CALL = 32;               // 더 보수적으로 설정
    const unsigned long MAX_PROCESSING_TIME_MS = 10; // 최대 처리 시간 제한

    unsigned long startTime = millis();
    int charCount = 0;
    int readAttempts = 0;

    while (charCount < MAX_CHARS_PER_CALL &&
           (millis() - startTime) < MAX_PROCESSING_TIME_MS)
    {
        char c;
        if (!safeSerialRead(c, readAttempts))
        {
            break; // 안전한 읽기 실패 시 종료
        }

        charCount++;
        readAttempts = 0; // 성공적인 읽기 후 시도 횟수 리셋

        if (c == '\n' || c == '\r')
        {
            if (bufferIndex > 0 && bufferIndex < sizeof(inputBuffer))
            {
                inputBuffer[bufferIndex] = '\0';
                executeCommand(inputBuffer);
                bufferIndex = 0;
            }
        }
        else if (isprint(c) && bufferIndex < sizeof(inputBuffer) - 2) // 더 보수적인 버퍼 검사
        {
            inputBuffer[bufferIndex++] = c;
        }
        else if (bufferIndex >= sizeof(inputBuffer) - 2)
        {
            // 입력이 버퍼 크기를 초과한 경우
            inputBuffer[sizeof(inputBuffer) - 1] = '\0';
            Serial.println("Error: command too long. Buffer overflow prevented.");
            bufferIndex = 0;
            break; // 즉시 종료
        }
    }
}

void CommandManagerService::executeCommand(const char *command)
{
    if (!commandProcessor)
        return;

    if (logger)
    {
        logger->debug("Executing command");
    }

    if (commandProcessor->isValidCommand(command))
    {
        commandProcessor->processCommand(command);
    }
    else
    {
        Serial.println("Invalid command. Type 'help' for available commands.");
    }
}

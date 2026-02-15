#pragma once
#include <Arduino.h>
#include <cstring>
#include "../domain/ICommandProcessor.h"
#include "../domain/ILogger.h"

/**
 * @brief 명령어 관리 서비스
 *
 * 시리얼 명령어 파싱 및 실행을 관리하는 어플리케이션 서비스
 */
class CommandManagerService
{
private:
    ICommandProcessor *commandProcessor;
    ILogger *logger;
    char inputBuffer[64];
    int bufferIndex;

public:
    CommandManagerService(ICommandProcessor *cmdProc, ILogger *log);
    ~CommandManagerService() = default;

    void initialize();
    void update();
    void processSerialInput();
    void executeCommand(const char *command);
};

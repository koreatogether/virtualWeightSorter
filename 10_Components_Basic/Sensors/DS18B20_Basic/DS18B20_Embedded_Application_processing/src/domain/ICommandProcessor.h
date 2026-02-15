#pragma once

/**
 * @brief 명령어 처리 인터페이스
 *
 * 시리얼 명령어 처리를 위한 추상 인터페이스
 */
class ICommandProcessor
{
public:
    virtual ~ICommandProcessor() = default;

    virtual void processCommand(const char *command) = 0;
    virtual bool isValidCommand(const char *command) const = 0;
    virtual void printHelp() const = 0;
    virtual void setResponseCallback(void (*callback)(const char *)) = 0;
};

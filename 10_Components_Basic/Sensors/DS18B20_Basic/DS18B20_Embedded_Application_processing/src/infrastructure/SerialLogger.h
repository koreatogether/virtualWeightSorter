#pragma once
#include <Arduino.h>
#include "../domain/ILogger.h"

/**
 * @brief 시리얼 로거 구현체
 *
 * Arduino Serial을 통한 로깅을 수행하는 구체적 구현
 */
class SerialLogger : public ILogger
{
private:
    LogLevel currentLogLevel;

public:
    SerialLogger();
    ~SerialLogger() = default;

    void log(LogLevel level, const char *message) override;
    void debug(const char *message) override;
    void info(const char *message) override;
    void warning(const char *message) override;
    void error(const char *message) override;
    void setLogLevel(LogLevel level) override;

private:
    const char *getLevelString(LogLevel level) const;
    void printTimestamp() const;
};

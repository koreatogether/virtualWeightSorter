#pragma once

/**
 * @brief 로거 인터페이스
 *
 * 시스템 로깅을 위한 추상 인터페이스
 */
class ILogger
{
public:
    enum LogLevel
    {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };

    virtual ~ILogger() = default;

    virtual void log(LogLevel level, const char *message) = 0;
    virtual void debug(const char *message) = 0;
    virtual void info(const char *message) = 0;
    virtual void warning(const char *message) = 0;
    virtual void error(const char *message) = 0;
    virtual void setLogLevel(LogLevel level) = 0;
};

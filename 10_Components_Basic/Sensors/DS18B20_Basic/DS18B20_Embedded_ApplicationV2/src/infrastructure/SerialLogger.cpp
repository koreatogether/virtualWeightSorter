#include "SerialLogger.h"

SerialLogger::SerialLogger() : currentLogLevel(INFO)
{
}

void SerialLogger::log(LogLevel level, const char *message)
{
    if (level < currentLogLevel)
        return;

    printTimestamp();
    Serial.print("[");
    Serial.print(getLevelString(level));
    Serial.print("] ");
    Serial.println(message);
}

void SerialLogger::debug(const char *message)
{
    log(DEBUG, message);
}

void SerialLogger::info(const char *message)
{
    log(INFO, message);
}

void SerialLogger::warning(const char *message)
{
    log(WARNING, message);
}

void SerialLogger::error(const char *message)
{
    log(ERROR, message);
}

void SerialLogger::setLogLevel(LogLevel level)
{
    currentLogLevel = level;
}

const char *SerialLogger::getLevelString(LogLevel level) const
{
    switch (level)
    {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARN";
    case ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

void SerialLogger::printTimestamp() const
{
    Serial.print(millis());
    Serial.print("ms ");
}

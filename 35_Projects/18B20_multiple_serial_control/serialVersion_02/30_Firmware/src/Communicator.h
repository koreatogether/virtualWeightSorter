#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#ifdef UNIT_TEST
#include "FakeArduino.h"
#else
#include <Arduino.h>
#endif
#include "config.h"

class Communicator
{
public:
    virtual ~Communicator() = default;
    virtual void begin();
    virtual void print(const char *message);
    virtual void println(const char *message);
    virtual void println();
    virtual void print(int value);
    virtual void println(int value);
    virtual void print(float value);
    virtual void println(float value);
    virtual void print(const __FlashStringHelper *message);
    virtual void println(const __FlashStringHelper *message);
    virtual int readUserInput(unsigned long timeout);
    virtual bool readInput(char *buffer, size_t maxLength, unsigned long timeout);
    virtual bool isSerialAvailable();
    virtual bool readUntil(char *buffer, size_t maxLength, char terminator);
    virtual const char* readNonBlockingLine(); // Returns pointer to internal buffer
    virtual void clearInputBuffer();
};

#endif // COMMUNICATOR_H
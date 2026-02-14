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
    virtual void print(String message);
    virtual void println(String message);
    virtual void print(int value);
    virtual void println(int value);
    virtual void print(float value);
    virtual void println(float value);
    virtual void println();
    virtual void print(const __FlashStringHelper *message);
    virtual void println(const __FlashStringHelper *message);
    virtual int readUserInput(unsigned long timeout);
    virtual String readStringInput(unsigned long timeout);
    virtual bool isSerialAvailable();
    virtual String readStringUntil(char terminator);
    virtual String readNonBlockingLine(); // New non-blocking function
    virtual void clearInputBuffer();
};

#endif // COMMUNICATOR_H
#ifndef KEYPADMANAGER_H
#define KEYPADMANAGER_H

#include <Arduino.h>

class KeypadManager
{
public:
    KeypadManager();
    void begin();
    // Reads input into buffer; returns true when a full code was read
    bool readCode(char *buffer, size_t len);
};

#endif // KEYPADMANAGER_H

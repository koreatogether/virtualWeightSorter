#ifndef KEYPADLOCKCONTROLLER_H
#define KEYPADLOCKCONTROLLER_H

#include <Arduino.h>
#include "keypadManager.h"

enum class Mode
{
    IDLE,
    ENTER_PASSWORD,
    ENTER_ADMIN_PASSWORD
};

class KeypadLockController
{
private:
    Mode currentMode;
    KeypadManager keypadManager;
    const String PASSWORD = "123456";
    const String ADMIN_PASSWORD = "000000";

public:
    KeypadLockController();
    void begin();
    void update();
};

#endif // KEYPADLOCKCONTROLLER_H
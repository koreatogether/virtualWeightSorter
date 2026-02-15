#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Arduino.h>

class LedController
{
public:
    LedController();
    void begin();
    void setSuccess(bool on);
    void setFailure(bool on);
    void setAdminMode(bool on);
};

#endif // LEDCONTROLLER_H

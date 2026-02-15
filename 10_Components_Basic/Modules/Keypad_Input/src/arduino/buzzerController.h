#ifndef BUZZERCONTROLLER_H
#define BUZZERCONTROLLER_H

#include <Arduino.h>

class BuzzerController
{
public:
    BuzzerController();
    void begin();
    void shortBeep();
    void longBeep();
};

#endif // BUZZERCONTROLLER_H

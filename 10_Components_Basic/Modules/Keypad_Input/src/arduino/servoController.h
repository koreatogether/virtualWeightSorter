#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H

#include <Arduino.h>

class ServoController
{
public:
    ServoController();
    void begin();
    void moveTo(int angle); // angle in degrees
};

#endif // SERVOCONTROLLER_H

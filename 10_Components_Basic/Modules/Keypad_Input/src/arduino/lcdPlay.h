#ifndef LCDPLAY_H
#define LCDPLAY_H

#include <Arduino.h>

class LcdPlay
{
public:
    LcdPlay();
    void begin();
    void show(const char *text);
    void clear();
};

#endif // LCDPLAY_H

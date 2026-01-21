#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "Config.h"
#include <LiquidCrystal_PCF8574.h>

extern LiquidCrystal_PCF8574 lcd;

void initDisplay();
void updateDisplay(uint8_t mode); // mode 0: Diff, mode 1: Individual
void showSensorShortAddresses(unsigned long durationMs = 4000);

#endif // DISPLAY_MANAGER_H

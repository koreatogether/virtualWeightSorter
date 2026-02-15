#ifndef BLYNK_MANAGER_H
#define BLYNK_MANAGER_H

#include "Config.h"

// Blynk 인터페이스 함수
void initBlynk();
void blynkSetInterval(unsigned long interval, void (*callback)());
void runBlynk();
void sendDataToBlynk(float *temps, uint8_t count);

#endif // BLYNK_MANAGER_H

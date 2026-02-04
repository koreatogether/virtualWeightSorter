#ifndef NEXTION_MANAGER_H
#define NEXTION_MANAGER_H

#include "Config.h"
#include <Nextion.h>
#include <RTC.h>

void initNextion();
void updateNextionDisplay();
void processNextionInput();

#endif // NEXTION_MANAGER_H

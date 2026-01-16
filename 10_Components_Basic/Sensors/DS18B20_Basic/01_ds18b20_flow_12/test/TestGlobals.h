#ifndef TEST_GLOBALS_H
#define TEST_GLOBALS_H

#include "MockDS18B20_Sensor.h"
#include "MockCommunicator.h"
#include "MockDataProcessor.h"
#include "../App.h"

extern MockDS18B20_Sensor* mockSensorManager;
extern MockCommunicator* mockComm;
extern MockDataProcessor* mockDataProcessor;
extern App* app;

extern void setUp(void);
extern void tearDown(void);

#endif // TEST_GLOBALS_H

#ifndef SERIAL_UI_H
#define SERIAL_UI_H

#include <Arduino.h>
#include "Communicator.h"
#include "DataProcessor.h"
#include "DS18B20_Sensor.h"

class SerialUI
{
public:
    SerialUI(Communicator* comm, DataProcessor* dp, DS18B20_Sensor* sm);

    void displayMainMenu();
    void displayThresholdMenu();
    void displayIdChangeMenu();
    void displaySensorTable();
    void displayCurrentThresholds();
    void displayTroubleshootingGuide();
    void displayMemoryAnalysisHelp();
    void displayGeneralHelp();
    
    void printCommandHelp();
    void printSensorReport(int deviceCount);
    void printTimeoutMessage();
    void printCalibrationStart();
    
private:
    Communicator* _comm;
    DataProcessor* _dp;
    DS18B20_Sensor* _sm;
};

#endif // SERIAL_UI_H

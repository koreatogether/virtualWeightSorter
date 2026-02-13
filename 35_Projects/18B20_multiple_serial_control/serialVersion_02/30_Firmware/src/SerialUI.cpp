#include "SerialUI.h"

SerialUI::SerialUI(Communicator* comm, DataProcessor* dp, DS18B20_Sensor* sm)
    : _comm(comm), _dp(dp), _sm(sm) {}

void SerialUI::displayMainMenu()
{
    _comm->println();
    _comm->println(F("========================================"));
    _comm->println(F("   DS18B20 Multi-Sensor Controller      "));
    _comm->println(F("========================================"));
    _comm->println(F("1. View Sensor Status Table"));
    _comm->println(F("2. Change Sensor Internal IDs (Dallas User Data)"));
    _comm->println(F("3. Temperature Threshold Settings"));
    _comm->println(F("4. Re-search Sensors"));
    _comm->println(F("5. System Information"));
    _comm->println(F("6. Normal Operation Mode"));
    _comm->println(F("----------------------------------------"));
    _comm->println(F("Enter your choice (1-6):"));
}

void SerialUI::displayIdChangeMenu()
{
    _comm->println();
    _comm->println(F("--- Sensor ID Management Menu ---"));
    _comm->println(F("1. Edit Individual Sensor ID"));
    _comm->println(F("2. Edit Multiple Selective IDs"));
    _comm->println(F("3. Auto-Assign Unique IDs (1, 2, 3...)"));
    _comm->println(F("4. Reset All Internal IDs to 0 (Uninitialized)"));
    _comm->println(F("5. Back to Main Menu"));
    _comm->println(F("---------------------------------"));
    _comm->println(F("Enter choice (1-5):"));
}

void SerialUI::displayThresholdMenu()
{
    _comm->println();
    _comm->println(F("--- Temperature Threshold Menu ---"));
    _comm->println(F("1. Set Individual Sensor Threshold"));
    _comm->println(F("2. Set Global Threshold (All Sensors)"));
    _comm->println(F("3. View Current Thresholds"));
    _comm->println(F("4. Clear All Thresholds"));
    _comm->println(F("5. Back to Main Menu"));
    _comm->println(F("----------------------------------"));
    _comm->println(F("Enter choice (1-5):"));
}

void SerialUI::displaySensorTable()
{
    _dp->printSensorTable();
    if (_dp->hasInvalidSensors())
    {
        _dp->printInvalidSensorTable();
    }
}

void SerialUI::displayCurrentThresholds()
{
    _comm->println();
    _comm->println(F("--- Current Threshold Settings ---"));
    int found = _sm->getDeviceCount();
    if (found == 0)
    {
        _comm->println(F("No sensors found."));
        return;
    }

    _comm->println(F("Sensor | ID | High Temp | Low Temp"));
    _comm->println(F("-------|----|-----------|---------"));
    for (int i = 0; i < found; i++)
    {
        int id = _sm->getUserDataByIndex(i);
        if (id > 0)
        {
            int8_t highTemp = _dp->getTh(id);
            int8_t lowTemp = _dp->getTl(id);

            _comm->print(F("   "));
            _comm->print(i + 1);
            _comm->print(F("   | "));
            _comm->print(id);
            _comm->print(F("  |   "));
            _comm->print(highTemp);
            _comm->print(F("°C   |  "));
            _comm->print(lowTemp);
            _comm->println(F("°C"));
        }
    }
    _comm->println(F("-------------------------------"));
}

void SerialUI::displayTroubleshootingGuide()
{
    _comm->println();
    _comm->println(F("--- DS18B20 Troubleshooting Guide ---"));
    _comm->println(F("1. Check Wiring: VCC(5V), GND, DQ(D2)"));
    _comm->println(F("2. 4.7k Ohm Pull-up Resistor: Must be between VCC and DQ"));
    _comm->println(F("3. External Power: Parasite power mode can be unstable"));
    _comm->println(F("4. Address Conflict: Ensure each sensor has a unique ID"));
    _comm->println(F("5. Data corruption: Check for long cables or EMI"));
    _comm->println(F("--------------------------------------"));
}

void SerialUI::displayGeneralHelp()
{
    _comm->println(F("=== AVAILABLE COMMANDS ==="));
    _comm->println(F("General Commands:"));
    _comm->println(F("  help, h    - Show this help menu"));
    _comm->println(F("  menu, m    - Open DS18B20 configuration menu"));
    _comm->println(F("  id, i      - View ID management menu"));
    _comm->println(F("  th, t      - View threshold menu"));
#if MEMORY_ANALYSIS_ENABLED
    _comm->println(F("Memory Analysis (Debug):"));
    _comm->println(F("  memfree    - Show current memory status"));
    _comm->println(F("  memtoggle  - Toggle periodic memory checks"));
#endif
    _comm->println(F("============================"));
}

void SerialUI::printSensorReport(int deviceCount)
{
    _comm->println();
    _comm->print(F("--- Sensor Status Report ("));
    _comm->print(deviceCount);
    _comm->println(F(" sensors detected) ---"));
    _dp->printSensorTable();
    if (_dp->hasInvalidSensors())
    {
        _dp->printInvalidSensorTable();
    }
}

void SerialUI::printTimeoutMessage()
{
    _comm->println(F("⏰ Request timeout. Returning to main loop."));
}

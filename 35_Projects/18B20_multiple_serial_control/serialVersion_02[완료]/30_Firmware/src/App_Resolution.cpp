#include "App.h"
#include "DS18B20_Sensor.h"
#include "DataProcessor.h"
#include "Communicator.h"
#include "SerialUI.h"

void App::displayResolutionMenu()
{
    if (forceMenuPrint || (millis() - lastMenuPrintMillis >= MENU_REPRINT_INTERVAL))
    {
        _ui->displayResolutionMenu();
        lastMenuPrintMillis = millis();
        forceMenuPrint = false;
    }

    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _ui->printTimeoutMessage();
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
    }
}

void App::processResolutionMenuInput()
{
    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr)
    {
        int num = atoi(input);
        if (num >= 1 && num <= 3)
        {
            switch (num)
            {
            case 1:
                currentAppState = SET_INDIVIDUAL_RESOLUTION;
                currentResSubState = RES_NONE;
                break;
            case 2:
                currentAppState = SET_GLOBAL_RESOLUTION;
                currentResSubState = RES_NONE;
                break;
            case 3:
                _comm->println(F("Returning to main menu."));
                currentAppState = MAIN_MENU;
                break;
            }
            forceMenuPrint = true;
            menuDisplayMillis = millis();
            lastMenuPrintMillis = millis();
        }
        else
        {
            _comm->println(F("🧼 Invalid input. Enter 1, 2, or 3."));
        }
    }
}

void App::processIndividualResolution()
{
    if (currentResSubState == RES_NONE)
    {
        _comm->println();
        _comm->println(F("--- Individual Resolution Setting ---"));
        int found = _sensorManager->getDeviceCount();
        if (found == 0)
        {
            _comm->println(F("No sensors found. Returning to menu."));
            currentAppState = RESOLUTION_MENU;
            return;
        }

        _ui->printSensorListWithCurrentSettings();
        _comm->print(F("Enter sensor number (1-"));
        _comm->print(found);
        _comm->println(F(") or 'c' to cancel:"));
        currentResSubState = RES_SELECT_SENSOR;
        menuDisplayMillis = millis();
        return;
    }

    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr)
    {
        if (strcmp(input, "c") == 0 || strcasecmp(input, "cancel") == 0)
        {
            _comm->println(F("Cancelled. Returning to menu."));
            currentAppState = RESOLUTION_MENU;
            return;
        }

        if (currentResSubState == RES_SELECT_SENSOR)
        {
            int sensorNum = atoi(input);
            int found = _sensorManager->getDeviceCount();
            if (sensorNum >= 1 && sensorNum <= found)
            {
                selectedSensorIndex = sensorNum - 1;
                _comm->print(F("Selected Sensor "));
                _comm->println(sensorNum);
                _comm->println(F("Enter resolution (9, 10, 11, or 12 bits):"));
                currentResSubState = RES_SELECT_BITS;
                menuDisplayMillis = millis();
            }
            else
            {
                _comm->println(F("Invalid sensor number. Try again:"));
            }
        }
        else if (currentResSubState == RES_SELECT_BITS)
        {
            int res = atoi(input);
            if (res >= 9 && res <= 12)
            {
                DeviceAddress addr;
                if (_sensorManager->getAddress(addr, selectedSensorIndex))
                {
                    _sensorManager->setResolution(addr, res);
                    _comm->print(F("✅ Resolution set to "));
                    _comm->print(res);
                    _comm->println(F(" bits for selected sensor."));
                }
                currentAppState = RESOLUTION_MENU;
                forceMenuPrint = true;
            }
            else
            {
                _comm->println(F("Invalid resolution. Enter 9, 10, 11, or 12:"));
            }
        }
    }
}

void App::processGlobalResolution()
{
    if (currentResSubState == RES_NONE)
    {
        _comm->println();
        _comm->println(F("--- Global Resolution Setting ---"));
        _comm->println(F("Enter resolution for ALL sensors (9, 10, 11, or 12 bits) or 'c' to cancel:"));
        currentResSubState = RES_SELECT_BITS;
        menuDisplayMillis = millis();
        return;
    }

    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr)
    {
        if (strcmp(input, "c") == 0 || strcasecmp(input, "cancel") == 0)
        {
            _comm->println(F("Cancelled."));
            currentAppState = RESOLUTION_MENU;
            return;
        }

        int res = atoi(input);
        if (res >= 9 && res <= 12)
        {
            _sensorManager->setResolution(res);
            _comm->print(F("✅ Global resolution set to "));
            _comm->print(res);
            _comm->println(F(" bits for all sensors."));
            currentAppState = RESOLUTION_MENU;
            forceMenuPrint = true;
        }
        else
        {
            _comm->println(F("Invalid resolution. Enter 9, 10, 11, or 12:"));
        }
    }
}

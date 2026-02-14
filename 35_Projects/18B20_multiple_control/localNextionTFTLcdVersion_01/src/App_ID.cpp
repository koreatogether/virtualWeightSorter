#include "App.h"
#include "DS18B20_Sensor.h"
#include "DataProcessor.h"
#include "Communicator.h"
#include "SerialUI.h"
#include "Utils.h"

void App::openIdManagementMenu()
{
    if (forceMenuPrint || (millis() - lastMenuPrintMillis >= MENU_REPRINT_INTERVAL))
    {
        _ui->displayIdChangeMenu();
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

void App::handleIdMenuInput()
{
    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr)
    {
        int num = atoi(input);
        if (num >= 1 && num <= 5)
        {
            switch (num)
            {
            case 1:
                currentAppState = EDIT_INDIVIDUAL_ID;
                break;
            case 2:
                currentAppState = EDIT_SELECTIVE_ID;
                break;
            case 3:
                currentAppState = AUTO_ASSIGN_IDS;
                break;
            case 4:
                currentAppState = RESET_ALL_IDS;
                break;
            case 5:
                _comm->println(F("Returning to main menu."));
                currentAppState = MAIN_MENU;
                forceMenuPrint = true;
                menuDisplayMillis = millis();
                lastMenuPrintMillis = millis();
                break;
            }
            if (currentAppState != MAIN_MENU && currentAppState != NORMAL_OPERATION)
            {
                currentEditIdSubState = EDIT_ID_NONE;
                menuDisplayMillis = millis();
                lastMenuPrintMillis = millis();
            }
        }
        else
        {
            _comm->println(F("🧼 Invalid input. Enter a number between 1 and 5."));
        }
    }
}

void App::processEditIndividualId()
{
    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _comm->println("⏰ Menu timeout. Returning to normal operation.");
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
        return;
    }

    if (currentEditIdSubState == EDIT_ID_NONE)
    {
        _comm->println();
        _comm->println("--- Individual Sensor ID Change ---");
        int found = _sensorManager->getDeviceCount();
        bool hasValidSensors = false;
        for (int i = 0; i < found; i++)
        {
            DeviceAddress addr;
            _sensorManager->getAddress(addr, i);
            int id = _sensorManager->getUserData(addr);
            if (id >= 1 && id <= SENSOR_COUNT)
            {
                hasValidSensors = true;
                break;
            }
        }
        if (!hasValidSensors)
        {
            _comm->println("No sensors with valid IDs found. Please assign IDs first using menu option 3 or 4.");
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            printCurrentSensorReport();
            return;
        }
        _dataProcessor->printSensorTable();
        _comm->print(F("Enter the number of the sensor you want to change (1-"));
        _comm->print(SENSOR_COUNT);
        _comm->println(F("). 0 to cancel."));
        currentEditIdSubState = EDIT_ID_SELECT_SENSOR;
    }

    if (currentEditIdSubState == EDIT_ID_SELECT_SENSOR)
    {
        const char* input = _comm->readNonBlockingLine();
        if (input != nullptr && strlen(input) > 0)
        {
            int sensorNum = atoi(input);
            if (sensorNum == 0)
            {
                _comm->println(F("Cancelling individual ID change."));
                currentAppState = NORMAL_OPERATION;
                _comm->clearInputBuffer();
                previousMillis = 0;
                printCurrentSensorReport();
                return;
            }
            if (sensorNum >= 1 && sensorNum <= SENSOR_COUNT)
            {
                DeviceAddress targetAddr;
                bool foundSensorInSlot = false;
                int foundSensorsCount = _sensorManager->getDeviceCount();
                for (int i = 0; i < foundSensorsCount; i++)
                {
                    DeviceAddress currentAddr;
                    _sensorManager->getAddress(currentAddr, i);
                    if (_sensorManager->getUserData(currentAddr) == sensorNum)
                    {
                        memcpy(targetAddr, currentAddr, 8);
                        foundSensorInSlot = true;
                        break;
                    }
                }
                if (foundSensorInSlot)
                {
                    memcpy(sensorsToChangeArray[0].addr, targetAddr, 8);
                    sensorsToChangeArray[0].currentId = _sensorManager->getUserData(targetAddr);
                    sensorsToChangeArray[0].index = sensorNum;
                    _comm->print(F("You selected sensor No. "));
                    _comm->print(sensorNum);
                    _comm->print(F(" (Current ID: "));
                    _comm->print(sensorsToChangeArray[0].currentId);
                    _comm->println(F(")."));
                    _dataProcessor->printAvailableIds();
                    _comm->print(F("Enter new ID (1-"));
                    _comm->print(SENSOR_COUNT);
                    _comm->print(F("): "));
                    currentEditIdSubState = EDIT_ID_ENTER_NEW_ID;
                }
                else
                {
                    _comm->print(F("No active sensor found in slot "));
                    _comm->print(sensorNum);
                    _comm->println(F(". Please select an active sensor."));
                }
            }
            else
            {
                _comm->println(F("Invalid sensor number. Please try again."));
            }
        }
    }

    if (currentEditIdSubState == EDIT_ID_ENTER_NEW_ID)
    {
        const char* input = _comm->readNonBlockingLine();
        if (input != nullptr && strlen(input) > 0)
        {
            int newId = atoi(input);
            if (newId < 1 || newId > SENSOR_COUNT || _sensorManager->isIdUsed(newId))
            {
                _comm->println(F("Invalid or already used ID. Please try again."));
                editIdTimeoutMillis = millis();
                return;
            }
            DeviceAddress addr;
            memcpy(addr, sensorsToChangeArray[0].addr, 8);
            _sensorManager->setUserData(addr, newId);
            delay(20);
            _comm->print(F("Sensor No. "));
            _comm->print(sensorsToChangeArray[0].index);
            _comm->print(F(" ID changed from "));
            _comm->print(sensorsToChangeArray[0].currentId);
            _comm->print(F(" to "));
            _comm->print(newId);
            _comm->println(F("."));
            _dataProcessor->printSensorTable();
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            previousMillis = 0;
            printCurrentSensorReport();
        }
    }
}

void App::processEditSelectiveId()
{
    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _comm->println(F("⏰ Menu timeout. Returning to normal operation."));
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
        return;
    }

    if (currentEditIdSubState == EDIT_ID_NONE)
    {
        _comm->println();
        _comm->println(F("--- Selective Sensor ID Change ---"));
        int found = _sensorManager->getDeviceCount();
        bool hasValidSensors = false;
        for (int i = 0; i < found; i++)
        {
            DeviceAddress addr;
            _sensorManager->getAddress(addr, i);
            int id = _sensorManager->getUserData(addr);
            if (id >= 1 && id <= SENSOR_COUNT)
            {
                hasValidSensors = true;
                break;
            }
        }
        if (!hasValidSensors)
        {
            _comm->println(F("No sensors with valid IDs found. Please assign IDs first using menu option 3 or 4."));
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            printCurrentSensorReport();
            return;
        }
        _dataProcessor->printSensorTable();
        _comm->println(F("Enter the numbers of the sensors you want to change (e.g., 1 3 5 or 2,4,6). 0 to cancel."));
        currentEditIdSubState = EDIT_SELECTIVE_GET_SENSORS;
        editIdTimeoutMillis = millis();
    }

    if (currentEditIdSubState == EDIT_SELECTIVE_GET_SENSORS)
    {
        if (millis() - editIdTimeoutMillis > UI_TIMEOUT)
        {
            _comm->println(F("⏰ Input timeout. Cancelling."));
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            return;
        }
        const char* input = _comm->readNonBlockingLine();
        if (input != nullptr && strlen(input) > 0)
        {
            if (strcmp(input, "0") == 0)
            {
                _comm->println(F("Cancelling."));
                currentAppState = NORMAL_OPERATION;
                _comm->clearInputBuffer();
                previousMillis = 0;
                printCurrentSensorReport();
                return;
            }
            parseSensorSelection(input);
            if (selectedSensorIndicesCount == 0)
            {
                _comm->println(F("No valid sensors selected. Please try again."));
                currentEditIdSubState = EDIT_ID_NONE;
                return;
            }
            _comm->print(F("Selected sensors: "));
            for (int i = 0; i < selectedSensorIndicesCount; i++)
            {
                _comm->print(sensorsToChangeArray[selectedSensorIndices[i]].index);
                if (i < selectedSensorIndicesCount - 1)
                    _comm->print(F(", "));
            }
            _comm->println();
            _comm->print(F("Proceed with these selections? (y/n): "));
            currentEditIdSubState = EDIT_SELECTIVE_CONFIRM;
            editIdTimeoutMillis = millis();
        }
    }

    if (currentEditIdSubState == EDIT_SELECTIVE_CONFIRM)
    {
        if (millis() - editIdTimeoutMillis > UI_TIMEOUT)
        {
            _comm->println(F("⏰ Input timeout. Cancelling."));
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            return;
        }
        const char* input = _comm->readNonBlockingLine();
        if (input != nullptr && strlen(input) > 0)
        {
            if (strcasecmp(input, "y") == 0)
            {
                currentSelectiveIndex = 0;
                currentEditIdSubState = EDIT_SELECTIVE_ENTER_NEW_IDS;
                _dataProcessor->printAvailableIds();
                int sensorTableIndex = sensorsToChangeArray[selectedSensorIndices[currentSelectiveIndex]].index;
                _comm->print(F("Enter new ID for sensor No. "));
                _comm->print(sensorTableIndex);
                _comm->print(F(": "));
                editIdTimeoutMillis = millis();
            }
            else
            {
                _comm->println(F("Cancelled. Restarting selection."));
                currentEditIdSubState = EDIT_ID_NONE;
                _comm->clearInputBuffer();
                previousMillis = 0;
                printCurrentSensorReport();
            }
        }
    }

    if (currentEditIdSubState == EDIT_SELECTIVE_ENTER_NEW_IDS)
    {
        if (millis() - editIdTimeoutMillis > UI_TIMEOUT)
        {
            _comm->println(F("⏰ Input timeout. Cancelling."));
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            return;
        }
        const char* input = _comm->readNonBlockingLine();
        if (input != nullptr && strlen(input) > 0)
        {
            int newId = atoi(input);
            if (newId < 1 || newId > SENSOR_COUNT || _sensorManager->isIdUsed(newId))
            {
                _comm->println(F("Invalid or already used ID. Please try again."));
                editIdTimeoutMillis = millis();
                return;
            }
            int arrayIndex = selectedSensorIndices[currentSelectiveIndex];
            DeviceAddress addr;
            memcpy(addr, sensorsToChangeArray[arrayIndex].addr, 8);
            _sensorManager->setUserData(addr, newId);
            delay(20);
            int sensorTableIndex = sensorsToChangeArray[arrayIndex].index;
            _comm->print(F("Sensor No. "));
            _comm->print(sensorTableIndex);
            _comm->print(F(" ID changed to "));
            _comm->println(newId);
            currentSelectiveIndex++;
            if (currentSelectiveIndex >= selectedSensorIndicesCount)
            {
                _comm->println(F("All selected sensors have been updated."));
                _dataProcessor->printSensorTable();
                currentAppState = NORMAL_OPERATION;
                previousMillis = 0;
            }
            else
            {
                _dataProcessor->printAvailableIds();
                int nextSensorTableIndex = sensorsToChangeArray[selectedSensorIndices[currentSelectiveIndex]].index;
                _comm->print(F("Enter new ID for sensor No. "));
                _comm->print(nextSensorTableIndex);
                _comm->print(F(": "));
                editIdTimeoutMillis = millis();
            }
        }
    }
}

void App::processAutoAssignIds()
{
    static int foundSensors = 0;
    static DeviceAddress addrs[SENSOR_COUNT]; 

    if (currentProcessIndex == -1) // 초기화 상태
    {
        _comm->println();
        _comm->println(F("--- Auto-assigning IDs by Address order ---"));
        foundSensors = _sensorManager->getDeviceCount();
        if (foundSensors == 0)
        {
            _comm->println(F("No sensors found. Returning to normal operation."));
            currentAppState = NORMAL_OPERATION;
            return;
        }

        for (int i = 0; i < foundSensors; i++)
        {
            _sensorManager->getAddress(addrs[i], i);
        }

        // 정렬
        for (int i = 0; i < foundSensors - 1; i++)
        {
            for (int j = 0; j < foundSensors - i - 1; j++)
            {
                if (memcmp(addrs[j], addrs[j + 1], 8) > 0)
                {
                    DeviceAddress temp;
                    memcpy(temp, addrs[j], 8);
                    memcpy(addrs[j + 1], addrs[j], 8);
                    memcpy(addrs[j + 1], temp, 8);
                }
            }
        }

        _comm->print(F("Found "));
        _comm->print(foundSensors);
        _comm->println(F(" sensors. Starting assignment..."));
        currentProcessIndex = 0;
        processLastMillis = millis();
        return;
    }

    if (currentProcessIndex < foundSensors)
    {
        if (millis() - processLastMillis >= 100)
        {
            _sensorManager->setUserData(addrs[currentProcessIndex], currentProcessIndex + 1);
            _comm->print(F(" Assigned ID "));
            _comm->print(currentProcessIndex + 1);
            _comm->print(F(" to sensor "));
            char addrStr[24] = "";
            addrToString(addrs[currentProcessIndex], addrStr);
            _comm->println(addrStr);
            
            currentProcessIndex++;
            processLastMillis = millis();
        }
    }
    else
    {
        _comm->println(F("✅ Auto-assignment complete."));
        _ui->displaySensorTable();
        currentAppState = NORMAL_OPERATION;
        previousMillis = 0;
        currentProcessIndex = -1; // 초기화
    }
}

void App::resetAllSensorIds()
{
    static int foundSensors = 0;

    if (currentProcessIndex == -1) // 초기화 상태
    {
        _comm->println();
        _comm->println(F("--- Resetting all sensor IDs ---"));
        foundSensors = _sensorManager->getDeviceCount();
        if (foundSensors == 0)
        {
            _comm->println(F("No sensors found."));
            currentAppState = NORMAL_OPERATION;
            return;
        }
        currentProcessIndex = 0;
        processLastMillis = millis();
        return;
    }

    if (currentProcessIndex < foundSensors)
    {
        if (millis() - processLastMillis >= 100)
        {
            DeviceAddress addr;
            if (_sensorManager->getAddress(addr, currentProcessIndex))
            {
                _sensorManager->setUserData(addr, 0);
                _comm->print(F(" Reset sensor "));
                _comm->print(currentProcessIndex + 1);
                _comm->println();
            }
            currentProcessIndex++;
            processLastMillis = millis();
        }
    }
    else
    {
        _comm->println(F("✅ All sensor IDs have been reset to 0."));
        _comm->println(F("You can now use 'Auto-assign' or other menu options to set new IDs."));
        _comm->println(F("--- Connected Sensors (Address List) ---"));
        _dataProcessor->printInvalidSensorTable();

        currentAppState = NORMAL_OPERATION;
        previousMillis = 0;
        currentProcessIndex = -1; // 초기화
    }
}

void App::parseSensorSelection(const char* input)
{
    sensorsToChangeCount = 0;
    int totalFoundSensors = _sensorManager->getDeviceCount();

    for (int i = 0; i < totalFoundSensors; i++)
    {
        DeviceAddress addr;
        _sensorManager->getAddress(addr, i);
        int currentId = _sensorManager->getUserData(addr);
        if (currentId >= 1 && currentId <= SENSOR_COUNT)
        {
            sensorsToChangeArray[sensorsToChangeCount].index = currentId;
            sensorsToChangeArray[sensorsToChangeCount].currentId = currentId;
            memcpy(sensorsToChangeArray[sensorsToChangeCount].addr, addr, 8);
            sensorsToChangeCount++;
        }
    }

    selectedSensorIndicesCount = 0;
    bool isSelected[SENSOR_COUNT + 1];
    memset(isSelected, 0, sizeof(isSelected));

    const char* ptr = input;
    char numBuf[16];
    size_t numPos = 0;

    while (true)
    {
        char c = *ptr;
        if (isdigit(c))
        {
            if (numPos < sizeof(numBuf) - 1)
                numBuf[numPos++] = c;
        }
        else
        {
            if (numPos > 0)
            {
                numBuf[numPos] = '\0';
                int id = atoi(numBuf);
                if (id >= 1 && id <= SENSOR_COUNT)
                {
                    for (int j = 0; j < sensorsToChangeCount; j++)
                    {
                        if (sensorsToChangeArray[j].currentId == id)
                        {
                            isSelected[id] = true;
                            break;
                        }
                    }
                }
                numPos = 0;
            }
            if (c == '\0') break;
        }
        ptr++;
    }

    for (int i = 1; i <= SENSOR_COUNT; i++)
    {
        if (isSelected[i])
        {
            for (int j = 0; j < sensorsToChangeCount; j++)
            {
                if (sensorsToChangeArray[j].currentId == i)
                {
                    selectedSensorIndices[selectedSensorIndicesCount++] = j;
                    break;
                }
            }
        }
    }
}

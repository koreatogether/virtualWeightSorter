#include "App.h"
#include "DS18B20_Sensor.h"
#include "DataProcessor.h"
#include "Communicator.h"
#include "SerialUI.h"
#include "Utils.h"

#ifdef UNIT_TEST
#include "test/MockMemoryUsageTester.h"
#elif defined(MEMORY_ANALYSIS_ENABLED) && MEMORY_ANALYSIS_ENABLED == 1
#include "MemoryUsageTester.h"
#endif

// PROGMEM에 저장할 상수 문자열들
const char PROGMEM NEW_REPORT_HEADER[] = "\n\n==================== NEW REPORT ====================";
const char PROGMEM INVALID_SENSORS_BORDER[] = "---------------------------------------------------------------------------------------------";
const char PROGMEM INVALID_SENSORS_MSG[] = "Currently identified invalid ID sensors are as follows:";
const char PROGMEM MENU_CALL_MSG[] = "Calling ID change menu...";
const char PROGMEM HELP_BORDER[] = "-----------------------------";
const char PROGMEM AVAILABLE_COMMANDS[] = "사용 가능한 명령어:";
const char PROGMEM MENU_COMMAND[] = "menu or m : Call ID change menu";
const char PROGMEM HELP_COMMAND[] = "help 또는 h : 도움말 표시";
const char PROGMEM UNKNOWN_COMMAND[] = "알 수 없는 명령어: ";

const char PROGMEM SENSOR_MENU_BORDER[] = "-------------------------------";
const char PROGMEM SENSOR_MENU_TITLE[] = "Sensor ID Change Menu:";
const char PROGMEM MENU_ITEM_1[] = "1. Change Individual Sensor ID";
const char PROGMEM MENU_ITEM_2[] = "2. Change Selective Sensor IDs";
const char PROGMEM MENU_ITEM_3[] = "3. Auto-assign IDs by Address";
const char PROGMEM MENU_ITEM_4[] = "4. Reset All Sensor IDs";
const char PROGMEM MENU_ITEM_5[] = "5. Cancel";
const char PROGMEM MENU_ITEM_6[] = "6. Threshold Settings";
const char PROGMEM MENU_PROMPT[] = "Enter the number of the desired menu and press Enter.";
#ifdef UNIT_TEST
#include <cstring> // memcpy, memcmp를 위해 추가
#else
#include <string.h> // memcpy, memcmp를 위해 추가
#endif

App::App(DS18B20_Sensor *sensorManager, DataProcessor *dataProcessor, Communicator *comm, SerialUI *ui, void *memoryTester)
    : _sensorManager(sensorManager),
      _dataProcessor(dataProcessor),
      _comm(comm),
      _ui(ui),
      _memoryTester(memoryTester),
      currentAppState(NORMAL_OPERATION),
      currentEditIdSubState(EDIT_ID_NONE),
      currentThresholdSubState(THRESHOLD_NONE),
      sensorsToChangeCount(0),
      selectedSensorIndex(-1),
      editIdTimeoutMillis(0),
      selectedSensorIndicesCount(0),
      currentSelectiveIndex(0),
      previousMillis(0),
      menuDisplayMillis(0),
      forceMenuPrint(false),
      selectedThresholdSensorIndex(-1),
      pendingHighThreshold(0.0),
      pendingLowThreshold(0.0),
      hasHighThreshold(false),
      hasLowThreshold(false),
      sensorSearchStartTime(0),
      sensorSearchAttempts(0),
      isInSensorSearchMode(false),
      currentProcessIndex(-1),
      processLastMillis(0) {}

void App::init()
{
    initializeSerial();
    initialize18b20();
    showDeviceTable();
    currentAppState = NORMAL_OPERATION;
}

void App::run()
{
    // 센서 검색 모드 처리
    if (isInSensorSearchMode)
    {
        performSensorSearch();
        return;
    }

    switch (currentAppState)
    {
    case NORMAL_OPERATION:
        show18b20Table_loop();
        handleUserCommands();
        break;
    case MAIN_MENU:
        openMainMenu();
        handleMainMenuInput();
        break;
    case ID_MANAGEMENT_MENU:
        openIdManagementMenu();
        handleIdMenuInput();
        break;
    case EDIT_INDIVIDUAL_ID:
        processEditIndividualId();
        break;
    case EDIT_SELECTIVE_ID:
        processEditSelectiveId();
        break;
    case AUTO_ASSIGN_IDS:
        processAutoAssignIds();
        break;
    case RESET_ALL_IDS:
        resetAllSensorIds();
        break;
    case THRESHOLD_MENU:
        displayThresholdMenu();
        processThresholdMenuInput();
        break;
    case SET_INDIVIDUAL_THRESHOLD:
        processIndividualThreshold();
        break;
    case SET_GLOBAL_THRESHOLD:
        processGlobalThreshold();
        break;
    }
}

void App::initializeSerial()
{
    _comm->begin();
    _comm->println(F("Serial communication initialized."));
    log_info("Serial communication started");
}

void App::initialize18b20()
{
    _sensorManager->begin();
    int found = _sensorManager->getDeviceCount();
    
    _comm->print(F("Scanning OneWire bus... Found "));
    _comm->print(found);
    _comm->println(F(" sensor(s)."));
    
    if (found == 0)
    {
        log_error("No DS18B20 sensors found physically. Starting sensor search mode...");
        startSensorSearchMode();
    }
    else if (_dataProcessor->hasInvalidSensors())
    {
        _comm->println(F("⚠️ Some sensors found but lack valid IDs (1-8)."));
        _comm->println(F("Please use the menu to assign IDs."));
    }
}

void App::showDeviceTable()
{
    _ui->displaySensorTable();
}

void App::show18b20Table_loop()
{
    if (millis() - previousMillis >= SENSOR_READ_INTERVAL)
    {
        previousMillis = millis();
        printCurrentSensorReport();
    }
}

void App::printCurrentSensorReport()
{
    int found = _sensorManager->getDeviceCount();
    _ui->printSensorReport(found);
}

void App::handleUserCommands()
{
    const char* command = _comm->readNonBlockingLine();
    if (command != nullptr)
    {
        if (strcmp(command, "menu") == 0 || strcmp(command, "m") == 0)
        {
            _comm->println(F("Calling main menu..."));
            currentAppState = MAIN_MENU;
            _comm->clearInputBuffer();
            forceMenuPrint = true;
            menuDisplayMillis = millis();
        }
        else if (strcmp(command, "help") == 0 || strcmp(command, "h") == 0)
        {
            _ui->displayGeneralHelp();
        }
        else if (strlen(command) > 0)
        {
            _comm->print(F("알 수 없는 명령어: "));
            _comm->println(command);
        }
    }
}

void App::showTroubleshootingGuide()
{
    _ui->displayTroubleshootingGuide();
}

void App::openMainMenu()
{
    if (forceMenuPrint || (millis() - menuDisplayMillis >= MENU_REPRINT_INTERVAL))
    {
        _ui->displayMainMenu();
        menuDisplayMillis = millis();
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

void App::handleMainMenuInput()
{
    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr && strlen(input) > 0)
    {
        int num = atoi(input);
        if (num >= 1 && num <= 6)
        {
            switch (num)
            {
            case 1:
                _ui->displaySensorTable();
                forceMenuPrint = true; // 메뉴 다시 보이기
                break;
            case 2:
                currentAppState = ID_MANAGEMENT_MENU;
                forceMenuPrint = true;
                break;
            case 3:
                currentAppState = THRESHOLD_MENU;
                currentThresholdSubState = THRESHOLD_NONE;
                forceMenuPrint = true;
                break;
            case 4:
                _comm->println(F("Restarting sensor search..."));
                startSensorSearchMode();
                break;
            case 5:
                _ui->displayTroubleshootingGuide();
                forceMenuPrint = true;
                break;
            case 6:
                _comm->println(F("Returning to normal operation."));
                currentAppState = NORMAL_OPERATION;
                _comm->clearInputBuffer();
                printCurrentSensorReport();
                break;
            }
            if (currentAppState != NORMAL_OPERATION && currentAppState != MAIN_MENU)
            {
                menuDisplayMillis = millis();
            }
        }
        else
        {
            _comm->println(F("🧼 Invalid input. Enter a number between 1 and 6."));
        }
    }
}

void App::openIdManagementMenu()
{
    if (forceMenuPrint || (millis() - menuDisplayMillis >= MENU_REPRINT_INTERVAL))
    {
        _ui->displayIdChangeMenu();
        menuDisplayMillis = millis();
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
    if (input != nullptr && strlen(input) > 0)
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
                break;
            }
            if (currentAppState != MAIN_MENU && currentAppState != NORMAL_OPERATION)
            {
                currentEditIdSubState = EDIT_ID_NONE;
                menuDisplayMillis = millis();
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
                    memcpy(addrs[j], addrs[j + 1], 8);
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

// 임계값 설정 메뉴 표시
void App::displayThresholdMenu()
{
    if (forceMenuPrint || (millis() - menuDisplayMillis >= MENU_REPRINT_INTERVAL))
    {
        _ui->displayThresholdMenu();
        menuDisplayMillis = millis();
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

// 임계값 메뉴 입력 처리
void App::processThresholdMenuInput()
{
    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _ui->printTimeoutMessage();
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
        return;
    }

    if (currentThresholdSubState == THRESHOLD_CLEAR_CONFIRM)
    {
        const char *input = _comm->readNonBlockingLine();
        if (input != nullptr && strlen(input) > 0)
        {
            if (strcmp(input, "y") == 0 || strcasecmp(input, "yes") == 0)
            {
                _dataProcessor->clearAllThresholds();
                _comm->println(F("✅ All thresholds cleared successfully!"));
                _ui->displaySensorTable();
            }
            else
            {
                _comm->println(F("Cancelled. No thresholds were cleared."));
            }
            currentThresholdSubState = THRESHOLD_NONE;
            forceMenuPrint = true;
            menuDisplayMillis = millis();
        }
        return;
    }

    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr && strlen(input) > 0)
    {
        if (strlen(input) == 1 && input[0] >= '1' && input[0] <= '5')
        {
            int num = atoi(input);
            switch (num)
            {
            case 1:
                currentAppState = SET_INDIVIDUAL_THRESHOLD;
                currentThresholdSubState = THRESHOLD_NONE;
                selectedThresholdSensorIndex = -1;
                break;
            case 2:
                currentAppState = SET_GLOBAL_THRESHOLD;
                currentThresholdSubState = THRESHOLD_NONE;
                break;
            case 3:
                viewCurrentThresholds();
                break;
            case 4:
                clearAllThresholds(); // 이는 currentThresholdSubState를 THRESHOLD_CLEAR_CONFIRM으로 변경함
                break;
            case 5:
                _comm->println(F("Returning to main menu."));
                currentAppState = MENU_ACTIVE;
                currentThresholdSubState = THRESHOLD_NONE;
                break;
            }
            if (currentThresholdSubState != THRESHOLD_CLEAR_CONFIRM)
            {
                forceMenuPrint = true;
                menuDisplayMillis = millis();
            }
        }
        else
        {
            _comm->println(F("🧼 Invalid input. Enter a number between 1 and 5."));
        }
    }
}

// 개별 센서 임계값 설정
void App::processIndividualThreshold()
{
    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _comm->println("⏰ Threshold setting timeout. Returning to normal operation.");
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
        return;
    }

    if (currentThresholdSubState == THRESHOLD_NONE)
    {
        _comm->println();
        _comm->println(F("--- Individual Sensor Threshold Setting ---"));
        int found = _sensorManager->getDeviceCount();
        if (found == 0)
        {
            _comm->println(F("No sensors found. Returning to threshold menu."));
            currentAppState = THRESHOLD_MENU;
            return;
        }

        _comm->println(F("Current sensors:"));
        for (int i = 0; i < found; i++)
        {
            int id = _sensorManager->getUserDataByIndex(i);
            if (id > 0)
            {
                int8_t highTemp = _dataProcessor->getTh(id);
                int8_t lowTemp = _dataProcessor->getTl(id);
                _comm->print(F("Sensor "));
                _comm->print(i + 1);
                _comm->print(F(" (ID: "));
                _comm->print(id);
                _comm->print(F(") - High: "));
                _comm->print(highTemp);
                _comm->print(F("°C, Low: "));
                _comm->print(lowTemp);
                _comm->println(F("°C"));
            }
        }
        _comm->print(F("Enter sensor number (1-"));
        _comm->print(found);
        _comm->println(F(") or 'c' to cancel:"));
        currentThresholdSubState = THRESHOLD_SELECT_SENSOR;
        menuDisplayMillis = millis();
        return;
    }

    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr && strlen(input) > 0)
    {
        if (strcmp(input, "c") == 0 || strcasecmp(input, "cancel") == 0)
        {
            _comm->println(F("Cancelled. Returning to threshold menu."));
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
            return;
        }

        if (currentThresholdSubState == THRESHOLD_SELECT_SENSOR)
        {
            int sensorNum = atoi(input);
            int found = _sensorManager->getDeviceCount();
            if (sensorNum >= 1 && sensorNum <= found)
            {
                selectedThresholdSensorIndex = sensorNum - 1;
                _comm->print(F("Selected Sensor "));
                _comm->println(sensorNum);
                _comm->println(F("Enter high temperature threshold (-55 to 125°C) or press Enter to skip:"));
                currentThresholdSubState = THRESHOLD_ENTER_HIGH;
                hasHighThreshold = false;
                menuDisplayMillis = millis();
            }
            else
            {
                _comm->println(F("Invalid sensor number. Try again:"));
            }
        }
        else if (currentThresholdSubState == THRESHOLD_ENTER_HIGH)
        {
            if (strlen(input) == 0)
            {
                // Skip high threshold
                hasHighThreshold = false;
                _comm->println(F("Skipped high threshold."));
            }
            else
            {
                int temp = atoi(input);
                if (temp >= -55 && temp <= 125)
                {
                    pendingHighThreshold = temp;
                    hasHighThreshold = true;
                    _comm->print(F("High threshold set to: "));
                    _comm->print(temp);
                    _comm->println(F("°C"));
                }
                else
                {
                    _comm->println(F("Invalid temperature. Enter value between -55 and 125°C:"));
                    return;
                }
            }
            _comm->println(F("Enter low temperature threshold (-55 to 125°C) or press Enter to skip:"));
            currentThresholdSubState = THRESHOLD_ENTER_LOW;
            hasLowThreshold = false;
            menuDisplayMillis = millis();
        }
        else if (currentThresholdSubState == THRESHOLD_ENTER_LOW)
        {
            if (strlen(input) == 0)
            {
                // Skip low threshold
                hasLowThreshold = false;
                _comm->println(F("Skipped low threshold."));
            }
            else
            {
                int temp = atoi(input);
                if (temp >= -55 && temp <= 125)
                {
                    if (hasHighThreshold && temp >= pendingHighThreshold)
                    {
                        _comm->println(F("Low threshold must be less than high threshold. Try again:"));
                        return;
                    }
                    pendingLowThreshold = temp;
                    hasLowThreshold = true;
                    _comm->print(F("Low threshold set to: "));
                    _comm->print(temp);
                    _comm->println(F("°C"));
                }
                else
                {
                    _comm->println(F("Invalid temperature. Enter value between -55 and 125°C:"));
                    return;
                }
            }

            // Apply settings
            int sensorId = _sensorManager->getUserDataByIndex(selectedThresholdSensorIndex);
            if (sensorId > 0)
            {
                int8_t currentHigh = _dataProcessor->getTh(sensorId);
                int8_t currentLow = _dataProcessor->getTl(sensorId);

                if (hasHighThreshold)
                    currentHigh = (int8_t)pendingHighThreshold;
                if (hasLowThreshold)
                    currentLow = (int8_t)pendingLowThreshold;

                _dataProcessor->setThreshold(sensorId, currentHigh, currentLow);
                _comm->println(F("✅ Thresholds successfully applied!"));
            }
            else
            {
                _comm->println(F("❌ Error: Could not find a valid ID for the selected sensor."));
            }

            // 테이블 업데이트
            _comm->println(F("Sensor table updated after threshold setting:"));
            _dataProcessor->printSensorTable();

            _comm->println(F("Returning to threshold menu."));
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
        }
    }
}

// 전체 센서 임계값 일괄 설정
void App::processGlobalThreshold()
{
    if (currentThresholdSubState == THRESHOLD_NONE)
    {
        _comm->println();
        _comm->println(F("--- Global Threshold Setting ---"));
        int found = _sensorManager->getDeviceCount();
        if (found == 0)
        {
            _comm->println(F("No sensors found. Returning to threshold menu."));
            currentAppState = THRESHOLD_MENU;
            return;
        }

        _comm->print(F("Will apply thresholds to all "));
        _comm->print(found);
        _comm->println(F(" sensors."));
        _comm->println(F("Enter high temperature threshold (-55 to 125°C) or press Enter to skip:"));
        currentThresholdSubState = THRESHOLD_ENTER_HIGH;
        hasHighThreshold = false;
        menuDisplayMillis = millis();
        return;
    }

    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr && strlen(input) > 0)
    {
        if (strcmp(input, "c") == 0 || strcasecmp(input, "cancel") == 0)
        {
            _comm->println(F("Cancelled. Returning to threshold menu."));
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
            return;
        }

        if (currentThresholdSubState == THRESHOLD_ENTER_HIGH)
        {
            if (strlen(input) == 0)
            {
                hasHighThreshold = false;
                _comm->println(F("Skipped high threshold."));
            }
            else
            {
                int temp = atoi(input);
                if (temp >= -55 && temp <= 125)
                {
                    pendingHighThreshold = temp;
                    hasHighThreshold = true;
                    _comm->print(F("High threshold set to: "));
                    _comm->print(temp);
                    _comm->println(F("°C"));
                }
                else
                {
                    _comm->println(F("Invalid temperature. Enter value between -55 and 125°C:"));
                    return;
                }
            }
            _comm->println(F("Enter low temperature threshold (-55 to 125°C) or press Enter to skip:"));
            currentThresholdSubState = THRESHOLD_ENTER_LOW;
            hasLowThreshold = false;
            menuDisplayMillis = millis();
        }
        else if (currentThresholdSubState == THRESHOLD_ENTER_LOW)
        {
            if (strlen(input) == 0)
            {
                hasLowThreshold = false;
                _comm->println(F("Skipped low threshold."));
            }
            else
            {
                int temp = atoi(input);
                if (temp >= -55 && temp <= 125)
                {
                    if (hasHighThreshold && temp >= pendingHighThreshold)
                    {
                        _comm->println(F("Low threshold must be less than high threshold. Try again:"));
                        return;
                    }
                    pendingLowThreshold = temp;
                    hasLowThreshold = true;
                    _comm->print(F("Low threshold set to: "));
                    _comm->print(temp);
                    _comm->println(F("°C"));
                }
                else
                {
                    _comm->println(F("Invalid temperature. Enter value between -55 and 125°C:"));
                    return;
                }
            }

            // Apply to all sensors
            int found = _sensorManager->getDeviceCount();
            int successCount = 0;
            bool anySet = hasHighThreshold || hasLowThreshold;

            if (anySet)
            {
                _comm->println(F("Applying thresholds to all sensors..."));
                for (int i = 0; i < found; i++)
                {
                    int sensorId = _sensorManager->getUserDataByIndex(i);
                    if (sensorId > 0)
                    {
                        int8_t currentHigh = _dataProcessor->getTh(sensorId);
                        int8_t currentLow = _dataProcessor->getTl(sensorId);
                        if (hasHighThreshold)
                            currentHigh = (int8_t)pendingHighThreshold;
                        if (hasLowThreshold)
                            currentLow = (int8_t)pendingLowThreshold;
                        _dataProcessor->setThreshold(sensorId, currentHigh, currentLow);
                        successCount++;
                    }
                }

                if (successCount > 0)
                {
                    _comm->println(F("✅ Thresholds successfully applied!"));
                    _comm->println(F("Sensor table updated after global threshold setting:"));
                    _dataProcessor->printSensorTable();
                }
            }
            else
            {
                _comm->println(F("No thresholds were set."));
            }

            _comm->println(F("Returning to threshold menu."));
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
        }
    }
}

// 현재 임계값 조회
void App::viewCurrentThresholds()
{
    _ui->displayCurrentThresholds();
}

// 모든 센서 임계값 초기화
void App::clearAllThresholds()
{
    _comm->println();
    _comm->println(F("--- Clear All Thresholds ---"));
    int found = _sensorManager->getDeviceCount();
    if (found == 0)
    {
        _comm->println(F("No sensors found."));
        return;
    }

    _comm->print(F("Are you sure you want to clear thresholds for all "));
    _comm->print(found);
    _comm->println(F(" sensors? (y/N):"));
    
    currentThresholdSubState = THRESHOLD_CLEAR_CONFIRM;
    menuDisplayMillis = millis();
}

// THRESHOLD_CLEAR_CONFIRM 처리를 위한 로직을 processThresholdMenuInput 또는 별도 메서드에 추가해야 함.
// 여기서는 간단히 App.run()의 switch문에 추가하거나 processThresholdMenuInput에서 분기 처리.
// App::run()의 SET_GLOBAL_THRESHOLD 처럼 처리하거나, processThresholdMenuInput이 loop에서 계속 호출되므로 거기서 처리.
// 하지만 currentAppState가 THRESHOLD_MENU일 때 processThresholdMenuInput이 호출됨.
// 따라서 clearAllThresholds를 호출하면 currentThresholdSubState가 바뀜.
// processThresholdMenuInput 내부에서 currentThresholdSubState를 체크하게 수정.

void App::checkSensorStatus()
{
    int found = _sensorManager->getDeviceCount();
    for (int i = 0; i < found; i++)
    {
        int id = _sensorManager->getUserDataByIndex(i);
        if (id > 0)
        {
            float temp = _sensorManager->getTempCByIndex(i);
            int8_t th = _dataProcessor->getTh(id);
            int8_t tl = _dataProcessor->getTl(id);

            if (temp > th)
            {
                _comm->print("Sensor ");
                _comm->print(id);
                _comm->print(": Temperature HIGH (");
                _comm->print(temp);
                _comm->println("°C)");
            }
            else if (temp < tl)
            {
                _comm->print("Sensor ");
                _comm->print(id);
                _comm->print(": Temperature LOW (");
                _comm->print(temp);
                _comm->println("°C)");
            }
        }
    }
}

void App::startSensorSearchMode()
{
    _comm->println("Starting sensor search mode...");
    sensorSearchStartTime = millis();
    sensorSearchAttempts = 0;
    isInSensorSearchMode = true;
    performSensorSearch();
}

void App::performSensorSearch()
{
    if (!isInSensorSearchMode) return;

    unsigned long currentTime = millis();

    // 2초 간격으로 검색 시도 (비차단)
    if (currentTime - processLastMillis < 2000) return;
    
    processLastMillis = currentTime;

    // 타임아웃 체크
    if (currentTime - sensorSearchStartTime > SEARCH_TIMEOUT_MS)
    {
        _comm->println(F("Sensor search timeout. Entering no-sensor mode..."));
        enterNoSensorMode();
        return;
    }

    sensorSearchAttempts++;
    _comm->print(F("Sensor search attempt "));
    _comm->print(sensorSearchAttempts);
    _comm->print(F(" of "));
    _comm->println(MAX_SEARCH_ATTEMPTS);

    // 센서 재검색
    _sensorManager->begin();
    int deviceCount = _sensorManager->getDeviceCount();

    if (deviceCount > 0)
    {
        _comm->print(F("Found "));
        _comm->print(deviceCount);
        _comm->println(F(" sensor(s). Initialization successful!"));
        isInSensorSearchMode = false;
        previousMillis = 0; // 즉시 읽기 유도
        return;
    }

    if (sensorSearchAttempts >= MAX_SEARCH_ATTEMPTS)
    {
        _comm->println(F("Maximum search attempts reached. Entering no-sensor mode..."));
        enterNoSensorMode();
    }
}

void App::enterNoSensorMode()
{
    isInSensorSearchMode = false;
    _comm->println(F("No sensors detected. Entering no-sensor mode."));
    _comm->println(F("The system will continue to run with limited functionality."));
    _comm->println(F("You can restart the system or check sensor connections."));
    
    // 이 상태에서는 loop()에서 app.run()이 호출되어도 
    // 특별히 하는 작업이 없으므로 시스템이 멈추지 않음.
    // 기존의 무한 루프(while(true))와 delay(10000)를 제거함.
    currentAppState = NORMAL_OPERATION; 
}

#include "exclude_memory_analysis.h" // 항상 가장 먼저 포함
#include "App.h"
#include "config.h"
#include "DS18B20_Sensor.h"
#include "DataProcessor.h"
#include "Communicator.h"
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

App::App(DS18B20_Sensor *sensorManager, DataProcessor *dataProcessor, Communicator *comm, void *memoryTester)
    : _sensorManager(sensorManager),
      _dataProcessor(dataProcessor),
      _comm(comm),
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
      isInSensorSearchMode(false) {}

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
    case MENU_ACTIVE:
        displayIdChangeMenu();
        processMenuInput();
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
    if (found == 0)
    {
        log_error("No DS18B20 sensors found. Starting sensor search mode...");
        startSensorSearchMode();
    }
}

void App::showDeviceTable()
{
    _dataProcessor->printSensorTable();
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
    _comm->println((__FlashStringHelper *)NEW_REPORT_HEADER);
    _dataProcessor->printSensorTable();
    checkSensorStatus(); // 센서 상태 체크 및 메시지 출력
    showTroubleshootingGuide();
    if (_dataProcessor->hasInvalidSensors())
    {
        _comm->println((__FlashStringHelper *)INVALID_SENSORS_BORDER);
        _comm->println((__FlashStringHelper *)INVALID_SENSORS_MSG);
        _comm->println((__FlashStringHelper *)INVALID_SENSORS_BORDER);
    }
}

void App::handleUserCommands()
{
    if (_comm->isSerialAvailable())
    {
        String command = _comm->readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        if (command == "menu" || command == "m")
        {
            _comm->println((__FlashStringHelper *)MENU_CALL_MSG);
            currentAppState = MENU_ACTIVE;
            _comm->clearInputBuffer();
            forceMenuPrint = true;
            menuDisplayMillis = millis();
        }
        else if (command == "help" || command == "h")
        {
            _comm->println("-----------------------------");
            _comm->println("사용 가능한 명령어:");
            _comm->println("menu or m : Call ID change menu");
            _comm->println("help 또는 h : 도움말 표시");
            _comm->println("-----------------------------");
        }
        else if (command.length() > 0)
        {
            _comm->print("알 수 없는 명령어: ");
            _comm->println(command);
        }
    }
}

void App::showTroubleshootingGuide()
{
    // Placeholder for troubleshooting guide
}

void App::displayIdChangeMenu()
{
    if (forceMenuPrint || (millis() - menuDisplayMillis >= MENU_REPRINT_INTERVAL))
    {
        _comm->println("-------------------------------");
        _comm->println("Sensor ID Change Menu:");
        _comm->println("1. Change Individual Sensor ID");
        _comm->println("2. Change Selective Sensor IDs");
        _comm->println("3. Auto-assign IDs by Address");
        _comm->println("4. Reset All Sensor IDs");
        _comm->println("5. Cancel");
        _comm->println("6. Threshold Settings");
        _comm->println("Enter the number of the desired menu and press Enter.");
        _comm->println("Menu will timeout in 60 seconds if no input is received.");
        _comm->println("-------------------------------");
        menuDisplayMillis = millis();
        forceMenuPrint = false;
    }

    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _comm->println("⏰ Menu timeout. Returning to normal operation.");
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
    }
}

void App::processMenuInput()
{
    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _comm->println("⏰ Menu timeout. Returning to normal operation.");
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
        return;
    }

    String input = _comm->readNonBlockingLine();
    if (input.length() > 0)
    {
        if (input.length() == 1 && input[0] >= '1' && input[0] <= '6')
        {
            int num = input.toInt();
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
                _comm->println("Cancelled. Returning to normal operation.");
                currentAppState = NORMAL_OPERATION;
                _comm->clearInputBuffer();
                printCurrentSensorReport();
                break;
            case 6:
                currentAppState = THRESHOLD_MENU;
                currentThresholdSubState = THRESHOLD_NONE;
                break;
            }
            if (currentAppState != NORMAL_OPERATION)
            {
                currentEditIdSubState = EDIT_ID_NONE;
                forceMenuPrint = true;
                menuDisplayMillis = millis();
            }
        }
        else
        {
            _comm->println("🧼 Invalid input. Enter a number between 1 and 6.");
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
        _comm->println("Enter the number of the sensor you want to change (1-" + String(SENSOR_COUNT) + "). 0 to cancel.");
        currentEditIdSubState = EDIT_ID_SELECT_SENSOR;
    }

    if (currentEditIdSubState == EDIT_ID_SELECT_SENSOR)
    {
        String input = _comm->readNonBlockingLine();
        if (input.length() > 0)
        {
            int sensorNum = input.toInt();
            if (sensorNum == 0)
            {
                _comm->println("Cancelling individual ID change.");
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
                    _comm->println("You selected sensor No. " + String(sensorNum) + " (Current ID: " + String(sensorsToChangeArray[0].currentId) + ").");
                    _dataProcessor->printAvailableIds();
                    _comm->print("Enter new ID (1-" + String(SENSOR_COUNT) + "): ");
                    currentEditIdSubState = EDIT_ID_ENTER_NEW_ID;
                }
                else
                {
                    _comm->println("No active sensor found in slot " + String(sensorNum) + ". Please select an active sensor.");
                }
            }
            else
            {
                _comm->println("Invalid sensor number. Please try again.");
            }
        }
    }

    if (currentEditIdSubState == EDIT_ID_ENTER_NEW_ID)
    {
        String input = _comm->readNonBlockingLine();
        if (input.length() > 0)
        {
            int newId = input.toInt();
            if (newId < 1 || newId > SENSOR_COUNT || _sensorManager->isIdUsed(newId))
            {
                _comm->println("Invalid or already used ID. Please try again.");
                editIdTimeoutMillis = millis();
                return;
            }
            DeviceAddress addr;
            memcpy(addr, sensorsToChangeArray[0].addr, 8);
            _sensorManager->setUserData(addr, newId);
            delay(20);
            _comm->println("Sensor No. " + String(sensorsToChangeArray[0].index) + " ID changed from " + String(sensorsToChangeArray[0].currentId) + " to " + String(newId) + ".");
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
        _comm->println("⏰ Menu timeout. Returning to normal operation.");
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
        return;
    }

    if (currentEditIdSubState == EDIT_ID_NONE)
    {
        _comm->println();
        _comm->println("--- Selective Sensor ID Change ---");
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
        _comm->println("Enter the numbers of the sensors you want to change (e.g., 1 3 5 or 2,4,6). 0 to cancel.");
        currentEditIdSubState = EDIT_SELECTIVE_GET_SENSORS;
        editIdTimeoutMillis = millis();
    }

    if (currentEditIdSubState == EDIT_SELECTIVE_GET_SENSORS)
    {
        if (millis() - editIdTimeoutMillis > UI_TIMEOUT)
        {
            _comm->println("⏰ Input timeout. Cancelling.");
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            return;
        }
        String input = _comm->readNonBlockingLine();
        if (input.length() > 0)
        {
            if (input == "0")
            {
                _comm->println("Cancelling.");
                currentAppState = NORMAL_OPERATION;
                _comm->clearInputBuffer();
                previousMillis = 0;
                printCurrentSensorReport();
                return;
            }
            parseSensorSelection(input);
            if (selectedSensorIndicesCount == 0)
            {
                _comm->println("No valid sensors selected. Please try again.");
                currentEditIdSubState = EDIT_ID_NONE;
                return;
            }
            _comm->print("Selected sensors: ");
            for (int i = 0; i < selectedSensorIndicesCount; i++)
            {
                _comm->print(String(sensorsToChangeArray[selectedSensorIndices[i]].index));
                if (i < selectedSensorIndicesCount - 1)
                    _comm->print(", ");
            }
            _comm->println();
            _comm->print("Proceed with these selections? (y/n): ");
            currentEditIdSubState = EDIT_SELECTIVE_CONFIRM;
            editIdTimeoutMillis = millis();
        }
    }

    if (currentEditIdSubState == EDIT_SELECTIVE_CONFIRM)
    {
        if (millis() - editIdTimeoutMillis > UI_TIMEOUT)
        {
            _comm->println("⏰ Input timeout. Cancelling.");
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            return;
        }
        String input = _comm->readNonBlockingLine();
        if (input.length() > 0)
        {
            if (input.equalsIgnoreCase("y"))
            {
                currentSelectiveIndex = 0;
                currentEditIdSubState = EDIT_SELECTIVE_ENTER_NEW_IDS;
                _dataProcessor->printAvailableIds();
                int sensorTableIndex = sensorsToChangeArray[selectedSensorIndices[currentSelectiveIndex]].index;
                _comm->print("Enter new ID for sensor No. " + String(sensorTableIndex) + ": ");
                editIdTimeoutMillis = millis();
            }
            else
            {
                _comm->println("Cancelled. Restarting selection.");
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
            _comm->println("⏰ Input timeout. Cancelling.");
            currentAppState = NORMAL_OPERATION;
            _comm->clearInputBuffer();
            return;
        }
        String input = _comm->readNonBlockingLine();
        if (input.length() > 0)
        {
            int newId = input.toInt();
            if (newId < 1 || newId > SENSOR_COUNT || _sensorManager->isIdUsed(newId))
            {
                _comm->println("Invalid or already used ID. Please try again.");
                editIdTimeoutMillis = millis();
                return;
            }
            int arrayIndex = selectedSensorIndices[currentSelectiveIndex];
            DeviceAddress addr;
            memcpy(addr, sensorsToChangeArray[arrayIndex].addr, 8);
            _sensorManager->setUserData(addr, newId);
            delay(20);
            int sensorTableIndex = sensorsToChangeArray[arrayIndex].index;
            _comm->println("Sensor No. " + String(sensorTableIndex) + " ID changed to " + String(newId));
            currentSelectiveIndex++;
            if (currentSelectiveIndex >= selectedSensorIndicesCount)
            {
                _comm->println("All selected sensors have been updated.");
                _dataProcessor->printSensorTable();
                currentAppState = NORMAL_OPERATION;
                previousMillis = 0;
            }
            else
            {
                _dataProcessor->printAvailableIds();
                int nextSensorTableIndex = sensorsToChangeArray[selectedSensorIndices[currentSelectiveIndex]].index;
                _comm->print("Enter new ID for sensor No. " + String(nextSensorTableIndex) + ": ");
                editIdTimeoutMillis = millis();
            }
        }
    }
}

void App::processAutoAssignIds()
{
    _comm->println("--- Auto-assigning IDs by Address ---");
    int found = _sensorManager->getDeviceCount();
    if (found == 0)
    {
        _comm->println("No sensors found.");
        currentAppState = NORMAL_OPERATION;
        return;
    }

    DeviceAddress addrs[found];
    for (int i = 0; i < found; i++)
    {
        _sensorManager->getAddress(addrs[i], i);
    }

    for (int i = 0; i < found - 1; i++)
    {
        for (int j = 0; j < found - i - 1; j++)
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

    for (int i = 0; i < found; i++)
    {
        _sensorManager->setUserData(addrs[i], i + 1);
        delay(20);
    }

    _comm->println("Auto-assignment complete.");
    _dataProcessor->printSensorTable();
    currentAppState = NORMAL_OPERATION;
    previousMillis = 0;
}

void App::resetAllSensorIds()
{
    _comm->println("--- Resetting all sensor IDs ---");
    int found = _sensorManager->getDeviceCount();
    for (int i = 0; i < found; i++)
    {
        DeviceAddress addr;
        if (_sensorManager->getAddress(addr, i))
        {
            _sensorManager->setUserData(addr, 0);
            delay(10);
        }
    }
    _comm->println("All sensor IDs have been reset to 0.");
    _comm->println("You can now use 'Auto-assign' or other menu options to set new IDs.");

    _comm->println("--- Connected Sensors (Address List) ---");
    _dataProcessor->printInvalidSensorTable();

    currentAppState = NORMAL_OPERATION;
    previousMillis = 0;
}

void App::parseSensorSelection(String input)
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
    bool isSelected[SENSOR_COUNT + 1] = {false};

    input.replace(',', ' ');
    input.trim();

    bool hasSpaces = input.indexOf(' ') >= 0;

    if (hasSpaces)
    {
        String currentNumber = "";
        for (int i = 0; i <= input.length(); i++)
        {
            char c = (i < input.length()) ? input.charAt(i) : ' ';
            if (isDigit(c))
            {
                currentNumber += c;
            }
            else if (c == ' ')
            {
                if (currentNumber.length() > 0)
                {
                    int id = currentNumber.toInt();
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
                    currentNumber = "";
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < input.length(); i++)
        {
            char c = input.charAt(i);
            if (isDigit(c))
            {
                int id = c - '0';
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
            }
        }
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
} // ???�일?� App.cpp??추�??�야 ?�는 displaySensorTable ?�수�??�함?�니??
// App.cpp ?�일 ?�에 ???�수�?직접 추�??�세??

// ?�서 ?�이블을 ?�면???�시?�는 메서??
void App::displaySensorTable()
{
    _comm->println("======================================");
    _comm->println("Displaying current sensor information:");
    _comm->println("======================================");

    // DataProcessor??printSensorTable 메서???�출
    _dataProcessor->printSensorTable();

    _comm->println("======================================");
}

// 임계값 설정 메뉴 표시
void App::displayThresholdMenu()
{
    if (forceMenuPrint || (millis() - menuDisplayMillis >= MENU_REPRINT_INTERVAL))
    {
        _comm->println("-------------------------------");
        _comm->println("Threshold Settings Menu:");
        _comm->println("1. Set Individual Threshold");
        _comm->println("2. Set Global Threshold");
        _comm->println("3. View Current Thresholds");
        _comm->println("4. Clear Thresholds");
        _comm->println("5. Back to Main Menu");
        _comm->println("Enter the number of the desired option and press Enter.");
        _comm->println("Menu will timeout in 60 seconds if no input is received.");
        _comm->println("-------------------------------");
        menuDisplayMillis = millis();
        forceMenuPrint = false;
    }

    if (millis() - menuDisplayMillis >= UI_TIMEOUT)
    {
        _comm->println("⏰ Menu timeout. Returning to normal operation.");
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
        _comm->println("⏰ Menu timeout. Returning to normal operation.");
        currentAppState = NORMAL_OPERATION;
        _comm->clearInputBuffer();
        printCurrentSensorReport();
        return;
    }

    String input = _comm->readNonBlockingLine();
    if (input.length() > 0)
    {
        if (input.length() == 1 && input[0] >= '1' && input[0] <= '5')
        {
            int num = input.toInt();
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
                clearAllThresholds();
                break;
            case 5:
                _comm->println("Returning to main menu.");
                currentAppState = MENU_ACTIVE;
                currentThresholdSubState = THRESHOLD_NONE;
                break;
            }
            forceMenuPrint = true;
            menuDisplayMillis = millis();
        }
        else
        {
            _comm->println("🧼 Invalid input. Enter a number between 1 and 5.");
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
        _comm->println("--- Individual Sensor Threshold Setting ---");
        int found = _sensorManager->getDeviceCount();
        if (found == 0)
        {
            _comm->println("No sensors found. Returning to threshold menu.");
            currentAppState = THRESHOLD_MENU;
            return;
        }

        _comm->println("Current sensors:");
        for (int i = 0; i < found; i++)
        {
            int id = _sensorManager->getUserDataByIndex(i);
            if (id > 0)
            {
                int8_t highTemp = _dataProcessor->getTh(id);
                int8_t lowTemp = _dataProcessor->getTl(id);
                _comm->print("Sensor ");
                _comm->print(i + 1);
                _comm->print(" (ID: ");
                _comm->print(id);
                _comm->print(") - High: ");
                _comm->print(highTemp);
                _comm->print("°C, Low: ");
                _comm->print(lowTemp);
                _comm->println("°C");
            }
        }
        _comm->println("Enter sensor number (1-" + String(found) + ") or 'c' to cancel:");
        currentThresholdSubState = THRESHOLD_SELECT_SENSOR;
        menuDisplayMillis = millis();
        return;
    }

    String input = _comm->readNonBlockingLine();
    if (input.length() > 0)
    {
        if (input.equalsIgnoreCase("c") || input.equalsIgnoreCase("cancel"))
        {
            _comm->println("Cancelled. Returning to threshold menu.");
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
            return;
        }

        if (currentThresholdSubState == THRESHOLD_SELECT_SENSOR)
        {
            int sensorNum = input.toInt();
            int found = _sensorManager->getDeviceCount();
            if (sensorNum >= 1 && sensorNum <= found)
            {
                selectedThresholdSensorIndex = sensorNum - 1;
                _comm->print("Selected Sensor ");
                _comm->println(sensorNum);
                _comm->println("Enter high temperature threshold (-55 to 125°C) or press Enter to skip:");
                currentThresholdSubState = THRESHOLD_ENTER_HIGH;
                hasHighThreshold = false;
                menuDisplayMillis = millis();
            }
            else
            {
                _comm->println("Invalid sensor number. Try again:");
            }
        }
        else if (currentThresholdSubState == THRESHOLD_ENTER_HIGH)
        {
            if (input.length() == 0)
            {
                // Skip high threshold
                hasHighThreshold = false;
                _comm->println("Skipped high threshold.");
            }
            else
            {
                int temp = input.toInt();
                if (temp >= -55 && temp <= 125)
                {
                    pendingHighThreshold = temp;
                    hasHighThreshold = true;
                    _comm->print("High threshold set to: ");
                    _comm->println(temp);
                    _comm->println("°C");
                }
                else
                {
                    _comm->println("Invalid temperature. Enter value between -55 and 125°C:");
                    return;
                }
            }
            _comm->println("Enter low temperature threshold (-55 to 125°C) or press Enter to skip:");
            currentThresholdSubState = THRESHOLD_ENTER_LOW;
            hasLowThreshold = false;
            menuDisplayMillis = millis();
        }
        else if (currentThresholdSubState == THRESHOLD_ENTER_LOW)
        {
            if (input.length() == 0)
            {
                // Skip low threshold
                hasLowThreshold = false;
                _comm->println("Skipped low threshold.");
            }
            else
            {
                int temp = input.toInt();
                if (temp >= -55 && temp <= 125)
                {
                    if (hasHighThreshold && temp >= pendingHighThreshold)
                    {
                        _comm->println("Low threshold must be less than high threshold. Try again:");
                        return;
                    }
                    pendingLowThreshold = temp;
                    hasLowThreshold = true;
                    _comm->print("Low threshold set to: ");
                    _comm->println(temp);
                    _comm->println("°C");
                }
                else
                {
                    _comm->println("Invalid temperature. Enter value between -55 and 125°C:");
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
                    currentHigh = pendingHighThreshold;
                if (hasLowThreshold)
                    currentLow = pendingLowThreshold;

                _dataProcessor->setThreshold(sensorId, currentHigh, currentLow);
                _comm->println("✅ Thresholds successfully applied!");
            }
            else
            {
                _comm->println("❌ Error: Could not find a valid ID for the selected sensor.");
            }

            // 테이블 업데이트
            _comm->println("Sensor table updated after threshold setting:");
            _dataProcessor->printSensorTable();

            _comm->println("Returning to threshold menu.");
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
        }
    }
}

// 전체 센서 임계값 일괄 설정
void App::processGlobalThreshold()
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
        _comm->println("--- Global Threshold Setting ---");
        int found = _sensorManager->getDeviceCount();
        if (found == 0)
        {
            _comm->println("No sensors found. Returning to threshold menu.");
            currentAppState = THRESHOLD_MENU;
            return;
        }

        _comm->print("Will apply thresholds to all ");
        _comm->print(found);
        _comm->println(" sensors.");
        _comm->println("Enter high temperature threshold (-55 to 125°C) or press Enter to skip:");
        currentThresholdSubState = THRESHOLD_ENTER_HIGH;
        hasHighThreshold = false;
        menuDisplayMillis = millis();
        return;
    }

    String input = _comm->readNonBlockingLine();
    if (input.length() > 0)
    {
        if (input.equalsIgnoreCase("c") || input.equalsIgnoreCase("cancel"))
        {
            _comm->println("Cancelled. Returning to threshold menu.");
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
            return;
        }

        if (currentThresholdSubState == THRESHOLD_ENTER_HIGH)
        {
            if (input.length() == 0)
            {
                hasHighThreshold = false;
                _comm->println("Skipped high threshold.");
            }
            else
            {
                int temp = input.toInt();
                if (temp >= -55 && temp <= 125)
                {
                    pendingHighThreshold = temp;
                    hasHighThreshold = true;
                    _comm->print("High threshold set to: ");
                    _comm->print(temp);
                    _comm->println("°C");
                }
                else
                {
                    _comm->println("Invalid temperature. Enter value between -55 and 125°C:");
                    return;
                }
            }
            _comm->println("Enter low temperature threshold (-55 to 125°C) or press Enter to skip:");
            currentThresholdSubState = THRESHOLD_ENTER_LOW;
            hasLowThreshold = false;
            menuDisplayMillis = millis();
        }
        else if (currentThresholdSubState == THRESHOLD_ENTER_LOW)
        {
            if (input.length() == 0)
            {
                hasLowThreshold = false;
                _comm->println("Skipped low threshold.");
            }
            else
            {
                int temp = input.toInt();
                if (temp >= -55 && temp <= 125)
                {
                    if (hasHighThreshold && temp >= pendingHighThreshold)
                    {
                        _comm->println("Low threshold must be less than high threshold. Try again:");
                        return;
                    }
                    pendingLowThreshold = temp;
                    hasLowThreshold = true;
                    _comm->print("Low threshold set to: ");
                    _comm->print(temp);
                    _comm->println("°C");
                }
                else
                {
                    _comm->println("Invalid temperature. Enter value between -55 and 125°C:");
                    return;
                }
            }

            // Apply to all sensors
            int found = _sensorManager->getDeviceCount();
            int successCount = 0;
            bool anySet = hasHighThreshold || hasLowThreshold;

            if (anySet)
            {
                _comm->println("Applying thresholds to all sensors...");
                for (int i = 0; i < found; i++)
                {
                    int sensorId = _sensorManager->getUserDataByIndex(i);
                    if (sensorId > 0)
                    {
                        int8_t currentHigh = _dataProcessor->getTh(sensorId);
                        int8_t currentLow = _dataProcessor->getTl(sensorId);
                        if (hasHighThreshold)
                            currentHigh = pendingHighThreshold;
                        if (hasLowThreshold)
                            currentLow = pendingLowThreshold;
                        _dataProcessor->setThreshold(sensorId, currentHigh, currentLow);
                        successCount++;
                    }
                }

                if (successCount > 0)
                {
                    _comm->println("✅ Thresholds successfully applied!");
                    _comm->println("Sensor table updated after global threshold setting:");
                    _dataProcessor->printSensorTable();
                }
            }
            else
            {
                _comm->println("No thresholds were set.");
            }

            _comm->println("Returning to threshold menu.");
            currentAppState = THRESHOLD_MENU;
            currentThresholdSubState = THRESHOLD_NONE;
        }
    }
}

// 현재 임계값 조회
void App::viewCurrentThresholds()
{
    _comm->println();
    _comm->println("--- Current Threshold Settings ---");
    int found = _sensorManager->getDeviceCount();
    if (found == 0)
    {
        _comm->println("No sensors found.");
        return;
    }

    _comm->println("Sensor | ID | High Temp | Low Temp");
    _comm->println("-------|----|-----------|---------");
    for (int i = 0; i < found; i++)
    {
        int id = _sensorManager->getUserDataByIndex(i);
        if (id > 0)
        {
            int8_t highTemp = _dataProcessor->getTh(id);
            int8_t lowTemp = _dataProcessor->getTl(id);

            _comm->print("   ");
            _comm->print(i + 1);
            _comm->print("   | ");
            _comm->print(id);
            _comm->print("  |   ");
            _comm->print(highTemp);
            _comm->print("°C   |  ");
            _comm->print(lowTemp);
            _comm->println("°C");
        }
    }
    _comm->println("-------------------------------");
}

// 모든 센서 임계값 초기화
void App::clearAllThresholds()
{
    _comm->println();
    _comm->println("--- Clear All Thresholds ---");
    int found = _sensorManager->getDeviceCount();
    if (found == 0)
    {
        _comm->println("No sensors found.");
        return;
    }

    _comm->print("Are you sure you want to clear thresholds for all ");
    _comm->print(found);
    _comm->println(" sensors? (y/N):");

    unsigned long clearStartTime = millis();
    while (millis() - clearStartTime < 10000) // 10초 대기
    {
        String input = _comm->readNonBlockingLine();
        if (input.length() > 0)
        {
            if (input.equalsIgnoreCase("y") || input.equalsIgnoreCase("yes"))
            {
                _dataProcessor->clearAllThresholds();
                _comm->println("✅ All thresholds cleared successfully!");
                _comm->println("Sensor table updated after clearing thresholds:");
                _dataProcessor->printSensorTable();
                return;
            }
            else
            {
                _comm->println("Cancelled. No thresholds were cleared.");
                return;
            }
        }
    }
    _comm->println("⏰ Timeout. No thresholds were cleared.");
}

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
    if (!isInSensorSearchMode)
    {
        return;
    }

    unsigned long currentTime = millis();

    // 타임아웃 체크
    if (currentTime - sensorSearchStartTime > SEARCH_TIMEOUT_MS)
    {
        _comm->println("Sensor search timeout. Entering no-sensor mode...");
        enterNoSensorMode();
        return;
    }

    sensorSearchAttempts++;
    _comm->print("Sensor search attempt ");
    _comm->print(sensorSearchAttempts);
    _comm->print(" of ");
    _comm->println(MAX_SEARCH_ATTEMPTS);

    // 센서 재검색
    _sensorManager->begin();
    int deviceCount = _sensorManager->getDeviceCount();

    if (deviceCount > 0)
    {
        _comm->print("Found ");
        _comm->print(deviceCount);
        _comm->println(" sensor(s). Initialization successful!");
        isInSensorSearchMode = false;
        return;
    }

    if (sensorSearchAttempts >= MAX_SEARCH_ATTEMPTS)
    {
        _comm->println("Maximum search attempts reached. Entering no-sensor mode...");
        enterNoSensorMode();
        return;
    }

    // 다음 시도를 위해 잠시 대기
    delay(2000);
}

void App::enterNoSensorMode()
{
    isInSensorSearchMode = false;
    _comm->println("No sensors detected. Entering no-sensor mode.");
    _comm->println("The system will continue to run with limited functionality.");
    _comm->println("You can restart the system or check sensor connections.");

    // 메모리 테스터는 계속 동작
    while (true)
    {
#ifdef UNIT_TEST
        // 테스트 환경에서는 Mock 객체 사용
        if (_memoryTester)
        {
            ((MockMemoryUsageTester *)_memoryTester)->logMemoryUsage();
        }
#elif defined(MEMORY_ANALYSIS_ENABLED) && MEMORY_ANALYSIS_ENABLED == 1
        // 메모리 분석 활성화된 경우만 실행
        if (_memoryTester)
        {
            ((MemoryUsageTester *)_memoryTester)->logMemoryUsage();
        }
#endif

        _comm->println("No sensors - memory monitoring only mode");
        delay(10000); // 10초 대기
    }
}

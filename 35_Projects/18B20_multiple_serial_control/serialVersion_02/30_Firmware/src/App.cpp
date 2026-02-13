#include "App.h"
#include "DS18B20_Sensor.h"
#include "DataProcessor.h"
#include "Communicator.h"
#include "SerialUI.h"
#include "Utils.h"
#include "config.h"

#if defined(MEMORY_ANALYSIS_ENABLED) && MEMORY_ANALYSIS_ENABLED == 1
#include "MemoryUsageTester.h"
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
      currentResSubState(RES_NONE),
      sensorsToChangeCount(0),
      selectedSensorIndex(-1),
      selectedResolution(12),
      editIdTimeoutMillis(0),
      selectedSensorIndicesCount(0),
      currentSelectiveIndex(0),
      previousMillis(0),
      menuDisplayMillis(0),
      lastMenuPrintMillis(0),
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
      processLastMillis(0)
{
    memset(sensorsToChangeArray, 0, sizeof(sensorsToChangeArray));
    memset(selectedSensorIndices, 0, sizeof(selectedSensorIndices));
}

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
    case RESOLUTION_MENU:
        displayResolutionMenu();
        processResolutionMenuInput();
        break;
    case SET_INDIVIDUAL_RESOLUTION:
        processIndividualResolution();
        break;
    case SET_GLOBAL_RESOLUTION:
        processGlobalResolution();
        break;
    }
}

void App::initializeSerial()
{
    _comm->begin();
}

void App::initialize18b20()
{
    _sensorManager->begin();
}

void App::showDeviceTable()
{
    _ui->displaySensorTable();
}

void App::show18b20Table_loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= SENSOR_READ_INTERVAL || previousMillis == 0)
    {
        previousMillis = currentMillis;
        printCurrentSensorReport();
    }
}

void App::printCurrentSensorReport()
{
    int deviceCount = _sensorManager->getDeviceCount();
    _ui->printSensorReport(deviceCount);
}

void App::handleUserCommands()
{
    const char *input = _comm->readNonBlockingLine();
    if (input != nullptr)
    {
        if (strcmp(input, "menu") == 0 || strcmp(input, "m") == 0)
        {
            currentAppState = MAIN_MENU;
            menuDisplayMillis = millis();
            lastMenuPrintMillis = millis();
            forceMenuPrint = true;
        }
        else if (strcmp(input, "id") == 0 || strcmp(input, "i") == 0)
        {
            currentAppState = ID_MANAGEMENT_MENU;
            menuDisplayMillis = millis();
            lastMenuPrintMillis = millis();
            forceMenuPrint = true;
        }
        else if (strcmp(input, "th") == 0 || strcmp(input, "t") == 0)
        {
            currentAppState = THRESHOLD_MENU;
            menuDisplayMillis = millis();
            lastMenuPrintMillis = millis();
            forceMenuPrint = true;
        }
        else if (strcmp(input, "help") == 0 || strcmp(input, "h") == 0)
        {
            _ui->displayGeneralHelp();
        }
#if defined(MEMORY_ANALYSIS_ENABLED) && MEMORY_ANALYSIS_ENABLED == 1
        else if (strcmp(input, "memfree") == 0)
        {
            ((MemoryUsageTester *)_memoryTester)->reportMemoryUsage();
        }
        else if (strcmp(input, "memtoggle") == 0)
        {
            ((MemoryUsageTester *)_memoryTester)->togglePeriodicCheck();
        }
#endif
    }
}

void App::showTroubleshootingGuide()
{
    _ui->displayTroubleshootingGuide();
}

void App::openMainMenu()
{
    if (forceMenuPrint || (millis() - lastMenuPrintMillis >= MENU_REPRINT_INTERVAL))
    {
        _ui->displayMainMenu();
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

void App::handleMainMenuInput()
{
    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr)
    {
        int num = atoi(input);
        if (num >= 1 && num <= 7)
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
                currentAppState = RESOLUTION_MENU;
                currentResSubState = RES_NONE;
                forceMenuPrint = true;
                break;
            case 6:
                _ui->displayTroubleshootingGuide();
                forceMenuPrint = true;
                break;
            case 7:
                _comm->println(F("Returning to normal operation."));
                currentAppState = NORMAL_OPERATION;
                _comm->clearInputBuffer();
                printCurrentSensorReport();
                break;
            }
            if (currentAppState != NORMAL_OPERATION && currentAppState != MAIN_MENU)
            {
                menuDisplayMillis = millis();
                lastMenuPrintMillis = millis();
            }
        }
        else
        {
            _comm->println(F("🧼 Invalid input. Enter a number between 1 and 7."));
        }
    }
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
            int8_t thVal = _dataProcessor->getTh(id);
            int8_t tlVal = _dataProcessor->getTl(id);

            if (temp > thVal)
            {
                _comm->print("Sensor ");
                _comm->print(id);
                _comm->print(": Temperature HIGH (");
                _comm->print(temp);
                _comm->println("°C)");
            }
            else if (temp < tlVal)
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
    
    currentAppState = NORMAL_OPERATION; 
}

#include "App.h"
#include "DS18B20_Sensor.h"
#include "DataProcessor.h"
#include "Communicator.h"
#include "SerialUI.h"

void App::displayThresholdMenu()
{
    if (forceMenuPrint || (millis() - lastMenuPrintMillis >= MENU_REPRINT_INTERVAL))
    {
        _ui->displayThresholdMenu();
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
    if (input != nullptr)
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
                currentAppState = MAIN_MENU;
                currentThresholdSubState = THRESHOLD_NONE;
                break;
            }
            if (currentThresholdSubState != THRESHOLD_CLEAR_CONFIRM)
            {
                forceMenuPrint = true;
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

        _ui->printThresholdSettings();
        _comm->print(F("Enter sensor number (1-"));
        _comm->print(found);
        _comm->println(F(") or 'c' to cancel:"));
        currentThresholdSubState = THRESHOLD_SELECT_SENSOR;
        menuDisplayMillis = millis();
        return;
    }

    const char* input = _comm->readNonBlockingLine();
    if (input != nullptr)
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
    if (input != nullptr)
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
            bool anySet = hasHighThreshold || hasLowThreshold;

            if (anySet)
            {
                int successCount = 0;
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

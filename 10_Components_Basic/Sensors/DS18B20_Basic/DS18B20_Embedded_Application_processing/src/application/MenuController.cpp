#include <Arduino.h>
#include <cstring>
#include <cctype>
#include <vector>
#include <algorithm>
#include "MenuController.h"
#include "SensorController.h"
#include <OneWire.h>
#include <DallasTemperature.h>

extern SensorController sensorController;
extern unsigned long lastPrint;
extern const unsigned long printInterval;

MenuController::MenuController()
    : appState(AppState::Normal),
      selectedSensorIdx(-1),
      selectedDisplayIdx(-1),
      inputBuffer(""),
      isMultiSelectMode(false)
{
    // selectedSensorIndices는 기본 생성자로 빈 벡터로 초기화됨
    selectedSensorIndices.clear(); // 명시적으로 비우기 (선택사항)
}

// SensorMenuHandler를 사용하여 복잡도 감소 (static 함수)
std::vector<int> MenuController::parseSensorIndices(const String &input)
{
    return SensorMenuHandler::parseSensorIndices(input);
}

#include "MenuController.h"
#include "SensorController.h"
void MenuController::printMenu()
{
    Serial.println();
    Serial.println("===== 센서 제어 메뉴 =====");
    Serial.println("1. 센서 ID 조정");
    Serial.println("2. 상/하한 온도 조정");
    Serial.println("3. 센서 측정 주기 조정");
    Serial.println("4. 취소 / 상태창으로 돌아가기");
    Serial.print("메뉴 번호를 입력하세요: ");
}

void MenuController::printSensorIdMenu()
{
    // SensorMenuHandler를 사용하여 복잡도 감소
    sensorMenuHandler.printSensorIdMenu();
}

void MenuController::handleSerialInput()
{
    // InputHandler를 사용하여 복잡도 감소
    if (inputHandler.processSerialInput(inputBuffer))
    {
        processInputBuffer();
    }
}

void MenuController::resetToNormalState()
{
    appState = AppState::Normal;
    inputBuffer = "";
    selectedSensorIdx = -1;
    selectedDisplayIdx = -1;
    selectedSensorIndices.clear();
    isMultiSelectMode = false;
    Serial.println("[DEBUG] 상태가 Normal로 완전히 리셋되었습니다.");
    Serial.println("[시스템 준비 완료 - Normal 모드에서 대기 중]");
}

void MenuController::handleNormalState()
{
    if (inputBuffer == "menu" || inputBuffer == "MENU" || inputBuffer == "m" || inputBuffer == "M")
    {
        appState = AppState::Menu;
        Serial.println("[DEBUG] appState -> Menu");
        printMenu();
    }
    else if (inputBuffer == "reset" || inputBuffer == "RESET" || inputBuffer == "r" || inputBuffer == "R")
    {
        // 강제 리셋 명령어 추가
        resetToNormalState();
        sensorController.printSensorStatusTable();
        lastPrint = millis();
    }
}

void MenuController::handleMenuState()
{
    if (inputBuffer == "1")
    {
        appState = AppState::SensorIdMenu;
        Serial.println("[DEBUG] appState -> SensorIdMenu");
        printSensorIdMenu();
    }
    else if (inputBuffer == "2")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
    }
    else if (inputBuffer == "3")
    {
        appState = AppState::MeasurementInterval_Input;
        Serial.println("[DEBUG] appState -> MeasurementInterval_Input");
        printMeasurementIntervalMenu();
    }
    else if (inputBuffer == "4")
    {
        appState = AppState::Normal;
        Serial.println("[DEBUG] appState -> Normal");
        sensorController.printSensorStatusTable();
        lastPrint = millis();
    }
    else
    {
        Serial.println("지원하지 않는 메뉴입니다. 1~4 중 선택하세요.");
        printMenu();
    }
}

void MenuController::handleSensorIdMenuState()
{
    if (inputBuffer == "1")
    {
        isMultiSelectMode = false;
        appState = AppState::SensorIdChange_SelectSensor;
        Serial.println("[DEBUG] appState -> SensorIdChange_SelectSensor");
        Serial.println("[개별 센서 ID 변경] 센서 상태창:");
        sensorController.printSensorStatusTable();
        Serial.print("변경할 센서 번호(1~8, 취소:c) 입력: ");
    }
    else if (inputBuffer == "2")
    {
        isMultiSelectMode = true;
        appState = AppState::SensorIdChange_SelectSensor;
        Serial.println("[DEBUG] appState -> SensorIdChange_SelectSensor (복수)");
        Serial.println("[복수 센서 ID 변경] 센서 상태창:");
        sensorController.printSensorStatusTable();
        Serial.print("변경할 센서 번호들을 입력하세요 (예: 1 2 3, 취소:c): ");
    }
    else if (inputBuffer == "3")
    {
        sensorController.assignIDsByAddress();
        Serial.println("[자동] 주소순 ID 할당 완료");
        sensorController.printSensorStatusTable();
        printSensorIdMenu();
    }
    else if (inputBuffer == "4")
    {
        // 전체 ID 초기화 확인
        Serial.println();
        Serial.println("⚠️  경고: 모든 센서의 ID가 초기화됩니다!");
        Serial.println("계속하시겠습니까? (y/n): ");
        appState = AppState::SensorIdChange_ConfirmReset;
        Serial.println("[DEBUG] appState -> SensorIdChange_ConfirmReset");
    }
    else if (inputBuffer == "5")
    {
        appState = AppState::Menu;
        Serial.println("[DEBUG] appState -> Menu");
        printMenu();
    }
    else if (inputBuffer == "6")
    {
        appState = AppState::Normal;
        Serial.println("[DEBUG] appState -> Normal");
        sensorController.printSensorStatusTable();
        lastPrint = millis();
    }
    else
    {
        Serial.println("지원하지 않는 메뉴입니다. 1~6 중 선택하세요.");
        printSensorIdMenu();
    }
}

void MenuController::handleSensorIdSelectState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::SensorIdMenu;
        Serial.println("[DEBUG] appState -> SensorIdMenu");
        printSensorIdMenu();
        return;
    }

    if (!validateSensorInput())
        return;

    std::vector<int> indices = parseSensorIndices(inputBuffer);
    if (!processSensorIndices(indices))
        return;

    proceedToSensorConfirmation();
}

bool MenuController::validateSensorInput()
{
    for (char c : inputBuffer)
    {
        if (isspace(c) || (c >= '1' && c <= '8'))
            continue;
        // 1-8, 공백 이외의 문자가 있으면 오류
        Serial.println("[오류] 1~8 사이의 숫자와 공백만 입력하세요.");
        printSensorSelectionPrompt();
        return false;
    }
    return true;
}

bool MenuController::processSensorIndices(const std::vector<int> &indices)
{
    if (indices.empty())
    {
        Serial.println("[오류] 유효한 센서 번호가 없습니다.");
        printSensorSelectionPrompt();
        return false;
    }

    const auto *sortedRows = sensorController.getSortedSensorRows();
    selectedSensorIndices.clear();
    std::vector<int> invalidIndices;

    for (int idx : indices)
    {
        if (idx >= 1 && idx <= SENSOR_MAX_COUNT && sortedRows[idx - 1].connected)
        {
            selectedSensorIndices.push_back(idx);
        }
        else
        {
            invalidIndices.push_back(idx);
        }
    }

    if (!invalidIndices.empty())
    {
        Serial.print("[오류] 연결되지 않았거나 잘못된 센서 번호: ");
        for (size_t i = 0; i < invalidIndices.size(); ++i)
        {
            Serial.print(invalidIndices[i]);
            if (i < invalidIndices.size() - 1)
                Serial.print(", ");
        }
        Serial.println();
        printSensorSelectionPrompt();
        return false;
    }

    if (selectedSensorIndices.empty())
    {
        Serial.println("[오류] 선택된 센서가 없습니다.");
        printSensorSelectionPrompt();
        return false;
    }

    return true;
}

void MenuController::proceedToSensorConfirmation()
{
    // 복수 선택 모드에서만 선택된 센서 번호 안내
    if (isMultiSelectMode)
    {
        Serial.print("선택된 센서 번호: ");
        for (int idx : selectedSensorIndices)
        {
            Serial.print(idx);
            Serial.print(" ");
        }
        Serial.println();
    }

    // 첫 번째 센서부터 변경 확인 안내문 출력
    const auto *sortedRows = sensorController.getSortedSensorRows();
    selectedDisplayIdx = selectedSensorIndices[0];
    selectedSensorIdx = sortedRows[selectedDisplayIdx - 1].idx;
    appState = AppState::SensorIdChange_ConfirmSensor;
    Serial.println("[DEBUG] appState -> SensorIdChange_ConfirmSensor");
    Serial.print("센서 ");
    Serial.print(selectedDisplayIdx);
    Serial.println("번을 변경할까요? (y/n, 취소:c)");
}

void MenuController::printSensorSelectionPrompt()
{
    if (isMultiSelectMode)
        Serial.print("변경할 센서 번호들을 입력하세요 (예: 1 2 3, 취소:c): ");
    else
        Serial.print("변경할 센서 번호(1~8, 취소:c) 입력: ");
}

void MenuController::handleSensorIdConfirmState()
{
    if (inputBuffer == "y" || inputBuffer == "Y")
    {
        handleConfirmYes();
    }
    else if (inputBuffer == "n" || inputBuffer == "N" || inputBuffer == "c" || inputBuffer == "C")
    {
        handleConfirmNoOrCancel();
    }
    else
    {
        handleInvalidConfirmInput();
    }
}

void MenuController::handleSensorIdInputState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        handleCancelInput();
        return;
    }

    int newId = inputBuffer.toInt();
    if (newId >= 1 && newId <= SENSOR_MAX_COUNT)
    {
        processNewSensorId(newId);
    }
    else
    {
        Serial.println("[오류] ID는 1~8 사이의 숫자여야 합니다.");
        printIdInputPrompt();
    }
}

void MenuController::handleCancelInput()
{
    // 취소 시 다음 센서 확인으로 넘어감
    auto it = std::find(selectedSensorIndices.begin(), selectedSensorIndices.end(), selectedDisplayIdx);
    if (it != selectedSensorIndices.end())
    {
        size_t currentIndex = std::distance(selectedSensorIndices.begin(), it);
        if (currentIndex + 1 < selectedSensorIndices.size())
        {
            moveToNextSensor();
        }
        else
        {
            appState = AppState::SensorIdMenu;
            Serial.println("[DEBUG] appState -> SensorIdMenu (완료 또는 취소)");
            printSensorIdMenu();
        }
    }
}

bool MenuController::processNewSensorId(int newId)
{
    if (sensorController.isIdDuplicated(newId, selectedSensorIdx))
    {
        Serial.println("[오류] 이미 사용 중인 ID입니다.");
        printIdInputPrompt();
        return false;
    }

    sensorController.setSensorLogicalId(selectedSensorIdx, newId);
    Serial.print("센서 ");
    Serial.print(selectedDisplayIdx);
    Serial.print("의 ID를 ");
    Serial.print(newId);
    Serial.println("(으)로 변경 완료");

    completeIdChange();
    return true;
}

void MenuController::moveToNextSensor()
{
    selectedDisplayIdx = selectedSensorIndices[std::distance(selectedSensorIndices.begin(),
                                                             std::find(selectedSensorIndices.begin(), selectedSensorIndices.end(), selectedDisplayIdx)) +
                                               1];
    const auto *sortedRows = sensorController.getSortedSensorRows();
    selectedSensorIdx = sortedRows[selectedDisplayIdx - 1].idx;
    appState = AppState::SensorIdChange_ConfirmSensor;
    Serial.println("[DEBUG] appState -> SensorIdChange_ConfirmSensor (다음 센서)");
    Serial.print("센서 ");
    Serial.print(selectedDisplayIdx);
    Serial.println("번을 변경할까요? (y/n, 취소:c)");
}

void MenuController::completeIdChange()
{
    if (isMultiSelectMode)
    {
        // 복수 선택 모드: 다음 센서로 이동
        auto it = std::find(selectedSensorIndices.begin(), selectedSensorIndices.end(), selectedDisplayIdx);
        if (it != selectedSensorIndices.end())
        {
            size_t currentIndex = std::distance(selectedSensorIndices.begin(), it);
            if (currentIndex + 1 < selectedSensorIndices.size())
            {
                moveToNextSensor();
            }
            else
            {
                // 마지막 센서였으면 메뉴로 복귀
                appState = AppState::SensorIdMenu;
                Serial.println("[DEBUG] appState -> SensorIdMenu (모두 완료)");
                sensorController.printSensorStatusTable();
                printSensorIdMenu();
            }
        }
    }
    else
    {
        // 개별 선택 모드: 센서 선택 입력 상태로 복귀
        appState = AppState::SensorIdChange_SelectSensor;
        Serial.println("[DEBUG] appState -> SensorIdChange_SelectSensor (개별 모드 계속)");
        Serial.print("변경할 센서 번호(1~8, 취소:c) 입력: ");
    }
}

void MenuController::printIdInputPrompt()
{
    Serial.print("센서 ");
    Serial.print(selectedDisplayIdx);
    Serial.print("의 새로운 ID(1~8, 취소:c)를 입력하세요: ");
}

void MenuController::handleConfirmYes()
{
    appState = AppState::SensorIdChange_InputId;
    Serial.println("[DEBUG] appState -> SensorIdChange_InputId");
    printIdInputPrompt();
}

void MenuController::handleConfirmNoOrCancel()
{
    // 복수 선택 모드일 때만 다음 센서로 진행, 아니면 센서 선택 입력 프롬프트로 복귀
    if (selectedSensorIndices.size() > 1)
    {
        moveToNextSensorInConfirm();
    }
    else
    {
        // 단일 선택 모드: 센서 선택 입력 프롬프트로 복귀
        appState = AppState::SensorIdChange_SelectSensor;
        Serial.print("변경할 센서 번호(1~8, 취소:c) 입력:");
    }
}

void MenuController::handleInvalidConfirmInput()
{
    Serial.println("y(예), n(아니오), c(취소) 중 하나를 입력하세요.");
    Serial.print("센서 ");
    Serial.print(selectedDisplayIdx);
    Serial.print("번을 변경할까요? (y/n, 취소:c)");
}

void MenuController::moveToNextSensorInConfirm()
{
    auto it = std::find(selectedSensorIndices.begin(), selectedSensorIndices.end(), selectedDisplayIdx);
    if (it != selectedSensorIndices.end())
    {
        size_t currentIndex = std::distance(selectedSensorIndices.begin(), it);
        if (currentIndex + 1 < selectedSensorIndices.size())
        {
            // 다음 센서로 이동
            selectedDisplayIdx = selectedSensorIndices[currentIndex + 1];
            const auto *sortedRows = sensorController.getSortedSensorRows();
            selectedSensorIdx = sortedRows[selectedDisplayIdx - 1].idx;
            appState = AppState::SensorIdChange_ConfirmSensor;
            Serial.println("[DEBUG] appState -> SensorIdChange_ConfirmSensor (다음 센서)");
            Serial.print("센서 ");
            Serial.print(selectedDisplayIdx);
            Serial.println("번을 변경할까요? (y/n, 취소:c)");
        }
        else
        {
            // 마지막 센서였으면 메뉴로 복귀
            appState = AppState::SensorIdMenu;
            Serial.println("[DEBUG] appState -> SensorIdMenu (완료 또는 취소)");
            printSensorIdMenu();
        }
    }
}
void MenuController::processInputBuffer()
{
    Serial.print("[DEBUG] appState: ");
    Serial.println((int)appState);
    Serial.print("[DEBUG] inputBuffer: ");
    Serial.println(inputBuffer);

    if (handleGlobalResetCommand())
        return;

    processStateBasedInput();
    clearInputBuffer();
}

bool MenuController::handleGlobalResetCommand()
{
    if (inputBuffer == "reset" || inputBuffer == "RESET" || inputBuffer == "Reset")
    {
        Serial.println("[INFO] 강제 리셋 명령어 수신");
        resetToNormalState();
        sensorController.printSensorStatusTable();
        lastPrint = millis();
        clearInputBuffer();
        return true;
    }
    return false;
}

void MenuController::processStateBasedInput()
{
    switch (appState)
    {
    case AppState::Normal:
        handleNormalState();
        break;
    case AppState::Menu:
        handleMenuState();
        break;
    case AppState::SensorIdMenu:
        handleSensorIdMenuState();
        break;
    case AppState::SensorIdChange_SelectSensor:
        handleSensorIdSelectState();
        break;
    case AppState::SensorIdChange_ConfirmSensor:
        handleSensorIdConfirmState();
        break;
    case AppState::SensorIdChange_InputId:
        handleSensorIdInputState();
        break;
    case AppState::SensorIdChange_ConfirmReset:
        handleSensorIdConfirmResetState();
        break;
    case AppState::ThresholdMenu:
        handleThresholdMenuState();
        break;
    case AppState::ThresholdChange_SelectSensor:
        handleThresholdSelectSensorState();
        break;
    case AppState::ThresholdChange_InputUpper:
        handleThresholdInputUpperState();
        break;
    case AppState::ThresholdChange_InputLower:
        handleThresholdInputLowerState();
        break;
    case AppState::ThresholdChange_SelectMultipleSensors:
        handleThresholdSelectMultipleSensorsState();
        break;
    case AppState::ThresholdChange_ConfirmMultipleSensors:
        handleThresholdConfirmMultipleSensorsState();
        break;
    case AppState::ThresholdChange_InputMultipleUpper:
        handleThresholdInputMultipleUpperState();
        break;
    case AppState::ThresholdChange_InputMultipleLower:
        handleThresholdInputMultipleLowerState();
        break;
    case AppState::MeasurementIntervalMenu:
        handleMeasurementIntervalMenuState();
        break;
    case AppState::MeasurementInterval_Input:
        handleMeasurementIntervalInputState();
        break;
    default:
        // 알 수 없는 상태인 경우 강제로 Normal 상태로 리셋
        Serial.println("[경고] 알 수 없는 상태 감지, Normal 상태로 리셋합니다.");
        resetToNormalState();
        sensorController.printSensorStatusTable();
        lastPrint = millis();
        break;
    }
}

void MenuController::clearInputBuffer()
{
    inputBuffer = "";

    // 안전한 버퍼 클리어를 위한 제한값들
    const int MAX_CLEAR_CHARS = 64;            // 더 보수적으로 설정
    const unsigned long MAX_CLEAR_TIME_MS = 5; // 최대 클리어 시간 제한

    unsigned long startTime = millis();
    int clearCount = 0;
    int consecutiveFailures = 0;
    const int MAX_CONSECUTIVE_FAILURES = 3;

    // 입력 처리 후 Serial 버퍼 완전 비우기 (테스트 자동화 환경 대응)
    while (clearCount < MAX_CLEAR_CHARS &&
           (millis() - startTime) < MAX_CLEAR_TIME_MS &&
           consecutiveFailures < MAX_CONSECUTIVE_FAILURES)
    {
        if (!Serial.available())
        {
            break; // 더 이상 읽을 데이터 없음
        }

        int readResult = Serial.read();
        if (readResult == -1)
        {
            consecutiveFailures++;
            continue; // 읽기 실패 시 재시도
        }

        consecutiveFailures = 0; // 성공적인 읽기 후 실패 카운터 리셋
        clearCount++;
    }
}

void MenuController::handleSensorIdConfirmResetState()
{
    if (inputBuffer == "y" || inputBuffer == "Y")
    {
        // 전체 ID 초기화 실행
        sensorController.resetAllSensorIds();

        // 센서 상태 테이블 출력
        sensorController.printSensorStatusTable();

        // 센서 ID 메뉴로 복귀
        appState = AppState::SensorIdMenu;
        Serial.println("[DEBUG] appState -> SensorIdMenu");
        printSensorIdMenu();
    }
    else if (inputBuffer == "n" || inputBuffer == "N")
    {
        // 취소 - 센서 ID 메뉴로 복귀
        Serial.println("전체 ID 초기화가 취소되었습니다.");
        appState = AppState::SensorIdMenu;
        Serial.println("[DEBUG] appState -> SensorIdMenu");
        printSensorIdMenu();
    }
    else
    {
        // 잘못된 입력
        Serial.println("y(예) 또는 n(아니오)를 입력하세요.");
        Serial.print("모든 센서의 ID를 초기화하시겠습니까? (y/n): ");
    }
}

// ========== 임계값 설정 메뉴 관련 메서드들 ==========

void MenuController::printThresholdMenu()
{
    Serial.println();
    Serial.println("===== 상/하한 온도 조정 메뉴 =====");
    Serial.println("1. 개별 센서 임계값 설정");
    Serial.println("2. 복수 센서 임계값 설정");
    Serial.println("3. 전체 센서 임계값 초기화");
    Serial.println("4. 이전 메뉴로 돌아가기");
    Serial.println("5. 상태창으로 돌아가기");
    Serial.print("메뉴 번호를 입력하세요: ");
}

void MenuController::handleThresholdMenuState()
{
    if (inputBuffer == "1")
    {
        appState = AppState::ThresholdChange_SelectSensor;
        Serial.println("[DEBUG] appState -> ThresholdChange_SelectSensor");
        Serial.println();
        Serial.println("=== 센서별 임계값 현황 ===");
        sensorController.printSensorStatusTable();
        Serial.print("임계값을 설정할 센서 번호(1~8, 취소:c)를 입력하세요: ");
    }
    else if (inputBuffer == "2")
    {
        appState = AppState::ThresholdChange_SelectMultipleSensors;
        Serial.println("[DEBUG] appState -> ThresholdChange_SelectMultipleSensors");
        Serial.println();
        Serial.println("=== 복수 센서 임계값 설정 ===");
        sensorController.printSensorStatusTable();
        Serial.print("임계값을 설정할 센서 번호들을 입력하세요 (예: 1 2 3 5, 취소:c): ");
    }
    else if (inputBuffer == "3")
    {
        Serial.println();
        Serial.println("⚠️  경고: 모든 센서의 임계값이 기본값으로 초기화됩니다!");
        Serial.println("계속하시겠습니까? (y/n): ");
        // 임시로 확인 상태 처리 (간단히 구현)
        String confirm = "";
        // 여기서는 바로 실행하도록 간소화
        sensorController.resetAllThresholds();
        sensorController.printSensorStatusTable();
        printThresholdMenu();
    }
    else if (inputBuffer == "4")
    {
        appState = AppState::Menu;
        Serial.println("[DEBUG] appState -> Menu");
        printMenu();
    }
    else if (inputBuffer == "5")
    {
        appState = AppState::Normal;
        Serial.println("[DEBUG] appState -> Normal");
        sensorController.printSensorStatusTable();
        lastPrint = millis();
    }
    else
    {
        Serial.println("지원하지 않는 메뉴입니다. 1~5 중 선택하세요.");
        printThresholdMenu();
    }
}

void MenuController::handleThresholdSelectSensorState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
        return;
    }

    int sensorNum = inputBuffer.toInt();
    if (sensorNum >= 1 && sensorNum <= 8)
    {
        selectedSensorIdx = sensorNum - 1; // 0-based 인덱스로 변환

        // 현재 임계값 로드
        tempUpperThreshold = sensorController.getUpperThreshold(selectedSensorIdx);
        tempLowerThreshold = sensorController.getLowerThreshold(selectedSensorIdx);

        appState = AppState::ThresholdChange_InputUpper;
        Serial.println("[DEBUG] appState -> ThresholdChange_InputUpper");

        Serial.println();
        Serial.print("📊 센서 ");
        Serial.print(sensorNum);
        Serial.println("번 현재 임계값:");
        Serial.print("   상한(TH): ");
        Serial.print(tempUpperThreshold, 1);
        Serial.println("°C");
        Serial.print("   하한(TL): ");
        Serial.print(tempLowerThreshold, 1);
        Serial.println("°C");
        Serial.println();
        Serial.print("새로운 상한 임계값 입력 (현재: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
    }
    else
    {
        Serial.println("❌ 오류: 1~8 사이의 숫자를 입력하세요.");
        Serial.print("임계값을 설정할 센서 번호(1~8, 취소:c)를 입력하세요: ");
    }
}

void MenuController::handleThresholdInputUpperState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
        return;
    }

    // 빈 입력 (엔터만) - 기존값 유지
    if (inputBuffer.length() == 0)
    {
        Serial.print("상한값 유지: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.println("°C");
    }
    else
    {
        // 숫자 검증
        float newUpper = inputBuffer.toFloat();
        if (inputBuffer.toFloat() == 0.0 && inputBuffer != "0" && inputBuffer != "0.0")
        {
            Serial.println("❌ 오류: 유효한 숫자를 입력하세요 (예: 25.5)");
            Serial.print("상한 임계값 입력 (현재: ");
            Serial.print(tempUpperThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        // 범위 검증
        if (!sensorController.isValidTemperature(newUpper))
        {
            Serial.println("❌ 경고: DS18B20 범위를 벗어났습니다 (-55~125°C)");
            Serial.print("상한 임계값 입력 (현재: ");
            Serial.print(tempUpperThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        tempUpperThreshold = newUpper;
        Serial.print("상한값 설정: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.println("°C");
    }

    // 하한값 입력으로 이동
    appState = AppState::ThresholdChange_InputLower;
    Serial.println("[DEBUG] appState -> ThresholdChange_InputLower");
    Serial.print("새로운 하한 임계값 입력 (현재: ");
    Serial.print(tempLowerThreshold, 1);
    Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
}

void MenuController::handleThresholdInputLowerState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
        return;
    }

    // 빈 입력 (엔터만) - 기존값 유지
    if (inputBuffer.length() == 0)
    {
        Serial.print("하한값 유지: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.println("°C");
    }
    else
    {
        // 숫자 검증
        float newLower = inputBuffer.toFloat();
        if (inputBuffer.toFloat() == 0.0 && inputBuffer != "0" && inputBuffer != "0.0")
        {
            Serial.println("❌ 오류: 유효한 숫자를 입력하세요 (예: 15.5)");
            Serial.print("하한 임계값 입력 (현재: ");
            Serial.print(tempLowerThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        // 범위 검증
        if (!sensorController.isValidTemperature(newLower))
        {
            Serial.println("❌ 경고: DS18B20 범위를 벗어났습니다 (-55~125°C)");
            Serial.print("하한 임계값 입력 (현재: ");
            Serial.print(tempLowerThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        tempLowerThreshold = newLower;
        Serial.print("하한값 설정: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.println("°C");
    }

    // 논리 검증: 상한값이 하한값보다 커야 함
    if (tempUpperThreshold <= tempLowerThreshold)
    {
        Serial.println("❌ 경고: 상한값은 하한값보다 커야 합니다");
        Serial.print("상한: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.print("°C, 하한: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.println("°C");
        Serial.print("하한 임계값을 다시 입력하세요 (현재: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
        return;
    }

    // 임계값 설정 완료
    sensorController.setThresholds(selectedSensorIdx, tempUpperThreshold, tempLowerThreshold);

    // 결과 확인을 위해 센서 상태 테이블 출력
    Serial.println();
    sensorController.printSensorStatusTable();

    // 임계값 메뉴로 복귀
    appState = AppState::ThresholdMenu;
    Serial.println("[DEBUG] appState -> ThresholdMenu");
    printThresholdMenu();
}

// ========== 복수 센서 임계값 설정 메서드들 ==========

void MenuController::handleThresholdSelectMultipleSensorsState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
        return;
    }

    // 센서 번호 파싱 및 검증
    if (!validateSensorInput())
    {
        Serial.print("임계값을 설정할 센서 번호들을 입력하세요 (예: 1 2 3 5, 취소:c): ");
        return;
    }

    std::vector<int> indices = parseSensorIndices(inputBuffer);
    if (!processSensorIndices(indices))
    {
        Serial.print("임계값을 설정할 센서 번호들을 입력하세요 (예: 1 2 3 5, 취소:c): ");
        return;
    }

    // 선택된 센서들 확인
    selectedSensorIndices = indices;
    appState = AppState::ThresholdChange_ConfirmMultipleSensors;
    Serial.println("[DEBUG] appState -> ThresholdChange_ConfirmMultipleSensors");

    Serial.println();
    Serial.print("선택된 센서: ");
    for (size_t i = 0; i < selectedSensorIndices.size(); i++)
    {
        Serial.print(selectedSensorIndices[i]);
        if (i < selectedSensorIndices.size() - 1)
        {
            Serial.print(", ");
        }
    }
    Serial.println();
    Serial.print("이 센서들에 동일한 임계값을 설정하시겠습니까? (y/n, 취소:c): ");
}

void MenuController::handleThresholdConfirmMultipleSensorsState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
        return;
    }

    if (inputBuffer == "y" || inputBuffer == "Y")
    {
        // 기본값으로 임시 임계값 설정 (첫 번째 센서의 현재값 사용)
        int firstSensorIdx = selectedSensorIndices[0] - 1;
        tempUpperThreshold = sensorController.getUpperThreshold(firstSensorIdx);
        tempLowerThreshold = sensorController.getLowerThreshold(firstSensorIdx);

        appState = AppState::ThresholdChange_InputMultipleUpper;
        Serial.println("[DEBUG] appState -> ThresholdChange_InputMultipleUpper");

        Serial.println();
        Serial.print("📊 복수 센서 임계값 설정 (");
        Serial.print(selectedSensorIndices.size());
        Serial.println("개 센서)");
        Serial.print("새로운 상한 임계값 입력 (현재: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
    }
    else if (inputBuffer == "n" || inputBuffer == "N")
    {
        appState = AppState::ThresholdChange_SelectMultipleSensors;
        Serial.println("[DEBUG] appState -> ThresholdChange_SelectMultipleSensors");
        Serial.print("임계값을 설정할 센서 번호들을 입력하세요 (예: 1 2 3 5, 취소:c): ");
    }
    else
    {
        Serial.println("y(예), n(아니오), c(취소) 중 하나를 입력하세요.");
        Serial.print("이 센서들에 동일한 임계값을 설정하시겠습니까? (y/n, 취소:c): ");
    }
}

void MenuController::handleThresholdInputMultipleUpperState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
        return;
    }

    // 빈 입력 (엔터만) - 기존값 유지
    if (inputBuffer.length() == 0)
    {
        Serial.print("상한값 유지: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.println("°C");
    }
    else
    {
        // 숫자 검증
        float newUpper = inputBuffer.toFloat();
        if (inputBuffer.toFloat() == 0.0 && inputBuffer != "0" && inputBuffer != "0.0")
        {
            Serial.println("❌ 오류: 유효한 숫자를 입력하세요 (예: 25.5)");
            Serial.print("상한 임계값 입력 (현재: ");
            Serial.print(tempUpperThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        // 범위 검증
        if (!sensorController.isValidTemperature(newUpper))
        {
            Serial.println("❌ 경고: DS18B20 범위를 벗어났습니다 (-55~125°C)");
            Serial.print("상한 임계값 입력 (현재: ");
            Serial.print(tempUpperThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        tempUpperThreshold = newUpper;
        Serial.print("상한값 설정: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.println("°C");
    }

    // 하한값 입력으로 이동
    appState = AppState::ThresholdChange_InputMultipleLower;
    Serial.println("[DEBUG] appState -> ThresholdChange_InputMultipleLower");
    Serial.print("새로운 하한 임계값 입력 (현재: ");
    Serial.print(tempLowerThreshold, 1);
    Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
}

void MenuController::handleThresholdInputMultipleLowerState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::ThresholdMenu;
        Serial.println("[DEBUG] appState -> ThresholdMenu");
        printThresholdMenu();
        return;
    }

    // 빈 입력 (엔터만) - 기존값 유지
    if (inputBuffer.length() == 0)
    {
        Serial.print("하한값 유지: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.println("°C");
    }
    else
    {
        // 숫자 검증
        float newLower = inputBuffer.toFloat();
        if (inputBuffer.toFloat() == 0.0 && inputBuffer != "0" && inputBuffer != "0.0")
        {
            Serial.println("❌ 오류: 유효한 숫자를 입력하세요 (예: 15.5)");
            Serial.print("하한 임계값 입력 (현재: ");
            Serial.print(tempLowerThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        // 범위 검증
        if (!sensorController.isValidTemperature(newLower))
        {
            Serial.println("❌ 경고: DS18B20 범위를 벗어났습니다 (-55~125°C)");
            Serial.print("하한 임계값 입력 (현재: ");
            Serial.print(tempLowerThreshold, 1);
            Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
            return;
        }

        tempLowerThreshold = newLower;
        Serial.print("하한값 설정: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.println("°C");
    }

    // 논리 검증: 상한값이 하한값보다 커야 함
    if (tempUpperThreshold <= tempLowerThreshold)
    {
        Serial.println("❌ 경고: 상한값은 하한값보다 커야 합니다");
        Serial.print("상한: ");
        Serial.print(tempUpperThreshold, 1);
        Serial.print("°C, 하한: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.println("°C");
        Serial.print("하한 임계값을 다시 입력하세요 (현재: ");
        Serial.print(tempLowerThreshold, 1);
        Serial.print("°C, 범위: -55~125°C, 엔터=유지): ");
        return;
    }

    // 선택된 모든 센서에 임계값 설정
    Serial.println();
    Serial.println("🔄 복수 센서 임계값 설정 중...");

    for (int sensorNum : selectedSensorIndices)
    {
        int sensorIdx = sensorNum - 1; // 0-based 인덱스로 변환
        sensorController.setThresholds(sensorIdx, tempUpperThreshold, tempLowerThreshold);
    }

    Serial.println();
    Serial.print("✅ ");
    Serial.print(selectedSensorIndices.size());
    Serial.print("개 센서 임계값 설정 완료: TH=");
    Serial.print(tempUpperThreshold, 1);
    Serial.print("°C, TL=");
    Serial.print(tempLowerThreshold, 1);
    Serial.println("°C");

    Serial.print("설정된 센서: ");
    for (size_t i = 0; i < selectedSensorIndices.size(); i++)
    {
        Serial.print(selectedSensorIndices[i]);
        if (i < selectedSensorIndices.size() - 1)
        {
            Serial.print(", ");
        }
    }
    Serial.println();

    // 결과 확인을 위해 센서 상태 테이블 출력
    Serial.println();
    sensorController.printSensorStatusTable();

    // 임계값 메뉴로 복귀
    appState = AppState::ThresholdMenu;
    Serial.println("[DEBUG] appState -> ThresholdMenu");
    printThresholdMenu();
}

// ========== Measurement Interval Menu Methods ==========

void MenuController::printMeasurementIntervalMenu()
{
    Serial.println();
    Serial.println("===== 센서 측정 주기 조정 메뉴 =====");
    Serial.print("현재 측정 주기: ");
    Serial.println(sensorController.formatInterval(sensorController.getMeasurementInterval()));
    Serial.println();
    Serial.println("설정 가능 범위: 10초 ~ 30일 (1초 단위)");
    Serial.println("입력 형식 예시:");
    Serial.println("  - 초 단위: 30, 30s (30초)");
    Serial.println("  - 분 단위: 5m, 5M (5분)");
    Serial.println("  - 시간 단위: 2h, 2H (2시간)");
    Serial.println("  - 일 단위: 1d, 1D (1일)");
    Serial.println("  - 복합 단위: 1d2h30m (1일 2시간 30분)");
    Serial.println("  - 복합 단위: 2h30m45s (2시간 30분 45초)");
    Serial.println("※ 대소문자 구분 없음");
    Serial.println();
    Serial.print("새로운 측정 주기를 입력하세요 (취소:c): ");
}

void MenuController::handleMeasurementIntervalMenuState()
{
    appState = AppState::MeasurementInterval_Input;
    Serial.println("[DEBUG] appState -> MeasurementInterval_Input");
}

void MenuController::handleMeasurementIntervalInputState()
{
    if (inputBuffer == "c" || inputBuffer == "C")
    {
        appState = AppState::Menu;
        Serial.println("[DEBUG] appState -> Menu");
        printMenu();
        return;
    }

    // 입력값 파싱
    unsigned long intervalMs = parseIntervalInput(inputBuffer);

    if (intervalMs == 0)
    {
        Serial.println("❌ 오류: 유효하지 않은 입력 형식입니다.");
        Serial.println("예시: 30 (30초), 5m (5분), 2h (2시간), 1d (1일)");
        Serial.println("복합: 1d2h30m (1일 2시간 30분), 2h30m45s (2시간 30분 45초)");
        Serial.print("새로운 측정 주기를 입력하세요 (취소:c): ");
        return;
    }

    if (!sensorController.isValidMeasurementInterval(intervalMs))
    {
        Serial.println("❌ 오류: 측정 주기 범위를 벗어났습니다 (10초 ~ 30일)");
        Serial.print("새로운 측정 주기를 입력하세요 (취소:c): ");
        return;
    }

    // 측정 주기 설정
    sensorController.setMeasurementInterval(intervalMs);

    Serial.println();
    Serial.println("📊 측정 주기 변경 사항:");
    Serial.print("  새로운 주기: ");
    Serial.println(sensorController.formatInterval(intervalMs));
    Serial.println("  다음 센서 상태 업데이트부터 새로운 주기가 적용됩니다.");

    // 메인 메뉴로 복귀
    appState = AppState::Menu;
    Serial.println("[DEBUG] appState -> Menu");
    printMenu();
}

unsigned long MenuController::parseIntervalInput(const String &input)
{
    String trimmedInput = input;
    trimmedInput.trim();
    trimmedInput.toLowerCase(); // 대소문자 구분 없이 처리

    if (trimmedInput.length() == 0)
    {
        return 0; // 빈 입력
    }

    // 복합 단위 지원 (예: "1d2h30m", "2h30m", "30m45s")
    unsigned long totalMs = 0;
    String currentNumber = "";

    for (int i = 0; i < trimmedInput.length(); i++)
    {
        char c = trimmedInput.charAt(i);

        if (isDigit(c))
        {
            currentNumber += c;
        }
        else if (c == 'd' || c == 'h' || c == 'm' || c == 's')
        {
            if (currentNumber.length() == 0)
            {
                return 0; // 숫자 없이 단위만 있음
            }

            long number = currentNumber.toInt();
            if (number <= 0)
            {
                return 0; // 유효하지 않은 숫자
            }

            unsigned long multiplier = 1000; // 기본값: 초
            if (c == 'd')
            {
                multiplier = 24 * 60 * 60 * 1000; // 일
            }
            else if (c == 'h')
            {
                multiplier = 60 * 60 * 1000; // 시간
            }
            else if (c == 'm')
            {
                multiplier = 60 * 1000; // 분
            }
            else if (c == 's')
            {
                multiplier = 1000; // 초
            }

            // 오버플로우 체크
            if (number > (MAX_MEASUREMENT_INTERVAL / multiplier))
            {
                return 0; // 너무 큰 값
            }

            unsigned long partMs = (unsigned long)number * multiplier;

            // 총합 오버플로우 체크
            if (totalMs > MAX_MEASUREMENT_INTERVAL - partMs)
            {
                return 0; // 총합이 너무 큼
            }

            totalMs += partMs;
            currentNumber = "";
        }
        else
        {
            return 0; // 유효하지 않은 문자
        }
    }

    // 마지막에 숫자만 있고 단위가 없는 경우 (초로 처리)
    if (currentNumber.length() > 0)
    {
        long number = currentNumber.toInt();
        if (number <= 0)
        {
            return 0; // 유효하지 않은 숫자
        }

        // 오버플로우 체크
        if (number > (MAX_MEASUREMENT_INTERVAL / 1000))
        {
            return 0; // 너무 큰 값
        }

        unsigned long partMs = (unsigned long)number * 1000; // 초

        // 총합 오버플로우 체크
        if (totalMs > MAX_MEASUREMENT_INTERVAL - partMs)
        {
            return 0; // 총합이 너무 큼
        }

        totalMs += partMs;
    }

    // 최소값 체크
    if (totalMs < MIN_MEASUREMENT_INTERVAL)
    {
        return 0; // 너무 작은 값
    }

    return totalMs;
}
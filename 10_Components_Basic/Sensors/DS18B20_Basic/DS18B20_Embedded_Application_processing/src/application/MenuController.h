#pragma once
#include <Arduino.h>
#include <vector>
#include "InputHandler.h"
#include "SensorMenuHandler.h"

enum class AppState
{
    Normal,
    Menu,
    SensorIdMenu,
    SensorIdChange_SelectSensor,
    SensorIdChange_ConfirmSensor,
    SensorIdChange_InputId,
    SensorIdChange_ConfirmId,
    SensorIdChange_Apply,
    SensorIdChange_ConfirmReset,
    ThresholdMenu,
    ThresholdChange_SelectSensor,
    ThresholdChange_InputUpper,
    ThresholdChange_InputLower,
    ThresholdChange_SelectMultipleSensors,
    ThresholdChange_ConfirmMultipleSensors,
    ThresholdChange_InputMultipleUpper,
    ThresholdChange_InputMultipleLower,
    MeasurementIntervalMenu,
    MeasurementInterval_Input,
};

class MenuController
{
public:
    // 복수 센서 인덱스 파싱 함수
    static std::vector<int> parseSensorIndices(const String &input);
    MenuController();

    void printMenu();
    void printSensorIdMenu();
    void printThresholdMenu();
    void printMeasurementIntervalMenu();
    void handleSerialInput();

    AppState getAppState() const { return appState; }
    void setAppState(AppState state) { appState = state; }
    void resetToNormalState(); // 상태를 Normal로 완전히 리셋하는 함수

    int getSelectedSensorIdx() const { return selectedSensorIdx; }
    int getSelectedDisplayIdx() const { return selectedDisplayIdx; }

private:
    // 복수 센서 선택용 멤버 변수
    std::vector<int> selectedSensorIndices;
    AppState appState;
    String inputBuffer;
    int selectedSensorIdx;
    int selectedDisplayIdx;
    bool isMultiSelectMode = false;
    
    // 헬퍼 클래스들
    InputHandler inputHandler;
    SensorMenuHandler sensorMenuHandler;
    
    // 임계값 설정용 임시 변수들
    float tempUpperThreshold;
    float tempLowerThreshold;

    void handleNormalState();
    void handleMenuState();
    void handleSensorIdMenuState();
    void handleSensorIdSelectState();
    void handleSensorIdConfirmState();
    void handleSensorIdInputState();
    void handleSensorIdConfirmResetState();
    
    // 임계값 설정 관련 메서드
    void handleThresholdMenuState();
    void handleThresholdSelectSensorState();
    void handleThresholdInputUpperState();
    void handleThresholdInputLowerState();
    
    // 복수 센서 임계값 설정 관련 메서드
    void handleThresholdSelectMultipleSensorsState();
    void handleThresholdConfirmMultipleSensorsState();
    void handleThresholdInputMultipleUpperState();
    void handleThresholdInputMultipleLowerState();
    
    // 측정 주기 설정 관련 메서드
    void handleMeasurementIntervalMenuState();
    void handleMeasurementIntervalInputState();
    
    // 측정 주기 입력 파싱 헬퍼 메서드
    unsigned long parseIntervalInput(const String& input);
    
    // Helper methods for handleSensorIdSelectState
    bool validateSensorInput();
    bool processSensorIndices(const std::vector<int>& indices);
    void proceedToSensorConfirmation();
    void printSensorSelectionPrompt();
    
    // Helper methods for handleSensorIdInputState
    void handleCancelInput();
    bool processNewSensorId(int newId);
    void moveToNextSensor();
    void completeIdChange();
    void printIdInputPrompt();
    
    // Helper methods for handleSensorIdConfirmState
    void handleConfirmYes();
    void handleConfirmNoOrCancel();
    void handleInvalidConfirmInput();
    void moveToNextSensorInConfirm();
    
    // Helper methods for handleSerialInput
    void processInputBuffer();
    bool handleGlobalResetCommand();
    void processStateBasedInput();
    void clearInputBuffer();
};

#ifndef APP_H
#define APP_H

#ifdef UNIT_TEST
#include "FakeArduino.h" // String, DeviceAddress, millis, delay 등을 위해 먼저 포함
#else
#include <Arduino.h>
#include <stdint.h>
#endif

// 전방 선언: 실제 클래스 정의를 포함하는 대신 컴파일러에게 클래스의 존재만 알립니다.
// 이는 헤더 파일 간의 순환 종속성을 방지하고 컴파일 시간을 줄여줍니다.
class DS18B20_Sensor;
class DataProcessor;
class SerialUI;
class Communicator;
#if defined(MEMORY_ANALYSIS_ENABLED) && MEMORY_ANALYSIS_ENABLED == 1
class MemoryUsageTester;
#endif

// 애플리케이션의 상태를 나타내는 열거형
enum AppState
{
    NORMAL_OPERATION,
    MAIN_MENU,
    ID_MANAGEMENT_MENU,
    EDIT_INDIVIDUAL_ID,
    EDIT_SELECTIVE_ID,
    AUTO_ASSIGN_IDS,
    RESET_ALL_IDS,
    THRESHOLD_MENU,
    SET_INDIVIDUAL_THRESHOLD,
    SET_GLOBAL_THRESHOLD
};

// ID 변경 메뉴의 세부 상태를 나타내는 열거형
enum EditIdSubState
{
    EDIT_ID_NONE,
    EDIT_ID_SELECT_SENSOR,
    EDIT_ID_ENTER_NEW_ID,
    EDIT_SELECTIVE_GET_SENSORS,
    EDIT_SELECTIVE_CONFIRM,
    EDIT_SELECTIVE_ENTER_NEW_IDS
};

// 임계값 설정 메뉴의 세부 상태를 나타내는 열거형
enum ThresholdSubState
{
    THRESHOLD_NONE,
    THRESHOLD_SELECT_SENSOR,
    THRESHOLD_ENTER_HIGH,
    THRESHOLD_ENTER_LOW,
    THRESHOLD_CONFIRM,
    THRESHOLD_CLEAR_CONFIRM
};

// ID 변경 대상 센서의 정보를 담는 구조체
struct SensorInfoToChange
{
    uint8_t addr[8];
    int currentId;
    int index;
};

class App
{
public:
    // 생성자: 외부에서 생성된 객체들의 포인터를 받아 초기화합니다. (의존성 주입)
    App(DS18B20_Sensor *sensorManager, DataProcessor *dataProcessor, Communicator *comm, SerialUI *ui, void *memoryTester);

    // 애플리케이션 초기화 메소드
    void init();

    // 애플리케이션의 메인 루프 역할을 하는 메소드
    void run();

private:
    // .ino 파일에 있던 함수들을 클래스의 private 메소드로 이동
    void initializeSerial();
    void showDeviceTable();
    void show18b20Table_loop();
    void showTroubleshootingGuide();
    void processEditSelectiveId();
    void parseSensorSelection(const char* input);
    void printCurrentSensorReport();
    void startSensorSearchMode();
    void performSensorSearch();
    void enterNoSensorMode();
    void openMainMenu();
    void checkSensorStatus();
    void handleMainMenuInput();
    void openIdManagementMenu();
    void handleIdMenuInput();
    void processEditIndividualId();
    void initialize18b20();
    void processAutoAssignIds();
    void resetAllSensorIds();
    void displaySensorTable();
    void displayThresholdMenu();
    void processThresholdMenuInput();
    void processIndividualThreshold();
    void processGlobalThreshold();
    void viewCurrentThresholds();
    void clearAllThresholds();
    void handleUserCommands();

    // .ino 파일에 있던 전역 변수들을 클래스의 private 멤버 변수로 이동
    DS18B20_Sensor *_sensorManager;
    DataProcessor *_dataProcessor;
    Communicator *_comm;
    SerialUI *_ui;
    void *_memoryTester;

    EditIdSubState currentEditIdSubState;
    ThresholdSubState currentThresholdSubState;

public: // For testing
    AppState currentAppState;

    SensorInfoToChange sensorsToChangeArray[8]; // SENSOR_COUNT 대신 상수 사용
    int sensorsToChangeCount;
    int selectedSensorIndex;
    unsigned long editIdTimeoutMillis;

    int selectedSensorIndices[8]; // SENSOR_COUNT 대신 상수 사용
    int selectedSensorIndicesCount;
    int currentSelectiveIndex;

    unsigned long previousMillis;
    unsigned long menuDisplayMillis;
    bool forceMenuPrint;

    // 센서 검색 모드 관련 변수
    unsigned long sensorSearchStartTime;
    int sensorSearchAttempts;
    bool isInSensorSearchMode;
    static const int MAX_SEARCH_ATTEMPTS = 5;
    static const unsigned long SEARCH_TIMEOUT_MS = 30000; // 30초

    // 임계값 설정 관련 변수
    int selectedThresholdSensorIndex;
    float pendingHighThreshold;
    float pendingLowThreshold;
    bool hasHighThreshold;
    bool hasLowThreshold;

    // 비차단 일괄 작업을 위한 변수
    int currentProcessIndex;
    unsigned long processLastMillis;
};

#endif // APP_H
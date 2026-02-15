/*
배선도
DS18B20 온도 센서와 아두이노 연결
DS18B20 ------------- Arduino
VCC ------------- 5V
GND ------------- GND
DQ ------------- D2
*/

#include "exclude_memory_analysis.h" // 항상 가장 먼저 포함
#include "config.h"
#include "DS18B20_Sensor.h"
#include "DataProcessor.h"
#include "Communicator.h"
#include "Utils.h"
#include "App.h"

#if MEMORY_ANALYSIS_ENABLED
#include "MemoryUsageTester.h"
#endif

// 실제 하드웨어와 통신하는 객체들을 생성합니다.
OneWire oneWire(ONE_WIRE_BUS);
DS18B20_Sensor sensorManager(&oneWire);
Communicator comm;
DataProcessor dataProcessor(&sensorManager, &comm);

// App 클래스의 인스턴스를 생성하고, 위에서 만든 객체들을 주입합니다.
#if MEMORY_ANALYSIS_ENABLED
// 메모리 사용량 측정을 위한 인스턴스
MemoryUsageTester memTester;
App app(&sensorManager, &dataProcessor, &comm, &memTester);
#else
// 메모리 분석 없이 App 인스턴스 생성
App app(&sensorManager, &dataProcessor, &comm, nullptr);
#endif

#if MEMORY_ANALYSIS_ENABLED
// 메모리 분석 설정
bool memoryAnalysisEnabled = false; // 기본적으로 비활성화
unsigned long lastMemoryCheck = 0;
const unsigned long memoryCheckInterval = 10000; // 10초마다 메모리 상태 확인

// 함수 선언 (forward declaration)
void performRuntimeMemoryTest();
#endif

// 함수 선언
void printHelpMenu();

void setup()
{
    // DS18B20 온도 센서 초기화
    Serial.begin(9600); // 시리얼 통신 초기화
    while (!Serial)
        ; // USB CDC 보드에서 시리얼 연결 대기 (필요시)

#if MEMORY_ANALYSIS_ENABLED
    Serial.println("=== DS18B20 + MEMORY ANALYSIS MODE ===");

    // 초기 메모리 상태 출력
    if (memoryAnalysisEnabled)
    {
        Serial.println("=== INITIAL MEMORY STATUS ===");
        memTester.printMemoryStatus("Before app initialization");
        memTester.printMemoryStatusCSV("setup", "start");
    }
#else
    Serial.println("=== DS18B20 SENSOR MODE ===");
#endif
    Serial.println("Serial OK");

    // App 초기화
    app.init();

#if MEMORY_ANALYSIS_ENABLED
    // 초기화 후 메모리 상태 확인
    if (memoryAnalysisEnabled)
    {
        memTester.printMemoryStatus("After app initialization");
        memTester.printMemoryStatusCSV("setup", "app_init_complete");
        Serial.println("=== INITIAL MEMORY ANALYSIS COMPLETE ===");
    }
    Serial.println("System ready - DS18B20 sensors + Memory analysis");
#else
    Serial.println("System ready - DS18B20 sensors");
#endif
}

void loop()
{
#if MEMORY_ANALYSIS_ENABLED
    // 주기적 메모리 분석 (백그라운드)
    if (memoryAnalysisEnabled && (millis() - lastMemoryCheck > memoryCheckInterval))
    {
        lastMemoryCheck = millis();

        Serial.println("=== PERIODIC MEMORY CHECK ===");
        memTester.printMemoryStatus("Periodic check");
        memTester.printMemoryStatusCSV("loop", "periodic_check");

        // 간단한 메모리 할당 테스트 (실제 사용 시나리오 시뮬레이션)
        performRuntimeMemoryTest();
    }
#endif

    // 시리얼 명령어 처리
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        input.trim();

#if MEMORY_ANALYSIS_ENABLED
        if (input == "memtest")
        {
            Serial.println("=== MANUAL MEMORY TEST ===");
            memTester.runBasicTest();
        }
        else if (input == "memstress")
        {
            Serial.println("=== MANUAL STRESS TEST ===");
            memTester.runStressTest(20);
        }
        else if (input == "memstruct")
        {
            Serial.println("=== MANUAL DATA STRUCTURE TEST ===");
            memTester.testDataStructures();
        }
        else if (input == "memfree")
        {
            memTester.printMemoryStatus("Manual check");
        }
        else if (input == "memtoggle")
        {
            memoryAnalysisEnabled = !memoryAnalysisEnabled;
            Serial.print("Memory analysis: ");
            Serial.println(memoryAnalysisEnabled ? "ENABLED" : "DISABLED");
        }
        else
#endif
            if (input == "help" || input == "h")
        {
            printHelpMenu();
        }
        else
        {
            // 기존 App 명령어들도 계속 처리
            // 여기에 기존 앱의 명령어 처리 로직을 유지할 수 있음
            Serial.println("Unknown command. Type 'help' for available commands.");
        }
    }

    // App의 메인 루프 실행 (DS18B20 센서 동작)
    app.run();
}

#if MEMORY_ANALYSIS_ENABLED
// 런타임 메모리 테스트 (실제 사용 시나리오 시뮬레이션)
void performRuntimeMemoryTest()
{
    // 온도 센서 데이터 처리 시 메모리 사용량 시뮬레이션
    String *tempStrings = new String[5];
    if (tempStrings != nullptr)
    {
        for (int i = 0; i < 5; i++)
        {
            tempStrings[i] = "Sensor_" + String(i) + "_Temp: 25.5C";
        }
        Serial.println("Runtime memory allocation: SUCCESS");
        memTester.printMemoryStatusCSV("runtime", "allocation_success");

        delete[] tempStrings;
        Serial.println("Runtime memory deallocation: SUCCESS");
        memTester.printMemoryStatusCSV("runtime", "deallocation_success");
    }
    else
    {
        Serial.println("Runtime memory allocation: FAILED");
        memTester.printMemoryStatusCSV("runtime", "allocation_failed");
    }
}
#endif

// 도움말 메뉴 출력
void printHelpMenu()
{
    Serial.println("=== AVAILABLE COMMANDS ===");
#if MEMORY_ANALYSIS_ENABLED
    Serial.println("DS18B20 + Memory Analysis System");
    Serial.println("Memory Commands:");
    Serial.println("  memtest    - Run basic memory test");
    Serial.println("  memstress  - Run stress memory test");
    Serial.println("  memstruct  - Run data structure test");
    Serial.println("  memfree    - Show current memory status");
    Serial.println("  memtoggle  - Enable/disable periodic memory analysis");
#else
    Serial.println("DS18B20 System");
#endif
    Serial.println("General Commands:");
    Serial.println("  help, h    - Show this help menu");
    Serial.println("  menu, m    - DS18B20 sensor menu (if available)");
    Serial.println("========================");
}

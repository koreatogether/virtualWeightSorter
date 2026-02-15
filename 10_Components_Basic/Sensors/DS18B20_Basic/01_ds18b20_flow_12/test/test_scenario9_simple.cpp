#include "unity/unity.h"
#include "App.h"
#include "MockDS18B20_Sensor.h"
#include "MockDataProcessor.h"
#include "MockCommunicator.h"
#include "MockMemoryUsageTester.h"
#include "TestGlobals.h"

// 로컬 변수로 선언 (TestGlobals.h의 extern과 충돌 방지)
static MockDS18B20_Sensor *localMockSensor;
static MockDataProcessor *localMockProcessor;
static MockCommunicator *localMockComm;
static MockMemoryUsageTester *localMockMemory;
static App *localApp;

// 테스트 초기화 헬퍼 함수
void setupTestEnvironment(void)
{
    localMockSensor = new MockDS18B20_Sensor();
    localMockComm = new MockCommunicator();
    localMockProcessor = new MockDataProcessor(localMockSensor, localMockComm);
    localMockMemory = new MockMemoryUsageTester();
    localApp = new App(localMockSensor, localMockProcessor, localMockComm, localMockMemory);

    // 기본 설정: 3개 센서 존재
    localMockSensor->setDeviceCount(3);

    // 기본 주소 설정 (테스트를 위한 더미 주소)
    DeviceAddress addr1 = {0x28, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    DeviceAddress addr2 = {0x28, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    DeviceAddress addr3 = {0x28, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    localMockSensor->addDevice(addr1, 1);
    localMockSensor->addDevice(addr2, 2);
    localMockSensor->addDevice(addr3, 3);
}

void tearDownTestEnvironment(void)
{
    delete localApp;
    delete localMockComm;
    delete localMockProcessor;
    delete localMockSensor;
}

// 시나리오 9: 개별 센서 임계값 설정 테스트
void test_individual_threshold_setting_normal_flow(void)
{
    setupTestEnvironment();

    // Given: 메뉴 활성화 상태
    localApp->currentAppState = THRESHOLD_MENU;

    // When: 개별 임계값 설정 선택
    localMockComm->addInput("1");
    while (localApp->currentAppState == THRESHOLD_MENU)
    {
        localApp->processThresholdMenuInput();
    }

    // Then: 개별 임계값 설정 상태로 전환
    TEST_ASSERT_EQUAL(SET_INDIVIDUAL_THRESHOLD, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);

    tearDownTestEnvironment();
}

void test_global_threshold_setting_normal_flow(void)
{
    setupTestEnvironment();

    // Given: 임계값 메뉴 상태
    localApp->currentAppState = THRESHOLD_MENU;

    // When: 전체 일괄 설정 선택
    localMockComm->addInput("2");
    while (localApp->currentAppState == THRESHOLD_MENU)
    {
        localApp->processThresholdMenuInput();
    }

    // Then: 전체 일괄 설정 상태로 전환
    TEST_ASSERT_EQUAL(SET_GLOBAL_THRESHOLD, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);

    tearDownTestEnvironment();
}

void test_threshold_menu_back_to_main(void)
{
    setupTestEnvironment();

    // Given: 임계값 메뉴 상태
    localApp->currentAppState = THRESHOLD_MENU;

    // When: 메인 메뉴로 돌아가기 선택
    localMockComm->addInput("5");
    while (localApp->currentAppState == THRESHOLD_MENU)
    {
        localApp->processThresholdMenuInput();
    }

    // Then: 메인 메뉴로 돌아감
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);

    tearDownTestEnvironment();
}

void runThresholdScenarioTests(void)
{
    RUN_TEST(test_individual_threshold_setting_normal_flow);
    RUN_TEST(test_global_threshold_setting_normal_flow);
    RUN_TEST(test_threshold_menu_back_to_main);
}

int run_scenario9_tests(void)
{
    UNITY_BEGIN();
    runThresholdScenarioTests();
    return UNITY_END();
}

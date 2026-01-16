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

void setUp(void)
{
    localMockSensor = new MockDS18B20_Sensor();
    localMockComm = new MockCommunicator();
    localMockProcessor = new MockDataProcessor(localMockSensor, localMockComm);
    localMockMemory = new MockMemoryUsageTester();
    localApp = new App(localMockSensor, localMockProcessor, localMockComm, localMockMemory);

    // 기본 설정: 3개 센서 존재
    localMockSensor->setDeviceCount(3);
    localMockSensor->setUserDataByIndex(0, 1); // 센서 1번
    localMockSensor->setUserDataByIndex(1, 2); // 센서 2번
    localMockSensor->setUserDataByIndex(2, 3); // 센서 3번

    // 기본 주소 ?�정 (?�스?��? ?�한 ?��? 주소)
    DeviceAddress addr1 = {0x28, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    DeviceAddress addr2 = {0x28, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    DeviceAddress addr3 = {0x28, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    locallocalMockSensor->addDevice(addr1, 1);
    locallocalMockSensor->addDevice(addr2, 2);
    locallocalMockSensor->addDevice(addr3, 3);
}

void tearDown(void)
{
    delete localApp;
    delete localMockComm;
    delete localMockProcessor;
    delete localMockSensor;
} // ?�나리오 9: 개별 ?�서 ?�계�??�정 ?�스??
void test_individual_threshold_setting_normal_flow(void)
{
    // Given: 메뉴 ?�성???�태
    localApp->currentAppState = THRESHOLD_MENU;

    // When: 개별 ?�계�??�정 ?�택
    localMockComm->addInput("1");
    localApp->processThresholdMenuInput();

    // Then: 개별 ?�계�??�정 ?�태�??�환
    TEST_ASSERT_EQUAL(SET_INDIVIDUAL_THRESHOLD, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);
}

void test_individual_threshold_sensor_selection(void)
{
    // Given: 개별 ?�계�??�정 ?�태
    localApp->currentAppState = SET_INDIVIDUAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_NONE;

    // When: ?�서 목록 ?�시 �??�서 ?�택
    localApp->processIndividualThreshold(); // ?�서 목록 ?�시

    // Then: ?�서 ?�택 ?�태�??�환
    TEST_ASSERT_EQUAL(THRESHOLD_SELECT_SENSOR, localApp->currentThresholdSubState);

    // When: ?�서 1�??�택
    localMockComm->addInput("1");
    localApp->processIndividualThreshold();

    // Then: ?�한 ?�도 ?�력 ?�태�??�환
    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_HIGH, localApp->currentThresholdSubState);
    TEST_ASSERT_EQUAL(0, localApp->selectedThresholdSensorIndex); // ?�서 1�?(index 0)
}

void test_individual_threshold_high_temperature_setting(void)
{
    // Given: ?�한 ?�도 ?�력 ?�태
    localApp->currentAppState = SET_INDIVIDUAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_ENTER_HIGH;
    localApp->selectedThresholdSensorIndex = 0;

    // When: ?�한 ?�도 75???�력
    localMockComm->addInput("75");
    localApp->processIndividualThreshold();

    // Then: ?�한 ?�도 ?�력 ?�태�??�환, ?�한 ?�도 ?�??
    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_LOW, localApp->currentThresholdSubState);
    TEST_ASSERT_TRUE(localApp->hasHighThreshold);
    TEST_ASSERT_EQUAL_FLOAT(75.0, localApp->pendingHighThreshold);
}

void test_individual_threshold_low_temperature_setting(void)
{
    // Given: ?�한 ?�도 ?�력 ?�태, ?�한 ?�도 ?��? ?�정??
    localApp->currentAppState = SET_INDIVIDUAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_ENTER_LOW;
    localApp->selectedThresholdSensorIndex = 0;
    localApp->hasHighThreshold = true;
    localApp->pendingHighThreshold = 75.0;

    // Mock ?�서??setThresholdHighByIndex, setThresholdLowByIndex ?�출 ?�공 ?�정
    localMockSensor->setThresholdResult(true);

    // When: ?�한 ?�도 10???�력
    localMockComm->addInput("10");
    localApp->processIndividualThreshold();

    // Then: ?�계�??�용, 메뉴�??�아�?
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);
    TEST_ASSERT_TRUE(localApp->hasLowThreshold);
    TEST_ASSERT_EQUAL_FLOAT(10.0, localApp->pendingLowThreshold);

    // Mock ?�서 메서???�출 ?�인
    TEST_ASSERT_TRUE(localMockSensor->wasSetThresholdHighByIndexCalled());
    TEST_ASSERT_TRUE(localMockSensor->wasSetThresholdLowByIndexCalled());
}

void test_individual_threshold_skip_high_temperature(void)
{
    // Given: ?�한 ?�도 ?�력 ?�태
    localApp->currentAppState = SET_INDIVIDUAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_ENTER_HIGH;
    localApp->selectedThresholdSensorIndex = 0;

    // When: Enter�??�력 (?�한 ?�도 ?�킵)
    localMockComm->addInput("");
    localApp->processIndividualThreshold();

    // Then: ?�한 ?�도 ?�력 ?�태�??�환, ?�한 ?�도 ?�킵??
    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_LOW, localApp->currentThresholdSubState);
    TEST_ASSERT_FALSE(localApp->hasHighThreshold);
}

void test_individual_threshold_invalid_temperature_range(void)
{
    // Given: ?�한 ?�도 ?�력 ?�태
    localApp->currentAppState = SET_INDIVIDUAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_ENTER_HIGH;
    localApp->selectedThresholdSensorIndex = 0;

    // When: 범위�?벗어???�도 ?�력 (150??
    localMockComm->addInput("150");
    localApp->processIndividualThreshold();

    // Then: ?�태 변�??�음 (?�시 ?�력 ?��?
    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_HIGH, localApp->currentThresholdSubState);
    TEST_ASSERT_FALSE(localApp->hasHighThreshold);
}

void test_individual_threshold_low_greater_than_high_error(void)
{
    // Given: ?�한 ?�도 ?�력 ?�태, ?�한 ?�도 50?�로 ?�정??
    localApp->currentAppState = SET_INDIVIDUAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_ENTER_LOW;
    localApp->selectedThresholdSensorIndex = 0;
    localApp->hasHighThreshold = true;
    localApp->pendingHighThreshold = 50.0;

    // When: ?�한보다 ?��? ?�한 ?�도 ?�력 (60??
    localMockComm->addInput("60");
    localApp->processIndividualThreshold();

    // Then: ?�태 변�??�음 (?�시 ?�력 ?��?
    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_LOW, localApp->currentThresholdSubState);
    TEST_ASSERT_FALSE(localApp->hasLowThreshold);
}

void test_individual_threshold_cancel_operation(void)
{
    // Given: ?�서 ?�택 ?�태
    localApp->currentAppState = SET_INDIVIDUAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_SELECT_SENSOR;

    // When: 취소 명령 ?�력
    localMockComm->addInput("c");
    localApp->processIndividualThreshold();

    // Then: ?�계�?메뉴�??�아�?
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);
}

// ?�나리오 10: ?�체 ?�서 ?�괄 ?�계�??�정 ?�스??
void test_global_threshold_setting_normal_flow(void)
{
    // Given: ?�계�?메뉴 ?�태
    localApp->currentAppState = THRESHOLD_MENU;

    // When: ?�체 ?�괄 ?�정 ?�택
    localMockComm->addInput("2");
    localApp->processThresholdMenuInput();

    // Then: ?�체 ?�괄 ?�정 ?�태�??�환
    TEST_ASSERT_EQUAL(SET_GLOBAL_THRESHOLD, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);
}

void test_global_threshold_both_temperatures_setting(void)
{
    // Given: ?�체 ?�괄 ?�정 ?�태, ?�한 ?�도 ?�력 준�?
    localApp->currentAppState = SET_GLOBAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_NONE;

    // When: ?�한 ?�도 80???�력
    localApp->processGlobalThreshold(); // 초기 메시지 ?�시
    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_HIGH, localApp->currentThresholdSubState);

    localMockComm->addInput("80");
    localApp->processGlobalThreshold();

    // Then: ?�한 ?�도 ?�력 ?�태�??�환
    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_LOW, localApp->currentThresholdSubState);
    TEST_ASSERT_TRUE(localApp->hasHighThreshold);
    TEST_ASSERT_EQUAL_FLOAT(80.0, localApp->pendingHighThreshold);

    // Mock ?�서???�괄 ?�정 ?�공 ?�정
    localMockSensor->setThresholdResult(true);

    // When: ?�한 ?�도 5???�력
    localMockComm->addInput("5");
    localApp->processGlobalThreshold();

    // Then: 모든 ?�서???�용, 메뉴�??�아�?
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);
    TEST_ASSERT_TRUE(localApp->hasLowThreshold);
    TEST_ASSERT_EQUAL_FLOAT(5.0, localApp->pendingLowThreshold);
}

void test_global_threshold_only_high_temperature(void)
{
    // Given: ?�체 ?�괄 ?�정 ?�태
    localApp->currentAppState = SET_GLOBAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_ENTER_HIGH;
    localMockSensor->setThresholdResult(true);

    // When: ?�한 ?�도�??�정, ?�한 ?�도 ?�킵
    localMockComm->addInput("85");
    localApp->processGlobalThreshold();

    TEST_ASSERT_EQUAL(THRESHOLD_ENTER_LOW, localApp->currentThresholdSubState);

    localMockComm->addInput(""); // ?�한 ?�도 ?�킵
    localApp->processGlobalThreshold();

    // Then: ?�한 ?�도�??�용??
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);
    TEST_ASSERT_TRUE(localApp->hasHighThreshold);
    TEST_ASSERT_FALSE(localApp->hasLowThreshold);
}

void test_global_threshold_no_sensors(void)
{
    // Given: ?�서가 ?�는 ?�태
    localMockSensor->setDeviceCount(0);
    localApp->currentAppState = SET_GLOBAL_THRESHOLD;
    localApp->currentThresholdSubState = THRESHOLD_NONE;

    // When: ?�체 ?�괄 ?�정 ?�행
    localApp->processGlobalThreshold();

    // Then: ?�계�?메뉴�??�아�?
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);
}

void test_threshold_view_current_settings(void)
{
    // Given: ?�계�?메뉴 ?�태
    localApp->currentAppState = THRESHOLD_MENU;

    // Mock ?�서???�계�??�정
    localMockSensor->setThresholdHighByIndex(0, 75);
    localMockSensor->setThresholdLowByIndex(0, 10);
    localMockSensor->setThresholdHighByIndex(1, 80);
    localMockSensor->setThresholdLowByIndex(1, 5);

    // When: ?�재 ?�계�?조회 ?�택
    localMockComm->addInput("3");
    localApp->processThresholdMenuInput();

    // Then: ?�계�?조회 ?�행??(?�태??그�?�?
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);

    // Mock ?�서??get 메서???�출 ?�인
    TEST_ASSERT_TRUE(localMockSensor->wasGetThresholdHighByIndexCalled());
    TEST_ASSERT_TRUE(localMockSensor->wasGetThresholdLowByIndexCalled());
}

void test_threshold_clear_all_thresholds(void)
{
    // Given: ?�계�?메뉴 ?�태
    localApp->currentAppState = THRESHOLD_MENU;
    localMockSensor->setClearThresholdsResult(true);

    // When: 모든 ?�계�?초기???�택
    localMockComm->addInput("4");
    localApp->processThresholdMenuInput();

    // Then: 초기???�인 ?�롬?�트 (추�? 구현 ?�요)
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);
}

void test_threshold_menu_back_to_main(void)
{
    // Given: ?�계�?메뉴 ?�태
    localApp->currentAppState = THRESHOLD_MENU;

    // When: 메인 메뉴�??�아가�??�택
    localMockComm->addInput("5");
    localApp->processThresholdMenuInput();

    // Then: 메인 메뉴�??�아�?
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);
    TEST_ASSERT_EQUAL(THRESHOLD_NONE, localApp->currentThresholdSubState);
}

void runThresholdScenarioTests(void)
{
    RUN_TEST(test_individual_threshold_setting_normal_flow);
    RUN_TEST(test_individual_threshold_sensor_selection);
    RUN_TEST(test_individual_threshold_high_temperature_setting);
    RUN_TEST(test_individual_threshold_low_temperature_setting);
    RUN_TEST(test_individual_threshold_skip_high_temperature);
    RUN_TEST(test_individual_threshold_invalid_temperature_range);
    RUN_TEST(test_individual_threshold_low_greater_than_high_error);
    RUN_TEST(test_individual_threshold_cancel_operation);

    RUN_TEST(test_global_threshold_setting_normal_flow);
    RUN_TEST(test_global_threshold_both_temperatures_setting);
    RUN_TEST(test_global_threshold_only_high_temperature);
    RUN_TEST(test_global_threshold_no_sensors);

    RUN_TEST(test_threshold_view_current_settings);
    RUN_TEST(test_threshold_clear_all_thresholds);
    RUN_TEST(test_threshold_menu_back_to_main);
}

int run_scenario9_tests(void)
{
    UNITY_BEGIN();
    runThresholdScenarioTests();
    return UNITY_END();
}

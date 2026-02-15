#include "unity/unity.h"
#include "App.h"
#include "MockDS18B20_Sensor.h"
#include "MockDataProcessor.h"
#include "MockCommunicator.h"
#include "MockMemoryUsageTester.h"
#include "TestGlobals.h"
#include <vector>

static MockDS18B20_Sensor *localMockSensor;
static MockDataProcessor *localMockProcessor;
static MockCommunicator *localMockComm;
static MockMemoryUsageTester *localMockMemory;
static App *localApp;

struct SensorTestCase
{
    int id;
    int8_t th;
    int8_t tl;
    std::vector<int8_t> normalTemps;
    std::vector<int8_t> abnormalTemps;
};

void setupTestEnvironmentV2()
{
    localMockSensor = new MockDS18B20_Sensor();
    localMockComm = new MockCommunicator();
    localMockProcessor = new MockDataProcessor(localMockSensor, localMockComm);
    localMockMemory = new MockMemoryUsageTester();
    localApp = new App(localMockSensor, localMockProcessor, localMockComm, localMockMemory);
    localMockSensor->setDeviceCount(4);
    DeviceAddress addr1 = {0x28, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    DeviceAddress addr2 = {0x28, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    DeviceAddress addr3 = {0x28, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    DeviceAddress addr4 = {0x28, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    localMockSensor->addDevice(addr1, 1);
    localMockSensor->addDevice(addr2, 2);
    localMockSensor->addDevice(addr3, 3);
    localMockSensor->addDevice(addr4, 4);
}

void tearDownTestEnvironmentV2()
{
    delete localApp;
    delete localMockComm;
    delete localMockProcessor;
    delete localMockSensor;
}

void test_sensor_threshold_status_check()
{
    setupTestEnvironmentV2();
    // 센서별 임계값 및 테스트 케이스 정의
    std::vector<SensorTestCase> cases = {
        {1, 30, 10, {15, 20, 25, 29, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}, {5, 9, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68}},
        {2, 40, 20, {21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40}, {10, 19, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78}},
        {3, 50, 30, {31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50}, {20, 29, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78}},
        {4, 60, 40, {41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60}, {30, 39, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98}}};

    // 각 센서별 임계값 설정 테스트
    for (const auto &tc : cases)
    {
        localMockProcessor->setThreshold(tc.id, tc.th, tc.tl);

        // 임계값이 정상적으로 설정되었는지 확인
        int8_t setTh = localMockProcessor->getTh(tc.id);
        int8_t setTl = localMockProcessor->getTl(tc.id);
        TEST_ASSERT_EQUAL(tc.th, setTh);
        TEST_ASSERT_EQUAL(tc.tl, setTl);

        // 경계값 분석(Boundary Value Analysis) 테스트
        // 상한 경계
        localMockSensor->setSensorTempCByIndex(tc.id - 1, tc.th + 1); // HIGH (해당 센서 인덱스)
        localApp->checkSensorStatus();
        TEST_ASSERT_TRUE(localMockComm->wasStringPrinted("Temperature HIGH"));
        localMockComm->clearHistory();

        localMockSensor->setSensorTempCByIndex(tc.id - 1, tc.th); // NORMAL
        localApp->checkSensorStatus();
        TEST_ASSERT_FALSE(localMockComm->wasStringPrinted("Temperature")); // 어떤 메시지도 출력되면 안됨
        localMockComm->clearHistory();

        // 하한 경계
        localMockSensor->setSensorTempCByIndex(tc.id - 1, tc.tl - 1); // LOW
        localApp->checkSensorStatus();
        TEST_ASSERT_TRUE(localMockComm->wasStringPrinted("Temperature LOW"));
        localMockComm->clearHistory();

        localMockSensor->setSensorTempCByIndex(tc.id - 1, tc.tl); // NORMAL
        localApp->checkSensorStatus();
        TEST_ASSERT_FALSE(localMockComm->wasStringPrinted("Temperature")); // 어떤 메시지도 출력되면 안됨
        localMockComm->clearHistory();
    }
    tearDownTestEnvironmentV2();
}

int run_scenario9_v2_tests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sensor_threshold_status_check);
    return UNITY_END();
}

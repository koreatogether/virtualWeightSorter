#include "unity/unity.h"
#include "FakeArduino.h"
#include "MockDS18B20_Sensor.h"
#include "MockCommunicator.h"
#include "MockDataProcessor.h"
#include "MockMemoryUsageTester.h"
#include "../App.h"

// 전역 Mock 객체들 정의
MockDS18B20_Sensor *mockSensorManager;
MockCommunicator *mockComm;
MockDataProcessor *mockDataProcessor;
MockMemoryUsageTester *mockMemoryTester;
App *app;

// 테스트 실행 전 초기화 함수
void setUp(void)
{
    mockSensorManager = new MockDS18B20_Sensor(nullptr);
    mockComm = new MockCommunicator();
    mockDataProcessor = new MockDataProcessor(mockSensorManager, mockComm);
    mockMemoryTester = new MockMemoryUsageTester();
    app = new App(mockSensorManager, mockDataProcessor, mockComm, mockMemoryTester);

    mockSensorManager->reset();
    mockComm->reset();
    mockDataProcessor->reset();
    mockMemoryTester->reset();
    setFakeMillis(0);
}

// 테스트 실행 후 정리 함수
void tearDown(void)
{
    delete app;
    delete mockDataProcessor;
    delete mockComm;
    delete mockSensorManager;
    delete mockMemoryTester;
}

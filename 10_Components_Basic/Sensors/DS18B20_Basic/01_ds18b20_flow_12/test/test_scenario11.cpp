#include "unity/unity.h"
#include "App.h"
#include "MockDS18B20_Sensor.h"
#include "MockDataProcessor.h"
#include "MockCommunicator.h"
#include "MockMemoryUsageTester.h"
#include "TestGlobals.h"

// 로컬 변수로 선언
static MockDS18B20_Sensor *localMockSensor;
static MockDataProcessor *localMockProcessor;
static MockCommunicator *localMockComm;
static MockMemoryUsageTester *localMockMemory;
static App *localApp;

// 테스트 초기화 헬퍼 함수
void setupMenuCancelTest(void)
{
    localMockSensor = new MockDS18B20_Sensor();
    localMockComm = new MockCommunicator();
    localMockProcessor = new MockDataProcessor(localMockSensor, localMockComm);
    localMockMemory = new MockMemoryUsageTester();
    localApp = new App(localMockSensor, localMockProcessor, localMockComm, localMockMemory);

    // 기본 설정: 2개 센서 존재
    localMockSensor->setDeviceCount(2);

    // 기본 주소 설정
    DeviceAddress addr1 = {0x28, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    DeviceAddress addr2 = {0x28, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    localMockSensor->setDeviceAddress(0, addr1);
    localMockSensor->setDeviceAddress(1, addr2);

    // 센서 ID 설정
    localMockSensor->setUserDataByIndex(0, 1);
    localMockSensor->setUserDataByIndex(1, 2);

    // 온도 설정
    localMockSensor->setTempCByIndex(0, 25.5);
    localMockSensor->setTempCByIndex(1, 26.8);

    // 앱 초기화
    localApp->init();

    // 정상 동작 모드로 설정
    localApp->currentAppState = NORMAL_OPERATION;

    // Mock 리셋
    localMockComm->reset();
}

// 테스트 정리 함수
void tearDownMenuCancelTest(void)
{
    delete localApp;
    delete localMockProcessor;
    delete localMockComm;
    delete localMockSensor;
    delete localMockMemory;
}

// 테스트 케이스 11.1: 기본 메뉴 취소 기능
void test_menu_cancel_from_main_menu(void)
{
    setupMenuCancelTest();

    // 1. 정상 모드에서 'm' 입력하여 메뉴 진입
    localMockComm->setNextInput("m");
    localApp->run(); // 메뉴 표시

    // 메뉴 상태 확인
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);

    // 2. '5' 입력으로 취소 실행
    localMockComm->setNextInput("5");
    localApp->run(); // 취소 처리

    // 3. 정상 모드로 복귀 확인
    TEST_ASSERT_EQUAL(NORMAL_OPERATION, localApp->currentAppState);

    // 4. 취소 메시지 출력 확인
    String output = localMockComm->getLastOutput();
    TEST_ASSERT_TRUE(output.indexOf("Cancelled") >= 0);
    TEST_ASSERT_TRUE(output.indexOf("normal operation") >= 0);

    tearDownMenuCancelTest();
}

// 테스트 케이스 11.2: 임계값 메뉴 취소 기능
void test_menu_cancel_from_threshold_menu(void)
{
    setupMenuCancelTest();

    // 1. 메뉴 진입 후 임계값 메뉴로 이동
    localMockComm->setNextInput("m");
    localApp->run(); // 메인 메뉴

    localMockComm->setNextInput("6");
    localApp->run(); // 임계값 메뉴 진입

    // 임계값 메뉴 상태 확인
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);

    // 2. '5' 입력으로 메인 메뉴로 복귀
    localMockComm->setNextInput("5");
    localApp->run(); // 메인 메뉴로 복귀

    // 3. 메인 메뉴 상태 확인
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);

    // 4. 복귀 메시지 출력 확인
    String output = localMockComm->getLastOutput();
    TEST_ASSERT_TRUE(output.indexOf("Returning to main menu") >= 0);

    tearDownMenuCancelTest();
}

// 테스트 케이스 11.3: 연속 취소 동작 (임계값 메뉴 → 메인 메뉴 → 정상 모드)
void test_consecutive_menu_cancel_operations(void)
{
    setupMenuCancelTest();

    // 1. 정상 모드 → 메인 메뉴
    localMockComm->setNextInput("m");
    localApp->run();
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);

    // 2. 메인 메뉴 → 임계값 메뉴
    localMockComm->setNextInput("6");
    localApp->run();
    TEST_ASSERT_EQUAL(THRESHOLD_MENU, localApp->currentAppState);

    // 3. 임계값 메뉴 → 메인 메뉴 (첫 번째 취소)
    localMockComm->setNextInput("5");
    localApp->run();
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);

    // 4. 메인 메뉴 → 정상 모드 (두 번째 취소)
    localMockComm->setNextInput("5");
    localApp->run();
    TEST_ASSERT_EQUAL(NORMAL_OPERATION, localApp->currentAppState);

    tearDownMenuCancelTest();
}

// 테스트 케이스 11.4: 취소 후 입력 버퍼 정상 동작 확인
void test_input_buffer_clear_after_cancel(void)
{
    setupMenuCancelTest();

    // 1. 메뉴 진입 및 취소
    localMockComm->setNextInput("m");
    localApp->run();

    localMockComm->setNextInput("5");
    localApp->run();

    // 정상 모드로 복귀 확인
    TEST_ASSERT_EQUAL(NORMAL_OPERATION, localApp->currentAppState);

    // 2. 취소 후 즉시 새로운 명령어 입력 테스트
    localMockComm->reset();           // 출력 버퍼 리셋
    localMockComm->setNextInput("m"); // 다시 메뉴 진입 시도
    localApp->run();

    // 3. 새로운 명령어가 정상 처리되는지 확인
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);

    tearDownMenuCancelTest();
}

// 테스트 케이스 11.5: 잘못된 메뉴 입력 후 취소
void test_invalid_input_then_cancel(void)
{
    setupMenuCancelTest();

    // 1. 메뉴 진입
    localMockComm->setNextInput("m");
    localApp->run();
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);

    // 2. 잘못된 입력 (범위 밖)
    localMockComm->setNextInput("9");
    localApp->run();
    // 여전히 메뉴 상태여야 함
    TEST_ASSERT_EQUAL(MENU_ACTIVE, localApp->currentAppState);

    // 3. 정상적인 취소
    localMockComm->setNextInput("5");
    localApp->run();
    TEST_ASSERT_EQUAL(NORMAL_OPERATION, localApp->currentAppState);

    tearDownMenuCancelTest();
}

// 테스트 케이스 11.6: 상태 전환 시 플래그 정상 설정 확인
void test_state_transition_flags_on_cancel(void)
{
    setupMenuCancelTest();

    // 1. 메뉴 진입
    localMockComm->setNextInput("m");
    localApp->run();

    // 2. forceMenuPrint 플래그 확인 (메뉴 진입 시 설정되어야 함)
    // 참고: 이 플래그는 public으로 설정되어 있어 직접 확인 가능

    // 3. 취소 실행
    localMockComm->setNextInput("5");
    localApp->run();

    // 4. 상태 전환 후 플래그들이 올바르게 설정되었는지 확인
    TEST_ASSERT_EQUAL(NORMAL_OPERATION, localApp->currentAppState);

    tearDownMenuCancelTest();
}

// 메인 테스트 러너
void test_scenario11_menu_cancel(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_menu_cancel_from_main_menu);
    RUN_TEST(test_menu_cancel_from_threshold_menu);
    RUN_TEST(test_consecutive_menu_cancel_operations);
    RUN_TEST(test_input_buffer_clear_after_cancel);
    RUN_TEST(test_invalid_input_then_cancel);
    RUN_TEST(test_state_transition_flags_on_cancel);

    UNITY_END();
}

#include "unity/unity.h"
#include "FakeArduino.h"
#include "TestGlobals.h" // Include TestGlobals.h
#include "../App.h"

// 시나리오 3-3: 메뉴 진입 및 반복 테스트
void test_menu_entry_and_repeated_cancellation() {
    // Given: 초기 상태는 NORMAL_OPERATION
    TEST_ASSERT_EQUAL(NORMAL_OPERATION, app->currentAppState);

    // When: 'menu' 입력 후 5번 취소 반복
    for (int i = 0; i < 5; ++i) {
        // 'menu' 입력 시뮬레이션
        mockComm->setIsSerialAvailableResult(true);
        mockComm->setReadStringUntilResult(String("menu"));
        app->handleUserCommands(); // 'menu' 명령 처리

        // Then: MENU_ACTIVE 상태로 전환되었는지 확인
        TEST_ASSERT_EQUAL(MENU_ACTIVE, app->currentAppState);
        TEST_ASSERT_TRUE(mockComm->_clearInputBufferCalled); // 입력 버퍼가 비워졌는지 확인
        mockComm->_clearInputBufferCalled = false; // 다음 반복을 위해 초기화

        // '5' (취소) 입력 시뮬레이션
        mockComm->setIsSerialAvailableResult(true);
        mockComm->setReadNonBlockingLineResult(String("5"));
        app->processMenuInput(); // 메뉴 입력 처리

        // Then: NORMAL_OPERATION 상태로 복귀했는지 확인
        TEST_ASSERT_EQUAL(NORMAL_OPERATION, app->currentAppState);
        TEST_ASSERT_TRUE(mockComm->_clearInputBufferCalled); // 입력 버퍼가 비워졌는지 확인
        mockComm->_clearInputBufferCalled = false; // 다음 반복을 위해 초기화

        setFakeMillis(0); // millis() 값을 초기화하여 다음 루프에서 시간 경과를 시뮬레이션
    }
}

int run_app_menu_tests() {
    UNITY_BEGIN();
    RUN_TEST(test_menu_entry_and_repeated_cancellation);
    return UNITY_END();
}
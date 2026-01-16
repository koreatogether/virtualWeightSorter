#include "SimpleTest.h"
#include <Arduino.h>

// 상태 정의 (원본 코드에서 가져옴)
enum SystemMode
{
    IDLE,
    SECURITY_LOCKDOWN
};

// 테스트할 상태 전환 로직 함수들
SystemMode processPasswordResult(SystemMode currentMode, bool isCorrect, int &failureCount, const int MAX_FAILURES = 5)
{
    if (isCorrect)
    {
        failureCount = 0;
        return IDLE;
    }
    else
    {
        failureCount++;
        if (failureCount >= MAX_FAILURES)
        {
            return SECURITY_LOCKDOWN;
        }
        return IDLE;
    }
}

bool shouldExitLockdown(unsigned long lockdownStartTime, unsigned long currentTime, const unsigned long LOCKDOWN_DURATION = 120000)
{
    return (currentTime - lockdownStartTime) >= LOCKDOWN_DURATION;
}

SystemMode updateLockdownMode(SystemMode currentMode, unsigned long lockdownStartTime, unsigned long currentTime, int &failureCount)
{
    if (currentMode == SECURITY_LOCKDOWN && shouldExitLockdown(lockdownStartTime, currentTime))
    {
        failureCount = 0;
        return IDLE;
    }
    return currentMode;
}

bool canManualUnlock(SystemMode currentMode)
{
    return currentMode != SECURITY_LOCKDOWN;
}

// 테스트 케이스들
void test_initial_state()
{
    SystemMode mode = IDLE;
    SimpleTest::assertEqual(mode, IDLE, "Initial state is IDLE");
    SimpleTest::assertTrue(canManualUnlock(mode), "Manual unlock allowed in IDLE");
}

void test_correct_password_flow()
{
    SystemMode mode = IDLE;
    int failureCount = 3; // 이미 실패가 있던 상태

    // 올바른 비밀번호 입력
    mode = processPasswordResult(mode, true, failureCount);

    SimpleTest::assertEqual(mode, IDLE, "Mode remains IDLE after correct password");
    SimpleTest::assertEqual(failureCount, 0, "Failure count resets to 0");
}

void test_single_failure()
{
    SystemMode mode = IDLE;
    int failureCount = 0;

    // 잘못된 비밀번호 1회
    mode = processPasswordResult(mode, false, failureCount);

    SimpleTest::assertEqual(mode, IDLE, "Mode stays IDLE after single failure");
    SimpleTest::assertEqual(failureCount, 1, "Failure count increments to 1");
}

void test_multiple_failures_below_threshold()
{
    SystemMode mode = IDLE;
    int failureCount = 0;

    // 4회 실패 (임계값 미만)
    for (int i = 0; i < 4; i++)
    {
        mode = processPasswordResult(mode, false, failureCount);
        SimpleTest::assertEqual(mode, IDLE, "Mode stays IDLE during failure " + String(i + 1));
    }

    SimpleTest::assertEqual(failureCount, 4, "Failure count reaches 4");
    SimpleTest::assertTrue(canManualUnlock(mode), "Manual unlock still allowed");
}

void test_lockdown_activation()
{
    SystemMode mode = IDLE;
    int failureCount = 4; // 이미 4회 실패

    // 5번째 실패로 보안 잠금 활성화
    mode = processPasswordResult(mode, false, failureCount);

    SimpleTest::assertEqual(mode, SECURITY_LOCKDOWN, "Mode changes to LOCKDOWN");
    SimpleTest::assertEqual(failureCount, 5, "Failure count reaches 5");
    SimpleTest::assertFalse(canManualUnlock(mode), "Manual unlock disabled");
}

void test_lockdown_duration_check()
{
    unsigned long startTime = 1000;
    unsigned long currentTime;

    // 잠금 지속 시간 내
    currentTime = startTime + 119000; // 1분 59초
    SimpleTest::assertFalse(shouldExitLockdown(startTime, currentTime), "Lockdown active at 1:59");

    // 잠금 지속 시간 만료
    currentTime = startTime + 120000; // 정확히 2분
    SimpleTest::assertTrue(shouldExitLockdown(startTime, currentTime), "Lockdown expires at 2:00");

    currentTime = startTime + 150000; // 2분 30초
    SimpleTest::assertTrue(shouldExitLockdown(startTime, currentTime), "Lockdown expired at 2:30");
}

void test_lockdown_exit_flow()
{
    SystemMode mode = SECURITY_LOCKDOWN;
    int failureCount = 5;
    unsigned long lockdownStart = 1000;
    unsigned long currentTime = lockdownStart + 120000; // 2분 후

    mode = updateLockdownMode(mode, lockdownStart, currentTime, failureCount);

    SimpleTest::assertEqual(mode, IDLE, "Mode returns to IDLE after lockdown");
    SimpleTest::assertEqual(failureCount, 0, "Failure count resets after lockdown");
}

void test_lockdown_not_expired()
{
    SystemMode mode = SECURITY_LOCKDOWN;
    int failureCount = 5;
    unsigned long lockdownStart = 1000;
    unsigned long currentTime = lockdownStart + 60000; // 1분 후 (아직 미만료)

    mode = updateLockdownMode(mode, lockdownStart, currentTime, failureCount);

    SimpleTest::assertEqual(mode, SECURITY_LOCKDOWN, "Mode stays LOCKDOWN when not expired");
    SimpleTest::assertEqual(failureCount, 5, "Failure count maintained during lockdown");
}

void test_manual_unlock_permissions()
{
    // IDLE 상태에서는 수동 해제 가능
    SimpleTest::assertTrue(canManualUnlock(IDLE), "Manual unlock allowed in IDLE");

    // SECURITY_LOCKDOWN 상태에서는 수동 해제 불가
    SimpleTest::assertFalse(canManualUnlock(SECURITY_LOCKDOWN), "Manual unlock blocked in LOCKDOWN");
}

void test_state_transition_sequence()
{
    SystemMode mode = IDLE;
    int failureCount = 0;

    // 시나리오: 3회 실패 → 성공 → 5회 실패 → 대기 → 성공

    // 3회 실패
    for (int i = 0; i < 3; i++)
    {
        mode = processPasswordResult(mode, false, failureCount);
        SimpleTest::assertEqual(mode, IDLE, "Mode stays IDLE during first 3 failures");
    }
    SimpleTest::assertEqual(failureCount, 3, "Failure count reaches 3");

    // 성공으로 리셋
    mode = processPasswordResult(mode, true, failureCount);
    SimpleTest::assertEqual(mode, IDLE, "Mode stays IDLE after success");
    SimpleTest::assertEqual(failureCount, 0, "Failure count resets after success");

    // 5회 실패로 잠금
    for (int i = 0; i < 5; i++)
    {
        mode = processPasswordResult(mode, false, failureCount);
    }
    SimpleTest::assertEqual(mode, SECURITY_LOCKDOWN, "Mode enters LOCKDOWN after 5 failures");
    SimpleTest::assertEqual(failureCount, 5, "Failure count reaches 5");

    // 2분 대기 후 해제
    unsigned long startTime = 0;
    unsigned long endTime = 120000;
    mode = updateLockdownMode(mode, startTime, endTime, failureCount);
    SimpleTest::assertEqual(mode, IDLE, "Mode returns to IDLE after lockdown expires");
    SimpleTest::assertEqual(failureCount, 0, "Failure count resets after lockdown expires");
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
    }

    SimpleTest::begin("State Machine Unit Tests");

    SimpleTest::runTest(test_initial_state, "Initial State");
    SimpleTest::runTest(test_correct_password_flow, "Correct Password Flow");
    SimpleTest::runTest(test_single_failure, "Single Failure");
    SimpleTest::runTest(test_multiple_failures_below_threshold, "Multiple Failures Below Threshold");
    SimpleTest::runTest(test_lockdown_activation, "Lockdown Activation");
    SimpleTest::runTest(test_lockdown_duration_check, "Lockdown Duration Check");
    SimpleTest::runTest(test_lockdown_exit_flow, "Lockdown Exit Flow");
    SimpleTest::runTest(test_lockdown_not_expired, "Lockdown Not Expired");
    SimpleTest::runTest(test_manual_unlock_permissions, "Manual Unlock Permissions");
    SimpleTest::runTest(test_state_transition_sequence, "State Transition Sequence");

    SimpleTest::summary();
}

void loop()
{
    // 테스트 완료
}
#include "SimpleTest.h"
#include <Arduino.h>

// 타이밍 관련 상수들 (원본 코드에서 가져옴)
const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long PASSWORD_ENTRY_TIMEOUT = 10000;
const unsigned long BUZZER_DURATION = 60000;
const unsigned long LOCKDOWN_DURATION = 120000;
const unsigned long UNLOCK_DURATION = 5000;
const unsigned long BUZZER_INTERVAL = 1000;

// 테스트할 타이밍 로직 함수들
bool isDebounceExpired(unsigned long lastPressTime, unsigned long currentTime)
{
    return (currentTime - lastPressTime) >= DEBOUNCE_DELAY;
}

bool isPasswordEntryTimedOut(unsigned long startTime, unsigned long currentTime)
{
    return (currentTime - startTime) > PASSWORD_ENTRY_TIMEOUT;
}

bool shouldAutoLock(unsigned long unlockTime, unsigned long currentTime)
{
    return (currentTime - unlockTime) >= UNLOCK_DURATION;
}

bool isBuzzerActiveTime(unsigned long buzzerStartTime, unsigned long currentTime)
{
    return (currentTime - buzzerStartTime) < BUZZER_DURATION;
}

bool isLockdownExpired(unsigned long lockdownStartTime, unsigned long currentTime)
{
    return (currentTime - lockdownStartTime) >= LOCKDOWN_DURATION;
}

bool shouldToggleBuzzer(unsigned long lastToggleTime, unsigned long currentTime)
{
    return (currentTime - lastToggleTime) >= BUZZER_INTERVAL;
}

unsigned long calculateElapsedTime(unsigned long startTime, unsigned long currentTime)
{
    if (currentTime >= startTime)
    {
        return currentTime - startTime;
    }
    // millis() 오버플로우 처리 (약 49.7일 후 발생)
    return (0xFFFFFFFF - startTime) + currentTime + 1;
}

// 테스트 케이스들
void test_debounce_timing()
{
    unsigned long lastPress = 1000;
    unsigned long currentTime;

    // 디바운스 시간 미만
    currentTime = lastPress + 100; // 100ms 후
    SimpleTest::assertFalse(isDebounceExpired(lastPress, currentTime), "Debounce not expired at 100ms");

    currentTime = lastPress + 199; // 199ms 후
    SimpleTest::assertFalse(isDebounceExpired(lastPress, currentTime), "Debounce not expired at 199ms");

    // 디바운스 시간 만료
    currentTime = lastPress + 200; // 정확히 200ms 후
    SimpleTest::assertTrue(isDebounceExpired(lastPress, currentTime), "Debounce expired at 200ms");

    currentTime = lastPress + 300; // 300ms 후
    SimpleTest::assertTrue(isDebounceExpired(lastPress, currentTime), "Debounce expired at 300ms");
}

void test_password_entry_timeout()
{
    unsigned long startTime = 5000;
    unsigned long currentTime;

    // 타임아웃 미만
    currentTime = startTime + 9999; // 9.999초 후
    SimpleTest::assertFalse(isPasswordEntryTimedOut(startTime, currentTime), "Password entry not timed out at 9.999s");

    currentTime = startTime + 10000; // 정확히 10초 후
    SimpleTest::assertFalse(isPasswordEntryTimedOut(startTime, currentTime), "Password entry not timed out at 10s");

    // 타임아웃 초과
    currentTime = startTime + 10001; // 10.001초 후
    SimpleTest::assertTrue(isPasswordEntryTimedOut(startTime, currentTime), "Password entry timed out at 10.001s");

    currentTime = startTime + 15000; // 15초 후
    SimpleTest::assertTrue(isPasswordEntryTimedOut(startTime, currentTime), "Password entry timed out at 15s");
}

void test_auto_lock_timing()
{
    unsigned long unlockTime = 2000;
    unsigned long currentTime;

    // 자동 잠금 시간 미만
    currentTime = unlockTime + 4999; // 4.999초 후
    SimpleTest::assertFalse(shouldAutoLock(unlockTime, currentTime), "Auto lock not triggered at 4.999s");

    // 자동 잠금 시간 만료
    currentTime = unlockTime + 5000; // 정확히 5초 후
    SimpleTest::assertTrue(shouldAutoLock(unlockTime, currentTime), "Auto lock triggered at 5s");

    currentTime = unlockTime + 7000; // 7초 후
    SimpleTest::assertTrue(shouldAutoLock(unlockTime, currentTime), "Auto lock triggered at 7s");
}

void test_buzzer_active_duration()
{
    unsigned long buzzerStart = 10000;
    unsigned long currentTime;

    // 부저 활성 시간 내
    currentTime = buzzerStart + 30000; // 30초 후
    SimpleTest::assertTrue(isBuzzerActiveTime(buzzerStart, currentTime), "Buzzer active at 30s");

    currentTime = buzzerStart + 59999; // 59.999초 후
    SimpleTest::assertTrue(isBuzzerActiveTime(buzzerStart, currentTime), "Buzzer active at 59.999s");

    // 부저 활성 시간 종료
    currentTime = buzzerStart + 60000; // 정확히 1분 후
    SimpleTest::assertFalse(isBuzzerActiveTime(buzzerStart, currentTime), "Buzzer not active at 60s");

    currentTime = buzzerStart + 90000; // 1분 30초 후
    SimpleTest::assertFalse(isBuzzerActiveTime(buzzerStart, currentTime), "Buzzer not active at 90s");
}

void test_lockdown_duration()
{
    unsigned long lockdownStart = 50000;
    unsigned long currentTime;

    // 잠금 지속 시간 내
    currentTime = lockdownStart + 60000; // 1분 후
    SimpleTest::assertFalse(isLockdownExpired(lockdownStart, currentTime), "Lockdown not expired at 1min");

    currentTime = lockdownStart + 119999; // 1분 59.999초 후
    SimpleTest::assertFalse(isLockdownExpired(lockdownStart, currentTime), "Lockdown not expired at 1:59.999");

    // 잠금 지속 시간 만료
    currentTime = lockdownStart + 120000; // 정확히 2분 후
    SimpleTest::assertTrue(isLockdownExpired(lockdownStart, currentTime), "Lockdown expired at 2min");

    currentTime = lockdownStart + 180000; // 3분 후
    SimpleTest::assertTrue(isLockdownExpired(lockdownStart, currentTime), "Lockdown expired at 3min");
}

void test_buzzer_toggle_timing()
{
    unsigned long lastToggle = 8000;
    unsigned long currentTime;

    // 토글 간격 미만
    currentTime = lastToggle + 500; // 0.5초 후
    SimpleTest::assertFalse(shouldToggleBuzzer(lastToggle, currentTime), "Buzzer toggle not ready at 0.5s");

    currentTime = lastToggle + 999; // 0.999초 후
    SimpleTest::assertFalse(shouldToggleBuzzer(lastToggle, currentTime), "Buzzer toggle not ready at 0.999s");

    // 토글 간격 만료
    currentTime = lastToggle + 1000; // 정확히 1초 후
    SimpleTest::assertTrue(shouldToggleBuzzer(lastToggle, currentTime), "Buzzer toggle ready at 1s");

    currentTime = lastToggle + 1500; // 1.5초 후
    SimpleTest::assertTrue(shouldToggleBuzzer(lastToggle, currentTime), "Buzzer toggle ready at 1.5s");
}

void test_elapsed_time_calculation()
{
    // 정상적인 경우
    SimpleTest::assertEqual(calculateElapsedTime(1000, 5000), 4000UL, "Normal elapsed time calculation");
    SimpleTest::assertEqual(calculateElapsedTime(0, 1000), 1000UL, "Elapsed time from 0");

    // 동일 시간
    SimpleTest::assertEqual(calculateElapsedTime(5000, 5000), 0UL, "Same time returns 0");

    // millis() 오버플로우 시뮬레이션
    unsigned long nearOverflow = 0xFFFFFFFE; // 최대값 - 1
    unsigned long afterOverflow = 100;
    unsigned long expected = 3 + 100; // (0xFFFFFFFF - 0xFFFFFFFE) + 100 + 1
    SimpleTest::assertEqual(calculateElapsedTime(nearOverflow, afterOverflow), expected, "Overflow calculation");
}

void test_timing_edge_cases()
{
    // 0에서 시작하는 시간
    SimpleTest::assertTrue(isDebounceExpired(0, 200), "Debounce expired from time 0 at 200ms");
    SimpleTest::assertFalse(isDebounceExpired(0, 199), "Debounce not expired from time 0 at 199ms");

    // 큰 시간 값
    unsigned long bigTime = 0xFFFFF000;
    SimpleTest::assertTrue(shouldAutoLock(bigTime, bigTime + 5000), "Auto lock with large time values");
    SimpleTest::assertFalse(shouldAutoLock(bigTime, bigTime + 4999), "Auto lock not triggered with large time values");
}

void test_multiple_timing_conditions()
{
    unsigned long baseTime = 10000;

    // 여러 조건이 동시에 만족되는 경우
    unsigned long testTime = baseTime + 15000; // 15초 후

    SimpleTest::assertTrue(isDebounceExpired(baseTime, testTime), "Debounce expired after 15s");
    SimpleTest::assertTrue(isPasswordEntryTimedOut(baseTime, testTime), "Password entry timed out after 15s");
    SimpleTest::assertTrue(shouldAutoLock(baseTime, testTime), "Should auto lock after 15s");
    SimpleTest::assertFalse(isBuzzerActiveTime(baseTime, testTime), "Buzzer not active after 15s");
    SimpleTest::assertFalse(isLockdownExpired(baseTime, testTime), "Lockdown not expired after 15s");
}

void test_sequential_timing_events()
{
    unsigned long startTime = 0;

    // 시간 순서대로 이벤트 확인
    // 200ms: 디바운스 만료
    SimpleTest::assertTrue(isDebounceExpired(startTime, 200), "1st event: Debounce expires at 200ms");

    // 5초: 자동 잠금
    SimpleTest::assertTrue(shouldAutoLock(startTime, 5000), "2nd event: Auto lock at 5s");

    // 10초: 비밀번호 입력 타임아웃
    SimpleTest::assertTrue(isPasswordEntryTimedOut(startTime, 10001), "3rd event: Password timeout at 10s");

    // 1분: 부저 정지
    SimpleTest::assertFalse(isBuzzerActiveTime(startTime, 60000), "4th event: Buzzer stops at 60s");

    // 2분: 보안 잠금 해제
    SimpleTest::assertTrue(isLockdownExpired(startTime, 120000), "5th event: Lockdown expires at 120s");
}

void setup()
{
    Serial.begin(9600);
    delay(2000);

    // 타이밍 로직 유닛 테스트 시작
    SimpleTest::begin("Timing Logic Unit Tests");

    // 모든 테스트 실행
    SimpleTest::runTest(test_debounce_timing, "Debounce Timing Tests");
    SimpleTest::runTest(test_password_entry_timeout, "Password Entry Timeout Tests");
    SimpleTest::runTest(test_auto_lock_timing, "Auto Lock Timing Tests");
    SimpleTest::runTest(test_buzzer_active_duration, "Buzzer Active Duration Tests");
    SimpleTest::runTest(test_lockdown_duration, "Lockdown Duration Tests");
    SimpleTest::runTest(test_buzzer_toggle_timing, "Buzzer Toggle Timing Tests");
    SimpleTest::runTest(test_elapsed_time_calculation, "Elapsed Time Calculation Tests");
    SimpleTest::runTest(test_timing_edge_cases, "Timing Edge Cases Tests");
    SimpleTest::runTest(test_multiple_timing_conditions, "Multiple Timing Conditions Tests");
    SimpleTest::runTest(test_sequential_timing_events, "Sequential Timing Events Tests");

    // 테스트 결과 요약
    SimpleTest::summary();

    Serial.println("\nTiming Logic Unit Tests completed!");
}

void loop()
{
    // 유닛 테스트는 한 번만 실행하므로 loop에서는 아무것도 하지 않음
    delay(1000);
}
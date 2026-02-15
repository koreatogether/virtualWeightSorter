#ifndef SIMPLE_TEST_H
#define SIMPLE_TEST_H

#include <Arduino.h>

class SimpleTest
{
private:
    static int totalTests;
    static int passedTests;
    static int failedTests;

public:
    static void begin(String suiteName)
    {
        Serial.println();
        Serial.println("=== " + suiteName + " ===");
        totalTests = 0;
        passedTests = 0;
        failedTests = 0;
    }

    static void assertEqual(int expected, int actual, String testName)
    {
        totalTests++;
        if (expected == actual)
        {
            Serial.println("[PASS] " + testName);
            passedTests++;
        }
        else
        {
            Serial.println("[FAIL] " + testName + " - Expected: " +
                           String(expected) + ", Got: " + String(actual));
            failedTests++;
        }
    }

    static void assertEqual(unsigned long expected, unsigned long actual, String testName)
    {
        totalTests++;
        if (expected == actual)
        {
            Serial.println("[PASS] " + testName);
            passedTests++;
        }
        else
        {
            Serial.println("[FAIL] " + testName + " - Expected: " +
                           String(expected) + ", Got: " + String(actual));
            failedTests++;
        }
    }

    static void assertEqual(String expected, String actual, String testName)
    {
        totalTests++;
        if (expected == actual)
        {
            Serial.println("[PASS] " + testName);
            passedTests++;
        }
        else
        {
            Serial.println("[FAIL] " + testName + " - Expected: '" +
                           expected + "', Got: '" + actual + "'");
            failedTests++;
        }
    }

    static void assertTrue(bool condition, String testName)
    {
        totalTests++;
        if (condition)
        {
            Serial.println("[PASS] " + testName);
            passedTests++;
        }
        else
        {
            Serial.println("[FAIL] " + testName + " - Expected: true, Got: false");
            failedTests++;
        }
    }

    static void assertFalse(bool condition, String testName)
    {
        totalTests++;
        if (!condition)
        {
            Serial.println("[PASS] " + testName);
            passedTests++;
        }
        else
        {
            Serial.println("[FAIL] " + testName + " - Expected: false, Got: true");
            failedTests++;
        }
    }

    static void runTest(void (*testFunction)(), String testName)
    {
        Serial.println("\n--- Running " + testName + " ---");
        testFunction();
    }

    static void summary()
    {
        Serial.println();
        Serial.println("=== TEST SUMMARY ===");
        Serial.println("Total: " + String(totalTests));
        Serial.println("Passed: " + String(passedTests));
        Serial.println("Failed: " + String(failedTests));

        if (failedTests == 0)
        {
            Serial.println("ALL TESTS PASSED! ✓");
        }
        else
        {
            Serial.println("SOME TESTS FAILED! ✗");
        }
        Serial.println("====================");
    }
};

// Static 변수 정의
int SimpleTest::totalTests = 0;
int SimpleTest::passedTests = 0;
int SimpleTest::failedTests = 0;

#endif
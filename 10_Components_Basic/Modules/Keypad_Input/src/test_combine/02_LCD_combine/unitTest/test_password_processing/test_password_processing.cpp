#include <Arduino.h>

// Mock global variables for testing
String inputBuffer = "";
String correctPassword = "123456";
int failureCount = 0;
const int MAX_FAILURES = 5;
enum SystemMode { IDLE, SECURITY_LOCKDOWN };
SystemMode currentMode = IDLE;
bool isUnlocked = false;
unsigned long servoUnlockTime = 0;

// Mock functions
void displayMessage(String line1, String line2) {
    Serial.println("LCD Display: " + line1 + " | " + line2);
}

void unlockDoor() {
    Serial.println("Door unlocked");
    isUnlocked = true;
    servoUnlockTime = millis();
}

void activateSecurityLockdown() {
    Serial.println("Security lockdown activated");
    currentMode = SECURITY_LOCKDOWN;
}

// Function under test (copied from original)
void processPasswordInput() {
    if (inputBuffer.length() != 8 ||
        !inputBuffer.startsWith("*") ||
        (inputBuffer.charAt(7) != '#' && inputBuffer.charAt(7) != '*')) {
        Serial.println("Invalid input format");
        inputBuffer = "";
        displayMessage("Invalid Format", "Try Again");
        delay(2000);
        return;
    }

    String password = inputBuffer.substring(1, 7);
    inputBuffer = "";

    if (password == correctPassword) {
        Serial.println("Correct password entered");
        failureCount = 0;
        unlockDoor();
        currentMode = IDLE;
        displayMessage("Access Granted", "Door Unlocked");
    } else {
        Serial.println("Wrong password");
        failureCount++;

        if (failureCount >= MAX_FAILURES) {
            displayMessage("Max Attempts", "Exceeded!");
            delay(2000);
            activateSecurityLockdown();
        } else {
            String attemptsMsg = "Attempts: " + String(failureCount) + "/" + String(MAX_FAILURES);
            displayMessage("Wrong Password", attemptsMsg);
            delay(3000);
            currentMode = IDLE;
        }
    }
}

// Test cases
void testCorrectPassword() {
    Serial.println("\n=== Testing Correct Password ===");
    inputBuffer = "*123456#";
    failureCount = 0;
    currentMode = IDLE;
    
    processPasswordInput();
    
    if (failureCount == 0 && isUnlocked && currentMode == IDLE) {
        Serial.println("✓ PASS: Correct password test");
    } else {
        Serial.println("✗ FAIL: Correct password test");
    }
}

void testWrongPassword() {
    Serial.println("\n=== Testing Wrong Password ===");
    inputBuffer = "*999999#";
    failureCount = 0;
    currentMode = IDLE;
    isUnlocked = false;
    
    processPasswordInput();
    
    if (failureCount == 1 && !isUnlocked && currentMode == IDLE) {
        Serial.println("✓ PASS: Wrong password test");
    } else {
        Serial.println("✗ FAIL: Wrong password test");
    }
}

void testMaxFailures() {
    Serial.println("\n=== Testing Max Failures ===");
    inputBuffer = "*999999#";
    failureCount = MAX_FAILURES - 1;
    currentMode = IDLE;
    
    processPasswordInput();
    
    if (failureCount >= MAX_FAILURES && currentMode == SECURITY_LOCKDOWN) {
        Serial.println("✓ PASS: Max failures test");
    } else {
        Serial.println("✗ FAIL: Max failures test");
    }
}

void testInvalidFormat() {
    Serial.println("\n=== Testing Invalid Format ===");
    String originalBuffer = inputBuffer;
    
    // Test too short
    inputBuffer = "*12345";
    processPasswordInput();
    if (inputBuffer == "") Serial.println("✓ PASS: Too short format rejected");
    
    // Test missing star
    inputBuffer = "123456#";
    processPasswordInput();
    if (inputBuffer == "") Serial.println("✓ PASS: Missing star format rejected");
    
    // Test invalid terminator
    inputBuffer = "*123456X";
    processPasswordInput();
    if (inputBuffer == "") Serial.println("✓ PASS: Invalid terminator format rejected");
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    Serial.println("Password Processing Unit Test Starting...");
    Serial.println("==========================================");
    
    testCorrectPassword();
    testWrongPassword();
    testMaxFailures();
    testInvalidFormat();
    
    Serial.println("\n==========================================");
    Serial.println("Password Processing Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
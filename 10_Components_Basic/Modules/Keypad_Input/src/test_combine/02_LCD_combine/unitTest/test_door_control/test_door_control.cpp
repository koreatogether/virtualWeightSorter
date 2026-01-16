#include <Arduino.h>
#include <Servo.h>

// Mock global variables for testing
Servo lockServo;
const int SERVO_PIN = 10;
const int LOCK_ANGLE = 0;
const int UNLOCK_ANGLE = 180;
const unsigned long UNLOCK_DURATION = 5000;

bool isUnlocked = false;
unsigned long servoUnlockTime = 0;

// Mock function
void displayMessage(String line1, String line2) {
    Serial.println("LCD Display: " + line1 + " | " + line2);
}

// Functions under test (copied from original)
void unlockDoor() {
    Serial.println("Door unlocked");
    lockServo.write(UNLOCK_ANGLE);
    isUnlocked = true;
    servoUnlockTime = millis();
}

void handleAutoLock() {
    if (isUnlocked && (millis() - servoUnlockTime >= UNLOCK_DURATION)) {
        Serial.println("Auto lock activated");
        lockServo.write(LOCK_ANGLE);
        isUnlocked = false;
        displayMessage("Auto Lock", "Activated");
        delay(2000);
    }
}

// Test cases
void testUnlockDoor() {
    Serial.println("\n=== Testing Door Unlock ===");
    isUnlocked = false;
    servoUnlockTime = 0;
    
    unlockDoor();
    
    if (isUnlocked && servoUnlockTime > 0) {
        Serial.println("✓ PASS: Door unlock test");
    } else {
        Serial.println("✗ FAIL: Door unlock test");
    }
}

void testAutoLockImmediate() {
    Serial.println("\n=== Testing Auto Lock (Not Ready) ===");
    isUnlocked = true;
    servoUnlockTime = millis(); // Just unlocked
    
    handleAutoLock();
    
    if (isUnlocked) {
        Serial.println("✓ PASS: Auto lock not triggered immediately");
    } else {
        Serial.println("✗ FAIL: Auto lock triggered too early");
    }
}

void testAutoLockAfterDelay() {
    Serial.println("\n=== Testing Auto Lock (After Delay) ===");
    isUnlocked = true;
    servoUnlockTime = millis() - (UNLOCK_DURATION + 1000); // Simulate time passed
    
    handleAutoLock();
    
    if (!isUnlocked) {
        Serial.println("✓ PASS: Auto lock triggered after delay");
    } else {
        Serial.println("✗ FAIL: Auto lock not triggered after delay");
    }
}

void testAutoLockWhenLocked() {
    Serial.println("\n=== Testing Auto Lock (Already Locked) ===");
    isUnlocked = false;
    servoUnlockTime = millis() - (UNLOCK_DURATION + 1000);
    
    handleAutoLock();
    
    if (!isUnlocked) {
        Serial.println("✓ PASS: Auto lock does nothing when already locked");
    } else {
        Serial.println("✗ FAIL: Auto lock changed lock state unexpectedly");
    }
}

void testCompleteUnlockLockCycle() {
    Serial.println("\n=== Testing Complete Unlock-Lock Cycle ===");
    
    // Start locked
    isUnlocked = false;
    
    // Unlock
    unlockDoor();
    if (!isUnlocked) {
        Serial.println("✗ FAIL: Failed to unlock door");
        return;
    }
    
    // Wait and check auto lock
    delay(100); // Short delay for immediate test
    handleAutoLock();
    if (!isUnlocked) {
        Serial.println("✗ FAIL: Auto lock triggered too early in cycle test");
        return;
    }
    
    // Simulate time passing
    servoUnlockTime = millis() - (UNLOCK_DURATION + 1000);
    handleAutoLock();
    
    if (!isUnlocked) {
        Serial.println("✓ PASS: Complete unlock-lock cycle test");
    } else {
        Serial.println("✗ FAIL: Auto lock not triggered in cycle test");
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    // Initialize servo (mock)
    lockServo.attach(SERVO_PIN);
    lockServo.write(LOCK_ANGLE);
    
    Serial.println("Door Control Unit Test Starting...");
    Serial.println("==================================");
    
    testUnlockDoor();
    testAutoLockImmediate();
    testAutoLockAfterDelay();
    testAutoLockWhenLocked();
    testCompleteUnlockLockCycle();
    
    Serial.println("\n==================================");
    Serial.println("Door Control Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
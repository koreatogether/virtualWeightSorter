#include <Arduino.h>
#include <Servo.h>

// Mock global variables for testing
enum SystemMode { IDLE, SECURITY_LOCKDOWN };
SystemMode currentMode = IDLE;

Servo lockServo;
const int SERVO_PIN = 10;
const int UNLOCK_ANGLE = 180;
bool isUnlocked = false;
unsigned long servoUnlockTime = 0;

// Mock functions
void displayMessage(String line1, String line2) {
    Serial.println("LCD Display: " + line1 + " | " + line2);
}

void turnOnBacklight() {
    Serial.println("Backlight turned on");
}

void unlockDoor() {
    Serial.println("Door unlocked");
    lockServo.write(UNLOCK_ANGLE);
    isUnlocked = true;
    servoUnlockTime = millis();
}

// Function under test (copied from original)
void handleManualUnlock() {
    if (currentMode == SECURITY_LOCKDOWN) {
        Serial.println("Manual unlock disabled - security lockdown active");
        displayMessage("Manual Unlock", "Disabled");
        turnOnBacklight();
        delay(2000);
        return;
    }

    Serial.println("Manual unlock activated");
    displayMessage("Manual Unlock", "Activated");
    turnOnBacklight();
    unlockDoor();
}

// Test cases
void testManualUnlockInIdleMode() {
    Serial.println("\n=== Testing Manual Unlock in IDLE Mode ===");
    currentMode = IDLE;
    isUnlocked = false;
    
    handleManualUnlock();
    
    if (isUnlocked) {
        Serial.println("✓ PASS: Manual unlock successful in IDLE mode");
    } else {
        Serial.println("✗ FAIL: Manual unlock failed in IDLE mode");
    }
}

void testManualUnlockInLockdownMode() {
    Serial.println("\n=== Testing Manual Unlock in LOCKDOWN Mode ===");
    currentMode = SECURITY_LOCKDOWN;
    isUnlocked = false;
    
    handleManualUnlock();
    
    if (!isUnlocked) {
        Serial.println("✓ PASS: Manual unlock correctly disabled in LOCKDOWN mode");
    } else {
        Serial.println("✗ FAIL: Manual unlock should be disabled in LOCKDOWN mode");
    }
}

void testManualUnlockWhenAlreadyUnlocked() {
    Serial.println("\n=== Testing Manual Unlock When Already Unlocked ===");
    currentMode = IDLE;
    isUnlocked = true;
    unsigned long originalTime = millis();
    servoUnlockTime = originalTime;
    
    handleManualUnlock();
    
    if (isUnlocked && servoUnlockTime > originalTime) {
        Serial.println("✓ PASS: Manual unlock updates unlock time even when already unlocked");
    } else {
        Serial.println("✗ FAIL: Manual unlock should still update unlock time");
    }
}

void testManualUnlockModeTransitions() {
    Serial.println("\n=== Testing Manual Unlock Mode Transitions ===");
    
    // Test from IDLE to unlocked
    currentMode = IDLE;
    isUnlocked = false;
    handleManualUnlock();
    
    bool idleToUnlocked = isUnlocked;
    
    // Test from LOCKDOWN (should remain locked)
    currentMode = SECURITY_LOCKDOWN;
    isUnlocked = false;
    handleManualUnlock();
    
    bool lockdownStaysLocked = !isUnlocked;
    
    if (idleToUnlocked && lockdownStaysLocked) {
        Serial.println("✓ PASS: Manual unlock respects mode transitions correctly");
    } else {
        Serial.println("✗ FAIL: Manual unlock mode transition test failed");
        Serial.println("  IDLE->Unlocked: " + String(idleToUnlocked));
        Serial.println("  LOCKDOWN stays locked: " + String(lockdownStaysLocked));
    }
}

void testRepeatedManualUnlock() {
    Serial.println("\n=== Testing Repeated Manual Unlock ===");
    currentMode = IDLE;
    isUnlocked = false;
    
    // First unlock
    handleManualUnlock();
    bool firstUnlock = isUnlocked;
    unsigned long firstTime = servoUnlockTime;
    
    delay(10); // Small delay to ensure time difference
    
    // Second unlock
    handleManualUnlock();
    bool secondUnlock = isUnlocked;
    unsigned long secondTime = servoUnlockTime;
    
    if (firstUnlock && secondUnlock && (secondTime > firstTime)) {
        Serial.println("✓ PASS: Repeated manual unlock works correctly");
    } else {
        Serial.println("✗ FAIL: Repeated manual unlock test failed");
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    // Initialize servo (mock)
    lockServo.attach(SERVO_PIN);
    
    Serial.println("Manual Unlock Unit Test Starting...");
    Serial.println("===================================");
    
    testManualUnlockInIdleMode();
    testManualUnlockInLockdownMode();
    testManualUnlockWhenAlreadyUnlocked();
    testManualUnlockModeTransitions();
    testRepeatedManualUnlock();
    
    Serial.println("\n===================================");
    Serial.println("Manual Unlock Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
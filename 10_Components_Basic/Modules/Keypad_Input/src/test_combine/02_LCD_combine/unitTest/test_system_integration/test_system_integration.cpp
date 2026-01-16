#include <Arduino.h>
#include <Servo.h>
#include <Adafruit_Keypad.h>

// Mock hardware components
Servo lockServo;
const int SERVO_PIN = 10;
const int LOCK_ANGLE = 0;
const int UNLOCK_ANGLE = 180;
const int BUZZER_PIN = 12;
const int MANUAL_BUTTON_PIN = 11;

// System state
enum SystemMode { IDLE, SECURITY_LOCKDOWN };
SystemMode currentMode = IDLE;

// Global variables
String inputBuffer = "";
String correctPassword = "123456";
int failureCount = 0;
const int MAX_FAILURES = 5;
bool isUnlocked = false;
bool buzzerActive = false;
bool buzzerState = false;
unsigned long lastBuzzerToggle = 0;
unsigned long servoUnlockTime = 0;
unsigned long lockdownStartTime = 0;
unsigned long buzzerStartTime = 0;

// Constants
const unsigned long UNLOCK_DURATION = 5000;
const unsigned long LOCKDOWN_DURATION = 120000;
const unsigned long BUZZER_DURATION = 60000;
const unsigned long BUZZER_INTERVAL = 1000;

// Test tracking variables
int displayCalls = 0;
String lastDisplayLine1 = "";
String lastDisplayLine2 = "";
int backlightCalls = 0;

// Mock functions
void displayMessage(String line1, String line2) {
    displayCalls++;
    lastDisplayLine1 = line1;
    lastDisplayLine2 = line2;
    Serial.println("Display: '" + line1 + "' | '" + line2 + "'");
}

void turnOnBacklight() {
    backlightCalls++;
    Serial.println("Backlight turned on");
}

// Core system functions (simplified versions)
void unlockDoor() {
    Serial.println("Door unlocked");
    lockServo.write(UNLOCK_ANGLE);
    isUnlocked = true;
    servoUnlockTime = millis();
}

void activateSecurityLockdown() {
    Serial.println("Maximum attempts exceeded - security lockdown activated");
    currentMode = SECURITY_LOCKDOWN;
    lockdownStartTime = millis();
    buzzerStartTime = millis();
    buzzerActive = true;
    buzzerState = false;
    lastBuzzerToggle = 0;

    lockServo.write(LOCK_ANGLE);
    isUnlocked = false;

    displayMessage("Security Lockdown", "2 Minutes");
}

void processPasswordInput() {
    if (inputBuffer.length() != 8 ||
        !inputBuffer.startsWith("*") ||
        (inputBuffer.charAt(7) != '#' && inputBuffer.charAt(7) != '*')) {
        Serial.println("Invalid input format");
        inputBuffer = "";
        displayMessage("Invalid Format", "Try Again");
        unsigned long errorStart = millis();
        while (millis() - errorStart < 2000) {
            // Non-blocking delay
        }
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
            unsigned long errorStart = millis();
            while (millis() - errorStart < 2000) {
                // Non-blocking delay
            }
            activateSecurityLockdown();
        } else {
            String attemptsMsg = "Attempts: " + String(failureCount) + "/" + String(MAX_FAILURES);
            displayMessage("Wrong Password", attemptsMsg);
            unsigned long errorStart = millis();
            while (millis() - errorStart < 3000) {
                // Non-blocking delay
            }
            currentMode = IDLE;
        }
    }
}

void handleAutoLock() {
    if (isUnlocked && (millis() - servoUnlockTime >= UNLOCK_DURATION)) {
        Serial.println("Auto lock activated");
        lockServo.write(LOCK_ANGLE);
        isUnlocked = false;
        displayMessage("Auto Lock", "Activated");
        unsigned long lockStart = millis();
        while (millis() - lockStart < 2000) {
            // Non-blocking delay
        }
    }
}

void handleBuzzer() {
    if (!buzzerActive)
        return;

    unsigned long currentTime = millis();

    if (currentTime - buzzerStartTime < BUZZER_DURATION) {
        if (currentTime - lastBuzzerToggle >= BUZZER_INTERVAL) {
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
            lastBuzzerToggle = currentTime;
        }
    } else {
        buzzerActive = false;
        buzzerState = false;
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void handleManualUnlock() {
    if (currentMode == SECURITY_LOCKDOWN) {
        Serial.println("Manual unlock disabled - security lockdown active");
        displayMessage("Manual Unlock", "Disabled");
        turnOnBacklight(); // 수동버튼 사용 시 백라이트 켜기
        unsigned long errorStart = millis();
        while (millis() - errorStart < 2000) {
            // Non-blocking delay
        }
        return;
    }

    Serial.println("Manual unlock activated");
    displayMessage("Manual Unlock", "Activated");
    turnOnBacklight(); // 수동버튼 사용 시 백라이트 켜기
    unlockDoor();
}

void handleLockdownMode() {
    unsigned long currentTime = millis();

    if (currentTime - lockdownStartTime >= LOCKDOWN_DURATION) {
        Serial.println("Security lockdown released - normal operation resumed");
        currentMode = IDLE;
        failureCount = 0;
        buzzerActive = false;
        digitalWrite(BUZZER_PIN, LOW);
        displayMessage("Lockdown End", "System Ready");
        unsigned long releaseStart = millis();
        while (millis() - releaseStart < 3000) {
            // Non-blocking delay
        }
    }
}

// Test helper functions
void resetSystemState() {
    currentMode = IDLE;
    inputBuffer = "";
    failureCount = 0;
    isUnlocked = false;
    buzzerActive = false;
    buzzerState = false;
    servoUnlockTime = 0;
    lockdownStartTime = 0;
    buzzerStartTime = 0;
    displayCalls = 0;
    lastDisplayLine1 = "";
    lastDisplayLine2 = "";
    backlightCalls = 0;
    digitalWrite(BUZZER_PIN, LOW);
    lockServo.write(LOCK_ANGLE);
}

void initializeSystem() {
    lockServo.attach(SERVO_PIN);
    lockServo.write(LOCK_ANGLE);
    pinMode(MANUAL_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

// Integration test cases
void testSuccessfulUnlockSequence() {
    Serial.println("\n=== Testing Successful Unlock Sequence ===");
    resetSystemState();
    
    // Simulate correct password input
    inputBuffer = "*123456#";
    processPasswordInput();
    
    // Check results
    bool doorUnlocked = isUnlocked;
    bool failuresReset = (failureCount == 0);
    bool correctDisplay = (lastDisplayLine1 == "Access Granted");
    bool idleMode = (currentMode == IDLE);
    
    if (doorUnlocked && failuresReset && correctDisplay && idleMode) {
        Serial.println("✓ PASS: Successful unlock sequence test");
    } else {
        Serial.println("✗ FAIL: Successful unlock sequence test");
        Serial.println("  Door unlocked: " + String(doorUnlocked));
        Serial.println("  Failures reset: " + String(failuresReset));
        Serial.println("  Correct display: " + String(correctDisplay));
        Serial.println("  IDLE mode: " + String(idleMode));
    }
}

void testFailureEscalationSequence() {
    Serial.println("\n=== Testing Failure Escalation Sequence ===");
    resetSystemState();
    
    // Multiple wrong attempts
    for (int i = 0; i < MAX_FAILURES; i++) {
        inputBuffer = "*999999#";
        processPasswordInput();
    }
    
    bool lockdownActivated = (currentMode == SECURITY_LOCKDOWN);
    bool buzzerStarted = buzzerActive;
    bool doorLocked = !isUnlocked;
    bool correctFailureCount = (failureCount >= MAX_FAILURES);
    
    if (lockdownActivated && buzzerStarted && doorLocked && correctFailureCount) {
        Serial.println("✓ PASS: Failure escalation sequence test");
    } else {
        Serial.println("✗ FAIL: Failure escalation sequence test");
        Serial.println("  Lockdown activated: " + String(lockdownActivated));
        Serial.println("  Buzzer started: " + String(buzzerStarted));
        Serial.println("  Door locked: " + String(doorLocked));
        Serial.println("  Correct failure count: " + String(correctFailureCount));
    }
}

void testAutoLockAfterUnlock() {
    Serial.println("\n=== Testing Auto Lock After Unlock ===");
    resetSystemState();
    
    // Unlock door first
    unlockDoor();
    bool initialUnlock = isUnlocked;
    
    // Simulate time passage
    servoUnlockTime = millis() - (UNLOCK_DURATION + 1000);
    handleAutoLock();
    
    bool autoLocked = !isUnlocked;
    
    if (initialUnlock && autoLocked) {
        Serial.println("✓ PASS: Auto lock after unlock test");
    } else {
        Serial.println("✗ FAIL: Auto lock after unlock test");
        Serial.println("  Initial unlock: " + String(initialUnlock));
        Serial.println("  Auto locked: " + String(autoLocked));
    }
}

void testManualUnlockInNormalMode() {
    Serial.println("\n=== Testing Manual Unlock in Normal Mode ===");
    resetSystemState();
    
    currentMode = IDLE;
    isUnlocked = false;
    
    handleManualUnlock();
    
    bool doorUnlocked = isUnlocked;
    bool correctDisplay = (lastDisplayLine1 == "Manual Unlock");
    
    if (doorUnlocked && correctDisplay) {
        Serial.println("✓ PASS: Manual unlock in normal mode test");
    } else {
        Serial.println("✗ FAIL: Manual unlock in normal mode test");
        Serial.println("  Door unlocked: " + String(doorUnlocked));
        Serial.println("  Correct display: " + String(correctDisplay));
    }
}

void testManualUnlockDuringLockdown() {
    Serial.println("\n=== Testing Manual Unlock During Lockdown ===");
    resetSystemState();
    
    currentMode = SECURITY_LOCKDOWN;
    isUnlocked = false;
    
    handleManualUnlock();
    
    bool stayLocked = !isUnlocked;
    bool correctDisplay = (lastDisplayLine1 == "Manual Unlock") && (lastDisplayLine2 == "Disabled");
    
    if (stayLocked && correctDisplay) {
        Serial.println("✓ PASS: Manual unlock during lockdown test");
    } else {
        Serial.println("✗ FAIL: Manual unlock during lockdown test");
        Serial.println("  Stay locked: " + String(stayLocked));
        Serial.println("  Correct display: " + String(correctDisplay));
    }
}

void testLockdownRecoverySequence() {
    Serial.println("\n=== Testing Lockdown Recovery Sequence ===");
    resetSystemState();
    
    // Activate lockdown
    activateSecurityLockdown();
    bool lockdownStart = (currentMode == SECURITY_LOCKDOWN);
    
    // Simulate time passage for lockdown expiry
    lockdownStartTime = millis() - (LOCKDOWN_DURATION + 1000);
    handleLockdownMode();
    
    bool returnToIdle = (currentMode == IDLE);
    bool failuresReset = (failureCount == 0);
    bool buzzerOff = !buzzerActive;
    
    if (lockdownStart && returnToIdle && failuresReset && buzzerOff) {
        Serial.println("✓ PASS: Lockdown recovery sequence test");
    } else {
        Serial.println("✗ FAIL: Lockdown recovery sequence test");
        Serial.println("  Lockdown started: " + String(lockdownStart));
        Serial.println("  Return to IDLE: " + String(returnToIdle));
        Serial.println("  Failures reset: " + String(failuresReset));
        Serial.println("  Buzzer off: " + String(buzzerOff));
    }
}

void testBuzzerIntegrationWithLockdown() {
    Serial.println("\n=== Testing Buzzer Integration with Lockdown ===");
    resetSystemState();
    
    // Activate lockdown (should start buzzer)
    activateSecurityLockdown();
    bool buzzerStarted = buzzerActive;
    
    // Handle buzzer (should toggle)
    handleBuzzer();
    // Note: actual buzzer toggling depends on timing, so we just check it's active
    
    // Simulate buzzer expiry
    buzzerStartTime = millis() - (BUZZER_DURATION + 1000);
    handleBuzzer();
    bool buzzerStopped = !buzzerActive;
    
    if (buzzerStarted && buzzerStopped) {
        Serial.println("✓ PASS: Buzzer integration with lockdown test");
    } else {
        Serial.println("✗ FAIL: Buzzer integration with lockdown test");
        Serial.println("  Buzzer started: " + String(buzzerStarted));
        Serial.println("  Buzzer stopped: " + String(buzzerStopped));
    }
}

void testCompleteSystemWorkflow() {
    Serial.println("\n=== Testing Complete System Workflow ===");
    resetSystemState();
    
    // Step 1: Wrong password
    inputBuffer = "*999999#";
    processPasswordInput();
    bool wrongHandled = (failureCount == 1) && !isUnlocked;
    
    // Step 2: Correct password
    inputBuffer = "*123456#";
    processPasswordInput();
    bool correctHandled = (failureCount == 0) && isUnlocked;
    
    // Step 3: Auto lock after time
    servoUnlockTime = millis() - (UNLOCK_DURATION + 1000);
    handleAutoLock();
    bool autoLocked = !isUnlocked;
    
    // Step 4: Manual unlock
    handleManualUnlock();
    bool manualUnlocked = isUnlocked;
    
    if (wrongHandled && correctHandled && autoLocked && manualUnlocked) {
        Serial.println("✓ PASS: Complete system workflow test");
    } else {
        Serial.println("✗ FAIL: Complete system workflow test");
        Serial.println("  Wrong handled: " + String(wrongHandled));
        Serial.println("  Correct handled: " + String(correctHandled));
        Serial.println("  Auto locked: " + String(autoLocked));
        Serial.println("  Manual unlocked: " + String(manualUnlocked));
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    initializeSystem();
    
    Serial.println("System Integration Unit Test Starting...");
    Serial.println("=======================================");
    
    testSuccessfulUnlockSequence();
    testFailureEscalationSequence();
    testAutoLockAfterUnlock();
    testManualUnlockInNormalMode();
    testManualUnlockDuringLockdown();
    testLockdownRecoverySequence();
    testBuzzerIntegrationWithLockdown();
    testCompleteSystemWorkflow();
    
    Serial.println("\n=======================================");
    Serial.println("System Integration Unit Test Complete");
    Serial.println("\nAll major system interactions tested!");
}

void loop() {
    // Test complete, no loop needed
}
#include <Arduino.h>
#include <Servo.h>

// Mock global variables for testing
enum SystemMode { IDLE, SECURITY_LOCKDOWN };
SystemMode currentMode = IDLE;

Servo lockServo;
const int SERVO_PIN = 10;
const int LOCK_ANGLE = 0;
const unsigned long LOCKDOWN_DURATION = 120000; // 2 minutes
const unsigned long BUZZER_DURATION = 60000; // 1 minute

unsigned long lockdownStartTime = 0;
unsigned long buzzerStartTime = 0;
bool isUnlocked = false;
bool buzzerActive = false;
int failureCount = 0;
const int BUZZER_PIN = 12;

// Mock functions
void displayMessage(String line1, String line2) {
    Serial.println("LCD Display: " + line1 + " | " + line2);
}

// Functions under test (copied from original)
void activateSecurityLockdown() {
    Serial.println("Maximum attempts exceeded - security lockdown activated");
    currentMode = SECURITY_LOCKDOWN;
    lockdownStartTime = millis();
    buzzerStartTime = millis();
    buzzerActive = true;

    lockServo.write(LOCK_ANGLE);
    isUnlocked = false;

    displayMessage("Security Lockdown", "2 Minutes");
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
        delay(3000);
    }
}

// Test cases
void testActivateSecurityLockdown() {
    Serial.println("\n=== Testing Activate Security Lockdown ===");
    currentMode = IDLE;
    isUnlocked = true;
    buzzerActive = false;
    lockdownStartTime = 0;
    
    activateSecurityLockdown();
    
    bool correctMode = (currentMode == SECURITY_LOCKDOWN);
    bool doorLocked = (!isUnlocked);
    bool buzzerOn = buzzerActive;
    bool timeSet = (lockdownStartTime > 0);
    
    if (correctMode && doorLocked && buzzerOn && timeSet) {
        Serial.println("✓ PASS: Security lockdown activation test");
    } else {
        Serial.println("✗ FAIL: Security lockdown activation test");
        Serial.println("  Mode correct: " + String(correctMode));
        Serial.println("  Door locked: " + String(doorLocked));
        Serial.println("  Buzzer active: " + String(buzzerOn));
        Serial.println("  Time set: " + String(timeSet));
    }
}

void testLockdownModeNotExpired() {
    Serial.println("\n=== Testing Lockdown Mode (Not Expired) ===");
    currentMode = SECURITY_LOCKDOWN;
    lockdownStartTime = millis();
    failureCount = 5;
    buzzerActive = true;
    
    handleLockdownMode();
    
    if (currentMode == SECURITY_LOCKDOWN && buzzerActive) {
        Serial.println("✓ PASS: Lockdown continues when not expired");
    } else {
        Serial.println("✗ FAIL: Lockdown should continue when not expired");
    }
}

void testLockdownModeExpired() {
    Serial.println("\n=== Testing Lockdown Mode (Expired) ===");
    currentMode = SECURITY_LOCKDOWN;
    lockdownStartTime = millis() - (LOCKDOWN_DURATION + 1000); // Simulate expired
    failureCount = 5;
    buzzerActive = true;
    
    handleLockdownMode();
    
    bool modeReset = (currentMode == IDLE);
    bool failuresReset = (failureCount == 0);
    bool buzzerOff = (!buzzerActive);
    
    if (modeReset && failuresReset && buzzerOff) {
        Serial.println("✓ PASS: Lockdown releases when expired");
    } else {
        Serial.println("✗ FAIL: Lockdown release test failed");
        Serial.println("  Mode reset: " + String(modeReset));
        Serial.println("  Failures reset: " + String(failuresReset));
        Serial.println("  Buzzer off: " + String(buzzerOff));
    }
}

void testLockdownTimingBoundary() {
    Serial.println("\n=== Testing Lockdown Timing Boundary ===");
    
    // Test just before expiry
    currentMode = SECURITY_LOCKDOWN;
    lockdownStartTime = millis() - (LOCKDOWN_DURATION - 1000); // 1 second before expiry
    
    handleLockdownMode();
    bool stillLocked = (currentMode == SECURITY_LOCKDOWN);
    
    // Test just after expiry
    lockdownStartTime = millis() - (LOCKDOWN_DURATION + 1000); // 1 second after expiry
    
    handleLockdownMode();
    bool nowReleased = (currentMode == IDLE);
    
    if (stillLocked && nowReleased) {
        Serial.println("✓ PASS: Lockdown timing boundary test");
    } else {
        Serial.println("✗ FAIL: Lockdown timing boundary test");
        Serial.println("  Still locked before expiry: " + String(stillLocked));
        Serial.println("  Released after expiry: " + String(nowReleased));
    }
}

void testMultipleLockdownCycles() {
    Serial.println("\n=== Testing Multiple Lockdown Cycles ===");
    
    // First lockdown
    currentMode = IDLE;
    isUnlocked = true;
    activateSecurityLockdown();
    bool firstActivation = (currentMode == SECURITY_LOCKDOWN && !isUnlocked);
    
    // Force expiry
    lockdownStartTime = millis() - (LOCKDOWN_DURATION + 1000);
    handleLockdownMode();
    bool firstRelease = (currentMode == IDLE);
    
    // Second lockdown
    isUnlocked = true;
    activateSecurityLockdown();
    bool secondActivation = (currentMode == SECURITY_LOCKDOWN && !isUnlocked);
    
    if (firstActivation && firstRelease && secondActivation) {
        Serial.println("✓ PASS: Multiple lockdown cycles test");
    } else {
        Serial.println("✗ FAIL: Multiple lockdown cycles test");
        Serial.println("  First activation: " + String(firstActivation));
        Serial.println("  First release: " + String(firstRelease));
        Serial.println("  Second activation: " + String(secondActivation));
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    // Initialize components (mock)
    lockServo.attach(SERVO_PIN);
    pinMode(BUZZER_PIN, OUTPUT);
    
    Serial.println("Security Lockdown Unit Test Starting...");
    Serial.println("=======================================");
    
    testActivateSecurityLockdown();
    testLockdownModeNotExpired();
    testLockdownModeExpired();
    testLockdownTimingBoundary();
    testMultipleLockdownCycles();
    
    Serial.println("\n=======================================");
    Serial.println("Security Lockdown Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
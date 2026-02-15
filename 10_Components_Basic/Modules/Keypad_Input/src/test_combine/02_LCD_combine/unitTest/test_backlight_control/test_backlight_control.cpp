#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// Mock LCD object
hd44780_I2Cexp lcd;

// Mock global variables for testing
enum SystemMode { IDLE, SECURITY_LOCKDOWN };
SystemMode currentMode = IDLE;

String inputBuffer = "";
bool isUnlocked = false;
bool backlightOn = true;
unsigned long backlightOnTime = 0;
const unsigned long BACKLIGHT_DURATION = 10000; // 10 seconds

// Test tracking variables
int backlightOnCalls = 0;
int backlightOffCalls = 0;
int lastBacklightValue = 255;

// Mock LCD backlight control
void mockSetBacklight(int value) {
    lastBacklightValue = value;
    if (value > 0) {
        backlightOnCalls++;
        Serial.println("Backlight ON (value: " + String(value) + ")");
    } else {
        backlightOffCalls++;
        Serial.println("Backlight OFF");
    }
}

// Functions under test (copied from original)
void turnOnBacklight() {
    if (!backlightOn) {
        mockSetBacklight(255);
        backlightOn = true;
    }
    backlightOnTime = millis();
}

void handleBacklight() {
    bool keepBacklightOn = false;
    
    if (!inputBuffer.isEmpty() ||
        currentMode == SECURITY_LOCKDOWN ||
        isUnlocked) {
        keepBacklightOn = true;
    }
    
    Serial.println("  handleBacklight(): keepBacklightOn = " + String(keepBacklightOn));
    Serial.println("  handleBacklight(): backlightOn = " + String(backlightOn));
    
    if (keepBacklightOn) {
        Serial.println("  handleBacklight(): Entering keepBacklightOn block");
        if (!backlightOn) {
            Serial.println("  handleBacklight(): Turning on backlight");
            mockSetBacklight(255);
            backlightOn = true;
        }
        unsigned long newTime = millis();
        Serial.println("  handleBacklight(): Setting backlightOnTime from " + String(backlightOnTime) + " to " + String(newTime));
        backlightOnTime = newTime; // 타이머 갱신
    }
    else if (backlightOn && (millis() - backlightOnTime >= BACKLIGHT_DURATION)) {
        Serial.println("  handleBacklight(): Turning off backlight due to timeout");
        mockSetBacklight(0);
        backlightOn = false;
    }
}

// Test helper functions
void resetBacklightTracking() {
    backlightOnCalls = 0;
    backlightOffCalls = 0;
    lastBacklightValue = 255;
    backlightOn = true;
    backlightOnTime = millis();
}

void initializeLcd() {
    int status = lcd.begin(16, 2);
    if (status) {
        Serial.print("LCD init failed: ");
        Serial.println(status);
    } else {
        mockSetBacklight(255);
        lcd.clear();
    }
}

// Test cases
void testTurnOnBacklight() {
    Serial.println("\n=== Testing Turn On Backlight ===");
    resetBacklightTracking();
    backlightOn = false;
    
    turnOnBacklight();
    
    bool backlight_activated = backlightOn;
    bool time_set = (backlightOnTime > 0);
    bool lcd_called = (backlightOnCalls > 0);
    
    if (backlight_activated && time_set && lcd_called) {
        Serial.println("✓ PASS: Turn on backlight test");
    } else {
        Serial.println("✗ FAIL: Turn on backlight test");
        Serial.println("  Backlight activated: " + String(backlight_activated));
        Serial.println("  Time set: " + String(time_set));
        Serial.println("  LCD called: " + String(lcd_called));
    }
}

void testTurnOnBacklightWhenAlreadyOn() {
    Serial.println("\n=== Testing Turn On Backlight (Already On) ===");
    resetBacklightTracking();
    backlightOn = true;
    unsigned long originalTime = backlightOnTime;
    delay(10);
    
    turnOnBacklight();
    
    bool time_updated = (backlightOnTime > originalTime);
    bool still_on = backlightOn;
    bool no_extra_lcd_call = (backlightOnCalls == 0); // Should not call LCD again
    
    if (time_updated && still_on && no_extra_lcd_call) {
        Serial.println("✓ PASS: Turn on backlight when already on test");
    } else {
        Serial.println("✗ FAIL: Turn on backlight when already on test");
        Serial.println("  Time updated: " + String(time_updated));
        Serial.println("  Still on: " + String(still_on));
        Serial.println("  No extra LCD call: " + String(no_extra_lcd_call));
    }
}

void testHandleBacklightKeepOn() {
    Serial.println("\n=== Testing Handle Backlight - Keep On Conditions ===");
    resetBacklightTracking();
    
    // Test password input condition
    inputBuffer = "*123";
    currentMode = IDLE;
    isUnlocked = false;
    backlightOn = false;
    
    handleBacklight();
    bool inputCondition = backlightOn;
    
    // Test lockdown condition
    inputBuffer = "";
    currentMode = SECURITY_LOCKDOWN;
    isUnlocked = false;
    backlightOn = false;
    
    handleBacklight();
    bool lockdownCondition = backlightOn;
    
    // Test unlocked condition
    inputBuffer = "";
    currentMode = IDLE;
    isUnlocked = true;
    backlightOn = false;
    
    handleBacklight();
    bool unlockedCondition = backlightOn;
    
    if (inputCondition && lockdownCondition && unlockedCondition) {
        Serial.println("✓ PASS: Handle backlight keep on conditions test");
    } else {
        Serial.println("✗ FAIL: Handle backlight keep on conditions test");
        Serial.println("  Input condition: " + String(inputCondition));
        Serial.println("  Lockdown condition: " + String(lockdownCondition));
        Serial.println("  Unlocked condition: " + String(unlockedCondition));
    }
}

void testHandleBacklightTimeout() {
    Serial.println("\n=== Testing Handle Backlight - Timeout ===");
    resetBacklightTracking();
    
    // Set up conditions for timeout
    inputBuffer = "";
    currentMode = IDLE;
    isUnlocked = false;
    backlightOn = true;
    backlightOnTime = millis() - (BACKLIGHT_DURATION + 1000); // Expired
    
    handleBacklight();
    
    bool backlight_off = !backlightOn;
    bool lcd_turned_off = (backlightOffCalls > 0);
    
    if (backlight_off && lcd_turned_off) {
        Serial.println("✓ PASS: Handle backlight timeout test");
    } else {
        Serial.println("✗ FAIL: Handle backlight timeout test");
        Serial.println("  Backlight off: " + String(backlight_off));
        Serial.println("  LCD turned off: " + String(lcd_turned_off));
    }
}

void testHandleBacklightNoTimeoutYet() {
    Serial.println("\n=== Testing Handle Backlight - No Timeout Yet ===");
    resetBacklightTracking();
    
    // Set up conditions for no timeout yet
    inputBuffer = "";
    currentMode = IDLE;
    isUnlocked = false;
    backlightOn = true;
    backlightOnTime = millis() - (BACKLIGHT_DURATION - 1000); // Still has time
    
    handleBacklight();
    
    bool backlight_still_on = backlightOn;
    bool no_lcd_off_call = (backlightOffCalls == 0);
    
    if (backlight_still_on && no_lcd_off_call) {
        Serial.println("✓ PASS: Handle backlight no timeout yet test");
    } else {
        Serial.println("✗ FAIL: Handle backlight no timeout yet test");
        Serial.println("  Backlight still on: " + String(backlight_still_on));
        Serial.println("  No LCD off call: " + String(no_lcd_off_call));
    }
}

void testBacklightTimingBoundary() {
    Serial.println("\n=== Testing Backlight Timing Boundary ===");
    resetBacklightTracking();
    
    // Test just before timeout
    inputBuffer = "";
    currentMode = IDLE;
    isUnlocked = false;
    backlightOn = true;
    backlightOnTime = millis() - (BACKLIGHT_DURATION - 100);
    
    handleBacklight();
    bool stillOnBeforeTimeout = backlightOn;
    
    // Test just after timeout
    backlightOn = true;
    backlightOnTime = millis() - (BACKLIGHT_DURATION + 100);
    
    handleBacklight();
    bool offAfterTimeout = !backlightOn;
    
    if (stillOnBeforeTimeout && offAfterTimeout) {
        Serial.println("✓ PASS: Backlight timing boundary test");
    } else {
        Serial.println("✗ FAIL: Backlight timing boundary test");
        Serial.println("  Still on before timeout: " + String(stillOnBeforeTimeout));
        Serial.println("  Off after timeout: " + String(offAfterTimeout));
    }
}

void testBacklightKeepOnRefreshesTimer() {
    Serial.println("\n=== Testing Backlight Keep-On Refreshes Timer ===");
    resetBacklightTracking();
    
    // Wait to ensure we have enough time to avoid overflow
    // This simulates a real scenario where the system has been running
    delay(6000); // Wait 6 seconds to ensure millis() > 5000
    
    // Set up keep-on condition
    inputBuffer = "*123";
    currentMode = IDLE;
    isUnlocked = false;
    backlightOn = true;
    
    // Now we can safely subtract without overflow, just like the real code
    unsigned long oldTime = millis() - 5000;
    backlightOnTime = oldTime;
    
    Serial.println("Before handleBacklight():");
    Serial.println("  inputBuffer: '" + inputBuffer + "'");
    Serial.println("  backlightOn: " + String(backlightOn));
    Serial.println("  oldTime: " + String(oldTime));
    Serial.println("  backlightOnTime: " + String(backlightOnTime));
    
    handleBacklight();
    
    Serial.println("After handleBacklight():");
    Serial.println("  backlightOnTime: " + String(backlightOnTime));
    Serial.println("  Current millis(): " + String(millis()));
    
    bool timer_refreshed = (backlightOnTime > oldTime);
    bool still_on = backlightOn;
    
    if (timer_refreshed && still_on) {
        Serial.println("✓ PASS: Backlight keep-on refreshes timer test");
    } else {
        Serial.println("✗ FAIL: Backlight keep-on refreshes timer test");
        Serial.println("  Timer refreshed: " + String(timer_refreshed));
        Serial.println("  Still on: " + String(still_on));
        Serial.println("  backlightOnTime (" + String(backlightOnTime) + ") > oldTime (" + String(oldTime) + "): " + String(backlightOnTime > oldTime));
    }
}

void testBacklightStateTransitions() {
    Serial.println("\n=== Testing Backlight State Transitions ===");
    resetBacklightTracking();
    
    // Off -> On (with keep-on condition)
    backlightOn = false;
    inputBuffer = "*123";
    handleBacklight();
    bool offToOn = backlightOn;
    
    // On -> Off (timeout)
    inputBuffer = "";
    currentMode = IDLE;
    isUnlocked = false;
    backlightOnTime = millis() - (BACKLIGHT_DURATION + 1000);
    handleBacklight();
    bool onToOff = !backlightOn;
    
    if (offToOn && onToOff) {
        Serial.println("✓ PASS: Backlight state transitions test");
    } else {
        Serial.println("✗ FAIL: Backlight state transitions test");
        Serial.println("  Off to On: " + String(offToOn));
        Serial.println("  On to Off: " + String(onToOff));
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    initializeLcd();
    
    Serial.println("Backlight Control Unit Test Starting...");
    Serial.println("=======================================");
    
    testTurnOnBacklight();
    testTurnOnBacklightWhenAlreadyOn();
    testHandleBacklightKeepOn();
    testHandleBacklightTimeout();
    testHandleBacklightNoTimeoutYet();
    testBacklightTimingBoundary();
    testBacklightKeepOnRefreshesTimer();
    testBacklightStateTransitions();
    
    Serial.println("\n=======================================");
    Serial.println("Backlight Control Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
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
String lastDisplayMessage = "";
unsigned long lastLcdUpdate = 0;
const unsigned long LCD_UPDATE_INTERVAL = 500;
const unsigned long LOCKDOWN_DURATION = 120000;
const unsigned long UNLOCK_DURATION = 5000;

unsigned long lockdownStartTime = 0;
unsigned long servoUnlockTime = 0;
bool isUnlocked = false;
bool backlightOn = true;

// Test tracking variables
String lastLine1 = "";
String lastLine2 = "";
int displayCallCount = 0;

// Mock function to track backlight calls
void turnOnBacklight() {
    Serial.println("Backlight turned on");
}

// Functions under test (copied from original)
void displayMessage(String line1, String line2) {
    // Track for testing
    lastLine1 = line1;
    lastLine2 = line2;
    displayCallCount++;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    turnOnBacklight();
    
    Serial.println("LCD Display: '" + line1 + "' | '" + line2 + "'");
}

void updateLcdDisplay() {
    unsigned long currentTime = millis();

    if (currentTime - lastLcdUpdate < LCD_UPDATE_INTERVAL)
        return;

    lastLcdUpdate = currentTime;

    String currentMessage = "";

    if (currentMode == SECURITY_LOCKDOWN) {
        unsigned long remainingTime = (LOCKDOWN_DURATION - (currentTime - lockdownStartTime)) / 1000;
        if (remainingTime > 60) {
            currentMessage = "LOCKED " + String(remainingTime / 60) + "m " + String(remainingTime % 60) + "s";
        } else {
            currentMessage = "LOCKED " + String(remainingTime) + "s";
        }

        if (lastDisplayMessage != currentMessage) {
            displayMessage("Security Lockdown", currentMessage);
            lastDisplayMessage = currentMessage;
        }
    }
    else if (isUnlocked) {
        unsigned long remainingUnlockTime = (UNLOCK_DURATION - (currentTime - servoUnlockTime)) / 1000;
        currentMessage = "UNLOCKED " + String(remainingUnlockTime + 1) + "s";

        if (lastDisplayMessage != currentMessage) {
            displayMessage("Access Granted", currentMessage);
            lastDisplayMessage = currentMessage;
        }
    }
    else if (!inputBuffer.isEmpty()) {
        if (inputBuffer.startsWith("*")) {
            int enteredDigits = inputBuffer.length() - 1;
            if (enteredDigits <= 6) {
                String stars = "";
                for (int i = 0; i < enteredDigits; i++) {
                    stars += "*";
                }
                currentMessage = "Enter: " + stars;

                if (lastDisplayMessage != currentMessage) {
                    displayMessage("Password Entry", currentMessage);
                    lastDisplayMessage = currentMessage;
                }
            }
        }
    }
    else if (currentMode == IDLE && inputBuffer.isEmpty()) {
        if (lastDisplayMessage != "Ready") {
            displayMessage("Security Lock", "Ready");
            lastDisplayMessage = "Ready";
        }
    }
}

// Test helper functions
void resetDisplayTracking() {
    lastLine1 = "";
    lastLine2 = "";
    displayCallCount = 0;
    lastDisplayMessage = "";
}

void initializeLcd() {
    int status = lcd.begin(16, 2);
    if (status) {
        Serial.print("LCD init failed: ");
        Serial.println(status);
    } else {
        lcd.setBacklight(255);
        lcd.clear();
    }
}

// Test cases
void testDisplayMessage() {
    Serial.println("\n=== Testing Display Message ===");
    resetDisplayTracking();
    
    displayMessage("Test Line 1", "Test Line 2");
    
    bool correctLine1 = (lastLine1 == "Test Line 1");
    bool correctLine2 = (lastLine2 == "Test Line 2");
    bool callCounted = (displayCallCount == 1);
    
    if (correctLine1 && correctLine2 && callCounted) {
        Serial.println("✓ PASS: Display message test");
    } else {
        Serial.println("✗ FAIL: Display message test");
        Serial.println("  Line 1 correct: " + String(correctLine1));
        Serial.println("  Line 2 correct: " + String(correctLine2));
        Serial.println("  Call counted: " + String(callCounted));
    }
}

void testUpdateLcdDisplayReady() {
    Serial.println("\n=== Testing LCD Update - Ready State ===");
    resetDisplayTracking();
    currentMode = IDLE;
    inputBuffer = "";
    lastLcdUpdate = 0;
    
    updateLcdDisplay();
    
    bool correctLine1 = (lastLine1 == "Security Lock");
    bool correctLine2 = (lastLine2 == "Ready");
    bool messageUpdated = (lastDisplayMessage == "Ready");
    
    if (correctLine1 && correctLine2 && messageUpdated) {
        Serial.println("✓ PASS: LCD update ready state test");
    } else {
        Serial.println("✗ FAIL: LCD update ready state test");
        Serial.println("  Line 1: '" + lastLine1 + "'");
        Serial.println("  Line 2: '" + lastLine2 + "'");
        Serial.println("  Message: '" + lastDisplayMessage + "'");
    }
}

void testUpdateLcdDisplayPasswordEntry() {
    Serial.println("\n=== Testing LCD Update - Password Entry ===");
    resetDisplayTracking();
    currentMode = IDLE;
    inputBuffer = "*123";
    lastLcdUpdate = 0;
    
    updateLcdDisplay();
    
    bool correctLine1 = (lastLine1 == "Password Entry");
    bool correctLine2 = (lastLine2 == "Enter: ***");
    
    if (correctLine1 && correctLine2) {
        Serial.println("✓ PASS: LCD update password entry test");
    } else {
        Serial.println("✗ FAIL: LCD update password entry test");
        Serial.println("  Line 1: '" + lastLine1 + "'");
        Serial.println("  Line 2: '" + lastLine2 + "'");
    }
}

void testUpdateLcdDisplayUnlocked() {
    Serial.println("\n=== Testing LCD Update - Unlocked State ===");
    resetDisplayTracking();
    currentMode = IDLE;
    inputBuffer = "";
    isUnlocked = true;
    servoUnlockTime = millis() - 2000; // 2 seconds ago
    lastLcdUpdate = 0;
    
    updateLcdDisplay();
    
    bool correctLine1 = (lastLine1 == "Access Granted");
    bool hasUnlockedMessage = lastLine2.indexOf("UNLOCKED") >= 0;
    
    if (correctLine1 && hasUnlockedMessage) {
        Serial.println("✓ PASS: LCD update unlocked state test");
    } else {
        Serial.println("✗ FAIL: LCD update unlocked state test");
        Serial.println("  Line 1: '" + lastLine1 + "'");
        Serial.println("  Line 2: '" + lastLine2 + "'");
    }
}

void testUpdateLcdDisplayLockdown() {
    Serial.println("\n=== Testing LCD Update - Lockdown State ===");
    resetDisplayTracking();
    currentMode = SECURITY_LOCKDOWN;
    inputBuffer = "";
    isUnlocked = false;
    lockdownStartTime = millis() - 30000; // 30 seconds ago
    lastLcdUpdate = 0;
    
    updateLcdDisplay();
    
    bool correctLine1 = (lastLine1 == "Security Lockdown");
    bool hasLockedMessage = lastLine2.indexOf("LOCKED") >= 0;
    
    if (correctLine1 && hasLockedMessage) {
        Serial.println("✓ PASS: LCD update lockdown state test");
    } else {
        Serial.println("✗ FAIL: LCD update lockdown state test");
        Serial.println("  Line 1: '" + lastLine1 + "'");
        Serial.println("  Line 2: '" + lastLine2 + "'");
    }
}

void testUpdateLcdDisplayThrottling() {
    Serial.println("\n=== Testing LCD Update Throttling ===");
    resetDisplayTracking();
    currentMode = IDLE;
    inputBuffer = "";
    lastLcdUpdate = millis(); // Set to current time to trigger throttling
    
    updateLcdDisplay();
    
    bool noDisplayCall = (displayCallCount == 0);
    
    if (noDisplayCall) {
        Serial.println("✓ PASS: LCD update throttling test");
    } else {
        Serial.println("✗ FAIL: LCD update should be throttled");
        Serial.println("  Display calls: " + String(displayCallCount));
    }
}

void testPasswordEntryProgression() {
    Serial.println("\n=== Testing Password Entry Progression ===");
    resetDisplayTracking();
    currentMode = IDLE;
    lastLcdUpdate = 0;
    
    // Test different password entry lengths
    String expectedStars[] = {"", "*", "**", "***", "****", "*****", "******"};
    
    bool allCorrect = true;
    for (int i = 0; i <= 6; i++) {
        // Reset tracking for each test
        resetDisplayTracking();
        lastDisplayMessage = "DIFFERENT"; // Force update by setting different message
        lastLcdUpdate = 0; // Reset throttling
        
        inputBuffer = "*";
        for (int j = 0; j < i; j++) {
            inputBuffer += "1";
        }
        
        Serial.println("Testing with inputBuffer: '" + inputBuffer + "' (length: " + String(inputBuffer.length()) + ")");
        Serial.println("enteredDigits would be: " + String(inputBuffer.length() - 1));
        
        updateLcdDisplay();
        
        String expectedLine2 = "Enter: " + expectedStars[i];
        Serial.println("Expected: '" + expectedLine2 + "', Got: '" + lastLine2 + "'");
        Serial.println("lastDisplayMessage after update: '" + lastDisplayMessage + "'");
        Serial.println("Display calls: " + String(displayCallCount));
        
        if (lastLine2 != expectedLine2) {
            allCorrect = false;
            Serial.println("  Failed at length " + String(i) + ": got '" + lastLine2 + "', expected '" + expectedLine2 + "'");
        }
    }
    
    if (allCorrect) {
        Serial.println("✓ PASS: Password entry progression test");
    } else {
        Serial.println("✗ FAIL: Password entry progression test");
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    initializeLcd();
    
    Serial.println("Display Control Unit Test Starting...");
    Serial.println("=====================================");
    
    testDisplayMessage();
    testUpdateLcdDisplayReady();
    testUpdateLcdDisplayPasswordEntry();
    testUpdateLcdDisplayUnlocked();
    testUpdateLcdDisplayLockdown();
    testUpdateLcdDisplayThrottling();
    testPasswordEntryProgression();
    
    Serial.println("\n=====================================");
    Serial.println("Display Control Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
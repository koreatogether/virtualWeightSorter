#include <Arduino.h>

// Mock global variables for testing
const int BUZZER_PIN = 12;
const unsigned long BUZZER_DURATION = 60000; // 1 minute
const unsigned long BUZZER_INTERVAL = 1000; // 1 second

bool buzzerActive = false;
bool buzzerState = false;
unsigned long buzzerStartTime = 0;
unsigned long lastBuzzerToggle = 0;

// Function under test (copied from original)
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

// Test helper functions
void initializeBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void activateBuzzer() {
    buzzerActive = true;
    buzzerStartTime = millis();
    buzzerState = false;
    lastBuzzerToggle = 0;
}

bool readBuzzerPin() {
    return digitalRead(BUZZER_PIN) == HIGH;
}

// Test cases
void testBuzzerInactive() {
    Serial.println("\n=== Testing Buzzer When Inactive ===");
    buzzerActive = false;
    buzzerState = false;
    digitalWrite(BUZZER_PIN, LOW);
    
    handleBuzzer();
    
    bool pinState = readBuzzerPin();
    if (!pinState && !buzzerActive) {
        Serial.println("✓ PASS: Buzzer remains inactive when not activated");
    } else {
        Serial.println("✗ FAIL: Buzzer should remain inactive");
        Serial.println("  Pin state: " + String(pinState));
        Serial.println("  Buzzer active: " + String(buzzerActive));
    }
}

void testBuzzerActivation() {
    Serial.println("\n=== Testing Buzzer Activation ===");
    activateBuzzer();
    
    // First call should not toggle yet (needs interval)
    handleBuzzer();
    
    bool stillActive = buzzerActive;
    bool timeSet = (buzzerStartTime > 0);
    
    if (stillActive && timeSet) {
        Serial.println("✓ PASS: Buzzer activation test");
    } else {
        Serial.println("✗ FAIL: Buzzer activation test");
        Serial.println("  Still active: " + String(stillActive));
        Serial.println("  Time set: " + String(timeSet));
    }
}

void testBuzzerToggling() {
    Serial.println("\n=== Testing Buzzer Toggling ===");
    activateBuzzer();
    buzzerStartTime = millis() - (BUZZER_INTERVAL + 100); // Ensure interval passed
    
    bool initialState = buzzerState;
    handleBuzzer();
    bool newState = buzzerState;
    
    if (initialState != newState) {
        Serial.println("✓ PASS: Buzzer toggles correctly");
    } else {
        Serial.println("✗ FAIL: Buzzer should toggle state");
        Serial.println("  Initial state: " + String(initialState));
        Serial.println("  New state: " + String(newState));
    }
}

void testBuzzerExpiry() {
    Serial.println("\n=== Testing Buzzer Expiry ===");
    activateBuzzer();
    buzzerStartTime = millis() - (BUZZER_DURATION + 1000); // Simulate expired
    buzzerState = true;
    digitalWrite(BUZZER_PIN, HIGH);
    
    handleBuzzer();
    
    bool buzzerOff = !buzzerActive;
    bool pinLow = !readBuzzerPin();
    
    if (buzzerOff && pinLow) {
        Serial.println("✓ PASS: Buzzer expires correctly");
    } else {
        Serial.println("✗ FAIL: Buzzer expiry test failed");
        Serial.println("  Buzzer off: " + String(buzzerOff));
        Serial.println("  Pin low: " + String(pinLow));
    }
}

void testBuzzerTimingBoundary() {
    Serial.println("\n=== Testing Buzzer Timing Boundary ===");
    
    // Test just before expiry
    activateBuzzer();
    buzzerStartTime = millis() - (BUZZER_DURATION - 1000);
    
    handleBuzzer();
    bool stillActiveBeforeExpiry = buzzerActive;
    
    // Test just after expiry
    buzzerStartTime = millis() - (BUZZER_DURATION + 1000);
    
    handleBuzzer();
    bool inactiveAfterExpiry = !buzzerActive;
    
    if (stillActiveBeforeExpiry && inactiveAfterExpiry) {
        Serial.println("✓ PASS: Buzzer timing boundary test");
    } else {
        Serial.println("✗ FAIL: Buzzer timing boundary test");
        Serial.println("  Active before expiry: " + String(stillActiveBeforeExpiry));
        Serial.println("  Inactive after expiry: " + String(inactiveAfterExpiry));
    }
}

void testMultipleBuzzerCycles() {
    Serial.println("\n=== Testing Multiple Buzzer Cycles ===");
    
    // First cycle
    activateBuzzer();
    bool firstActivation = buzzerActive;
    
    // Force expiry
    buzzerStartTime = millis() - (BUZZER_DURATION + 1000);
    handleBuzzer();
    bool firstExpiry = !buzzerActive;
    
    // Second cycle
    activateBuzzer();
    bool secondActivation = buzzerActive;
    
    if (firstActivation && firstExpiry && secondActivation) {
        Serial.println("✓ PASS: Multiple buzzer cycles test");
    } else {
        Serial.println("✗ FAIL: Multiple buzzer cycles test");
        Serial.println("  First activation: " + String(firstActivation));
        Serial.println("  First expiry: " + String(firstExpiry));
        Serial.println("  Second activation: " + String(secondActivation));
    }
}

void testBuzzerStateToggleSequence() {
    Serial.println("\n=== Testing Buzzer Toggle Sequence ===");
    activateBuzzer();
    
    // Test realistic toggle sequence with actual delays
    bool toggleStates[4];
    
    for (int i = 0; i < 4; i++) {
        bool prevState = buzzerState;
        
        // Wait for interval to pass, then call handleBuzzer
        delay(BUZZER_INTERVAL + 50); // Wait slightly longer than interval
        handleBuzzer();
        
        toggleStates[i] = (prevState != buzzerState);
        Serial.println("  Toggle " + String(i) + ": " + String(toggleStates[i]));
    }
    
    bool allTogglesWorked = true;
    for (int i = 0; i < 4; i++) {
        if (!toggleStates[i]) {
            allTogglesWorked = false;
            break;
        }
    }
    
    if (allTogglesWorked) {
        Serial.println("✓ PASS: Buzzer toggle sequence test");
    } else {
        Serial.println("✗ FAIL: Buzzer toggle sequence test");
        for (int i = 0; i < 4; i++) {
            Serial.println("  Toggle " + String(i) + ": " + String(toggleStates[i]));
        }
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    initializeBuzzer();
    
    Serial.println("Buzzer Control Unit Test Starting...");
    Serial.println("====================================");
    
    testBuzzerInactive();
    testBuzzerActivation();
    testBuzzerToggling();
    testBuzzerExpiry();
    testBuzzerTimingBoundary();
    testMultipleBuzzerCycles();
    testBuzzerStateToggleSequence();
    
    Serial.println("\n====================================");
    Serial.println("Buzzer Control Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
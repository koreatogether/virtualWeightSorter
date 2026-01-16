#include <Arduino.h>
#include <Adafruit_Keypad.h>

// Mock keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Mock global variables for testing
String inputBuffer = "";
unsigned long lastKeyPressTime = 0;
unsigned long passwordEntryStartTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long PASSWORD_ENTRY_TIMEOUT = 10000;

// Test tracking variables
int processPasswordCalls = 0;
int displayMessageCalls = 0;
String lastDisplayLine1 = "";
String lastDisplayLine2 = "";
bool backlightTurned = false;

// Mock functions
void displayMessage(String line1, String line2) {
    displayMessageCalls++;
    lastDisplayLine1 = line1;
    lastDisplayLine2 = line2;
    Serial.println("Display: '" + line1 + "' | '" + line2 + "'");
}

void turnOnBacklight() {
    backlightTurned = true;
    Serial.println("Backlight turned on");
}

void processPasswordInput() {
    processPasswordCalls++;
    Serial.println("processPasswordInput() called");
    inputBuffer = ""; // Reset buffer as original function does
}

// Mock keypad event structure for testing
struct MockKeypadEvent {
    char key;
    bool pressed;
};

MockKeypadEvent mockEvents[10];
int mockEventCount = 0;
int mockEventIndex = 0;

void addMockKeyEvent(char key, bool pressed = true) {
    if (mockEventCount < 10) {
        mockEvents[mockEventCount].key = key;
        mockEvents[mockEventCount].pressed = pressed;
        mockEventCount++;
    }
}

bool mockKeypadAvailable() {
    return mockEventIndex < mockEventCount;
}

keypadEvent mockKeypadRead() {
    keypadEvent e;
    if (mockEventIndex < mockEventCount) {
        e.bit.KEY = mockEvents[mockEventIndex].key;
        e.bit.EVENT = mockEvents[mockEventIndex].pressed ? KEY_JUST_PRESSED : KEY_JUST_RELEASED;
        mockEventIndex++;
    }
    return e;
}

// Reset mock state
void resetMockState() {
    mockEventCount = 0;
    mockEventIndex = 0;
    processPasswordCalls = 0;
    displayMessageCalls = 0;
    lastDisplayLine1 = "";
    lastDisplayLine2 = "";
    backlightTurned = false;
    inputBuffer = "";
    lastKeyPressTime = 0;
    passwordEntryStartTime = 0;
}

// Modified handleIdleMode function for testing (using mock keypad)
void handleIdleMode() {
    while (mockKeypadAvailable()) {
        keypadEvent e = mockKeypadRead();

        if (e.bit.EVENT == KEY_JUST_PRESSED) {
            unsigned long currentTime = millis();

            if (currentTime - lastKeyPressTime < DEBOUNCE_DELAY) {
                continue;
            }

            lastKeyPressTime = currentTime;
            char key = (char)e.bit.KEY;

            // Input timeout check
            if (!inputBuffer.isEmpty() && (currentTime - passwordEntryStartTime > PASSWORD_ENTRY_TIMEOUT)) {
                if (inputBuffer.startsWith("*") && (inputBuffer.length() - 1) == 6) {
                    Serial.println("Input timeout - processing 6 digit entry");
                    inputBuffer += '#';
                    processPasswordInput();
                } else {
                    inputBuffer = "";
                    Serial.println("Input timeout - buffer cleared");
                    displayMessage("Timeout", "Try Again");
                    unsigned long timeoutStart = millis();
                    while (millis() - timeoutStart < 2000) {
                        // Non-blocking delay - other operations can continue
                    }
                }
            }

            if (key == '*' && inputBuffer.isEmpty()) {
                inputBuffer = "*";
                passwordEntryStartTime = currentTime;
                Serial.println("Password entry started");
                displayMessage("Password Entry", "Enter: ");
                turnOnBacklight();
            }
            else if (inputBuffer.startsWith("*")) {
                size_t dataLen = inputBuffer.length() - 1;

                if ((key >= '0' && key <= '9') && dataLen < 6) {
                    inputBuffer += key;
                    Serial.print("*");
                }
                else if ((key == '#' || key == '*') && dataLen == 6) {
                    inputBuffer += key;
                    Serial.print("*");
                    Serial.println();
                    processPasswordInput();
                }
                else {
                    size_t dataLen2 = inputBuffer.length() - 1;
                    if (dataLen2 == 6) {
                        Serial.println("Invalid terminator - processing anyway");
                        inputBuffer += '#';
                        processPasswordInput();
                    } else {
                        Serial.println("Invalid input format");
                        inputBuffer = "";
                        displayMessage("Invalid Input", "Try Again");
                        unsigned long errorStart = millis();
                        while (millis() - errorStart < 2000) {
                            // Non-blocking delay - other operations can continue
                        }
                    }
                }
            }
            else {
                inputBuffer = "";
            }
        }
    }
}

// Test cases
void testPasswordEntryStart() {
    Serial.println("\n=== Testing Password Entry Start ===");
    resetMockState();
    
    addMockKeyEvent('*');
    handleIdleMode();
    
    bool bufferStarted = (inputBuffer == "*");
    bool displayCalled = (displayMessageCalls > 0);
    bool correctDisplay = (lastDisplayLine1 == "Password Entry");
    bool backlightOn = backlightTurned;
    bool timeSet = (passwordEntryStartTime > 0);
    
    if (bufferStarted && displayCalled && correctDisplay && backlightOn && timeSet) {
        Serial.println("✓ PASS: Password entry start test");
    } else {
        Serial.println("✗ FAIL: Password entry start test");
        Serial.println("  Buffer started: " + String(bufferStarted));
        Serial.println("  Display called: " + String(displayCalled));
        Serial.println("  Correct display: " + String(correctDisplay));
        Serial.println("  Backlight on: " + String(backlightOn));
        Serial.println("  Time set: " + String(timeSet));
    }
}

void testDigitEntry() {
    Serial.println("\n=== Testing Digit Entry ===");
    resetMockState();
    lastKeyPressTime = 0; // Reset debounce timer
    
    // Start password entry naturally with '*' key press
    addMockKeyEvent('*');
    handleIdleMode();
    delay(DEBOUNCE_DELAY + 10);
    
    // Add one digit at a time with proper timing
    addMockKeyEvent('1');
    handleIdleMode();
    delay(DEBOUNCE_DELAY + 10);
    
    addMockKeyEvent('2');
    handleIdleMode();
    delay(DEBOUNCE_DELAY + 10);
    
    addMockKeyEvent('3');
    handleIdleMode();
    
    bool correctBuffer = (inputBuffer == "*123");
    bool timeSet = (passwordEntryStartTime > 0);
    
    if (correctBuffer && timeSet) {
        Serial.println("✓ PASS: Digit entry test");
    } else {
        Serial.println("✗ FAIL: Digit entry test");
        Serial.println("  Buffer: '" + inputBuffer + "'");
        Serial.println("  Expected: '*123'");
        Serial.println("  Time set: " + String(timeSet));
    }
}

void testValidPasswordCompletion() {
    Serial.println("\n=== Testing Valid Password Completion ===");
    resetMockState();
    lastKeyPressTime = 0;
    
    // Natural sequence: *, 6 digits, then terminator
    char keys[] = {'*', '1', '2', '3', '4', '5', '6', '#'};
    
    for (int i = 0; i < 8; i++) {
        addMockKeyEvent(keys[i]);
        handleIdleMode();
        if (i < 7) {
            delay(DEBOUNCE_DELAY + 10);
        }
    }
    
    bool processedPassword = (processPasswordCalls > 0);
    bool bufferCleared = (inputBuffer == "");
    
    if (processedPassword && bufferCleared) {
        Serial.println("✓ PASS: Valid password completion test");
    } else {
        Serial.println("✗ FAIL: Valid password completion test");
        Serial.println("  Password processed: " + String(processedPassword));
        Serial.println("  Buffer cleared: " + String(bufferCleared));
    }
}

void testInvalidDigitEntry() {
    Serial.println("\n=== Testing Invalid Digit Entry ===");
    resetMockState();
    lastKeyPressTime = 0;
    
    // Natural sequence: *, 5 digits, then invalid character
    char keys[] = {'*', '1', '2', '3', '4', '5', 'A'};
    
    for (int i = 0; i < 7; i++) {
        addMockKeyEvent(keys[i]);
        handleIdleMode();
        if (i < 6) {
            delay(DEBOUNCE_DELAY + 10);
        }
    }
    
    // With 5 digits + invalid character, should trigger "Invalid input format"
    bool bufferCleared = (inputBuffer == "");
    bool displayCalled = (displayMessageCalls > 0);
    
    if (bufferCleared && displayCalled) {
        Serial.println("✓ PASS: Invalid digit entry test");
    } else {
        Serial.println("✗ FAIL: Invalid digit entry test");
        Serial.println("  Buffer cleared: " + String(bufferCleared));
        Serial.println("  Display called: " + String(displayCalled));
        Serial.println("  Buffer: '" + inputBuffer + "'");
    }
}

void testInvalidTerminatorAutoProcess() {
    Serial.println("\n=== Testing Invalid Terminator Auto Process ===");
    resetMockState();
    lastKeyPressTime = 0;
    
    // Natural sequence: *, 6 digits, then invalid terminator
    char keys[] = {'*', '1', '2', '3', '4', '5', '6', 'A'};
    
    for (int i = 0; i < 8; i++) {
        addMockKeyEvent(keys[i]);
        handleIdleMode();
        if (i < 7) {
            delay(DEBOUNCE_DELAY + 10);
        }
    }
    
    bool processedPassword = (processPasswordCalls > 0);
    bool bufferCleared = (inputBuffer == "");
    
    if (processedPassword && bufferCleared) {
        Serial.println("✓ PASS: Invalid terminator auto process test");
    } else {
        Serial.println("✗ FAIL: Invalid terminator auto process test");
        Serial.println("  Password processed: " + String(processedPassword));
        Serial.println("  Buffer cleared: " + String(bufferCleared));
    }
}

void testInvalidInputFormat() {
    Serial.println("\n=== Testing Invalid Input Format ===");
    resetMockState();
    
    // Set up partial entry
    inputBuffer = "*123";
    passwordEntryStartTime = millis();
    
    // Try invalid terminator with incomplete password
    addMockKeyEvent('A');
    handleIdleMode();
    
    bool bufferCleared = (inputBuffer == "");
    bool errorDisplayed = (lastDisplayLine1 == "Invalid Input");
    
    if (bufferCleared && errorDisplayed) {
        Serial.println("✓ PASS: Invalid input format test");
    } else {
        Serial.println("✗ FAIL: Invalid input format test");
        Serial.println("  Buffer cleared: " + String(bufferCleared));
        Serial.println("  Error displayed: " + String(errorDisplayed));
        Serial.println("  Display line 1: '" + lastDisplayLine1 + "'");
    }
}

void testNonPasswordKeypress() {
    Serial.println("\n=== Testing Non-Password Keypress ===");
    resetMockState();
    
    // Press key without starting with *
    addMockKeyEvent('1');
    handleIdleMode();
    
    bool bufferEmpty = (inputBuffer == "");
    bool noProcessing = (processPasswordCalls == 0);
    
    if (bufferEmpty && noProcessing) {
        Serial.println("✓ PASS: Non-password keypress test");
    } else {
        Serial.println("✗ FAIL: Non-password keypress test");
        Serial.println("  Buffer empty: " + String(bufferEmpty));
        Serial.println("  No processing: " + String(noProcessing));
    }
}

void testPasswordEntrySequence() {
    Serial.println("\n=== Testing Complete Password Entry Sequence ===");
    resetMockState();
    lastKeyPressTime = 0; // Reset debounce timer
    
    // Complete sequence with proper timing between each key
    char keys[] = {'*', '1', '2', '3', '4', '5', '6', '#'};
    
    for (int i = 0; i < 8; i++) {
        addMockKeyEvent(keys[i]);
        handleIdleMode();
        if (i < 7) { // Don't delay after the last key
            delay(DEBOUNCE_DELAY + 10); // Wait for debounce between keys
        }
    }
    
    bool processedPassword = (processPasswordCalls > 0);
    bool bufferCleared = (inputBuffer == "");
    bool displayShown = (displayMessageCalls > 0);
    
    if (processedPassword && bufferCleared && displayShown) {
        Serial.println("✓ PASS: Complete password entry sequence test");
    } else {
        Serial.println("✗ FAIL: Complete password entry sequence test");
        Serial.println("  Password processed: " + String(processedPassword));
        Serial.println("  Buffer cleared: " + String(bufferCleared));
        Serial.println("  Display shown: " + String(displayShown));
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    customKeypad.begin();
    
    Serial.println("Idle Mode Unit Test Starting...");
    Serial.println("===============================");
    
    testPasswordEntryStart();
    testDigitEntry();
    testValidPasswordCompletion();
    testInvalidDigitEntry();
    testInvalidTerminatorAutoProcess();
    testInvalidInputFormat();
    testNonPasswordKeypress();
    testPasswordEntrySequence();
    
    Serial.println("\n===============================");
    Serial.println("Idle Mode Unit Test Complete");
}

void loop() {
    // Test complete, no loop needed
}
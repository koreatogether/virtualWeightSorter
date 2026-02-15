```cpp
#include <Adafruit_Keypad.h>
#include <Servo.h>

    // Constants for Keypad Configuration
    const byte KEYPAD_ROWS = 4; // Number of rows in the keypad
const byte KEYPAD_COLS = 4;     // Number of columns in the keypad

// Keymap definition (2D array representing the keys)
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Pin definitions for rows and columns (adjust based on your hardware wiring)
byte rowPins[KEYPAD_ROWS] = {9, 8, 7, 6}; // Pins connected to rows 1-4
byte colPins[KEYPAD_COLS] = {5, 4, 3, 2}; // Pins connected to columns 1-4

// Global Keypad Object
Adafruit_Keypad customKeypad = Adafruit_Keypad(
    makeKeymap(keys), // Use the defined keymap
    rowPins,          // Row pins array
    colPins,          // Column pins array
    KEYPAD_ROWS,      // Number of rows
    KEYPAD_COLS       // Number of columns
);

// Setup Function: Initializes hardware and serial communication
void setup()
{
    Serial.begin(115200); // Start serial communication at 115200 baud rate for debugging
    while (!Serial)
    {     // Wait for serial port to connect (necessary for some boards)
        ; // Empty loop to wait
    }
    Serial.println("Starting Adafruit 4x4 Keypad Test");

    customKeypad.begin(); // Initialize the keypad library
}

// Helper Function: Handles and logs keypad events for clarity
void handleKeyEvent(const keypadEvent &e)
{
    char key = (char)e.bit.KEY; // Extract the key character from the event

    // Print timestamp for better logging
    Serial.print("[");
    Serial.print(millis()); // Current milliseconds since boot
    Serial.print(" ms] ");

    // Handle different event types
    switch (e.bit.EVENT)
    {
    case KEY_JUST_PRESSED:
        Serial.print("Key '");
        Serial.print(key);
        Serial.println("' pressed");
        break;
    case KEY_JUST_RELEASED:
        Serial.print("Key '");
        Serial.print(key);
        Serial.println("' released");
        break;
    case KEY_HELD:
        Serial.print("Key '");
        Serial.print(key);
        Serial.println("' held");
        break;
    default:
        Serial.print("Unknown event: ");
        Serial.println(e.bit.EVENT);
        break;
    }
}

// Main Loop: Continuously checks for keypad events
void loop()
{
    customKeypad.tick(); // Update the keypad state (must be called regularly)

    // Process all queued events
    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read(); // Read the next event
        handleKeyEvent(e);                   // Handle and log the event
    }
}
```
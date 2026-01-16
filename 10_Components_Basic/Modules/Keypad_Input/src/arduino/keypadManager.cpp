#include "keypadManager.h"
#include <Adafruit_Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

KeypadManager::KeypadManager() {}

void KeypadManager::begin()
{
    customKeypad.begin();
}

bool KeypadManager::readCode(char *buffer, size_t len)
{
    static String inputBuffer = "";
    customKeypad.tick();

    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            char key = (char)e.bit.KEY;
            if (key == '*' && inputBuffer.length() == 0)
            {
                inputBuffer += key;
            }
            else if (inputBuffer.length() > 0 && inputBuffer.length() < 8)
            {
                inputBuffer += key;
            }
            else if ((key == '#' || key == '*') && inputBuffer.length() == 7)
            {
                inputBuffer += key;
                if (inputBuffer.length() <= len - 1)
                {
                    strcpy(buffer, inputBuffer.c_str());
                    inputBuffer = "";
                    return true;
                }
                inputBuffer = "";
            }
        }
    }
    return false;
}

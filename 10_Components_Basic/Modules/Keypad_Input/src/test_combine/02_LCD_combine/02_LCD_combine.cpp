#include <Adafruit_Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// LCD 설정
hd44780_I2Cexp lcd;

// 키패드 설정
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

// 서보모터 설정
Servo lockServo;
const int SERVO_PIN = 10;
const int LOCK_ANGLE = 0;
const int UNLOCK_ANGLE = 180;

// 수동 해제 버튼
const int MANUAL_BUTTON_PIN = 11;

// 부저 설정
const int BUZZER_PIN = 12;

// 시스템 상태 열거형
enum SystemMode
{
    IDLE,
    SECURITY_LOCKDOWN
};

// 전역 변수
SystemMode currentMode = IDLE;
String inputBuffer = "";
const String correctPassword = "123456";
int failureCount = 0;
const int MAX_FAILURES = 5;

// 타이밍 관리 변수
unsigned long lockdownStartTime = 0;
unsigned long buzzerStartTime = 0;
unsigned long servoUnlockTime = 0;
bool isUnlocked = false;
bool buzzerActive = false;
bool buzzerState = false;
unsigned long lastBuzzerToggle = 0;
unsigned long lastButtonPressTime = 0;
unsigned long lastKeyPressTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// 비밀번호 입력 타임아웃
unsigned long passwordEntryStartTime = 0;
const unsigned long PASSWORD_ENTRY_TIMEOUT = 10000;

// 시간 상수 정의
const unsigned long BUZZER_DURATION = 60000;
const unsigned long LOCKDOWN_DURATION = 120000;
const unsigned long UNLOCK_DURATION = 5000;
const unsigned long BUZZER_INTERVAL = 1000;

// LCD 표시 관리 변수
unsigned long lastLcdUpdate = 0;
const unsigned long LCD_UPDATE_INTERVAL = 500;
String lastDisplayMessage = "";

// 백라이트 관리 변수
unsigned long backlightOnTime = 0;
const unsigned long BACKLIGHT_DURATION = 10000; // 10초
bool backlightOn = true;

// 함수 프로토타입 선언
void displayMessage(String line1, String line2);
void updateLcdDisplay();
void handleIdleMode();
void handleLockdownMode();
void processPasswordInput();
void unlockDoor();
void handleAutoLock();
void handleManualUnlock();
void activateSecurityLockdown();
void handleBuzzer();
void turnOnBacklight();
void handleBacklight();
void playShortBeep();
void playUnlockBeep();
void playLockBeep();

void setup()
{
    Serial.begin(9600);
    customKeypad.begin();

    // LCD 초기화
    int status = lcd.begin(16, 2);
    if (status)
    {
        Serial.print("LCD init failed: ");
        Serial.println(status);
        while (1)
            ;
    }

    lcd.setBacklight(255);
    lcd.clear();

    // 서보모터 초기화
    lockServo.attach(SERVO_PIN);
    lockServo.write(LOCK_ANGLE);

    // 핀 모드 설정 (외부 풀업 모듈 사용)
    pinMode(MANUAL_BUTTON_PIN, INPUT); // 풀업 모듈 사용으로 내부 풀업 비활성화
    pinMode(BUZZER_PIN, OUTPUT);

    // 시작 메시지
    displayMessage("Security Lock", "Ready");
    turnOnBacklight(); // 시작 시 백라이트 켜기
    Serial.println("Keypad Servo LCD Lock System Started");
    Serial.println("Password format: *123456# or *123456*");
    Serial.println("Manual unlock button available");
}

void loop()
{
    // 수동 해제 버튼 확인 (풀업 모듈은 평상시 LOW, 눌렀을 때 HIGH)
    if (digitalRead(MANUAL_BUTTON_PIN) == HIGH && (millis() - lastButtonPressTime >= 200))
    {
        playShortBeep(); // 수동 버튼 누를 때 짧은 비프음
        handleManualUnlock();
        lastButtonPressTime = millis();
    }

    // 키패드 이벤트 업데이트
    customKeypad.tick();

    // 현재 시스템 모드에 따른 처리
    switch (currentMode)
    {
    case IDLE:
        handleIdleMode();
        break;
    case SECURITY_LOCKDOWN:
        handleLockdownMode();
        break;
    }

    // 서보모터 자동 잠금 처리
    handleAutoLock();

    // 부저 제어
    handleBuzzer();

    // LCD 업데이트
    updateLcdDisplay();
    
    // 백라이트 관리
    handleBacklight();
}

void displayMessage(String line1, String line2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    turnOnBacklight(); // 메시지 표시할 때마다 백라이트 켜기
}

void updateLcdDisplay()
{
    unsigned long currentTime = millis();

    if (currentTime - lastLcdUpdate < LCD_UPDATE_INTERVAL)
        return;

    lastLcdUpdate = currentTime;

    String currentMessage = "";

    if (currentMode == SECURITY_LOCKDOWN)
    {
        unsigned long remainingTime = (LOCKDOWN_DURATION - (currentTime - lockdownStartTime)) / 1000;
        if (remainingTime > 60)
        {
            currentMessage = "LOCKED " + String(remainingTime / 60) + "m " + String(remainingTime % 60) + "s";
        }
        else
        {
            currentMessage = "LOCKED " + String(remainingTime) + "s";
        }

        if (lastDisplayMessage != currentMessage)
        {
            displayMessage("Security Lockdown", currentMessage);
            lastDisplayMessage = currentMessage;
        }
    }
    else if (isUnlocked)
    {
        unsigned long remainingUnlockTime = (UNLOCK_DURATION - (currentTime - servoUnlockTime)) / 1000;
        currentMessage = "UNLOCKED " + String(remainingUnlockTime + 1) + "s";

        if (lastDisplayMessage != currentMessage)
        {
            displayMessage("Access Granted", currentMessage);
            lastDisplayMessage = currentMessage;
        }
    }
    else if (inputBuffer.length() > 0)
    {
        if (inputBuffer.startsWith("*"))
        {
            int enteredDigits = inputBuffer.length() - 1;
            if (enteredDigits <= 6)
            {
                String stars = "";
                for (int i = 0; i < enteredDigits; i++)
                {
                    stars += "*";
                }
                currentMessage = "Enter: " + stars;

                if (lastDisplayMessage != currentMessage)
                {
                    displayMessage("Password Entry", currentMessage);
                    lastDisplayMessage = currentMessage;
                }
            }
        }
    }
    // IDLE 상태에서는 항상 Ready 메시지를 표시 (키 입력이 없을 때만)
    else if (currentMode == IDLE && inputBuffer.length() == 0)
    {
        if (lastDisplayMessage != "Ready")
        {
            displayMessage("Security Lock", "Ready");
            lastDisplayMessage = "Ready";
        }
    }
}

void handleIdleMode()
{
    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();

        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            unsigned long currentTime = millis();

            if (currentTime - lastKeyPressTime < DEBOUNCE_DELAY)
            {
                continue;
            }

            lastKeyPressTime = currentTime;
            char key = (char)e.bit.KEY;
            playShortBeep(); // 키 입력 시 짧은 비프음

            // 입력 타임아웃 검사
            if (inputBuffer.length() > 0 && (currentTime - passwordEntryStartTime > PASSWORD_ENTRY_TIMEOUT))
            {
                if (inputBuffer.startsWith("*") && (inputBuffer.length() - 1) == 6)
                {
                    Serial.println();
                    Serial.println("Input timeout - processing 6 digit entry");
                    inputBuffer += '#';
                    processPasswordInput();
                }
                else
                {
                    inputBuffer = "";
                    Serial.println();
                    Serial.println("Input timeout - buffer cleared");
                    displayMessage("Timeout", "Try Again");
                    unsigned long timeoutStart = millis();
                    while (millis() - timeoutStart < 2000)
                    {
                    }
                }
            }

            if (key == '*' && inputBuffer.length() == 0)
            {
                inputBuffer = "*";
                passwordEntryStartTime = currentTime;
                Serial.println("Password entry started");
                displayMessage("Password Entry", "Enter: ");
                turnOnBacklight(); // 비밀번호 입력 시작 시 백라이트 켜기
            }
            else if (inputBuffer.startsWith("*"))
            {
                size_t dataLen = inputBuffer.length() - 1;

                if ((key >= '0' && key <= '9') && dataLen < 6)
                {
                    inputBuffer += key;
                    Serial.print("*");
                }
                else if ((key == '#' || key == '*') && dataLen == 6)
                {
                    inputBuffer += key;
                    Serial.print("*");
                    Serial.println();
                    processPasswordInput();
                }
                else
                {
                    size_t dataLen2 = inputBuffer.length() - 1;
                    if (dataLen2 == 6)
                    {
                        Serial.println();
                        Serial.println("Invalid terminator - processing anyway");
                        inputBuffer += '#';
                        processPasswordInput();
                    }
                    else
                    {
                        Serial.println();
                        Serial.println("Invalid input format");
                        inputBuffer = "";
                        displayMessage("Invalid Input", "Try Again");
                        unsigned long errorStart = millis();
                        while (millis() - errorStart < 2000)
                        {
                        }
                    }
                }
            }
            else
            {
                inputBuffer = "";
            }
        }
    }
}

void handleLockdownMode()
{
    unsigned long currentTime = millis();

    if (currentTime - lockdownStartTime >= LOCKDOWN_DURATION)
    {
        Serial.println("Security lockdown released - normal operation resumed");
        currentMode = IDLE;
        failureCount = 0;
        buzzerActive = false;
        digitalWrite(BUZZER_PIN, LOW);
        displayMessage("Lockdown End", "System Ready");
        unsigned long releaseStart = millis();
        while (millis() - releaseStart < 3000)
        {
        }
    }
}

void processPasswordInput()
{
    if (inputBuffer.length() != 8 ||
        !inputBuffer.startsWith("*") ||
        (inputBuffer.charAt(7) != '#' && inputBuffer.charAt(7) != '*'))
    {
        Serial.println("Invalid input format");
        inputBuffer = "";
        displayMessage("Invalid Format", "Try Again");
        unsigned long errorStart = millis();
        while (millis() - errorStart < 2000)
        {
        }
        return;
    }

    String password = inputBuffer.substring(1, 7);
    inputBuffer = "";

    if (password == correctPassword)
    {
        Serial.println("Correct password entered");
        failureCount = 0;
        unlockDoor();
        currentMode = IDLE;
        displayMessage("Access Granted", "Door Unlocked");
    }
    else
    {
        Serial.println("Wrong password");
        failureCount++;

        if (failureCount >= MAX_FAILURES)
        {
            displayMessage("Max Attempts", "Exceeded!");
            unsigned long errorStart = millis();
            while (millis() - errorStart < 2000)
            {
            }
            activateSecurityLockdown();
        }
        else
        {
            String attemptsMsg = "Attempts: " + String(failureCount) + "/" + String(MAX_FAILURES);
            displayMessage("Wrong Password", attemptsMsg);
            unsigned long errorStart = millis();
            while (millis() - errorStart < 3000)
            {
            }
            currentMode = IDLE;
        }
    }
}

void unlockDoor()
{
    Serial.println("Door unlocked");
    lockServo.write(UNLOCK_ANGLE);
    isUnlocked = true;
    servoUnlockTime = millis();
    playUnlockBeep(); // 잠금 해제 성공 시 비프음
}

void handleAutoLock()
{
    if (isUnlocked && (millis() - servoUnlockTime >= UNLOCK_DURATION))
    {
        Serial.println("Auto lock activated");
        lockServo.write(LOCK_ANGLE);
        isUnlocked = false;
        playLockBeep(); // 자동 잠금 시 비프음
        displayMessage("Auto Lock", "Activated");
        unsigned long lockStart = millis();
        while (millis() - lockStart < 2000)
        {
        }
    }
}

void handleManualUnlock()
{
    if (currentMode == SECURITY_LOCKDOWN)
    {
        Serial.println("Manual unlock disabled - security lockdown active");
        displayMessage("Manual Unlock", "Disabled");
        turnOnBacklight(); // 수동버튼 사용 시 백라이트 켜기
        unsigned long errorStart = millis();
        while (millis() - errorStart < 2000)
        {
        }
        return;
    }

    Serial.println("Manual unlock activated");
    displayMessage("Manual Unlock", "Activated");
    turnOnBacklight(); // 수동버튼 사용 시 백라이트 켜기
    unlockDoor();
}

void activateSecurityLockdown()
{
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

void handleBuzzer()
{
    if (!buzzerActive)
        return;

    unsigned long currentTime = millis();

    if (currentTime - buzzerStartTime < BUZZER_DURATION)
    {
        if (currentTime - lastBuzzerToggle >= BUZZER_INTERVAL)
        {
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
            lastBuzzerToggle = currentTime;
        }
    }
    else
    {
        buzzerActive = false;
        buzzerState = false;
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void turnOnBacklight()
{
    if (!backlightOn)
    {
        lcd.setBacklight(255);
        backlightOn = true;
    }
    backlightOnTime = millis();
}

void handleBacklight()
{
    // 특정 상태에서는 백라이트를 계속 켜둠
    bool keepBacklightOn = false;
    
    // 비밀번호 입력 중이거나 락다운/언락 상태에서는 백라이트 유지
    if (inputBuffer.length() > 0 ||                    // 비밀번호 입력 중
        currentMode == SECURITY_LOCKDOWN ||          // 락다운 모드
        isUnlocked)                                  // 언락 상태
    {
        keepBacklightOn = true;
    }
    
    if (keepBacklightOn)
    {
        if (!backlightOn)
        {
            lcd.setBacklight(255);
            backlightOn = true;
        }
        backlightOnTime = millis(); // 타이머 갱신
    }
    else if (backlightOn && (millis() - backlightOnTime >= BACKLIGHT_DURATION))
    {
        lcd.setBacklight(0);
        backlightOn = false;
    }
}

void playShortBeep()
{
    tone(BUZZER_PIN, 1000, 100); // 1000Hz, 100ms 짧은 비프음
}

void playUnlockBeep()
{
    tone(BUZZER_PIN, 1500, 200); // 1500Hz, 200ms 잠금 해제 비프음
}

void playLockBeep()
{
    tone(BUZZER_PIN, 800, 300); // 800Hz, 300ms 잠금 비프음
}
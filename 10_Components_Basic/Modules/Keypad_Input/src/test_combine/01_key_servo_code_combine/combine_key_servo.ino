#include <Adafruit_Keypad.h>
#include <Servo.h>

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

// 키패드 객체 생성
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 서보모터 설정
Servo lockServo;
const int SERVO_PIN = 10;
const int LOCK_ANGLE = 0;     // 잠금 각도
const int UNLOCK_ANGLE = 180; // 해제 각도

// 수동 해제 버튼
const int MANUAL_BUTTON_PIN = 11;

// 부저 설정
const int BUZZER_PIN = 12;

// 시스템 상태 열거형
enum SystemMode
{
    IDLE,             // 대기 모드
    SECURITY_LOCKDOWN // 보안 잠금 모드
};

// 전역 변수
SystemMode currentMode = IDLE;
String inputBuffer = "";
const String correctPassword = "123456"; // 정확한 비밀번호
int failureCount = 0;                    // 실패 횟수 카운터
const int MAX_FAILURES = 5;              // 최대 허용 실패 횟수

// 타이밍 관리 변수
unsigned long lockdownStartTime = 0;      // 보안 잠금 시작 시간
unsigned long buzzerStartTime = 0;        // 부저 시작 시간
unsigned long servoUnlockTime = 0;        // 서보모터 해제 시작 시간
bool isUnlocked = false;                  // 현재 해제 상태
bool buzzerActive = false;                // 부저 활성화 상태
bool buzzerState = false;                 // 부저 현재 상태
unsigned long lastButtonPressTime = 0;    // 버튼 디바운싱용 마지막 누름 시간
unsigned long lastKeyPressTime = 0;       // 키패드 디바운싱용 마지막 누름 시간
const unsigned long DEBOUNCE_DELAY = 200; // 키패드 디바운스 딜레이 (200ms)

// 비밀번호 입력 타임아웃
unsigned long passwordEntryStartTime = 0;
const unsigned long PASSWORD_ENTRY_TIMEOUT = 10000; // 10초 이내로 입력해야 함

// 시간 상수 정의 (밀리초)
const unsigned long BUZZER_DURATION = 60000;    // 1분 - 부저 동작 시간
const unsigned long LOCKDOWN_DURATION = 120000; // 2분 - 보안 잠금 지속 시간
const unsigned long UNLOCK_DURATION = 5000;     // 5초 - 해제 상태 유지 시간
const unsigned long BUZZER_INTERVAL = 1000;     // 1초 - 부저 on/off 간격

void setup()
{
    Serial.begin(9600);
    customKeypad.begin();

    // 서보모터 초기화 및 잠금 위치로 설정
    lockServo.attach(SERVO_PIN);
    lockServo.write(LOCK_ANGLE);

    // 핀 모드 설정
    pinMode(MANUAL_BUTTON_PIN, INPUT_PULLUP); // 풀업 저항 활성화
    pinMode(BUZZER_PIN, OUTPUT);

    // 시작 메시지 출력
    Serial.println("키패드 서보모터 통합 잠금장치 시작");
    Serial.println("비밀번호 형식: *123456# 또는 *123456*");
    Serial.println("수동 해제 버튼 사용 가능");
}

void loop()
{
    // 수동 해제 버튼 확인 (최우선 처리)
    if (digitalRead(MANUAL_BUTTON_PIN) == LOW && (millis() - lastButtonPressTime >= 200))
    {
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

    // 서보모터 자동 잠금 타이밍 처리
    handleAutoLock();

    // 보안 잠금 시 부저 제어
    handleBuzzer();
}

void handleIdleMode()
{
    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();

        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            unsigned long currentTime = millis();

            // 디바운스 처리: 마지막 키 입력으로부터 일정 시간 이내이면 무시
            if (currentTime - lastKeyPressTime < DEBOUNCE_DELAY)
            {
                continue; // 디바운스 기간 중이므로 무시
            }

            lastKeyPressTime = currentTime; // 마지막 입력 시간 업데이트

            char key = (char)e.bit.KEY;

            // 입력 타임아웃 검사
            if (!inputBuffer.isEmpty() && (currentTime - passwordEntryStartTime > PASSWORD_ENTRY_TIMEOUT))
            {
                // 이전 입력이 오래되어 초기화
                // If we already have 6 digits, treat this as a submission (so incorrect passwords are counted)
                if (inputBuffer.startsWith("*") && (inputBuffer.length() - 1) == 6)
                {
                    Serial.println();
                    Serial.println("입력 시간 초과 - 6자리 입력 처리 중");
                    // append a terminator and process
                    inputBuffer += '#';
                    processPasswordInput();
                }
                else
                {
                    inputBuffer = "";
                    Serial.println();
                    Serial.println("입력 시간 초과 - 입력 버퍼 초기화");
                }
            }

            if (key == '*' && inputBuffer.isEmpty())
            {
                inputBuffer = "*";
                passwordEntryStartTime = currentTime;
                Serial.println("비밀번호 입력 시작");
            }
            else if (inputBuffer.startsWith("*"))
            {
                // 허용되는 흐름: 다음 6개는 숫자(0-9)만 허용, 그 다음은 종료문자('#' 또는 '*')
                size_t dataLen = inputBuffer.length() - 1; // 이미 입력된 숫자 수

                if ((key >= '0' && key <= '9') && dataLen < 6)
                {
                    inputBuffer += key;
                    Serial.print("*");
                }
                else if ((key == '#' || key == '*') && dataLen == 6)
                {
                    inputBuffer += key; // 종료 문자 추가
                    Serial.print("*");
                    Serial.println();
                    processPasswordInput();
                }
                else
                {
                    // 잘못된 입력 패턴
                    size_t dataLen2 = inputBuffer.length() - 1;
                    if (dataLen2 == 6)
                    {
                        // If user entered 6 digits but then pressed an unexpected key,
                        // treat as submission so failureCount increments (avoids bypassing lockdown)
                        Serial.println();
                        Serial.println("Invalid terminator key - 처리로 간주");
                        inputBuffer += '#';
                        processPasswordInput();
                    }
                    else
                    {
                        // 입력 버퍼 초기화 및 알림
                        Serial.println();
                        Serial.println("Invalid input format");
                        inputBuffer = "";
                    }
                }
            }
            else
            {
                // 잘못된 입력 패턴 - 입력 버퍼 초기화
                inputBuffer = "";
            }
        }
    }
}


void handleLockdownMode()
{
    unsigned long currentTime = millis();

    // 보안 잠금 2분 대기 시간 만료 확인
    if (currentTime - lockdownStartTime >= LOCKDOWN_DURATION)
    {
        Serial.println("보안 잠금 해제 - 정상 작동 복귀");
        currentMode = IDLE;
        failureCount = 0;
        buzzerActive = false;
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void processPasswordInput()
{
    // *XXXXXX# 또는 *XXXXXX* 형식 검증
    if (inputBuffer.length() != 8 ||
        !inputBuffer.startsWith("*") ||
        (inputBuffer.charAt(7) != '#' && inputBuffer.charAt(7) != '*'))
    {
        Serial.println("Invalid input format");
        inputBuffer = "";
        return;
    }

    // 입력 문자열에서 6자리 비밀번호 추출 (2-7번째 문자)
    String password = inputBuffer.substring(1, 7);
    inputBuffer = "";

    if (password == correctPassword)
    {
        // 올바른 비밀번호 입력됨
        Serial.println("Correct password entered");
        failureCount = 0; // 실패 카운터 초기화
        unlockDoor();
        currentMode = IDLE;
    }
    else
    {
        // 잘못된 비밀번호
        Serial.println("Wrong password");
        failureCount++;

        if (failureCount >= MAX_FAILURES)
        {
            activateSecurityLockdown();
        }
        else
        {
            currentMode = IDLE; // 실패했지만 보안 잠금이 아니면 IDLE 유지
        }
    }
}

void unlockDoor()
{
    Serial.println("잠금 해제");
    lockServo.write(UNLOCK_ANGLE); // 서보모터를 해제 위치로 회전
    isUnlocked = true;
    servoUnlockTime = millis(); // 해제 시작 시간 기록
}

void handleAutoLock()
{
    // 5초 후 자동 잠금 처리
    if (isUnlocked && (millis() - servoUnlockTime >= UNLOCK_DURATION))
    {
        Serial.println("자동 잠금");
        lockServo.write(LOCK_ANGLE); // 서보모터를 잠금 위치로 회전
        isUnlocked = false;
    }
}

void handleManualUnlock()
{
    // During security lockdown manual unlocking should be disabled
    if (currentMode == SECURITY_LOCKDOWN)
    {
        Serial.println("수동 해제 불가 - 보안 잠금 활성화됨");
        return;
    }

    Serial.println("수동 해제");
    unlockDoor(); // 수동 해제도 동일한 해제 프로세스 사용
}

void activateSecurityLockdown()
{
    Serial.println("Maximum attempts exceeded - security lockdown activated");
    currentMode = SECURITY_LOCKDOWN;
    lockdownStartTime = millis();
    buzzerStartTime = millis();
    buzzerActive = true;

    // 보안 잠금 시 서보모터를 강제로 잠금 위치로 고정
    lockServo.write(LOCK_ANGLE);
    isUnlocked = false;
}

void handleBuzzer()
{
    if (!buzzerActive)
        return;

    unsigned long currentTime = millis();

    // 보안 잠금 시 1분간 경고 부저 작동
    if (currentTime - buzzerStartTime < BUZZER_DURATION)
    {
        // 1초 간격으로 부저 켜짐/꺼짐 반복
        static unsigned long lastBuzzerToggle = 0;
        if (currentTime - lastBuzzerToggle >= BUZZER_INTERVAL)
        {
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
            lastBuzzerToggle = currentTime;
        }
    }
    else
    {
        // 1분 경과 후 부저 정지
        buzzerActive = false;
        digitalWrite(BUZZER_PIN, LOW);
    }
}
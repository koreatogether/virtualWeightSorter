/**
 * @file    KeypadDoorLockSystem.ino
 * @author  Your Name
 * @brief   4x4 키패드를 이용한 서보모터 도어락 시스템
 * @version 2.0
 * @date    2025-01-26
 *
 * @details
 * - 올바른 암호(1234) 입력 시 서보모터가 0°에서 180°로 회전 (문 열림)
 * - '*' 키 입력 시 서보모터가 180°에서 0°로 회전 (문 닫힘)
 * - '#' 키로 입력 초기화
 * - 허수(decoy) 숫자를 앞뒤에 넣어도 암호 인식 가능
 *
 * @hardware
 * - Arduino Uno/Nano
 * - 4x4 Matrix Keypad
 * - SG90 Servo Motor
 * - Power Supply (5V)
 *
 * @connections
 * Keypad Rows: D9, D8, D7, D6
 * Keypad Cols: D5, D4, D3, D2
 * Servo Signal: D10
 */

#include <Adafruit_Keypad.h>
#include <Servo.h>

// ===========================
// 상수 정의 (Constants)
// ===========================
namespace Config
{
    // 하드웨어 핀 설정
    constexpr uint8_t SERVO_PIN = 10;

    // 키패드 설정
    constexpr uint8_t KEYPAD_ROWS = 4;
    constexpr uint8_t KEYPAD_COLS = 4;

    // 서보모터 각도
    constexpr uint8_t SERVO_LOCKED_ANGLE = 0;
    constexpr uint8_t SERVO_UNLOCKED_ANGLE = 180;
    constexpr uint8_t SERVO_SPEED_DELAY = 15; // ms between angle steps

    // 암호 설정
    constexpr char CORRECT_PASSWORD[] = "1234";
    constexpr uint8_t PASSWORD_LENGTH = 4;
    constexpr uint8_t MAX_INPUT_LENGTH = 20; // 허수 포함 최대 입력 길이

    // 특수 키 정의
    constexpr char KEY_ENTER = '#';
    constexpr char KEY_CLEAR = 'C';
    constexpr char KEY_LOCK = '*';
    constexpr char KEY_DELETE = 'D';

    // 시스템 설정
    constexpr uint32_t SERIAL_BAUD_RATE = 115200;
    constexpr uint16_t AUTO_LOCK_DELAY = 5000; // 5초 후 자동 잠금
    constexpr uint8_t MAX_ATTEMPTS = 3;        // 최대 시도 횟수
    constexpr uint16_t LOCKOUT_TIME = 30000;   // 30초 잠금
}

// ===========================
// 키패드 매트릭스 정의
// ===========================
char keyMatrix[Config::KEYPAD_ROWS][Config::KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// 하드웨어 연결 핀 (실제 연결에 맞게 조정)
byte rowPins[Config::KEYPAD_ROWS] = {9, 8, 7, 6};
byte colPins[Config::KEYPAD_COLS] = {5, 4, 3, 2};

// ===========================
// 전역 객체 및 변수
// ===========================
Adafruit_Keypad keypad = Adafruit_Keypad(
    makeKeymap(keyMatrix),
    rowPins, colPins,
    Config::KEYPAD_ROWS, Config::KEYPAD_COLS);

Servo doorServo;

// ===========================
// 시스템 상태 관리 클래스
// ===========================
class DoorLockSystem
{
private:
    enum class State
    {
        LOCKED,
        UNLOCKED,
        INPUT_MODE,
        LOCKOUT
    };

    State currentState;
    char inputBuffer[Config::MAX_INPUT_LENGTH + 1];
    uint8_t inputIndex;
    uint8_t failedAttempts;
    uint32_t lastActivityTime;
    uint32_t lockoutStartTime;

public:
    DoorLockSystem() : currentState(State::LOCKED),
                       inputIndex(0),
                       failedAttempts(0),
                       lastActivityTime(0),
                       lockoutStartTime(0)
    {
        clearInput();
    }

    /**
     * @brief 키 입력 처리
     * @param key 입력된 키
     */
    void processKey(char key)
    {
        // 잠금 상태 체크
        if (isInLockout())
        {
            showLockoutMessage();
            return;
        }

        lastActivityTime = millis();

        // 상태별 키 처리
        switch (currentState)
        {
        case State::LOCKED:
            handleLockedState(key);
            break;

        case State::UNLOCKED:
            handleUnlockedState(key);
            break;

        case State::INPUT_MODE:
            handleInputMode(key);
            break;

        default:
            break;
        }
    }

    /**
     * @brief 자동 잠금 체크 (일정 시간 후)
     */
    void checkAutoLock()
    {
        if (currentState == State::UNLOCKED)
        {
            if (millis() - lastActivityTime > Config::AUTO_LOCK_DELAY)
            {
                lockDoor();
                Serial.println(F("\n[AUTO] 자동 잠금 실행"));
            }
        }
    }

    /**
     * @brief 잠금 상태 체크
     */
    bool isInLockout()
    {
        if (currentState == State::LOCKOUT)
        {
            if (millis() - lockoutStartTime > Config::LOCKOUT_TIME)
            {
                currentState = State::LOCKED;
                failedAttempts = 0;
                Serial.println(F("\n[SYSTEM] 잠금 해제됨"));
                return false;
            }
            return true;
        }
        return false;
    }

private:
    /**
     * @brief LOCKED 상태에서의 키 처리
     */
    void handleLockedState(char key)
    {
        if (isDigit(key))
        {
            currentState = State::INPUT_MODE;
            addToInput(key);
            Serial.println(F("\n[INPUT] 암호 입력 시작"));
            displayMaskedInput();
        }
        else
        {
            showInstruction();
        }
    }

    /**
     * @brief UNLOCKED 상태에서의 키 처리
     */
    void handleUnlockedState(char key)
    {
        if (key == Config::KEY_LOCK)
        {
            lockDoor();
            Serial.println(F("\n[LOCK] 수동 잠금 실행"));
        }
    }

    /**
     * @brief INPUT_MODE 상태에서의 키 처리
     */
    void handleInputMode(char key)
    {
        if (isDigit(key))
        {
            if (inputIndex < Config::MAX_INPUT_LENGTH)
            {
                addToInput(key);
                displayMaskedInput();
            }
            else
            {
                Serial.println(F("\n[ERROR] 입력 길이 초과"));
            }
        }
        else if (key == Config::KEY_ENTER)
        {
            checkPassword();
        }
        else if (key == Config::KEY_CLEAR)
        {
            clearInput();
            currentState = State::LOCKED;
            Serial.println(F("\n[CLEAR] 입력 초기화"));
        }
        else if (key == Config::KEY_DELETE)
        {
            deleteLastChar();
            displayMaskedInput();
        }
    }

    /**
     * @brief 암호 확인 (허수 포함 처리)
     */
    void checkPassword()
    {
        if (containsPassword())
        {
            unlockDoor();
            failedAttempts = 0;
            Serial.println(F("\n[SUCCESS] ✓ 암호 인증 성공!"));
            printAccessLog(true);
        }
        else
        {
            failedAttempts++;
            Serial.print(F("\n[FAILED] ✗ 암호 불일치 ("));
            Serial.print(failedAttempts);
            Serial.print(F("/"));
            Serial.print(Config::MAX_ATTEMPTS);
            Serial.println(F(")"));

            if (failedAttempts >= Config::MAX_ATTEMPTS)
            {
                enterLockout();
            }

            clearInput();
            currentState = State::LOCKED;
        }
    }

    /**
     * @brief 입력된 문자열에 올바른 암호가 포함되어 있는지 확인
     * @return 암호 포함 여부
     */
    bool containsPassword()
    {
        String input = String(inputBuffer);
        return input.indexOf(Config::CORRECT_PASSWORD) >= 0;
    }

    /**
     * @brief 문 잠금 해제
     */
    void unlockDoor()
    {
        currentState = State::UNLOCKED;
        smoothServoMove(Config::SERVO_LOCKED_ANGLE, Config::SERVO_UNLOCKED_ANGLE);
        clearInput();

        Serial.println(F("\n╔════════════════════╗"));
        Serial.println(F("║   🔓 DOOR OPEN     ║"));
        Serial.println(F("╚════════════════════╝"));
    }

    /**
     * @brief 문 잠금
     */
    void lockDoor()
    {
        currentState = State::LOCKED;
        smoothServoMove(Config::SERVO_UNLOCKED_ANGLE, Config::SERVO_LOCKED_ANGLE);

        Serial.println(F("\n╔════════════════════╗"));
        Serial.println(F("║   🔒 DOOR LOCKED   ║"));
        Serial.println(F("╚════════════════════╝"));
    }

    /**
     * @brief 잠금 모드 진입
     */
    void enterLockout()
    {
        currentState = State::LOCKOUT;
        lockoutStartTime = millis();

        Serial.println(F("\n╔════════════════════════════╗"));
        Serial.println(F("║  ⚠️  SECURITY LOCKOUT      ║"));
        Serial.println(F("║  Too many failed attempts  ║"));
        Serial.println(F("║  Please wait 30 seconds    ║"));
        Serial.println(F("╚════════════════════════════╝"));
    }

    /**
     * @brief 서보모터 부드러운 이동
     */
    void smoothServoMove(int fromAngle, int toAngle)
    {
        int step = (fromAngle < toAngle) ? 1 : -1;

        for (int angle = fromAngle; angle != toAngle; angle += step)
        {
            doorServo.write(angle);
            delay(Config::SERVO_SPEED_DELAY);
        }
        doorServo.write(toAngle);
    }

    /**
     * @brief 입력 버퍼에 문자 추가
     */
    void addToInput(char c)
    {
        if (inputIndex < Config::MAX_INPUT_LENGTH)
        {
            inputBuffer[inputIndex++] = c;
            inputBuffer[inputIndex] = '\0';
        }
    }

    /**
     * @brief 마지막 입력 문자 삭제
     */
    void deleteLastChar()
    {
        if (inputIndex > 0)
        {
            inputBuffer[--inputIndex] = '\0';
            Serial.println(F("\n[DELETE] 마지막 문자 삭제"));
        }
    }

    /**
     * @brief 입력 버퍼 초기화
     */
    void clearInput()
    {
        memset(inputBuffer, 0, sizeof(inputBuffer));
        inputIndex = 0;
    }

    /**
     * @brief 마스킹된 입력 표시
     */
    void displayMaskedInput()
    {
        Serial.print(F("\n[INPUT] "));
        for (uint8_t i = 0; i < inputIndex; i++)
        {
            Serial.print(F("*"));
        }
        Serial.print(F(" ("));
        Serial.print(inputIndex);
        Serial.println(F(" digits)"));
    }

    /**
     * @brief 숫자 키 확인
     */
    bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    /**
     * @brief 사용 안내 메시지
     */
    void showInstruction()
    {
        Serial.println(F("\n[INFO] 숫자를 입력하여 암호 입력을 시작하세요"));
    }

    /**
     * @brief 잠금 상태 메시지
     */
    void showLockoutMessage()
    {
        uint32_t remainingTime = (Config::LOCKOUT_TIME - (millis() - lockoutStartTime)) / 1000;
        Serial.print(F("\n[LOCKOUT] 남은 시간: "));
        Serial.print(remainingTime);
        Serial.println(F(" 초"));
    }

    /**
     * @brief 접근 로그 출력
     */
    void printAccessLog(bool success)
    {
        Serial.print(F("\n[LOG] "));
        Serial.print(success ? F("SUCCESS") : F("FAILED"));
        Serial.print(F(" | Time: "));
        Serial.print(millis() / 1000);
        Serial.print(F("s | Input Length: "));
        Serial.println(inputIndex);
    }
};

// ===========================
// 전역 시스템 인스턴스
// ===========================
DoorLockSystem doorLock;

// ===========================
// 키 이벤트 처리 함수
// ===========================
void handleKeyEvent(const keypadEvent &e)
{
    if (e.bit.EVENT == KEY_JUST_PRESSED)
    {
        char key = (char)e.bit.KEY;

        // 키 입력 피드백
        Serial.print(F("\n[KEY] '"));
        Serial.print(key);
        Serial.print(F("' pressed at "));
        Serial.print(millis());
        Serial.println(F(" ms"));

        // 시스템에 키 전달
        doorLock.processKey(key);
    }
}

// ===========================
// 시스템 초기화
// ===========================
void setup()
{
    // 시리얼 통신 초기화
    Serial.begin(Config::SERIAL_BAUD_RATE);
    while (!Serial)
    {
        ; // Leonardo 등 일부 보드 대기
    }

    printWelcomeMessage();

    // 서보모터 초기화
    doorServo.attach(Config::SERVO_PIN);
    doorServo.write(Config::SERVO_LOCKED_ANGLE);

    // 키패드 초기화
    keypad.begin();

    Serial.println(F("\n[SYSTEM] ✓ 초기화 완료"));
    Serial.println(F("[SYSTEM] 도어락 시스템 준비됨\n"));

    printUsageInstructions();
}

// ===========================
// 메인 루프
// ===========================
void loop()
{
    // 키패드 상태 업데이트
    keypad.tick();

    // 키 이벤트 처리
    while (keypad.available())
    {
        keypadEvent e = keypad.read();
        handleKeyEvent(e);
    }

    // 자동 잠금 체크
    doorLock.checkAutoLock();

    // 잠금 상태 체크
    doorLock.isInLockout();
}

// ===========================
// 유틸리티 함수
// ===========================
void printWelcomeMessage()
{
    Serial.println(F("\n"));
    Serial.println(F("╔══════════════════════════════╗"));
    Serial.println(F("║  SMART DOOR LOCK SYSTEM V2.0 ║"));
    Serial.println(F("║  Enhanced Security Edition    ║"));
    Serial.println(F("╚══════════════════════════════╝"));
}

void printUsageInstructions()
{
    Serial.println(F("\n📌 사용 방법:"));
    Serial.println(F("├─ 암호 입력: 숫자 키 → 1234 포함 → #"));
    Serial.println(F("├─ 문 잠금: * 키"));
    Serial.println(F("├─ 입력 취소: C 키"));
    Serial.println(F("├─ 마지막 문자 삭제: D 키"));
    Serial.println(F("└─ 허수 기능: 1234 앞뒤로 임의 숫자 가능"));
    Serial.println(F("\n⚠️  보안 기능:"));
    Serial.println(F("├─ 3회 실패 시 30초 잠금"));
    Serial.println(F("└─ 5초 후 자동 잠금\n"));
}
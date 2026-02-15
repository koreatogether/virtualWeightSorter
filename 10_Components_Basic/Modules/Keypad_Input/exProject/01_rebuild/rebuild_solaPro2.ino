/**​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​*****
 * 프로젝트: 4x4 키패드 + 서보모터 보안 시스템
 * 기능:
 *   - 1234 암호 입력 시 서보모터 0° → 180° 회전
 *   - * 키 입력 시 서보모터 180° → 0° 역회전
 *   - 실시간 키 입력 상태 모니터링
 **​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​****​​*****/

#include <Adafruit_Keypad.h> // 키패드 제어 라이브러리
#include <Servo.h>           // 서보모터 제어 라이브러리

// ==================== 키패드 설정 ==================== //
const byte ROWS = 4; // 키패드 행 수
const byte COLS = 4; // 키패드 열 수

// 키패드 키 배열 (실제 하드웨어에 맞게 수정)
char keymap[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// 키패드 행/열 핀 할당 (실제 연결에 맞게 수정)
byte rowPins[ROWS] = {9, 8, 7, 6}; // D9, D8, D7, D6
byte colPins[COLS] = {5, 4, 3, 2}; // D5, D4, D3, D2

// 키패드 객체 생성
Adafruit_Keypad keypad = Adafruit_Keypad(
    makeKeymap(keymap),
    rowPins, colPins,
    ROWS, COLS);

// ==================== 서보모터 설정 ==================== //
Servo securityServo;        // 서보모터 객체
const int SERVO_PIN = 10;   // 서보모터 연결 핀
const int OPEN_ANGLE = 180; // 개방 각도
const int CLOSED_ANGLE = 0; // 폐쇄 각도
const long MOVE_DELAY = 15; // 서보 이동 간격 (ms)

// ==================== 시스템 변수 ==================== //
String inputPassword = "";         // 입력 중인 암호
String correctPassword = "1234";   // 올바른 암호
unsigned long inputTimeout = 3000; // 입력 타임아웃 (ms)
unsigned long lastInputTime = 0;   // 마지막 입력 시간

// ==================== 설정 함수 ==================== //
void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10); // 시리얼 포트 대기

    keypad.begin();                    // 키패드 초기화
    securityServo.attach(SERVO_PIN);   // 서보모터 연결
    securityServo.write(CLOSED_ANGLE); // 초기 위치 설정

    Serial.println("[시스템] 보안 시스템 초기화 완료");
    Serial.print("[설정] 올바른 암호: ");
    Serial.println(correctPassword);
    Serial.println("[안내] 3초 내 암호 입력 필요");
}

// ==================== 키 이벤트 처리 함수 ==================== //
void handleKeyEvent(const keypadEvent &e)
{
    char key = (char)e.bit.KEY;
    lastInputTime = millis(); // 입력 시간 갱신

    switch (e.bit.EVENT)
    {
    case KEY_JUST_PRESSED:
        if (isDigit(key) || key == '*')
        {
            inputPassword += key;
            Serial.print("[입력] 현재 암호: ");
            Serial.println(inputPassword);

            if (key == '*')
            {
                if (inputPassword == "*")
                {
                    reverseServo();
                    inputPassword = ""; // 입력 초기화
                }
            }
            else if (inputPassword.length() == correctPassword.length())
            {
                if (inputPassword == correctPassword)
                {
                    openServo();
                }
                else
                {
                    Serial.println("[오류] 잘못된 암호");
                    inputPassword = ""; // 입력 초기화
                }
            }
        }
        break;

    case KEY_JUST_RELEASED:
        // 키 떼기 이벤트 무시
        break;

    default:
        Serial.print("[경고] 알 수 없는 이벤트: ");
        Serial.println(e.bit.EVENT);
    }
}

// ==================== 서보 제어 함수 ==================== //
void openServo()
{
    Serial.println("[작동] 서보모터 개방 시작");
    for (int angle = CLOSED_ANGLE; angle <= OPEN_ANGLE; angle++)
    {
        securityServo.write(angle);
        delay(MOVE_DELAY);
    }
    inputPassword = ""; // 입력 초기화
}

void reverseServo()
{
    Serial.println("[작동] 서보모터 폐쇄 시작");
    for (int angle = OPEN_ANGLE; angle >= CLOSED_ANGLE; angle--)
    {
        securityServo.write(angle);
        delay(MOVE_DELAY);
    }
    inputPassword = ""; // 입력 초기화
}

// ==================== 메인 루프 ==================== //
void loop()
{
    keypad.tick(); // 키패드 상태 업데이트

    // 입력 타임아웃 체크
    if (millis() - lastInputTime > inputTimeout && inputPassword.length() > 0)
    {
        Serial.println("[타임아웃] 입력 초기화");
        inputPassword = "";
    }

    // 키 이벤트 처리
    while (keypad.available())
    {
        keypadEvent event = keypad.read();
        handleKeyEvent(event);
    }
}

// ==================== 헬퍼 함수 ==================== //
bool isDigit(char c)
{
    return c == '0' || c == '1' || c == '2' ||
           c == '3' || c == '4' || c == '5' ||
           c == '6' || c == '7' || c == '8' || c == '9';
}
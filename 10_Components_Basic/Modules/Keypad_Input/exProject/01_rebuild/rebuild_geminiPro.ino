#include <Adafruit_Keypad.h>
#include <Servo.h>

// --- 하드웨어 설정: 핀 번호 ---
// 키패드 행(ROW)과 열(COLUMN)에 연결된 아두이노 핀 번호를 정의합니다.
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {9, 8, 7, 6};
byte colPins[KEYPAD_COLS] = {5, 4, 3, 2};

// 서보모터가 연결된 PWM 핀 번호를 정의합니다.
const int SERVO_PIN = 10;

// --- 상수 및 설정값 ---
// 올바른 비밀번호를 설정합니다.
const String CORRECT_PASSWORD = "1234";
// 입력된 비밀번호를 저장할 변수입니다.
String enteredPassword = "";

// --- 객체 생성 ---
// 키패드 레이아웃을 정의합니다.
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Adafruit_Keypad 라이브러리를 사용하여 키패드 객체를 생성합니다.
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
// 서보모터 객체를 생성합니다.
Servo myServo;

// --- 초기 설정 함수 ---
void setup()
{
    // 시리얼 통신을 115200 속도로 시작합니다. (디버깅 및 상태 확인용)
    Serial.begin(115200);
    Serial.println("키패드와 서보모터 제어 시스템 시작");

    // 키패드를 초기화합니다.
    customKeypad.begin();

    // 서보모터를 지정된 핀에 연결하고 초기 위치(0도)로 설정합니다.
    myServo.attach(SERVO_PIN);
    myServo.write(0);
    Serial.println("서보모터가 0도로 설정되었습니다.");
}

// --- 메인 루프 함수 ---
void loop()
{
    // 키패드의 상태를 지속적으로 확인합니다.
    customKeypad.tick();

    // 처리할 키패드 이벤트가 있는지 확인합니다.
    while (customKeypad.available())
    {
        // 가장 오래된 이벤트를 큐에서 가져옵니다.
        keypadEvent e = customKeypad.read();

        // 키가 '눌렸을 때'만 특정 동작을 수행하도록 처리합니다.
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            char pressedKey = (char)e.bit.KEY;
            Serial.print("눌린 키: ");
            Serial.println(pressedKey);

            // 입력된 키에 따라 동작을 결정합니다.
            handleKeyPress(pressedKey);
        }
    }
}

// --- 사용자 정의 함수 ---

/**
 * @brief 눌린 키에 따라 적절한 동작을 수행하는 함수
 * @param key 사용자가 누른 키 문자
 */
void handleKeyPress(char key)
{
    // '*' 키가 눌리면 서보모터를 닫습니다(0도로 이동).
    if (key == '*')
    {
        Serial.println("'닫힘' 명령 수신");
        closeServo();
        enteredPassword = ""; // 비밀번호 입력을 초기화합니다.
    }
    // '#' 키가 눌리면 현재까지 입력된 비밀번호를 초기화합니다.
    else if (key == '#')
    {
        Serial.println("입력 초기화");
        enteredPassword = "";
    }
    // 그 외 숫자 또는 문자 키가 눌리면 비밀번호에 추가합니다.
    else
    {
        enteredPassword += key;
        Serial.print("현재 입력된 비밀번호: ");
        Serial.println(enteredPassword);

        // 입력된 비밀번호가 설정된 비밀번호와 일치하는지 확인합니다.
        if (enteredPassword == CORRECT_PASSWORD)
        {
            Serial.println("비밀번호 일치!");
            openServo();
            enteredPassword = ""; // 성공 후 비밀번호 초기화
        }
        // 만약 입력된 비밀번호 길이가 올바른 비밀번호 길이보다 길어지면,
        // 잘못된 입력으로 간주하고 초기화합니다.
        else if (enteredPassword.length() >= CORRECT_PASSWORD.length())
        {
            Serial.println("비밀번호 불일치. 다시 시도하세요.");
            enteredPassword = ""; // 실패 후 비밀번호 초기화
        }
    }
}

/**
 * @brief 서보모터를 180도로 회전시키는 함수 (문 열림)
 */
void openServo()
{
    myServo.write(180);
    Serial.println("서보모터가 180도로 이동했습니다. (열림)");
}

/**
 * @brief 서보모터를 0도로 회전시키는 함수 (문 닫힘)
 */
void closeServo()
{
    myServo.write(0);
    Serial.println("서보모터가 0도로 이동했습니다. (닫힘)");
}
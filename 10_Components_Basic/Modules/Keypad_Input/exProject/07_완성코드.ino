#include <Adafruit_Keypad.h>
#include <Servo.h>

// 키패드 설정
const byte KEYPAD_ROWS = 4; // 행 개수
const byte KEYPAD_COLS = 4; // 열 개수

// 키패드 배열 정의
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// 행과 열 핀 정의
byte rowPins[KEYPAD_ROWS] = {9, 8, 7, 6}; // 행1, 행2, 행3, 행4 연결 핀
byte colPins[KEYPAD_COLS] = {5, 4, 3, 2}; // 열1, 열2, 열3, 열4 연결 핀

// 키패드 객체 생성
Adafruit_Keypad customKeypad = Adafruit_Keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    KEYPAD_ROWS,
    KEYPAD_COLS);

// 서보모터 객체 생성
Servo myServo;

// 부저 핀 설정
const int BUZZER_PIN = 11;
const int BUZZER_FREQUENCY = 1000;
const int BUZZER_DURATION = 1000;

// 서보모터 핀 설정
const int SERVO_PIN = 10;
const int SERVO_OPEN_ANGLE = 180;
const int SERVO_CLOSE_ANGLE = 0;
const int AUTO_LOCK_DELAY = 5000;

// 올바른 암호
const String correctPassword = "12345";
String inputPassword = "";

// 타이머 변수
unsigned long previousMillis = 0;
const long interval = 5000;
bool servoOpen = false;

void setup()
{
    Serial.begin(9600);
    Serial.println("Adafruit 4x4 키패드 테스트");

    customKeypad.begin();             // 키패드 초기화
    myServo.attach(SERVO_PIN);        // 서보모터 핀 설정
    myServo.write(SERVO_CLOSE_ANGLE); // 서보모터 초기 위치 설정
    pinMode(BUZZER_PIN, OUTPUT);      // 부저 핀 설정
}

void loop()
{
    // 키패드 이벤트 업데이트
    customKeypad.tick();

    // 키패드의 모든 키 확인
    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();
        char key = (char)e.bit.KEY; // 역활 : 키패드에서 입력된 키값을 저장

        // 키 눌림/뗌 상태 확인
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            handleKeyPress(key);
        }
    }

    // 서보모터 자동 잠금 처리
    unsigned long currentMillis = millis();
    if (servoOpen && currentMillis - previousMillis >= interval)
    {
        myServo.write(SERVO_CLOSE_ANGLE); // 서보모터 0도로 복귀
        servoOpen = false;
    }
}

void handleKeyPress(char key)
{
    Serial.print(key);
    Serial.println(" 키가 눌렸습니다");

    // 입력된 키가 숫자일 경우 암호에 추가
    if (key >= '0' && key <= '9')
    {
        inputPassword += key;
    }

    // 입력된 암호가 올바른지 확인
    if (inputPassword.length() == correctPassword.length())
    {
        checkPassword();
    }

    // A 키가 눌리면 서보모터를 0도로 복귀
    if (key == 'A')
    {
        resetServo();
    }
}

void checkPassword()
{
    if (inputPassword == correctPassword)
    {
        Serial.println("암호가 입력되었습니다");
        myServo.write(SERVO_OPEN_ANGLE); // 서보모터 180도로 이동
        previousMillis = millis();       // 타이머 초기화
        servoOpen = true;
    }
    else
    {
        Serial.println("잘못된 암호입니다");
        tone(BUZZER_PIN, BUZZER_FREQUENCY, BUZZER_DURATION);
        // 암호 초기화
    }
    inputPassword = "";
}

void resetServo()
{
    Serial.println("서보모터가 180도로 복귀합니다(수동 열림)");
    myServo.write(SERVO_OPEN_ANGLE); // 서보모터 180도로 이동
    previousMillis = millis(); // 타이머 초기화
    servoOpen = true;
}
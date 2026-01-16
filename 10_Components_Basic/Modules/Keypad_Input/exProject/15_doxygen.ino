// 09_doxygen.ino
/**
 * @file 09_doxygen.ino
 * @brief 08_중복코드.ino을 기반으로 Doxygen 처리된 스케치파일
 *
 * 이 스케치는 키패드로 서보모터 잠금 시스템을 제어합니다.
 * 키패드를 통해 암호 입력 및 서보모터 제어 기능을 제공합니다.
 * 도큐멘테이션을 위한 Doxygen 주석이 포함되어 있습니다.
 */

#include <Adafruit_Keypad.h>
#include <Servo.h>

/**
 * @brief 키패드의 행 개수
 */
const byte KEYPAD_ROWS = 4;

/**
 * @brief 키패드의 열 개수
 */
const byte KEYPAD_COLS = 4;

/**
 * @brief 4x4 키패드의 키 배열
 */
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/**
 * @brief 키패드 행에 대한 핀 설정
 */
byte rowPins[KEYPAD_ROWS] = {9, 8, 7, 6};

/**
 * @brief 키패드 열에 대한 핀 설정
 */
byte colPins[KEYPAD_COLS] = {5, 4, 3, 2};

/**
 * @brief Adafruit_Keypad 라이브러리를 사용하여 키패드 객체 초기화
 */
Adafruit_Keypad customKeypad = Adafruit_Keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    KEYPAD_ROWS,
    KEYPAD_COLS
);

/**
 * @brief 서보 객체
 */
Servo myServo;

/**
 * @brief 부저 핀 설정
 */
const int BUZZER_PIN = 11;

/**
 * @brief 부저 주파수 (Hz)
 */
const int BUZZER_FREQUENCY = 1000;

/**
 * @brief 부저 지속 시간 (밀리초)
 */
const int BUZZER_DURATION = 1000;

/**
 * @brief 서보핀 설정
 */
const int SERVO_PIN = 10;

/**
 * @brief 서보모터 개방 각도
 */
const int SERVO_OPEN_ANGLE = 180;

/**
 * @brief 서보모터 잠김 각도
 */
const int SERVO_CLOSE_ANGLE = 0;

/**
 * @brief 자동 잠금 시간 간격 (밀리초)
 */
const long AUTO_LOCK_INTERVAL = 5000;

/**
 * @brief 서보모터 잠금 해제에 사용되는 올바른 암호
 */
const String correctPassword = "12345";

/**
 * @brief 입력된 암호를 저장하는 변수
 */
String inputPassword = "";

/**
 * @brief 서보 타이머를 위한 타임스탬프
 */
unsigned long previousMillis = 0;

/**
 * @brief 서보모터가 개방되었는지 여부를 나타내는 플래그
 */
bool servoOpen = false;

/**
 * @brief Arduino 초기화 함수
 * 
 * Serial, 키패드, 서보모터 및 부저를 초기화합니다.
 */
void setup()
{
    Serial.begin(9600);
    Serial.println("Adafruit 4x4 키패드 테스트");

    customKeypad.begin();
    myServo.attach(SERVO_PIN);
    myServo.write(SERVO_CLOSE_ANGLE);
    pinMode(BUZZER_PIN, OUTPUT);
}

/**
 * @brief Arduino 메인 루프 함수
 * 
 * 키패드 이벤트를 처리하고 시간 간격을 기반으로 자동 잠금을 확인합니다.
 */
void loop()
{
    customKeypad.tick();
    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();
        char key = (char)e.bit.KEY; 
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            handleKeyPress(key);
        }
    }

    // 타임 인터벌에 따른 자동 잠금 처리
    unsigned long currentMillis = millis();
    if (servoOpen && currentMillis - previousMillis >= AUTO_LOCK_INTERVAL)
    {
        myServo.write(SERVO_CLOSE_ANGLE);
        servoOpen = false;
    }
}

/**
 * @brief 키패드에서 키 입력 이벤트를 처리합니다.
 * 
 * @param key 눌린 키의 문자
 */
void handleKeyPress(char key)
{
    Serial.print(key);
    Serial.println(" 키가 눌렸습니다");

    // 키가 숫자인 경우 입력 암호에 추가
    if (key >= '0' && key <= '9')
    {
        inputPassword += key;
    }

    // 암호 길이가 올바르면 검증 수행
    if (inputPassword.length() == correctPassword.length())
    {
        checkPassword();
    }

    // 'A' 키 입력 시 수동 서보모터 개방
    if (key == 'A')
    {
        manualOpen();
    }
}

/**
 * @brief 서보모터를 개방하고 타이머를 업데이트합니다.
 * 
 * @param msg 서보모터 개방 시 출력할 메시지
 */
void openServo(const String &msg)
{
    Serial.println(msg);
    myServo.write(SERVO_OPEN_ANGLE);
    previousMillis = millis();
    servoOpen = true;
}

/**
 * @brief 입력된 암호가 올바른 암호와 일치하는지 확인합니다.
 */
void checkPassword()
{
    if (inputPassword == correctPassword)
    {
        openServo("암호가 입력되었습니다");
    }
    else
    {
        Serial.println("잘못된 암호입니다");
        tone(BUZZER_PIN, BUZZER_FREQUENCY, BUZZER_DURATION);
    }
    inputPassword = "";
}

/**
 * @brief A 키를 눌렀을 때 서보모터를 수동으로 개방합니다.
 */
void manualOpen()
{
    openServo("서보모터가 180도로 복귀합니다(수동 열림)");
}

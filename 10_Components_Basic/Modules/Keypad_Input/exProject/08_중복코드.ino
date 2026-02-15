/*
유튜브 시작 링크:https://youtube.com/shorts/pRJszHt6bEI?si=0kyNn2X_lrHkvKaf

4x4 키패드로 1234암호를 입력하면 서보모터는 0 에서 180도로 움직이고
다시 *키를 누르면 180도에서 0도로 역회전한다.

*/

/*
아두이노 코딩을 위한 계획은 다음과 같습니다:

1. Adafruit 4x4 키패드 라이브러리를 사용하여 키패드를 초기화합니다.
2. Servo 라이브러리를 사용하여 서보모터를 초기화합니다.
3. 키패드에서 키를 누르면 해당 키를 시리얼 모니터에 출력합니다.
4. 키패드에서 숫자 키를 누르면 암호에 추가합니다.
5. 암호가 입력되면 올바른 암호인지 확인합니다.
6. 올바른 암호가 입력되면 서보모터를 180도로 회전합니다.
7. 서보모터가 180도로 회전한 후 5초가 지나면 0도로 회전합니다.
8. A 키를 누르면 서보모터를 180도로 수동으로 회전합니다.
9. 코드를 리팩토링하여 중복 코드를 제거합니다.
10. 코드를 최적화하여 매직 넘버를 제거합니다.
11. 코드를 최종적으로 완성합니다.


*/

/*
코딩 규칙
아두이노 코딩 가이드에 따라서
상수는 대문자로 쓰고 언더스코어(_)를 사용한다.
매직 넘버는 사용하지 않는다.
하드코딩을 최대한 피한다.
최대한 연관성 있는 코드끼리 모은다.
시리얼 디버깅은 디버깅 헬퍼 함수를 쓴다.
*/
/*
가상 시뮬레이터 링크:
1차 , 2차
https://wokwi.com/projects/423724383019625473

3차 서보모터 추가
https://wokwi.com/projects/423726481160561665

4차 수동복귀 추가
https://wokwi.com/projects/423726789660033025

5차 상용절차따라 추가
https://wokwi.com/projects/423728062302135297

6차 매직넘버제거 추가
https://wokwi.com/projects/423728232002641921

7차 완성코드
https://wokwi.com/projects/423728879404001281

8차 리팩토링완성코드
https://wokwi.com/projects/423730314944579585


*/

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
byte rowPins[KEYPAD_ROWS] = {9, 8, 7, 6};
byte colPins[KEYPAD_COLS] = {5, 4, 3, 2};

// 키패드 객체 생성
Adafruit_Keypad customKeypad = Adafruit_Keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    KEYPAD_ROWS,
    KEYPAD_COLS);

// 서보모터 객체 생성
Servo myServo;

// 부저 핀 및 관련 상수 설정
const int BUZZER_PIN = 11;
const int BUZZER_FREQUENCY = 1000;
const int BUZZER_DURATION = 1000;

// 서보모터 핀 및 각도 설정
const int SERVO_PIN = 10;
const int SERVO_OPEN_ANGLE = 180;
const int SERVO_CLOSE_ANGLE = 0;

// 자동 잠금 시간 (밀리초)
const long interval = 5000;

// 올바른 암호
const String correctPassword = "12345";
String inputPassword = "";

// 타이머 변수 및 상태 변수
unsigned long previousMillis = 0;
bool servoOpen = false;

void setup()
{
    Serial.begin(9600);
    Serial.println("Adafruit 4x4 키패드 테스트");

    customKeypad.begin();
    myServo.attach(SERVO_PIN);
    myServo.write(SERVO_CLOSE_ANGLE);
    pinMode(BUZZER_PIN, OUTPUT);
}

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

    // 숫자 키 입력 시 암호에 추가
    if (key >= '0' && key <= '9')
    {
        inputPassword += key;
    }

    // 암호 길이가 맞으면 검증
    if (inputPassword.length() == correctPassword.length())
    {
        checkPassword();
    }

    // A 키 입력 시 수동 서보모터 개방
    if (key == 'A')
    {
        manualOpen();
    }
}

// 중복 코드를 제거하기 위한 공통 함수
void openServo(const String &msg)
{
    Serial.println(msg);
    myServo.write(SERVO_OPEN_ANGLE);
    previousMillis = millis();
    servoOpen = true;
}

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

void manualOpen()
{
    openServo("서보모터가 180도로 복귀합니다(수동 열림)");
}

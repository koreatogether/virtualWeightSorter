/*
유튜브 시작 링크:https://youtube.com/shorts/pRJszHt6bEI?si=0kyNn2X_lrHkvKaf

4x4 키패드로 1234암호를 입력하면 서보모터는 0 에서 180도로 움직이고
다시 *키를 누르면 180도에서 0도로 역회전한다.

서보까지 완성 된 후
약간의 변화를 주자면 허수 넣는 것까지 시도 해볼 것
*/

/*
아두이노 코딩을 위한 계획은 다음과 같습니다:

필요한 라이브러리 포함:
adafruit Keypad 라이브러리: 4x4 키패드를 사용하기 위해 필요합니다.
Servo 라이브러리: 서보모터를 제어하기 위해 필요합니다.
변수 및 객체 선언:

키패드 배열 및 키맵핑 설정
Keypad 객체 생성
Servo 객체 생성
입력된 암호를 저장할 변수 선언
설정 함수 (setup) 작성:

서보모터 초기 위치 설정 (0도)
시리얼 통신 시작 (디버깅용)
메인 루프 함수 (loop) 작성:

키패드 입력을 읽어 암호 입력 처리
올바른 암호가 입력되면 서보모터를 0도에서 180도로 이동
* 키가 입력되면 서보모터를 180도에서 0도로 이동
암호 확인 함수 작성:

입력된 암호가 올바른지 확인하는 함수 작성
이 계획을 바탕으로 코드를 작성하면 됩니다.
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

잠금 순서에 대한 고민을 해야함

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

// 올바른 암호
const String correctPassword = "12345";
String inputPassword = "";

void setup()
{
    Serial.begin(9600);
    Serial.println("Adafruit 4x4 키패드 테스트");

    customKeypad.begin();        // 키패드 초기화
    myServo.attach(10);          // 서보모터 핀 설정
    myServo.write(0);            // 서보모터 초기 위치 설정
    pinMode(BUZZER_PIN, OUTPUT); // 부저 핀 설정
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
        myServo.write(180); // 서보모터 180도로 이동
        delay(5000);        // 5초 후 자동 잠금
        myServo.write(0);   // 서보모터 0도로 복귀
    }
    else
    {
        Serial.println("잘못된 암호입니다");
        tone(BUZZER_PIN, 1000, 1000);
        // 암호 초기화
    }
    inputPassword = "";
}

void resetServo()
{
    Serial.println("서보모터가 0도로 복귀합니다");
    myServo.write(0); // 서보모터 0도로 복귀
}
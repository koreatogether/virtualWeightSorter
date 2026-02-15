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

// 행과 열 핀 정의 (하드웨어에 맞게 조정하세요)
byte rowPins[KEYPAD_ROWS] = {9, 8, 7, 6}; // 행1, 행2, 행3, 행4 연결 핀
byte colPins[KEYPAD_COLS] = {5, 4, 3, 2}; // 열1, 열2, 열3, 열4 연결 핀

// 키패드 객체 생성
Adafruit_Keypad customKeypad = Adafruit_Keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    KEYPAD_ROWS,
    KEYPAD_COLS);

void setup()
{
    // 시리얼 속도는 개발용으로 높게 설정해도 됩니다
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    } // 일부 보드에서 시리얼이 준비될 때까지 대기
    Serial.println("Adafruit 4x4 키패드 테스트 시작");

    customKeypad.begin(); // 키패드 초기화
}
// 도움 함수: 키 이벤트를 깔끔하게 출력
void handleKeyEvent(const keypadEvent &e)
{
    char key = (char)e.bit.KEY;

    // 타임스탬프를 붙여서 로그를 보기 쉽게 함
    Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] ");

    switch (e.bit.EVENT)
    {
    case KEY_JUST_PRESSED:
        Serial.print("키 ");
        Serial.print(key);
        Serial.println(" 가 눌렸습니다");
        break;
    case KEY_JUST_RELEASED:
        Serial.print("키 ");
        Serial.print(key);
        Serial.println(" 가 떼졌습니다");
        break;
    case KEY_HELD:
        Serial.print("키 ");
        Serial.print(key);
        Serial.println(" 가 눌린 상태(held)");
        break;
    default:
        Serial.print("알 수 없는 이벤트: ");
        Serial.println(e.bit.EVENT);
        break;
    }
}

void loop()
{
    // 키패드 상태 업데이트 (아두이노의 메인 루프에서 주기적으로 호출)
    customKeypad.tick();

    // 입력 큐에 대기중인 이벤트가 있으면 처리
    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();
        handleKeyEvent(e);
    }
}
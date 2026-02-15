#include <Adafruit_Keypad.h>

// r1, c1 핀만 테스트하기 위한 간단한 키패드 설정
const byte ROWS = 1;
const byte COLS = 1;
char keys[ROWS][COLS] = {
    {'1'} // r1, c1에 해당하는 '1' 키만 테스트
};
byte rowPins[ROWS] = {9}; // r1 핀만 사용
byte colPins[COLS] = {5}; // c1 핀만 사용

// 키패드 객체 생성
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup()
{
    Serial.begin(9600);
    customKeypad.begin();

    Serial.println("빠른 r1, c1 핀 테스트 시작");
    Serial.println("r1 핀(9번)과 c1 핀(5번)만 연결하세요");
    Serial.println("'1' 키를 눌러서 핀 연결을 확인하세요");
    Serial.println("결과: '1' 키가 눌리면 r1, c1 핀이 올바르게 연결된 것입니다");
}

void loop()
{
    customKeypad.tick();

    while (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();

        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            char key = (char)e.bit.KEY;

            if (key == '1')
            {
                Serial.println("성공: r1, c1 핀이 올바르게 연결되었습니다!");
                Serial.println("'1' 키가 눌렸습니다 (행 0, 열 0)");
            }
            else
            {
                Serial.print("알 수 없는 키: ");
                Serial.println(key);
            }
        }
    }
}

#include <Adafruit_Keypad.h>

// 4x4 키패드 전체 테스트를 위한 설정
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {9, 8, 7, 6}; // r1=9, r2=8, r3=7, r4=6
byte colPins[COLS] = {5, 4, 3, 2}; // c1=5, c2=4, c3=3, c4=2

// 키패드 객체 생성
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 디바운스 처리를 위한 변수
unsigned long lastKeyPressTime = 0;
const unsigned long DEBOUNCE_DELAY = 200; // 200ms 디바운스 딜레이

// 키 값으로부터 행과 열을 찾는 함수
void findRowCol(char key, int &row, int &col)
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (keys[r][c] == key)
            {
                row = r;
                col = c;
                return;
            }
        }
    }
    row = -1;
    col = -1;
}

void setup()
{
    Serial.begin(9600);
    customKeypad.begin();

    Serial.println("4x4 키패드 전체 테스트 시작");
    Serial.println("키패드의 모든 버튼을 눌러서 테스트하세요");
    Serial.println("키패드 레이아웃:");
    Serial.println("1 2 3 A");
    Serial.println("4 5 6 B");
    Serial.println("7 8 9 C");
    Serial.println("* 0 # D");
    Serial.println("결과: 눌린 키와 행/열 정보가 표시됩니다");
}

void loop()
{
    customKeypad.tick();

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
            int row, col;
            findRowCol(key, row, col);

            Serial.print("키 눌림: ");
            Serial.print(key);
            Serial.print(" (행 ");
            Serial.print(row);
            Serial.print(", 열 ");
            Serial.print(col);
            Serial.println(")");

            // 특별한 키에 대한 추가 정보
            if (key == '1' && row == 0 && col == 0)
            {
                Serial.println("  -> r1, c1 핀에 해당하는 '1' 키입니다!");
            }

            // 키 종류 판별
            if (key >= '0' && key <= '9')
            {
                Serial.println("  -> 숫자 키");
            }
            else if (key == '*' || key == '#')
            {
                Serial.println("  -> 특수 키");
            }
            else if (key >= 'A' && key <= 'D')
            {
                Serial.println("  -> 문자 키");
            }
        }
    }
}

## #togetherF, 코드 가독성부터 깔끔하게 다듬고, 목표 동작(비번·서보 제어) 까지 한 번에 완성해볼게 !

    아래는 네가 적어준 계획을 그대로 반영하면서도 “가독성 최우선” 기준으로 구조,
    네이밍, 로그, 에러 처리(디바운스 / 입력 관리) 까지 챙긴 전체 코드야.포인트는 다음이야 : -상수 / 핀 / 키맵 / 상태를 위로 모으고, 의미 있는 이름으로 통일 - 입력 버퍼를 명확히 관리하고, 명령 키(예 : ‘*’) 는 분리 처리 - 서보 동작을 함수로 캡슐화(열림 / 닫힘, 스윕 속도 조절) - 시리얼 로그를 일관 포맷으로 출력 - 나중에 “허수(가짜 입력) 주입 테스트”를 쉽게 끼워 넣을 수 있게 입력 처리 레이어 분리

                                                                                                                                                                                               ## #완성 코드(Adafruit_Keypad + Servo)

```cpp
#include <Adafruit_Keypad.h>
#include <Servo.h>

                                                                                                                                                                                               // =========================
                                                                                                                                                                                               // 하드웨어 설정
                                                                                                                                                                                               // =========================
                                                                                                                                                                                               const byte ROWS = 4;
const byte COLS = 4;

char KEYMAP[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte ROW_PINS[ROWS] = {9, 8, 7, 6}; // R1~R4
byte COL_PINS[COLS] = {5, 4, 3, 2}; // C1~C4

// 서보 핀 및 각도
const int SERVO_PIN = 10;
const int SERVO_CLOSED = 0;     // 닫힘(초기)
const int SERVO_OPENED = 180;   // 열림
const int SERVO_STEP_DELAY = 5; // 스윕 속도(ms) — 가독성/안전성 위해 천천히

// 비밀번호
const char PASSCODE[] = "1234";
const size_t PASS_LEN = sizeof(PASSCODE) - 1;

// 명령 키
const char KEY_CLOSE = '*'; // 닫기(180 -> 0)
const char KEY_CLEAR = '#'; // 입력 초기화

// =========================
// 전역 객체/상태
// =========================
Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(KEYMAP), ROW_PINS, COL_PINS, ROWS, COLS);
Servo servoMotor;

char inputBuf[8] = {0}; // 입력 버퍼(여유롭게)
size_t inputLen = 0;
bool isOpened = false; // 현재 서보 상태

// =========================
// 유틸: 로그 출력
// =========================
void logLine(const char *tag, const String &msg)
{
    Serial.print("[");
    Serial.print(millis());
    Serial.print("ms][");
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(msg);
}

// =========================
void attachServoOnce()
{
    static bool attached = false;
    if (!attached)
    {
        servoMotor.attach(SERVO_PIN);
        attached = true;
    }
}

void moveServoSmooth(int fromDeg, int toDeg)
{
    attachServoOnce();
    if (fromDeg == toDeg)
        return;

    int step = (toDeg > fromDeg) ? 1 : -1;
    for (int angle = fromDeg; angle != toDeg; angle += step)
    {
        servoMotor.write(angle);
        delay(SERVO_STEP_DELAY);
    }
    servoMotor.write(toDeg);
}

// 상태 기반 동작
void openLock()
{
    if (isOpened)
    {
        logLine("SERVO", "이미 열림 상태");
        return;
    }
    logLine("SERVO", "열림(0 -> 180) 수행");
    moveServoSmooth(SERVO_CLOSED, SERVO_OPENED);
    isOpened = true;
}

void closeLock()
{
    if (!isOpened)
    {
        logLine("SERVO", "이미 닫힘 상태");
        return;
    }
    logLine("SERVO", "닫힘(180 -> 0) 수행");
    moveServoSmooth(SERVO_OPENED, SERVO_CLOSED);
    isOpened = false;
}

// =========================
// 입력 처리
// =========================
void clearInput()
{
    memset(inputBuf, 0, sizeof(inputBuf));
    inputLen = 0;
    logLine("INPUT", "버퍼 초기화");
}

void pushDigit(char k)
{
    if (inputLen >= sizeof(inputBuf) - 1)
    {
        logLine("INPUT", "버퍼 가득참. 초기화");
        clearInput();
    }
    inputBuf[inputLen++] = k;
    inputBuf[inputLen] = '\0';

    String s = "입력: ";
    s += inputBuf;
    s += " (";
    s += inputLen;
    s += "/";
    s += PASS_LEN;
    s += ")";
    logLine("INPUT", s);
}

bool isPassMatched()
{
    return (inputLen == PASS_LEN) && (strncmp(inputBuf, PASSCODE, PASS_LEN) == 0);
}

void handleKeyChar(char k)
{
    if (k == KEY_CLOSE)
    {
        // 닫기 명령
        closeLock();
        clearInput();
        return;
    }
    if (k == KEY_CLEAR)
    {
        // 입력 리셋
        clearInput();
        return;
    }

    // 숫자/일반 키 처리
    if (isDigit(k))
    {
        pushDigit(k);

        // 길이 도달 시 판정
        if (inputLen == PASS_LEN)
        {
            if (isPassMatched())
            {
                logLine("AUTH", "비밀번호 일치");
                openLock();
            }
            else
            {
                logLine("AUTH", "비밀번호 불일치");
            }
            clearInput();
        }
    }
    else
    {
        // A/B/C/D 등은 현재 미사용 — 필요 시 기능 매핑 가능
        String s = "미사용 키 입력: ";
        s += k;
        logLine("INPUT", s);
    }
}

// 허수(가짜 입력) 주입용 훅: 필요 시 테스트 벡터를 여기서 공급
// 예: return true 시 kOut에 '1','2','3','4','*' 등 시뮬레이션 입력
bool injectFakeInput(char &kOut)
{
    // 기본: 비활성
    return false;
}

// =========================
// 라이프사이클
// =========================
void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    logLine("SYS", "시작");
    keypad.begin();

    attachServoOnce();
    servoMotor.write(SERVO_CLOSED);
    isOpened = false;
    logLine("SERVO", "초기 각도 0도(닫힘)");
}

void loop()
{
    keypad.tick();

    // 1) 가짜 입력 훅 먼저 체크(허수 투입 테스트용)
    char injected;
    if (injectFakeInput(injected))
    {
        handleKeyChar(injected);
        return;
    }

    // 2) 실제 키패드 이벤트 처리
    while (keypad.available())
    {
        keypadEvent e = keypad.read();
        char k = (char)e.bit.KEY;

        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            String s = "키 눌림: ";
            s += k;
            logLine("KEY", s);
            handleKeyChar(k);
        }
        else if (e.bit.EVENT == KEY_JUST_RELEASED)
        {
            // 필요시 릴리즈 처리
        }
        else if (e.bit.EVENT == KEY_HELD)
        {
            // 길게 누름 처리 필요하면 여기에
        }
    }
}
/*

### 왜 이 구조가 가독성 좋은가
- 상수/설정/상태/핵심동작/입력처리/라이프사이클 순으로 위→아래 흐름이 명확함
- 의미 있는 함수 이름(openLock/closeLock/handleKeyChar/clearInput)으로 의도가 바로 보임
- 로그 태그(SYS, SERVO, INPUT, AUTH, KEY)로 시리얼 모니터에서 필터링이 쉬움
- 비밀번호 판정과 명령 키를 분리하여 유지보수 편함
- 허수 입력 테스트(injectFakeInput) 훅을 따로 둬서, 실제 하드웨어 없이도 로직 검증 가능

### 하드웨어 연결 팁
- 서보 전원은 5V, GND를 충분한 전류 소스에서. 가능하면 외부 5V 전원 사용 + GND 공통
- 키패드 핀 배치는 실제 모듈 표기와 맞춰야 함(행/열 순서 뒤집히면 키맵이 틀어짐)
- SERVO_STEP_DELAY로 속도 조절 가능(값 줄이면 빠르게, 늘리면 부드럽게)

### “허수 넣는 것” 테스트하는 빠른 방법
- injectFakeInput 함수에서 간단히 시퀀스 넣기:
  - 부트 후 1,2,3,4 순차 주입 → 열림
  - 잠시 후 ‘*’ 주입 → 닫힘
- 혹은 시리얼에서 특정 키워드 수신 시 가짜 입력 주입하도록 확장해도 OK

원하면 허수 주입용 코드 스니펫도 바로 붙여줄게. 그리고 A/B/C/D 키에 다른 기능(예: 토글 잠금, 비번 변경 모드, 서보 캘리브레이션) 매핑도 쉽게 확장 가능하니까, 어떤 UX 생각 중인지 말해줘!

*/
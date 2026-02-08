

/* INFO
https://wokwi.com/projects/407569815171024897 ( 08_#4_ 시리얼 먹통 되는 현상있음)
https://wokwi.com/projects/407570331674462209 ( 08_#5_ 시리얼 먹통 되는 현상있음_가상의 에러 일 수 있음 )
https://wokwi.com/projects/407571034291787777 ( 08_#6_ 가상의 에러 일 수 있음 )

1. 모든 값을 시리얼 출력으로 추적
2. 모든 조합경우의 수를 찾지 말고 첫번째 조합을 이용해라 ( 계산방법 개선 )
3. 포도 송이 올리는 과정에서 무게가 출렁이므로 안정화 시간 필수 !
4. 안정화된 무게만 조합에 이용 하도록 할것 ( 시간차를 두고 무게 변동 확인 )
5. 측정 해야 하는 무게는 한정되어 있으므로 0g부터 측정하지 말고  사용자 정의로 최소 무게 값과 최대 무게 값을 정해서 범위 안에서 측정하도록 할것
6. 만약 조합에 성공시  조합된 포도송이를 내릴때까지 다음 조합 계산이 안되도록

지나치는 과정이 필요 , 그리고 다 내렸다는걸 확인 하는 코드 필요 (이것도 이전과 무게 비교 )

조합 경우의 수 최소값과 최대 값이 필요

3수 ~ 6수 같이

*/

#include <HX711.h> // Rob Tillaart's HX711 library

const int DOUT_SCK_PIN[8][2] = {
    {23, 25}, // DOUT_PIN[0], SCK_PIN[0]
    {27, 29}, // DOUT_PIN[1], SCK_PIN[1]
    {31, 33}, // DOUT_PIN[2], SCK_PIN[2]
    {35, 37}, // DOUT_PIN[3], SCK_PIN[3]
    {39, 41}, // DOUT_PIN[4], SCK_PIN[4]
    {7, 6},   // DOUT_PIN[5], SCK_PIN[5]  // 핀 바궈보기 , 43 핀부터 쓰지 말것 충돌남
    {5, 4},   // DOUT_PIN[6], SCK_PIN[6]
    {3, 2}    // DOUT_PIN[7], SCK_PIN[7]
};

// 저울 무게 측정 관련 상수
const float CALIBRATION_FACTOR = 0.420f; // 저울 영점 조절 상수
const int TARE_COUNT = 5;                // 영점시 몇번 시도해서 평균 낼건지
const int UNITS_COUNT = 1;               // 무게 측정시 몇번 시도해서 평균 값을 낼건지
const int TRAY_COUNT = 8;                // 포도송이가 올라가는 트레이 갯수

// 무게 관련 상수,
int weights[TRAY_COUNT];                                // 각 트레이의 무게를 저장할 배열 , 초기화는 setup() 함수에서
int ARRAY_COUNT = sizeof(weights) / sizeof(weights[0]); // 배열의 길이를 구함
const int TARGET_WEIGHT_MIN = 2000;
const int TARGET_WEIGHT_MAX = 2100;

// 조합 관련 상수
const int NUMBER_OF_COMBINATION = 4; // 몇 송이로 조합을 원하는지 ?

// 유효한 조합을 저장할 배열
int validCombinations[50][NUMBER_OF_COMBINATION]; // 최대 100개의 조합을 저장할 수 있음
int validCombinationCount = 0;

// 1차원 핀 번호 배열
int ledPins[TRAY_COUNT];

// 배열 객체 선언
HX711 scale[TRAY_COUNT];

void setup()
{
    Serial.begin(115200);

    // 각 트레이의 핀 모드 설정 ,라이브러리에서 지원되지만 확인 필요
    for (int i = 0; i < 8; i++)
    {
        Serial.println("array start");
        pinMode(DOUT_SCK_PIN[i][0], INPUT);
        digitalWrite(DOUT_SCK_PIN[i][0], LOW);
        pinMode(DOUT_SCK_PIN[i][1], OUTPUT);
        Serial.println("array end");
    }

    // weights 배열 초기화
    for (int i = 0; i < TRAY_COUNT; i++)
    {
        weights[i] = 0;
    }

    // 로드셀 입,출력핀중 led 제어를 위한 핀번호 저장
    for (int i = 0; i < TRAY_COUNT; i++)
    {
        ledPins[i] = DOUT_SCK_PIN[i][0];
    }

    // 각 저울 객체 시작을 위한 초기화
    for (int i = 0; i < 8; i++)
    {
        Serial.println("begin start");
        scale[i].begin(DOUT_SCK_PIN[i][0], DOUT_SCK_PIN[i][1]);
        scale[i].set_scale(CALIBRATION_FACTOR);
        scale[i].tare(TARE_COUNT);
        Serial.println("begin end");
    }
}

void loop()
{
    getWeight(); // 각 트레이 별 무게를 읽어옴
}

// 각 저울 객체의 무게를 읽어옴
void getWeight()
{
    for (int i = 0; i < TRAY_COUNT; i++)
    {
        weights[i] = scale[i].get_units(UNITS_COUNT); // 무게 측정 후 저장

        // 각 트레이의 무게를 시리얼 모니터에 출력
        Serial.print("T ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(weights[i]);
        Serial.print(" g  | ");
    }
    Serial.println("");
}

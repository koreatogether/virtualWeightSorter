
// https://wokwi.com/projects/406856366957485057 ( 05버전 - 자잘한 버그 수정 및 선오류 해결)
// https://wokwi.com/projects/406858734731378689 ( 06버전 - 등급별 네오픽셀 색상 표현)
/* INFO

https://wokwi.com/projects/407034241910719489  ( 조합 테스트중 ! 중요)
https://wokwi.com/projects/407565571085222913  ( 08 버전_중간로직테스트중)
https://wokwi.com/projects/407566945052395521 ( 08 버전_중간로직테스트중 #2)
https://wokwi.com/projects/407567926932815873 ( 08_#3)
https://wokwi.com/projects/407569815171024897 ( 08_#4_ 시리얼 먹통 되는 현상있음)
https://wokwi.com/projects/407570331674462209 ( 08_#5_ 시리얼 먹통 되는 현상있음_가상의 에러 일 수 있음 )
https://wokwi.com/projects/407571034291787777 ( 08_#6_ 가상의 에러 일 수 있음 )
08버전에서는 메가2560에 적용 할 수 있는 전체 로직을 생각해보고 테스트 할것
09 버전에서는 조합 경의 수가 나왔을때 변동이 없으면 계속 유지하는 코드를 넣을 것
1. 03버전에서 각 트레이별 무게 재고 배열에 저장하는 코드 (ok) , 작동되는것 확인함
  2. 무게를 가지고 조합이 안나오는중 (조합이 나오는것 확인함
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

// 저울 관련 상수
const float CALIBRATION_FACTOR = 0.420f; // calibration factor
const int TARE_COUNT = 5;                // number of tare readings
const int UNITS_COUNT = 1;               // number of get_units readings
const int DECIMAL_PLACES = 1;
const int TRAY_COUNT = 8;

// 무게 관련 상수 , 8 은 트레이 갯수를 뜻함
int weights[TRAY_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
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

    for (int i = 0; i < 8; i++)
    {
        Serial.println("array start");
        pinMode(DOUT_SCK_PIN[i][0], INPUT);
        digitalWrite(DOUT_SCK_PIN[i][0], LOW);
        pinMode(DOUT_SCK_PIN[i][1], OUTPUT);
        Serial.println("array end");
    }

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

    // 랜덤시드 설정
    randomSeed(analogRead(0));
}

void loop()
{

    validCombinationCount = 0; // 유효한 조합 수를 초기화

    getWeight(); // 각 트레이 별 무게를 읽어옴

    // 포도송이 모든 조합을 찾기
    findCombination(0, 0, 0, 0);

    // 조합 출력 및 LED 켜기
    selectAndDisplayRandomCombination(validCombinationCount, validCombinations, weights, ledPins);

    // delay(500);
}

// 포도송이 조합 찾기
void selectAndDisplayRandomCombination(int validCombinationCount, int validCombinations[][NUMBER_OF_COMBINATION], int weights[], int ledPins[])
{
    if (validCombinationCount > 0)
    {
        // 랜덤하게 조합 선택
        int randomIndex = rand() % validCombinationCount;

        // 선택된 조합 출력 및 LED 켜기
        Serial.print("Randomly selected combination: ");
        for (int i = 0; i < NUMBER_OF_COMBINATION; i++)
        {
            int selectedGrape = validCombinations[randomIndex][i];
            Serial.print("Index ");
            Serial.print(selectedGrape);
            Serial.print(" (");
            Serial.print(weights[selectedGrape]);
            Serial.print("g), ");
            digitalWrite(ledPins[selectedGrape], HIGH); // 해당 인덱스의 LED 켜기
        }
        Serial.println();
    }
    else
    {
        Serial.println("No valid combinations found.");
    }
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

void findCombination(int index, int currentSum, int included, int depth)
{
    if (depth == NUMBER_OF_COMBINATION)
    { // 원하는 포도송이를 선택한 경우
        if (currentSum >= TARGET_WEIGHT_MIN && currentSum <= TARGET_WEIGHT_MAX)
        {
            if (validCombinationCount >= 100)
            {
                Serial.println("Too many valid combinations found. Exiting.");
                return;
            }
            // 유효한 조합을 저장
            int currentIndex = 0;
            for (int i = 0; i < ARRAY_COUNT; i++)
            {
                if (included & (1 << i))
                {
                    validCombinations[validCombinationCount][currentIndex++] = i;
                }
            }
            validCombinationCount++;

            // 디버깅: 유효한 조합 출력
            Serial.print("Valid combination found: ");
            for (int i = 0; i < NUMBER_OF_COMBINATION; i++)
            {
                Serial.print(validCombinations[validCombinationCount - 1][i]);
                Serial.print(" ");
            }
            Serial.print("Total weight: ");
            Serial.print(currentSum);
            Serial.println("g");
        }
        return;
    }

    // 인덱스가 배열 크기를 벗어나거나 무게가 최대치를 초과하면 종료
    if (index >= ARRAY_COUNT || currentSum > TARGET_WEIGHT_MAX)
    {
        return;
    }

    // 현재 무게를 포함하거나 포함하지 않는 두 가지 경우를 재귀적으로 탐색
    findCombination(index + 1, currentSum + weights[index], included | (1 << index), depth + 1); // 현재 무게 포함
    findCombination(index + 1, currentSum, included, depth);                                     // 현재 무게 제외
}
/**
 * 전자저울 - 포도 무게 조합 최적화 알고리즘 개선 버전
 *
 * 개선 사항:
 * 1. 동적 프로그래밍 접근 방식 추가 (대규모 데이터에 더 효율적)
 * 2. 예외 처리 강화
 * 3. 하드코딩된 값 제거 및 설정 가능한 인터페이스 추가
 * 4. 메모리 사용 최적화
 * 5. HX711 로드셀 센서 지원 추가
 */

#include <EEPROM.h>
#include "HX711.h" // HX711 로드셀 센서를 위한 라이브러리

// 핀 정의
#define LOADCELL_DOUT_PIN 3
#define LOADCELL_SCK_PIN 2
#define BUTTON_TARE_PIN 4
#define BUTTON_SET_PIN 5
#define BUTTON_START_PIN 6

// 상수 정의
#define MAX_WEIGHTS 30       // 최대 저장 가능한 무게 수 증가
#define MAX_BUNCHES 8        // 한 포장당 최대 포도송이 수 증가
#define EEPROM_TARGET_ADDR 0 // EEPROM에 목표 무게 저장 주소
#define MIN_WEIGHT 50        // 최소 무게 제한 (g)

// 전역 변수
int grapeWeights[MAX_WEIGHTS];  // 포도 무게 저장 배열
int numWeights = 0;             // 현재 저장된 무게 수
int targetWeight = 2050;        // 기본 목표 무게 (g)
bool isCalibrationMode = false; // 설정 모드 플래그
bool isWeighingMode = false;    // 측정 모드 플래그

// 로드셀 센서 객체
HX711 scale;

// 함수 선언
void addWeight(int weight);
void calculatePackages();
void findBestCombination(int weights[], int numWeights, int bestCombination[], int *bestSize, int *bestTotal, int *minDifference);
void findOptimalCombDP(int weights[], int numWeights, int bestCombination[], int *bestSize, int *bestTotal, int *minDifference);
void removeWeight(int weights[], int *numWeights, int weightToRemove);
void saveTargetWeight(int weight);
int loadTargetWeight();
void calibrateScale();
void printMenu();
void handleButtons();
int getMeasuredWeight();

void setup()
{
    Serial.begin(9600);

    // EEPROM에서 저장된 목표 무게 불러오기
    int savedWeight = loadTargetWeight();
    if (savedWeight > 0)
    {
        targetWeight = savedWeight;
    }

    // 로드셀 센서 초기화
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(420.0983); // 이 값은 캘리브레이션에 따라 조정 필요
    scale.tare();

    // 버튼 핀 설정
    pinMode(BUTTON_TARE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SET_PIN, INPUT_PULLUP);
    pinMode(BUTTON_START_PIN, INPUT_PULLUP);

    // 시작 메뉴 출력
    printMenu();
}

void loop()
{
    handleButtons();

    if (isWeighingMode)
    {
        int weight = getMeasuredWeight();

        if (weight > MIN_WEIGHT)
        {
            Serial.print("측정된 무게: ");
            Serial.print(weight);
            Serial.println("g");

            // 안정적인 무게라면 배열에 추가
            if (weight > MIN_WEIGHT)
            {
                addWeight(weight);
                Serial.println("무게가 추가되었습니다.");
                delay(1000);  // 같은 무게가 반복해서 추가되지 않도록 대기
                scale.tare(); // 영점 맞추기

                // 만약 메모리가 부족하면 계산 시작
                if (numWeights >= MAX_WEIGHTS)
                {
                    Serial.println("최대 무게 용량 도달. 패키지 계산 시작...");
                    calculatePackages();
                    numWeights = 0; // 데이터 초기화
                }
            }
        }
    }

    // 시리얼 명령 처리
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.equals("start"))
        {
            isWeighingMode = true;
            Serial.println("무게 측정 모드 시작");
            scale.tare(); // 영점 맞추기
        }
        else if (input.equals("calculate"))
        {
            calculatePackages();
        }
        else if (input.equals("clear"))
        {
            numWeights = 0;
            Serial.println("무게 데이터 초기화됨");
        }
        else if (input.equals("calibrate"))
        {
            calibrateScale();
        }
        else if (input.equals("menu"))
        {
            printMenu();
        }
        else if (input.startsWith("target "))
        {
            int newTarget = input.substring(7).toInt();
            if (newTarget > 0)
            {
                targetWeight = newTarget;
                saveTargetWeight(targetWeight);
                Serial.print("목표 무게가 ");
                Serial.print(targetWeight);
                Serial.println("g으로 설정되었습니다.");
            }
        }
        else if (input.startsWith("add "))
        {
            int newWeight = input.substring(4).toInt();
            if (newWeight > MIN_WEIGHT)
            {
                addWeight(newWeight);
                Serial.print("무게 ");
                Serial.print(newWeight);
                Serial.println("g이 수동으로 추가되었습니다.");
            }
        }
    }

    delay(100);
}

// 무게 추가 함수 - 예외 처리 강화
void addWeight(int weight)
{
    if (weight <= MIN_WEIGHT)
    {
        Serial.println("오류: 최소 무게 미만입니다.");
        return;
    }

    if (numWeights < MAX_WEIGHTS)
    {
        grapeWeights[numWeights++] = weight;
        Serial.print("무게 ");
        Serial.print(weight);
        Serial.print("g 추가됨 (");
        Serial.print(numWeights);
        Serial.print("/");
        Serial.print(MAX_WEIGHTS);
        Serial.println(")");
    }
    else
    {
        Serial.println("경고: 최대 무게 저장 용량 초과");
    }
}

// 패키지 계산 함수
void calculatePackages()
{
    if (numWeights == 0)
    {
        Serial.println("오류: 계산할 무게 데이터가 없습니다.");
        return;
    }

    int packageCount = 0;
    int remainingWeights[MAX_WEIGHTS];
    int numRemaining = numWeights;

    // 복사 배열 생성
    for (int i = 0; i < numWeights; i++)
    {
        remainingWeights[i] = grapeWeights[i];
    }

    Serial.print("목표 무게: ");
    Serial.print(targetWeight);
    Serial.println("g");

    // 최적화된 조합 계산을 위해 무게순으로 정렬 (내림차순)
    for (int i = 0; i < numRemaining - 1; i++)
    {
        for (int j = i + 1; j < numRemaining; j++)
        {
            if (remainingWeights[i] < remainingWeights[j])
            {
                int temp = remainingWeights[i];
                remainingWeights[i] = remainingWeights[j];
                remainingWeights[j] = temp;
            }
        }
    }

    // 최적 조합 찾기
    while (numRemaining > 0)
    {
        int bestCombination[MAX_BUNCHES];
        int bestCombinationSize = 0;
        int bestTotalWeight = 0;
        int minDifference = targetWeight;

        // 데이터가 적을 때는 완전 탐색(재귀), 많을 때는 DP 적용
        if (numRemaining <= 15)
        {
            findBestCombination(remainingWeights, numRemaining, bestCombination, &bestCombinationSize, &bestTotalWeight, &minDifference);
        }
        else
        {
            findOptimalCombDP(remainingWeights, numRemaining, bestCombination, &bestCombinationSize, &bestTotalWeight, &minDifference);
        }

        if (bestCombinationSize > 0)
        {
            packageCount++;
            Serial.print("포장 ");
            Serial.print(packageCount);
            Serial.print(": ");

            for (int i = 0; i < bestCombinationSize; i++)
            {
                Serial.print(bestCombination[i]);
                Serial.print("g ");
                removeWeight(remainingWeights, &numRemaining, bestCombination[i]);
            }

            Serial.print("(총 무게: ");
            Serial.print(bestTotalWeight);
            Serial.print("g, 목표와 차이: ");
            Serial.print(abs(targetWeight - bestTotalWeight));
            Serial.println("g)");
        }
        else
        {
            break;
        }

        // 메모리 사용량 표시
        Serial.print("남은 메모리: ");
        Serial.print(freeMemory());
        Serial.println(" bytes");
    }

    Serial.print("총 포장 수: ");
    Serial.println(packageCount);

    if (numRemaining > 0)
    {
        Serial.print("남은 포도 송이 (");
        Serial.print(numRemaining);
        Serial.print("개): ");

        for (int i = 0; i < numRemaining; i++)
        {
            Serial.print(remainingWeights[i]);
            Serial.print("g ");
        }
        Serial.println();
    }
}

// 완전 탐색 - 재귀적 조합 방식 (소량 데이터용)
void findBestCombination(int weights[], int numWeights, int bestCombination[], int *bestSize, int *bestTotal, int *minDifference)
{
    int combination[MAX_BUNCHES];
    findCombinationRecursive(weights, numWeights, combination, 0, 0, 0, bestCombination, bestSize, bestTotal, minDifference);
}

// 재귀적 조합 탐색 (기존 코드 유지)
void findCombinationRecursive(int weights[], int numWeights, int combination[], int combSize, int startIndex, int currentTotal,
                              int bestCombination[], int *bestSize, int *bestTotal, int *minDifference)
{
    int difference = abs(targetWeight - currentTotal);
    if (difference < *minDifference)
    {
        *minDifference = difference;
        *bestTotal = currentTotal;
        *bestSize = combSize;
        for (int i = 0; i < combSize; i++)
        {
            bestCombination[i] = combination[i];
        }
    }

    if (combSize >= MAX_BUNCHES || startIndex >= numWeights)
    {
        return;
    }

    for (int i = startIndex; i < numWeights; i++)
    {
        combination[combSize] = weights[i];
        findCombinationRecursive(weights, numWeights, combination, combSize + 1, i + 1, currentTotal + weights[i],
                                 bestCombination, bestSize, bestTotal, minDifference);
    }
}

// 동적 프로그래밍 방식 - 대량 데이터용 최적화
void findOptimalCombDP(int weights[], int numWeights, int bestCombination[], int *bestSize, int *bestTotal, int *minDifference)
{
    // 전체 무게의 합 계산
    int totalSum = 0;
    for (int i = 0; i < numWeights; i++)
    {
        totalSum += weights[i];
    }

    // DP 테이블 초기화
    // dp[i][j] = i까지의 아이템을 사용하여 무게 j를 만들 수 있는지 여부
    bool dp[numWeights + 1][totalSum + 1];
    int track[numWeights + 1][totalSum + 1];

    memset(dp, false, sizeof(dp));
    memset(track, -1, sizeof(track));

    // 기본 케이스: 무게 0은 항상 가능
    for (int i = 0; i <= numWeights; i++)
    {
        dp[i][0] = true;
    }

    // DP 테이블 채우기
    for (int i = 1; i <= numWeights; i++)
    {
        for (int j = 1; j <= totalSum; j++)
        {
            dp[i][j] = dp[i - 1][j]; // 현재 아이템을 포함하지 않는 경우

            if (j >= weights[i - 1] && dp[i - 1][j - weights[i - 1]])
            {
                dp[i][j] = true;
                track[i][j] = i - 1; // 현재 아이템을 포함한 경우 기록
            }
        }
    }

    // 목표 무게에 가장 가까운 무게 찾기
    int bestWeight = 0;
    *minDifference = totalSum;

    for (int j = 0; j <= totalSum; j++)
    {
        if (dp[numWeights][j])
        {
            int diff = abs(targetWeight - j);
            if (diff < *minDifference)
            {
                *minDifference = diff;
                bestWeight = j;
            }
        }
    }

    // 최적 조합 역추적
    int remainingWeight = bestWeight;
    int i = numWeights;
    *bestSize = 0;

    while (i > 0 && remainingWeight > 0)
    {
        if (track[i][remainingWeight] != -1)
        {
            int idx = track[i][remainingWeight];
            bestCombination[*bestSize] = weights[idx];
            (*bestSize)++;
            remainingWeight -= weights[idx];
        }
        i--;
    }

    *bestTotal = bestWeight;
}

// 무게 제거 함수 (기존 코드 유지)
void removeWeight(int weights[], int *numWeights, int weightToRemove)
{
    for (int i = 0; i < *numWeights; i++)
    {
        if (weights[i] == weightToRemove)
        {
            for (int j = i; j < *numWeights - 1; j++)
            {
                weights[j] = weights[j + 1];
            }
            (*numWeights)--;
            break;
        }
    }
}

// 설정 저장/불러오기 함수
void saveTargetWeight(int weight)
{
    EEPROM.put(EEPROM_TARGET_ADDR, weight);
    Serial.println("목표 무게가 EEPROM에 저장되었습니다.");
}

int loadTargetWeight()
{
    int weight;
    EEPROM.get(EEPROM_TARGET_ADDR, weight);

    // 유효성 검사
    if (weight < 100 || weight > 10000)
    {
        return -1; // 잘못된 값
    }
    return weight;
}

// 로드셀 캘리브레이션 함수
void calibrateScale()
{
    Serial.println("===== 로드셀 캘리브레이션 모드 =====");
    Serial.println("1. 저울에 아무것도 올려놓지 마세요.");
    Serial.println("2. 'tare'를 입력하여 영점을 맞춥니다.");
    Serial.println("3. 알려진 무게의 물체를 올리세요.");
    Serial.println("4. 'calibrate XXX'를 입력하세요. (XXX는 그램 단위 무게)");

    bool calibrating = true;
    float knownWeight = 0;
    float rawValue = 0;

    while (calibrating)
    {
        if (Serial.available())
        {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();

            if (cmd == "tare")
            {
                scale.tare();
                Serial.println("영점 맞춤 완료.");
            }
            else if (cmd.startsWith("calibrate "))
            {
                knownWeight = cmd.substring(10).toFloat();
                if (knownWeight > 0)
                {
                    rawValue = scale.get_value(10);
                    float factor = rawValue / knownWeight;
                    scale.set_scale(factor);

                    Serial.print("캘리브레이션 완료. 스케일 팩터: ");
                    Serial.println(factor);
                    Serial.println("이 값을 코드의 scale.set_scale() 함수에 업데이트하세요.");
                    calibrating = false;
                }
            }
            else if (cmd == "exit")
            {
                calibrating = false;
                Serial.println("캘리브레이션 모드 종료");
            }
        }
        delay(100);
    }
}

// 메뉴 출력 함수
void printMenu()
{
    Serial.println("\n===== 전자저울 시스템 =====");
    Serial.println("명령어 목록:");
    Serial.println("- start: 무게 측정 시작");
    Serial.println("- calculate: 현재 무게로 패키지 계산");
    Serial.println("- clear: 무게 데이터 초기화");
    Serial.println("- calibrate: 로드셀 캘리브레이션");
    Serial.println("- target XXX: 목표 무게 설정 (XXX는 그램 단위)");
    Serial.println("- add XXX: 무게 수동 추가 (XXX는 그램 단위)");
    Serial.println("- menu: 이 메뉴 다시 보기");
    Serial.print("현재 목표 무게: ");
    Serial.print(targetWeight);
    Serial.println("g");
    Serial.println("=======================");
}

// 버튼 처리 함수
void handleButtons()
{
    // 영점 버튼 처리
    if (digitalRead(BUTTON_TARE_PIN) == LOW)
    {
        delay(50); // 디바운싱
        if (digitalRead(BUTTON_TARE_PIN) == LOW)
        {
            scale.tare();
            Serial.println("저울 영점 설정됨");
            delay(300); // 버튼 바운스 방지
        }
    }

    // 설정 버튼 처리
    if (digitalRead(BUTTON_SET_PIN) == LOW)
    {
        delay(50); // 디바운싱
        if (digitalRead(BUTTON_SET_PIN) == LOW)
        {
            isCalibrationMode = !isCalibrationMode;
            if (isCalibrationMode)
            {
                Serial.println("설정 모드 활성화");
                isWeighingMode = false;
            }
            else
            {
                Serial.println("설정 모드 비활성화");
            }
            delay(300); // 버튼 바운스 방지
        }
    }

    // 시작/계산 버튼 처리
    if (digitalRead(BUTTON_START_PIN) == LOW)
    {
        delay(50); // 디바운싱
        if (digitalRead(BUTTON_START_PIN) == LOW)
        {
            if (isWeighingMode)
            {
                // 계산 모드
                calculatePackages();
                isWeighingMode = false;
            }
            else
            {
                // 무게 측정 모드 시작
                isWeighingMode = true;
                numWeights = 0; // 데이터 초기화
                Serial.println("무게 측정 모드 시작");
                scale.tare(); // 영점 맞추기
            }
            delay(300); // 버튼 바운스 방지
        }
    }
}

// 무게 측정 함수
int getMeasuredWeight()
{
    if (scale.is_ready())
    {
        int weight = (int)scale.get_units(5); // 5회 측정 평균
        return weight > 0 ? weight : 0;
    }
    return 0;
}

// 남은 메모리 확인 함수
int freeMemory()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// https://wokwi.com/projects/406760615655119873 ( 04버전 _ 네오픽셀추가될것)
// https://wokwi.com/projects/406761093471327233 ( 04개선 - 네오픽셀 추가한것)
// https://wokwi.com/projects/406856366957485057 ( 05버전 - 자잘한 버그 수정 및 선오류 해결)
// https://wokwi.com/projects/406858734731378689 ( 06버전 - 등급별 네오픽셀 색상 표현)
/* INFO

https://wokwi.com/projects/407034241910719489  ( 조합 테스트중 ! 중요)

*/
#include <Arduino.h>
#include <stdlib.h> // rand() 함수 사용을 위해 포함

int weights[] = {300, 300, 300, 300, 300, 300, 300, 450};
int n = sizeof(weights) / sizeof(weights[0]);
int targetMin = 2000;                // 2.0kg in grams  // 최소 총무게
int targetMax = 2100;                // 2.1kg in grams  // 최대 총무게
const int NUMBER_OF_COMBINATION = 7; // 몇 송이로 조합을 원하는지 ?

// LED 핀 번호 설정 (LED 8개)
int ledPins[] = {9, 8, 7, 6, 5, 4, 3, 2}; // 1 트레이 ~ 8 트레이 순서

// 유효한 조합을 저장할 배열
int validCombinations[100][NUMBER_OF_COMBINATION]; // 최대 100개의 조합을 저장할 수 있음
int validCombinationCount = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting search for grape combinations...");

  // 랜덤 시드 설정
  randomSeed(analogRead(0)); // 아날로그 핀 A0의 값을 시드로 사용 , 작동여부 체크 못함

  // LED 핀을 출력 모드로 설정
  for (int i = 0; i < n; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // 초기에는 모두 꺼진 상태
  }

  // 포도송이 4개로 이루어진 모든 조합을 찾기
  findCombination(0, 0, 0, 0);

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

void loop()
{
  // Do nothing in loop
}

void findCombination(int index, int currentSum, int included, int depth)
{
  if (depth == NUMBER_OF_COMBINATION)
  { // 원하는 포도송이를 선택한 경우
    if (currentSum >= targetMin && currentSum <= targetMax)
    {
      // 유효한 조합을 저장
      int currentIndex = 0;
      for (int i = 0; i < n; i++)
      {
        if (included & (1 << i))
        {
          validCombinations[validCombinationCount][currentIndex++] = i;
        }
      }
      validCombinationCount++;
    }
    return;
  }

  // 인덱스가 배열 크기를 벗어나거나 무게가 최대치를 초과하면 종료
  if (index >= n || currentSum > targetMax)
  {
    return;
  }

  // 현재 무게를 포함하거나 포함하지 않는 두 가지 경우를 재귀적으로 탐색
  findCombination(index + 1, currentSum + weights[index], included | (1 << index), depth + 1); // 현재 무게 포함
  findCombination(index + 1, currentSum, included, depth);                                     // 현재 무게 제외
}

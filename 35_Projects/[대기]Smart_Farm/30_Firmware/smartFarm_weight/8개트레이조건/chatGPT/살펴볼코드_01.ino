/*
네, 말씀하신 내용은 무게를 합산하여 목표 무게(예: 2050g)에 가깝게 맞추는 조합 문제를 설명하는 것입니다. 이 문제는 다음과 같은 방식으로 해결할 수 있습니다:

1. **모든 가능한 조합의 합 계산**: 모든 포도 송이들의 무게를 사용하여 가능한 조합을 계산하고, 각 조합의 무게 합을 구합니다.
2. **목표 무게와의 비교**: 각 조합의 무게 합이 목표 무게(2050g)와 얼마나 가까운지를 평가합니다.
3. **조합 수 늘리기**: 만약 3개의 송이 조합으로 목표 무게를 맞출 수 없다면, 4개 또는 5개의 송이 조합을 사용하여 목표 무게에 가까운 조합을 찾습니다.

### 문제 해결 방법

1. **입력**: 포도 송이의 무게가 주어집니다. 예를 들어, 포도 송이 8개의 무게를 각각 `W1`, `W2`, ..., `W8`이라고 합시다.

2. **모든 조합 생성**: 포도 송이의 무게를 사용하여 가능한 모든 조합을 생성합니다.
   - **3개 조합**: 포도 송이 8개 중 3개를 선택하여 모든 조합의 무게를 계산합니다.
   - **4개 조합**: 포도 송이 8개 중 4개를 선택하여 모든 조합의 무게를 계산합니다.
   - **5개 조합**: 포도 송이 8개 중 5개를 선택하여 모든 조합의 무게를 계산합니다.

3. **목표 무게와 비교**:
   - 각 조합의 무게 합을 계산하고, 이 무게 합이 목표 무게(2050g)와 얼마나 가까운지를 평가합니다.
   - 목표 무게에 가장 가까운 조합을 찾습니다.

4. **최적 조합 선택**:
   - 목표 무게에 가장 가까운 조합을 선택합니다.
   - 3개 조합이 목표에 도달하지 못하면, 4개 또는 5개 조합을 시도하여 목표 무게에 가까운 조합을 찾습니다.

### 아두이노 코드 예제

아두이노에서 이런 조합을 찾는 데는 데이터베이스나 복잡한 연산이 필요할 수 있으므로, 대체로 컴퓨터 프로그래밍 언어(예: Python)에서 구현하는 것이 더 효율적입니다. 그러나, 아두이노에서 간단한 조합을 계산하는 예를 들면:
*/

#include <Arduino.h>

const int grapeCount = 8;
int weights[grapeCount] = {/* 포도 송이들의 무게 입력 */};
int targetWeight = 2050;

void setup()
{
    Serial.begin(9600);

    // 3개 조합 시도
    findBestCombination(3);

    // 4개 조합 시도 (3개 조합으로 충분하지 않은 경우)
    findBestCombination(4);

    // 5개 조합 시도 (4개 조합으로 충분하지 않은 경우)
    findBestCombination(5);
}

void loop()
{
    // 메인 루프는 필요 없음
}

void findBestCombination(int numOfGrapes)
{
    int bestCombinationSum = 0;
    int bestCombinationIndex = -1;
    int numCombinations = factorial(grapeCount) / (factorial(numOfGrapes) * factorial(grapeCount - numOfGrapes));

    for (int i = 0; i < numCombinations; i++)
    {
        int sum = 0;
        // 조합을 생성하여 sum 계산 (예를 들어 조합 인덱스 i 사용)
        // 실제 조합 생성 로직이 필요합니다
        if (abs(targetWeight - sum) < abs(targetWeight - bestCombinationSum))
        {
            bestCombinationSum = sum;
            bestCombinationIndex = i;
        }
    }

    Serial.print("Best combination with ");
    Serial.print(numOfGrapes);
    Serial.print(" grapes: ");
    Serial.println(bestCombinationSum);
}

int factorial(int n)
{
    int result = 1;
    for (int i = 2; i <= n; i++)
    {
        result *= i;
    }
    return result;
}

/*
### 주의사항
- **조합 생성**: 실제 조합을 생성하고 계산하는 로직은 이 코드 예제에서 단순화되어 있습니다. 조합 생성 알고리즘이 필요합니다.
- **성능**: 포도 송이 수가 많아지면 조합의 수가 급격히 증가하므로, 성능과 메모리 문제를 고려해야 합니다.

이와 같은 접근법을 사용하여 다양한 조합을 시도하고 목표 무게에 가장 근접한 조합을 찾을 수 있습니다.
*/
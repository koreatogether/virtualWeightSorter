#define MAX_WEIGHTS 20
#define MAX_BUNCHES 5

int grapeWeights[MAX_WEIGHTS];
int numWeights = 0;
const int targetWeight = 2050;

void setup()
{
    Serial.begin(9600);

    // 예시 데이터 입력
    addWeight(300);
    addWeight(400);
    addWeight(350);
    addWeight(450);
    addWeight(500);
    addWeight(380);
    addWeight(420);
    addWeight(470);
    addWeight(390);
    addWeight(410);

    calculatePackages();
}

void loop()
{
    // 아두이노 루프에서는 아무 것도 하지 않습니다.
}

void addWeight(int weight)
{
    if (numWeights < MAX_WEIGHTS)
    {
        grapeWeights[numWeights++] = weight;
    }
}

void calculatePackages()
{
    int packageCount = 0;
    int remainingWeights[MAX_WEIGHTS];
    int numRemaining = numWeights;

    for (int i = 0; i < numWeights; i++)
    {
        remainingWeights[i] = grapeWeights[i];
    }

    Serial.print("목표 무게: ");
    Serial.print(targetWeight);
    Serial.println("g");

    while (numRemaining > 0)
    {
        int bestCombination[MAX_BUNCHES];
        int bestCombinationSize = 0;
        int bestTotalWeight = 0;
        int minDifference = targetWeight;

        findBestCombination(remainingWeights, numRemaining, bestCombination, &bestCombinationSize, &bestTotalWeight, &minDifference);

        if (bestCombinationSize > 0)
        {
            packageCount++;
            Serial.print("포장 ");
            Serial.print(packageCount);
            Serial.print(": ");
            for (int i = 0; i < bestCombinationSize; i++)
            {
                Serial.print(bestCombination[i]);
                Serial.print(" ");
                removeWeight(remainingWeights, &numRemaining, bestCombination[i]);
            }
            Serial.print("(총 무게: ");
            Serial.print(bestTotalWeight);
            Serial.println("g)");
        }
        else
        {
            break;
        }
    }

    Serial.print("포장 수: ");
    Serial.println(packageCount);

    if (numRemaining > 0)
    {
        Serial.print("남은 포도 송이: ");
        for (int i = 0; i < numRemaining; i++)
        {
            Serial.print(remainingWeights[i]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void findBestCombination(int weights[], int numWeights, int bestCombination[], int *bestSize, int *bestTotal, int *minDifference)
{
    int combination[MAX_BUNCHES];
    findCombinationRecursive(weights, numWeights, combination, 0, 0, 0, bestCombination, bestSize, bestTotal, minDifference);
}

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
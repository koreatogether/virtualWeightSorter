/**
 * 로또 체크기 - 개선 버전
 * 리뷰 개선사항을 적용한 버전
 * 가상시뮬레이터 : https://wokwi.com/projects/424190764525301761
 */

// 1. 매직 넘버 제거 - 상수 정의
const int MIN_LOTTO_NUMBER = 1;
const int MAX_LOTTO_NUMBER = 45;
const int LOTTO_COUNT = 6;
const int BUFFER_SIZE = 64;

// 5. 명시적 상태 관리 적용
enum State
{
    INIT,
    INPUT_WINNING_NUMBERS,
    INPUT_BONUS_NUMBER,
    INPUT_USER_NUMBERS,
    DISPLAY_RESULT,
    TEST_MODE
};

// 전역 변수 선언
int winningNumbers[LOTTO_COUNT] = {0}; // 당첨 번호
int bonusNumber = 0;                   // 보너스 번호
int userNumbers[LOTTO_COUNT] = {0};    // 사용자 번호
State currentState = INIT;             // 현재 상태
char inputBuffer[BUFFER_SIZE];         // 4. String 대신 char 배열 사용

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        ; // 시리얼 포트가 연결될 때까지 대기
    }

    Serial.println(F("로또 번호 체커 프로그램을 시작합니다."));
    Serial.println(F("명령어: 'test'를 입력하면 테스트 모드로 진입합니다."));
    currentState = INPUT_WINNING_NUMBERS;
}

void loop()
{
    // 5. 명시적인 상태 기반 처리 (FSM 패턴 적용)
    switch (currentState)
    {
    case INPUT_WINNING_NUMBERS:
        processWinningNumbersInput();
        break;

    case INPUT_BONUS_NUMBER:
        processBonusNumberInput();
        break;

    case INPUT_USER_NUMBERS:
        processUserNumbersInput();
        break;

    case DISPLAY_RESULT:
        displayResult();
        resetProgram();
        break;

    case TEST_MODE:
        runTests();
        resetProgram();
        break;

    default:
        currentState = INPUT_WINNING_NUMBERS;
        break;
    }
}

// 1, 6. 중복 코드 제거 및 일관된 입력 처리 - 공통 번호 입력 함수
bool readNumbersFromInput(int *numbers, int count, const char *prompt)
{
    Serial.println(prompt);

    if (!waitForInput())
    {
        return false;
    }

    // 입력이 "test"인지 확인
    if (strcmp(inputBuffer, "test") == 0)
    {
        currentState = TEST_MODE;
        return false;
    }

    // 문자열 파싱
    char *token = strtok(inputBuffer, " ,");
    int index = 0;

    while (token != NULL && index < count)
    {
        int number = atoi(token);

        // 유효성 검사
        if (number < MIN_LOTTO_NUMBER || number > MAX_LOTTO_NUMBER)
        {
            Serial.print(F("오류: "));
            Serial.print(number);
            Serial.println(F("는 유효한 로또 번호가 아닙니다. 1부터 45 사이의 번호를 입력하세요."));
            return false;
        }

        // 중복 검사
        for (int i = 0; i < index; i++)
        {
            if (numbers[i] == number)
            {
                Serial.print(F("오류: "));
                Serial.print(number);
                Serial.println(F("는 이미 입력한 번호입니다. 중복 없이 입력하세요."));
                return false;
            }
        }

        numbers[index++] = number;
        token = strtok(NULL, " ,");
    }

    // 입력 개수 검사
    if (index != count)
    {
        Serial.print(F("오류: 정확히 "));
        Serial.print(count);
        Serial.println(F("개의 숫자를 입력해야 합니다."));
        return false;
    }

    return true;
}

// 단일 번호 입력 함수 (보너스 번호용)
bool readSingleNumber(int *number, const char *prompt, int *existingNumbers, int count)
{
    Serial.println(prompt);

    if (!waitForInput())
    {
        return false;
    }

    // 입력이 "test"인지 확인
    if (strcmp(inputBuffer, "test") == 0)
    {
        currentState = TEST_MODE;
        return false;
    }

    *number = atoi(inputBuffer);

    // 유효성 검사
    if (*number < MIN_LOTTO_NUMBER || *number > MAX_LOTTO_NUMBER)
    {
        Serial.print(F("오류: "));
        Serial.print(*number);
        Serial.println(F("는 유효한 로또 번호가 아닙니다. 1부터 45 사이의 번호를 입력하세요."));
        return false;
    }

    // 중복 검사
    for (int i = 0; i < count; i++)
    {
        if (existingNumbers[i] == *number)
        {
            Serial.print(F("오류: "));
            Serial.print(*number);
            Serial.println(F("는 이미 당첨 번호에 포함된 번호입니다. 다른 번호를 입력하세요."));
            return false;
        }
    }

    return true;
}

// 입력 대기 함수
bool waitForInput()
{
    memset(inputBuffer, 0, BUFFER_SIZE);
    int index = 0;

    while (true)
    {
        if (Serial.available())
        {
            char c = Serial.read();

            if (c == '\n' || c == '\r')
            {
                if (index > 0)
                {
                    inputBuffer[index] = '\0';
                    return true;
                }
            }
            else if (index < BUFFER_SIZE - 1)
            {
                inputBuffer[index++] = c;
            }
        }
        yield(); // 다른 작업 허용
    }
}

// 당첨 번호 입력 처리
void processWinningNumbersInput()
{
    static bool isFirstAttempt = true;

    // 첫 번째 시도에서 메시지를 출력하는 부분을 제거하고
    // readNumbersFromInput 함수에서 메시지를 출력하도록 함
    if (readNumbersFromInput(winningNumbers, LOTTO_COUNT, "당첨 번호 6개를 입력하세요 (쉼표나 공백으로 구분):"))
    {
        // 번호 정렬
        sortNumbers(winningNumbers, LOTTO_COUNT);

        // 입력된 번호 출력
        Serial.print(F("입력된 당첨 번호: "));
        printNumbers(winningNumbers, LOTTO_COUNT);

        currentState = INPUT_BONUS_NUMBER;
        isFirstAttempt = true;
    }
}

// 보너스 번호 입력 처리
void processBonusNumberInput()
{
    static bool isFirstAttempt = true;

    // 첫 번째 시도에서 메시지를 출력하는 부분을 제거하고
    // readSingleNumber 함수가 메시지를 출력하도록 함
    if (readSingleNumber(&bonusNumber, "보너스 번호를 입력하세요:", winningNumbers, LOTTO_COUNT))
    {
        Serial.print(F("보너스 번호: "));
        Serial.println(bonusNumber);

        currentState = INPUT_USER_NUMBERS;
        isFirstAttempt = true;
    }
}

// 사용자 번호 입력 처리
void processUserNumbersInput()
{
    static bool isFirstAttempt = true;

    // 첫 번째 시도에서 메시지를 출력하는 부분을 제거하고
    // readNumbersFromInput 함수가 메시지를 출력하도록 함
    if (readNumbersFromInput(userNumbers, LOTTO_COUNT, "내 로또 번호 6개를 입력하세요 (쉼표나 공백으로 구분):"))
    {
        // 번호 정렬
        sortNumbers(userNumbers, LOTTO_COUNT);

        // 입력된 번호 출력
        Serial.print(F("입력된 내 번호: "));
        printNumbers(userNumbers, LOTTO_COUNT);

        currentState = DISPLAY_RESULT;
        isFirstAttempt = true;
    }
}

// 번호 정렬 함수
void sortNumbers(int *numbers, int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (numbers[j] > numbers[j + 1])
            {
                // Swap
                int temp = numbers[j];
                numbers[j] = numbers[j + 1];
                numbers[j + 1] = temp;
            }
        }
    }
}

// 번호 출력 함수
void printNumbers(int *numbers, int count)
{
    for (int i = 0; i < count; i++)
    {
        Serial.print(numbers[i]);
        if (i < count - 1)
        {
            Serial.print(", ");
        }
    }
    Serial.println();
}

// 결과 표시
void displayResult()
{
    int matchCount = countMatches(winningNumbers, userNumbers, LOTTO_COUNT);
    bool hasBonusMatch = hasBonus(userNumbers, bonusNumber, LOTTO_COUNT);

    Serial.println(F("\n===== 결과 ====="));
    Serial.print(F("맞은 번호 개수: "));
    Serial.println(matchCount);

    Serial.print(F("당첨 번호: "));
    printNumbers(winningNumbers, LOTTO_COUNT);

    Serial.print(F("보너스 번호: "));
    Serial.println(bonusNumber);

    Serial.print(F("내 번호: "));
    printNumbers(userNumbers, LOTTO_COUNT);

    // 로또 등수 계산
    int rank = calculateRank(matchCount, hasBonusMatch);
    displayRank(rank);
}

// 일치하는 번호 개수 계산
int countMatches(int *winningNumbers, int *userNumbers, int count)
{
    int matches = 0;

    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < count; j++)
        {
            if (winningNumbers[i] == userNumbers[j])
            {
                matches++;
                break;
            }
        }
    }

    return matches;
}

// 보너스 번호 일치 여부 확인
bool hasBonus(int *userNumbers, int bonusNumber, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (userNumbers[i] == bonusNumber)
        {
            return true;
        }
    }
    return false;
}

// 등수 계산
int calculateRank(int matchCount, bool hasBonusMatch)
{
    switch (matchCount)
    {
    case 6:
        return 1; // 1등: 6개 모두 일치
    case 5:
        return hasBonusMatch ? 2 : 3; // 2등: 5개 + 보너스, 3등: 5개 일치
    case 4:
        return 4; // 4등: 4개 일치
    case 3:
        return 5; // 5등: 3개 일치
    default:
        return 0; // 미당첨
    }
}

// 등수 표시
void displayRank(int rank)
{
    Serial.print(F("결과: "));

    switch (rank)
    {
    case 1:
        Serial.println(F("1등입니다! 축하합니다!"));
        break;
    case 2:
        Serial.println(F("2등입니다! 축하합니다!"));
        break;
    case 3:
        Serial.println(F("3등입니다! 축하합니다!"));
        break;
    case 4:
        Serial.println(F("4등입니다. 축하합니다!"));
        break;
    case 5:
        Serial.println(F("5등입니다. 축하합니다!"));
        break;
    default:
        Serial.println(F("아쉽게도 당첨되지 않았습니다."));
        break;
    }
}

// 프로그램 초기화
void resetProgram()
{
    // 배열 초기화
    memset(winningNumbers, 0, sizeof(winningNumbers));
    memset(userNumbers, 0, sizeof(userNumbers));
    bonusNumber = 0;

    // 상태 초기화
    currentState = INPUT_WINNING_NUMBERS;

    Serial.println(F("\n프로그램을 재시작합니다."));
    // 안내 메시지 중복 출력 제거 - readNumbersFromInput에서 출력됨
}

// 3. 테스트 코드 강화
void runTests()
{
    Serial.println(F("\n===== 테스트 모드 ====="));

    // 테스트 케이스 1: 1등 (6개 일치)
    int testWinning1[] = {1, 2, 3, 4, 5, 6};
    int testUser1[] = {1, 2, 3, 4, 5, 6};
    int testBonus1 = 7;

    testRankCalculation(testWinning1, testUser1, testBonus1, "1등 테스트 (6개 일치)", 1);

    // 테스트 케이스 2: 2등 (5개 + 보너스 일치)
    int testWinning2[] = {1, 2, 3, 4, 5, 6};
    int testUser2[] = {1, 2, 3, 4, 5, 7};
    int testBonus2 = 7;

    testRankCalculation(testWinning2, testUser2, testBonus2, "2등 테스트 (5개 + 보너스 일치)", 2);

    // 테스트 케이스 3: 3등 (5개 일치)
    int testWinning3[] = {1, 2, 3, 4, 5, 6};
    int testUser3[] = {1, 2, 3, 4, 5, 10};
    int testBonus3 = 7;

    testRankCalculation(testWinning3, testUser3, testBonus3, "3등 테스트 (5개 일치)", 3);

    // 테스트 케이스 4: 4등 (4개 일치)
    int testWinning4[] = {1, 2, 3, 4, 5, 6};
    int testUser4[] = {1, 2, 3, 4, 10, 20};
    int testBonus4 = 7;

    testRankCalculation(testWinning4, testUser4, testBonus4, "4등 테스트 (4개 일치)", 4);

    // 테스트 케이스 5: 5등 (3개 일치)
    int testWinning5[] = {1, 2, 3, 4, 5, 6};
    int testUser5[] = {1, 2, 3, 10, 20, 30};
    int testBonus5 = 7;

    testRankCalculation(testWinning5, testUser5, testBonus5, "5등 테스트 (3개 일치)", 5);

    // 테스트 케이스 6: 미당첨 (2개 일치)
    int testWinning6[] = {1, 2, 3, 4, 5, 6};
    int testUser6[] = {1, 2, 10, 20, 30, 40};
    int testBonus6 = 7;

    testRankCalculation(testWinning6, testUser6, testBonus6, "미당첨 테스트 (2개 일치)", 0);

    Serial.println(F("테스트가 완료되었습니다."));
}

// 테스트 케이스 실행 함수
void testRankCalculation(int *winning, int *user, int bonus, const char *testName, int expectedRank)
{
    Serial.print(F("\n테스트: "));
    Serial.println(testName);

    Serial.print(F("당첨 번호: "));
    printNumbers(winning, LOTTO_COUNT);

    Serial.print(F("보너스 번호: "));
    Serial.println(bonus);

    Serial.print(F("내 번호: "));
    printNumbers(user, LOTTO_COUNT);

    int matches = countMatches(winning, user, LOTTO_COUNT);
    bool hasBonus = false;
    for (int i = 0; i < LOTTO_COUNT; i++)
    {
        if (user[i] == bonus)
        {
            hasBonus = true;
            break;
        }
    }

    int rank = calculateRank(matches, hasBonus);
    Serial.print(F("맞은 개수: "));
    Serial.print(matches);
    Serial.print(F(", 보너스 일치: "));
    Serial.println(hasBonus ? "예" : "아니오");

    Serial.print(F("계산된 등수: "));
    if (rank > 0)
    {
        Serial.print(rank);
        Serial.println(F("등"));
    }
    else
    {
        Serial.println(F("미당첨"));
    }

    Serial.print(F("테스트 결과: "));
    if (rank == expectedRank)
    {
        Serial.println(F("성공"));
    }
    else
    {
        Serial.print(F("실패 (예상: "));
        if (expectedRank > 0)
        {
            Serial.print(expectedRank);
            Serial.print(F("등, 실제: "));
        }
        else
        {
            Serial.print(F("미당첨, 실제: "));
        }
        if (rank > 0)
        {
            Serial.print(rank);
            Serial.println(F("등)"));
        }
        else
        {
            Serial.println(F("미당첨)"));
        }
    }
}
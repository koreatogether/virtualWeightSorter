#include <Arduino.h>

// 상수 정의
const int MIN_LOTTO_NUMBER = 1;
const int MAX_LOTTO_NUMBER = 45;
const int LOTTO_COUNT = 6;

// 프로그램 상태 관리
enum State
{
    INPUT_WINNING_NUMBERS,
    INPUT_BONUS_NUMBER,
    INPUT_USER_NUMBERS,
    CHECK_RESULT,
    END
};

// 전역 변수 최소화
int winningNumbers[LOTTO_COUNT];
int bonusNumber = 0;
int userNumbers[LOTTO_COUNT];
State currentState = INPUT_WINNING_NUMBERS; // 초기 상태

// 함수 선언
bool isValidNumber(int number);
bool hasDuplicate(int numbers[], int size);
bool readNumbers(int numbers[], int size);
void processNumbers(int numbers[], int size);
void runTests();
int checkLottoResult(int winningNumbers[], int bonusNumber, int userNumbers[]);

void setup()
{
    Serial.begin(9600);
    Serial.println("로또 체크기 프로그램 시작!");
    runTests(); // 테스트 실행
    Serial.println("당첨 번호 6개를 입력하세요 (띄어쓰기로 구분):");
}

void loop()
{
    switch (currentState)
    {
    case INPUT_WINNING_NUMBERS:
        if (readNumbers(winningNumbers, LOTTO_COUNT))
        {
            currentState = INPUT_BONUS_NUMBER;
            Serial.println("보너스 번호 1개를 입력하세요:");
        }
        else
        {
            Serial.println("당첨 번호 6개를 다시 입력해주세요");
        }
        break;
    case INPUT_BONUS_NUMBER:
        if (Serial.available() > 0)
        {
            String input = Serial.readStringUntil('\n');
            if (input.length() > 0)
            {
                int num = input.toInt();
                if (isValidNumber(num))
                {
                    bonusNumber = num;
                    currentState = INPUT_USER_NUMBERS;
                    Serial.println("사용자 번호 6개를 입력하세요 (띄어쓰기로 구분):");
                }
                else
                {
                    Serial.println("유효하지 않은 번호입니다. 1~45 사이의 숫자를 입력하세요.");
                }
            }
        }
        break;
    case INPUT_USER_NUMBERS:
        if (readNumbers(userNumbers, LOTTO_COUNT))
        {
            currentState = CHECK_RESULT;
        }
        else
        {
            Serial.println("사용자 번호 6개를 다시 입력해주세요");
        }
        break;
    case CHECK_RESULT:
        processNumbers(userNumbers, LOTTO_COUNT);
        int result = checkLottoResult(winningNumbers, bonusNumber, userNumbers);
        if (result == 0)
        {
            Serial.println("꽝입니다!");
        }
        else
        {
            Serial.print(result);
            Serial.println("등 당첨!");
        }
        currentState = END;
        break;
    case END:
        Serial.println("로또 확인 프로그램 종료!");
        while (true)
            ;
        break;
    }
}

// 입력 번호가 유효한지 검사 (1~45)
bool isValidNumber(int number)
{
    return number >= MIN_LOTTO_NUMBER && number <= MAX_LOTTO_NUMBER;
}

// 번호 배열에 중복이 있는지 검사
bool hasDuplicate(int numbers[], int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = i + 1; j < size; j++)
        {
            if (numbers[i] == numbers[j])
            {
                return true;
            }
        }
    }
    return false;
}

// 번호 읽어오기
bool readNumbers(int numbers[], int size)
{
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        int numCount = 0;
        for (int i = 0; i < input.length(); i++)
        {
            if (isDigit(input.charAt(i)))
            {
                int j = i;
                while (j < input.length() && isDigit(input.charAt(j)))
                {
                    j++;
                }
                int num = input.substring(i, j).toInt();
                if (isValidNumber(num))
                {
                    numbers[numCount++] = num;
                    i = j - 1;
                }
                else
                {
                    Serial.println("유효하지 않은 번호가 입력되었습니다. 1~45 사이의 숫자를 입력하세요.");
                    return false;
                }
            }
        }
        if (numCount != size)
        {
            Serial.print(size);
            Serial.println("개의 숫자를 입력해야 합니다.");
            return false;
        }
        if (hasDuplicate(numbers, size))
        {
            Serial.println("중복된 번호가 입력되었습니다. 다시 입력하세요.");
            return false;
        }
    }
    return false;
}
void processNumbers(int numbers[], int size)
{
    Serial.print("입력한 번호: ");
    for (int i = 0; i < size; i++)
    {
        Serial.print(numbers[i]);
        Serial.print(" ");
    }
    Serial.println();
}

// 테스트 함수 (간단한 테스트 로직 추가)
void runTests()
{
    Serial.println("Running automated tests...");
    // 테스트 케이스 1: 유효한 번호 테스트
    if (!isValidNumber(1) || !isValidNumber(45))
    {
        Serial.println("Error: isValidNumber() failed!");
    }

    // 테스트 케이스 2: 중복 검사 테스트
    int testNumbers1[] = {1, 2, 3, 4, 5, 6};
    int testNumbers2[] = {1, 2, 3, 4, 5, 5};
    if (hasDuplicate(testNumbers1, 6) || !hasDuplicate(testNumbers2, 6))
    {
        Serial.println("Error: hasDuplicate() failed!");
    }
    // 테스트 케이스 3 : 번호 범위 벗어난 경우
    if (isValidNumber(0) || isValidNumber(46))
    {
        Serial.println("Error: number range test failed!");
    }

    Serial.println("Tests completed.");
}

int checkLottoResult(int winningNumbers[], int bonusNumber, int userNumbers[])
{
    int matchCount = 0;
    bool bonusMatch = false;
    for (int i = 0; i < LOTTO_COUNT; i++)
    {
        for (int j = 0; j < LOTTO_COUNT; j++)
        {
            if (winningNumbers[i] == userNumbers[j])
            {
                matchCount++;
            }
        }
    }
    for (int i = 0; i < LOTTO_COUNT; i++)
    {
        if (userNumbers[i] == bonusNumber)
        {
            bonusMatch = true;
            break;
        }
    }
    if (matchCount == 6)
    {
        return 1;
    }
    else if (matchCount == 5 && bonusMatch)
    {
        return 2;
    }
    else if (matchCount == 5)
    {
        return 3;
    }
    else if (matchCount == 4)
    {
        return 4;
    }
    else if (matchCount == 3)
    {
        return 5;
    }
    return 0;
}

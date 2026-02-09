
#include <Arduino.h>

// 로또 당첨 번호 (사용자 입력)
int winningNumbers[6];
int bonusNumber;

// 사용자 입력 복권 번호 저장
int userNumbers[6];

bool isValidNumber(int number)
{
    return number >= 1 && number <= 45;
}

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

void readWinningNumbers()
{
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        int numCount = 0;
        for (int i = 0; i < input.length(); i++)
        {
            if (isDigit(input.charAt(i)))
            {
                int num = input.substring(i).toInt();
                if (isValidNumber(num))
                {
                    winningNumbers[numCount++] = num;
                    while (isDigit(input.charAt(i)))
                    {
                        i++;
                    }
                }
                else
                {
                    Serial.println("유효하지 않은 번호가 입력되었습니다. 1~45 사이의 숫자를 입력하세요.");
                    Serial.println("당첨 번호 6개를 입력하세요 (띄어쓰기로 구분):");
                    return;
                }
            }
        }
        if (numCount != 6)
        {
            Serial.println("6개의 숫자를 입력해야 합니다.");
            Serial.println("당첨 번호 6개를 입력하세요 (띄어쓰기로 구분):");
            return;
        }
        if (hasDuplicate(winningNumbers, 6))
        {
            Serial.println("중복된 번호가 입력되었습니다. 다시 입력하세요.");
            Serial.println("당첨 번호 6개를 입력하세요 (띄어쓰기로 구분):");
            return;
        }

        Serial.println("보너스 번호 1개를 입력하세요:");
    }
}

void readBonusNumber()
{
    if (Serial.available() > 0)
    {
        bonusNumber = Serial.parseInt();
        if (!isValidNumber(bonusNumber))
        {
            Serial.println("유효하지 않은 번호가 입력되었습니다. 1~45 사이의 숫자를 입력하세요.");
            Serial.println("보너스 번호 1개를 입력하세요:");
            return;
        }

        // 입력 완료 후 안내 메시지 출력
        Serial.println("입력이 완료되었습니다.");
        Serial.print("당첨 번호: ");
        for (int i = 0; i < 6; i++)
        {
            Serial.print(winningNumbers[i]);
            Serial.print(" ");
        }
        Serial.print("+ ");
        Serial.println(bonusNumber);
    }
}

void readUserNumbersAndCheck()
{
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        int numCount = 0;
        for (int i = 0; i < input.length(); i++)
        {
            if (isDigit(input.charAt(i)))
            {
                int num = input.substring(i).toInt();
                if (isValidNumber(num))
                {
                    userNumbers[numCount++] = num;
                    while (isDigit(input.charAt(i)))
                    {
                        i++;
                    }
                }
                else
                {
                    Serial.println("유효하지 않은 번호가 입력되었습니다. 1~45 사이의 숫자를 입력하세요.");
                    Serial.println("복권 번호 6개를 입력하세요 (띄어쓰기로 구분):");
                    return;
                }
            }
        }
        if (numCount != 6)
        {
            Serial.println("6개의 숫자를 입력해야 합니다.");
            Serial.println("복권 번호 6개를 입력하세요 (띄어쓰기로 구분):");
            return;
        }
        if (hasDuplicate(userNumbers, 6))
        {
            Serial.println("중복된 번호가 입력되었습니다. 다시 입력하세요.");
            Serial.println("복권 번호 6개를 입력하세요 (띄어쓰기로 구분):");
            return;
        }

        // 당첨 등수 확인
        int matchCount = 0;
        bool hasBonus = false;
        Serial.print("맞춘 번호: ");
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                if (userNumbers[i] == winningNumbers[j])
                {
                    matchCount++;
                    Serial.print(userNumbers[i]);
                    Serial.print(" ");
                    break; // 중복 출력 방지
                }
            }
            if (userNumbers[i] == bonusNumber)
            {
                hasBonus = true;
            }
        }
        Serial.println(); // 줄 바꿈

        // 결과 출력
        Serial.print("맞춘 개수: ");
        Serial.print(matchCount);

        if (matchCount == 6)
        {
            Serial.println("개, 축하합니다! 1등입니다!");
        }
        else if (matchCount == 5 && hasBonus)
        {
            Serial.println("개 + 보너스 번호, 축하합니다! 2등입니다!");
        }
        else if (matchCount == 5)
        {
            Serial.println("개, 축하합니다! 3등입니다!");
        }
        else if (matchCount == 4)
        {
            Serial.println("개, 축하합니다! 4등입니다!");
        }
        else if (matchCount == 3)
        {
            Serial.println("개, 축하합니다! 5등입니다!");
        }
        else
        {
            Serial.println("개, 아쉽지만 다음 기회에...");
        }

        // 다시 입력 받기 안내 메시지 (반복)
        Serial.println("복권 번호 6개를 입력하세요 (띄어쓰기로 구분):");
    }
}

void runTests()
{
    // 단순 예시 테스트 코드
    Serial.println("Running automated tests...");
    // ...add more test logic...
}

// 새로운 함수로 분리된 에러 처리
bool validateNumbers(int numbers[], int size)
{
    if (hasDuplicate(numbers, size))
    {
        Serial.println("중복된 번호가 입력되었습니다. 다시 입력하세요.");
        return false;
    }
    return true;
}

// 당첨 번호 읽기
bool processWinningNumbers()
{
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        int numCount = 0;
        for (int i = 0; i < input.length(); i++)
        {
            if (isDigit(input.charAt(i)))
            {
                int num = input.substring(i).toInt();
                if (isValidNumber(num))
                {
                    winningNumbers[numCount++] = num;
                    while (isDigit(input.charAt(i)))
                    {
                        i++;
                    }
                }
                else
                {
                    Serial.println("유효하지 않은 번호가 입력되었습니다. 1~45 사이의 숫자를 입력하세요.");
                    return false;
                }
            }
        }
        if (numCount != 6)
        {
            Serial.println("6개의 숫자를 입력해야 합니다.");
            return false;
        }
        if (hasDuplicate(winningNumbers, 6))
        {
            Serial.println("중복된 번호가 입력되었습니다. 다시 입력하세요.");
            return false;
        }

        Serial.println("보너스 번호 1개를 입력하세요:");
    }
    return true;
}

// 보너스 번호 읽기
bool processBonusNumber()
{
    if (Serial.available() > 0)
    {
        bonusNumber = Serial.parseInt();
        if (!isValidNumber(bonusNumber))
        {
            Serial.println("유효하지 않은 번호가 입력되었습니다. 1~45 사이의 숫자를 입력하세요.");
            return false;
        }

        // 입력 완료 후 안내 메시지 출력
        Serial.println("입력이 완료되었습니다.");
        Serial.print("당첨 번호: ");
        for (int i = 0; i < 6; i++)
        {
            Serial.print(winningNumbers[i]);
            Serial.print(" ");
        }
        Serial.print("+ ");
        Serial.println(bonusNumber);
    }
    return true;
}

// 사용자 번호 읽기 및 당첨 확인
void processUserNumbersAndCheck()
{
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        int numCount = 0;
        for (int i = 0; i < input.length(); i++)
        {
            if (isDigit(input.charAt(i)))
            {
                int num = input.substring(i).toInt();
                if (isValidNumber(num))
                {
                    userNumbers[numCount++] = num;
                    while (isDigit(input.charAt(i)))
                    {
                        i++;
                    }
                }
                else
                {
                    Serial.println("유효하지 않은 번호가 입력되었습니다. 1~45 사이의 숫자를 입력하세요.");
                    return;
                }
            }
        }
        if (numCount != 6)
        {
            Serial.println("6개의 숫자를 입력해야 합니다.");
            return;
        }
        if (hasDuplicate(userNumbers, 6))
        {
            Serial.println("중복된 번호가 입력되었습니다. 다시 입력하세요.");
            return;
        }

        // 당첨 등수 확인
        int matchCount = 0;
        bool hasBonus = false;
        Serial.print("맞춘 번호: ");
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                if (userNumbers[i] == winningNumbers[j])
                {
                    matchCount++;
                    Serial.print(userNumbers[i]);
                    Serial.print(" ");
                    break; // 중복 출력 방지
                }
            }
            if (userNumbers[i] == bonusNumber)
            {
                hasBonus = true;
            }
        }
        Serial.println(); // 줄 바꿈

        // 결과 출력
        Serial.print("맞춘 개수: ");
        Serial.print(matchCount);

        if (matchCount == 6)
        {
            Serial.println("개, 축하합니다! 1등입니다!");
        }
        else if (matchCount == 5 && hasBonus)
        {
            Serial.println("개 + 보너스 번호, 축하합니다! 2등입니다!");
        }
        else if (matchCount == 5)
        {
            Serial.println("개, 축하합니다! 3등입니다!");
        }
        else if (matchCount == 4)
        {
            Serial.println("개, 축하합니다! 4등입니다!");
        }
        else if (matchCount == 3)
        {
            Serial.println("개, 축하합니다! 5등입니다!");
        }
        else
        {
            Serial.println("개, 아쉽지만 다음 기회에...");
        }

        // 다시 입력 받기 안내 메시지 (반복)
        Serial.println("복권 번호 6개를 입력하세요 (띄어쓰기로 구분):");
    }
}

void setup()
{
    Serial.begin(9600);
    runTests();
    Serial.println("로또 당첨 번호 확인 프로그램");
    Serial.println("당첨 번호 6개를 입력하세요 (띄어쓰기로 구분):");
}

void loop()
{
    // 당첨 번호가 읽혔는지 확인
    if (winningNumbers[0] == 0)
    {
        // 읽히지 않았다면 당첨 번호를 읽음
        if (!processWinningNumbers())
        {
            Serial.println("당첨 번호 6개를 입력하세요 (띄어쓰기로 구분):");
        }
    }
    // 보너스 번호가 읽혔는지 확인
    else if (bonusNumber == 0)
    {
        // 읽히지 않았다면 보너스 번호를 읽음
        if (!processBonusNumber())
        {
            Serial.println("보너스 번호 1개를 입력하세요:");
        }
    }
    else
    {
        // 당첨 번호와 보너스 번호가 모두 읽혔다면 사용자 번호를 읽고 확인
        processUserNumbersAndCheck();
    }
}
#include <Arduino.h>

// 로또 당첨 번호 (예시)
int winningNumbers[] = {3, 6, 21, 30, 34, 35}; // 1등 번호
int bonusNumber = 22;                          // 보너스 번호

// 사용자 입력 복권 번호 저장
int userNumbers[6];

void setup()
{
    Serial.begin(9600);
    Serial.println("로또 당첨 번호 확인 프로그램");
    Serial.println("당첨 번호:");
    for (int i = 0; i < 6; i++)
    {
        Serial.print(winningNumbers[i]);
        Serial.print(" ");
    }
    Serial.print("+ ");
    Serial.println(bonusNumber);
    Serial.println("복권 번호 6개를 입력하세요 (띄어쓰기로 구분):");
}

void loop()
{
    // 시리얼 입력 대기
    if (Serial.available() >= 17)
    { // 6개 숫자 + 5개 공백 + 개행 문자
        for (int i = 0; i < 6; i++)
        {
            userNumbers[i] = Serial.parseInt();
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
        Serial.println(matchCount);
        if (matchCount == 6)
        {
            Serial.println("축하합니다! 1등입니다!");
        }
        else if (matchCount == 5 && hasBonus)
        {
            Serial.println("축하합니다! 2등입니다!");
        }
        else if (matchCount == 5)
        {
            Serial.println("축하합니다! 3등입니다!");
        }
        else if (matchCount == 4)
        {
            Serial.println("축하합니다! 4등입니다!");
        }
        else if (matchCount == 3)
        {
            Serial.println("축하합니다! 5등입니다!");
        }
        else
        {
            Serial.println("아쉽지만 다음 기회에...");
        }

        // 다시 입력 받기
        Serial.println("복권 번호 6개를 입력하세요 (띄어쓰기로 구분):");
    }
}
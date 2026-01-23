#include <SoftwareSerial.h>

// RX=10번(Nextion의 파란선 TX 연결)
// TX=11번(Nextion의 노란선 RX 연결)
SoftwareSerial nextionTest(10, 11);

void setup()
{
    Serial.begin(115200);    // PC 화면용 (빠르게)
    nextionTest.begin(9600); // Nextion 통신용 (기본값)

    Serial.println("=========================================");
    Serial.println("Nextion 하드웨어 통신 테스트 모드 시작");
    Serial.println("1. Nextion 전원을 뺐다가 다시 꽂아보세요.");
    Serial.println("2. 부팅 시 16진수 코드(00 00 00 ff ff ff 등)가 뜨면 RX(10번) 정상입니다.");
    Serial.println("=========================================");
}

void loop()
{
    // 1. Nextion -> 아두이노 -> PC (데이터 수신 확인)
    if (nextionTest.available())
    {
        int data = nextionTest.read();
        // 보기 편하게 16진수로 출력
        if (data < 16)
            Serial.print("0");
        Serial.print(data, HEX);
        Serial.print(" ");
    }

    // 2. PC -> 아두이노 -> Nextion (심플 테스트)
    if (Serial.available())
    {
        char c = Serial.read();
        nextionTest.write(c);
    }
}
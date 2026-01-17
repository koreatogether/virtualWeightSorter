#include <math.h> // pow 및 log10 함수 사용을 위한 math 라이브러리

const int analogPin = A0; // MQ-7 센서가 연결된 아날로그 핀
const float RL = 10.0;    // 브라켓 보드의 부하 저항 (kΩ 단위) - 실제 값으로 변경 필요
const float R0 = 27.0;    // 깨끗한 공기 중 센서 저항 (kΩ 단위) - 실제 측정 또는 데이터시트 값으로 변경 필요

// CO 농도를 계산하는 함수
float readCOConcentration()
{
    int analogValue = analogRead(analogPin);
    float voltage = (analogValue * 5.0) / 1023.0;
    float Rs = ((5.0 * RL) / voltage) - RL;
    float ratio = Rs / R0;
    float ppm = pow(10, (2.8 - log10(ratio)));

    return ppm;
}

void setup()
{
    Serial.begin(9600); // 시리얼 통신 초기화
    delay(20000);       // 센서 워밍업 시간 (데이터시트에 따라 조절)
    Serial.println("MQ-7 센서 초기화 완료. CO 농도 측정을 시작합니다.");
}

void loop()
{
    // 각 단계별 값 계산
    int analogValue = analogRead(analogPin);
    float voltage = (analogValue * 5.0) / 1023.0;
    float Rs = ((5.0 * RL) / voltage) - RL;
    float ratio = Rs / R0;
    float ppm = readCOConcentration();

    // 모든 계산값 출력
    Serial.print("ADC 값: ");
    Serial.print(analogValue);
    Serial.print(" | 전압(V): ");
    Serial.print(voltage);
    Serial.print(" | Rs(kΩ): ");
    Serial.print(Rs);
    Serial.print(" | Rs/R0: ");
    Serial.print(ratio);
    Serial.print(" | PPM: ");
    Serial.println(ppm);

    delay(1000);
}
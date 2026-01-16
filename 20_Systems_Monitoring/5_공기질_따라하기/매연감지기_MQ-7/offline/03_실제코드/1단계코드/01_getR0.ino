const int sensorPin = A0;       // MQ-7 센서가 연결된 핀
const int RL = 10;              // 로드 저항 (kΩ)
const float Vc = 5.0;           // 입력 전압 (V)
const int sampleTimes = 50;     // 샘플링 횟수
const int sampleInterval = 500; // 샘플링 간격 (ms)

float calculateRs(int adcValue)
{
    float voltage = (adcValue / 1024.0) * Vc; // ADC 값을 전압으로 변환
    float Rs = (Vc - voltage) / voltage * RL; // Rs 계산
    return Rs;
}

void setup()
{
    Serial.begin(9600);
    pinMode(sensorPin, INPUT);
}

void loop()
{
    float Rs_sum = 0.0;

    // 여러 번 샘플링하여 평균 Rs 값을 계산
    for (int i = 0; i < sampleTimes; i++)
    {
        int adcValue = analogRead(sensorPin); // ADC 값 읽기
        float Rs = calculateRs(adcValue);
        Rs_sum += Rs;
        delay(sampleInterval);
    }

    float R0 = Rs_sum / sampleTimes; // 평균 Rs 값을 R0로 설정
    Serial.print("R0 값: ");
    Serial.println(R0);

    // 이후 R0 값을 저장하거나 다른 용도로 사용
    // 예: EEPROM에 저장 등
}
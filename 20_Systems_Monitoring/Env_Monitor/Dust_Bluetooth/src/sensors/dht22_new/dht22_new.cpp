#include "dht22_new.h"
#include "dht22_new_validation.h"

// DHT22 센서 객체 정의
DHT dht22_sensor(DHT22_PIN, DHT22_TYPE);

// DHT22 센서 초기화 함수
void init_dht22_sensor()
{
    // DHT22 센서 초기화
    dht22_sensor.begin();

    Serial.println("DHT22 센서 초기화 완료");
    delay(2000); // 센서 안정화 시간
}

// DHT22 센서 메인 루프 처리 함수
void dht22_loop()
{
    static unsigned long lastReadTime = 0;

    // millis()를 사용한 타이밍 체크 (논블로킹)
    if (millis() - lastReadTime >= DHT22_READ_INTERVAL_MS)
    {
        lastReadTime = millis();

        // DHT22 센서 데이터 읽기
        float humidity = dht22_sensor.readHumidity();
        float temperature = dht22_sensor.readTemperature();

        // 데이터 유효성 확인
        if (isnan(humidity) || isnan(temperature))
        {
            Serial.println("DHT22 센서에서 데이터를 읽을 수 없습니다!");
            return;
        }

        if (!isDht22ReadingValid(humidity, temperature))
        {
            Serial.println("DHT22 센서 데이터가 허용 범위를 벗어났습니다!");
            Serial.print("습도: ");
            Serial.print(humidity);
            Serial.print(" %\t");
            Serial.print("온도: ");
            Serial.print(temperature);
            Serial.println(" °C");
            return;
        }

        // 데이터 출력
        Serial.print("습도: ");
        Serial.print(humidity);
        Serial.print(" %\t");
        Serial.print("온도: ");
        Serial.print(temperature);
        Serial.println(" °C");
    }
}
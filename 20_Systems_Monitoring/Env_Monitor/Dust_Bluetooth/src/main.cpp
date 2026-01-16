// 라이브러리 불러오기
#include <Arduino.h>
#include "sensors/dht22_new/dht22_new.h"
#include "sensors/pms7003_new/pms7003_new.h"

// setup 함수
void setup()
{
    // 시리얼 통신 초기화
    Serial.begin(9600);
    Serial.println("DHT22 및 PMS7003 센서 테스트 시작");

    // DHT22 센서 초기화
    init_dht22_sensor();

    // PMS7003 센서 초기화
    init_pms7003_sensor();
}

// loop 함수
void loop()
{
    // DHT22 센서 처리
    dht22_loop();

    // PMS7003 센서 처리
    pms7003_loop();
}
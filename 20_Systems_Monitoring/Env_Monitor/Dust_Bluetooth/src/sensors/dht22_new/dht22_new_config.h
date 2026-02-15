#ifndef DHT22_NEW_CONFIG_H
#define DHT22_NEW_CONFIG_H

// DHT22 센서 설정 값들 (Adafruit DHT 라이브러리용)
#define DHT22_READ_INTERVAL_MS 2000 // 센서 읽기 간격 (2초)
#define DHT22_TIMEOUT_MS 1000       // 센서 타임아웃 시간
#define DHT22_MAX_RETRY_COUNT 3     // 최대 재시도 횟수

#define DHT22_MIN_TEMPERATURE_C -40.0f
#define DHT22_MAX_TEMPERATURE_C 80.0f
#define DHT22_MIN_HUMIDITY 0.0f
#define DHT22_MAX_HUMIDITY 100.0f

// DHT22 센서 핀 설정
#define DHT22_PIN 4      // DHT22 데이터 핀
#define DHT22_TYPE DHT22 // DHT22 센서 타입

// DHT22 센서 타입 설정
#define DHT22_SENSOR_TYPE 22

#endif // DHT22_NEW_CONFIG_H
#ifndef PMS7003_NEW_CONFIG_H
#define PMS7003_NEW_CONFIG_H

// PMS7003 센서 설정 값들 (PMS 라이브러리용)
#define PMS7003_READ_INTERVAL_MS 20000 // 센서 읽기 간격 (20초) - DHT22 10번 측정 기간
#define PMS7003_TIMEOUT_MS 1000        // 센서 타임아웃 시간
#define PMS7003_MAX_RETRY_COUNT 3      // 최대 재시도 횟수

// PMS7003 센서 UART 설정
#define PMS7003_BAUD_RATE 9600 // UART 통신 속도

// PMS7003 센서 타입 설정
#define PMS7003_SENSOR_TYPE PMS7003

#endif // PMS7003_NEW_CONFIG_H
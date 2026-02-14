#ifndef CONFIG_H
#define CONFIG_H

// DS18B20 Sensor Configuration
const int ONE_WIRE_BUS = 2; // DS18B20 센서가 연결된 핀
const int SENSOR_COUNT = 8; // 관리할 총 센서 수

// Temperature Thresholds
const float UPPER_TEMP_LIMIT = 30.0; // 온도 상한 임계값
const float LOWER_TEMP_LIMIT = 10.0; // 온도 하한 임계값

// Serial Communication
const long SERIAL_BAUD_RATE = 9600; // 시리얼 통신 속도

// Timing Intervals (milliseconds)
const unsigned long SENSOR_READ_INTERVAL = 15000;  // 센서 테이블 출력 주기 (15초)
const unsigned long UI_TIMEOUT = 60000;            // 사용자 입력 타임아웃 (60초)
const unsigned long MENU_REPRINT_INTERVAL = 15000; // 메뉴 재출력 주기 (15초)

#endif // CONFIG_H

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// 핀 설정
const uint8_t ONE_WIRE_BUS = 2;       // DS18B20 데이터 핀
const uint8_t LCD_I2C_ADDRESS = 0x27; // LCD I2C 주소

// 시스템 설정
const uint8_t MAX_SENSORS = 4;                         // 최대 센서 수
const unsigned long SAMPLE_INTERVAL_MS = 2000;         // 측정 주기
const unsigned long DISPLAY_ROTATE_MS = 7000;          // 화면 전환 주기
const unsigned long MISSING_SENSOR_REMINDER_MS = 5000; // 센서 미검출 알림 간격

#endif // CONFIG_H

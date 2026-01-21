#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- 하드웨어 핀 설정 ---
constexpr uint8_t ONE_WIRE_BUS = 2;       // DS18B20 데이터 핀
constexpr uint8_t LCD_I2C_ADDRESS = 0x27; // LCD I2C 주소
constexpr uint8_t BUTTON_PIN = 3;         // 화면 전환 버튼 핀 (Internal Pull-up 사용)

// --- 시스템 및 타이머 설정 ---
constexpr uint8_t MAX_SENSORS = 4;                         // 최대 센서 수
constexpr uint8_t MAX_PAGES = 4;                           // 전체 페이지 수
constexpr unsigned long SAMPLE_INTERVAL_MS = 2000;         // 측정 주기
constexpr unsigned long DEBOUNCE_DELAY_MS = 50;            // 버튼 디바운스 시간
constexpr unsigned long MISSING_SENSOR_REMINDER_MS = 5000; // 센서 미검출 알림 간격

#endif // CONFIG_H

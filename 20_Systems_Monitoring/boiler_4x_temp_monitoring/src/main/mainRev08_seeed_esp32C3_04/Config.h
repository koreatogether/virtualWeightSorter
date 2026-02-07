#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <HardwareSerial.h>

// --- 하드웨어 핀 및 시리얼 설정 ---
constexpr uint8_t ONE_WIRE_BUS = 4; // XIAO D2 (GPIO 4) - DS18B20 데이타 핀

#define RX1_PIN 7 // XIAO D5 (GPIO 7)
#define TX1_PIN 6 // XIAO D4 (GPIO 6)

// 하드웨어 시리얼 1 인스턴스 (Nextion HMI용)
extern HardwareSerial MySerial1;
#define HMISerial MySerial1

#define DEBUG_ENABLE 1 // PC 디버깅 켜기

// --- 센서 및 시스템 설정 ---
constexpr uint8_t MAX_SENSORS = 4;
constexpr unsigned long SAMPLE_INTERVAL_MS = 2000;
constexpr unsigned long DEBOUNCE_DELAY_MS = 50;

// --- Nextion 설정 ---
const int WAVEFORM_ID = 11;
const int WAVEFORM_HEIGHT = 141;
const int WAVEFORM_WIDTH = 300;  // 한번에 표시 가능한 X축 포인트 수
const int HISTORY_SAMPLES = 120; // 버튼 클릭 시 재생할 과거 데이터 수 (4분)
const int TEMP_MIN = 0;
const int TEMP_MAX = 100;
const int LINE_STEPS = 5;
const int SEND_DELAY_MS = 1;

// --- 웹 대시보드 설정 ---
#define DASHBOARD_PASS "1234" // 대시보드 접속 비밀번호

#endif // CONFIG_H

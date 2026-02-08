#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <HardwareSerial.h>

// --- 하드웨어 핀 및 시리얼 설정 ---
constexpr uint8_t ONE_WIRE_BUS = 4; // XIAO D2 (GPIO 4) - DS18B20 데이터 핀

constexpr uint8_t RX1_PIN = 7;      // XIAO D5 (GPIO 7)
constexpr uint8_t TX1_PIN = 6;      // XIAO D4 (GPIO 6)

// 하드웨어 시리얼 1 인스턴스 (Nextion HMI용)
extern HardwareSerial MySerial1;
#define HMISerial MySerial1

constexpr bool DEBUG_ENABLE = true; // PC 디버깅 활성화

// --- 센서 및 시스템 설정 ---
constexpr uint8_t MAX_SENSORS = 4;
constexpr uint32_t SAMPLE_INTERVAL_MS = 2000;
constexpr uint32_t DEBOUNCE_DELAY_MS = 50;

// --- Nextion 설정 ---
constexpr int WAVEFORM_ID = 11;
constexpr int WAVEFORM_HEIGHT = 141;
constexpr int WAVEFORM_WIDTH = 300;  // 한 번에 표시 가능한 X축 포인트 수
constexpr int HISTORY_SAMPLES = 120; // 버튼 클릭 시 재생할 과거 데이터 수 (4분)
constexpr int TEMP_MIN = 0;
constexpr int TEMP_MAX = 100;
constexpr int LINE_STEPS = 5;
constexpr int SEND_DELAY_MS = 1;

// --- 웹 대시보드 설정 ---
constexpr const char* DASHBOARD_PASS = "1234"; // 대시보드 접속 비밀번호

#endif // CONFIG_H

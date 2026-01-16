#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "application/SensorController.h"
#include "application/MenuController.h"

constexpr uint8_t ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long lastPrint = 0;

// 컨트롤러 인스턴스
SensorController sensorController;
MenuController menuController;
bool firstLoop = true;

void setup()
{
    Serial.begin(115200);
    delay(100); // 시리얼 안정화 대기
    
    Serial.println();
    Serial.println("=== DS18B20 시스템 시작 ===");
    Serial.println("1. 시리얼 통신 초기화 완료");
    
    setupSerialAndSensor();
    
    Serial.println("2. 센서 및 EEPROM 초기화 완료");
    
    // 추가 안전장치: setup 완료 후 1초 대기하여 시리얼 통신 안정화
    delay(500);
    
    // 강제로 Normal 상태 확인 및 설정
    menuController.resetToNormalState();
    
    Serial.println("3. 메뉴 컨트롤러 초기화 완료");
    Serial.println("=== 시스템 초기화 완료 ===");
    Serial.print("현재 AppState: ");
    Serial.println((int)menuController.getAppState());
    Serial.println("센서 제어 메뉴 진입: 'menu' 또는 'm' 입력");
    Serial.println();
}

void loop()
{
    menuController.handleSerialInput();
    unsigned long now = millis();
    if (menuController.getAppState() == AppState::Normal)
    {
        handleNormalState(now);
    }
}

void setupSerialAndSensor()
{
    Serial.print("Firmware build: ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);
    
    Serial.print("DS18B20 센서 초기화 중...");
    sensors.begin();
    Serial.println(" 완료");
    
    // EEPROM 임계값 초기화 (Serial 초기화 후에 실행)
    sensorController.initializeThresholds();
    
    // 명시적으로 Normal 상태로 초기화
    menuController.setAppState(AppState::Normal);
    Serial.println("시스템 상태: Normal 모드");
}

void handleNormalState(unsigned long now)
{
    if (firstLoop)
    {
        sensorController.printSensorStatusTable();
        lastPrint = now;
        firstLoop = false;
    }
    else if (now - lastPrint >= sensorController.getMeasurementInterval())
    {
        sensorController.printSensorStatusTable();
        lastPrint = now;
    }
}

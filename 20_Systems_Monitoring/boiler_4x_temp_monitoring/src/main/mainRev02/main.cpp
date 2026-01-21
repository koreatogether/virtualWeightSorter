#include "Config.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "BlynkManager.h"

unsigned long lastSampleMs = 0;
uint8_t currentDisplayMode = 0; // 0: Diff, 1: Individual

// 화면 모드 전환 함수
void rotateDisplayMode()
{
    currentDisplayMode = (currentDisplayMode + 1) % 2;
    lcd.clear(); // 전환 시 잔상 제거
}

// 데이터 갱신 및 전송
void updateAndSendData()
{
    if (foundSensors > 0)
    {
        readTemperatures();
        updateDisplay(currentDisplayMode);
        sendDataToBlynk(sensorTemps, foundSensors);
    }
}

void setup()
{
    Serial.begin(115200);

    initDisplay();
    initSensors();
    initBlynk();

    if (foundSensors == 0)
    {
        lcd.setCursor(0, 1);
        lcd.print("No DS18B20 found");
    }
    else
    {
        showSensorShortAddresses(4000);
    }

    // 2초마다 데이터 갱신 및 전송
    blynkSetInterval(SAMPLE_INTERVAL_MS, updateAndSendData);
    
    // 5초마다 화면 전환
    blynkSetInterval(DISPLAY_ROTATE_MS, rotateDisplayMode);
}

void loop()
{
    runBlynk();
}

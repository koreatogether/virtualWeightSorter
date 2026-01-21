#include "BlynkManager.h"
#include "blynkpassWord.h"
#include <WiFiS3.h>
#include <BlynkSimpleWiFi.h>
#include <DallasTemperature.h> // DEVICE_DISCONNECTED_C 사용을 위해

BlynkTimer timer;

void initBlynk()
{
    // Blynk 시작 (WiFi 연결 포함)
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void blynkSetInterval(unsigned long interval, void (*callback)())
{
    timer.setInterval(interval, callback);
}

void sendDataToBlynk(float *temps, uint8_t count)
{
    // Blynk 가상 핀(V1~V4)으로 온도 전송
    for (uint8_t i = 0; i < count; i++)
    {
        if (temps[i] != DEVICE_DISCONNECTED_C)
        {
            Blynk.virtualWrite(i + 1, temps[i]); // V1, V2, V3, V4
        }
    }
}

void runBlynk()
{
    Blynk.run();
    timer.run();
}

#include "WiFiManager.h"
#include "wifiCredentials.h"
#include <time.h>

WebServer server(80);

static unsigned long lastWiFiCheck = 0;
constexpr uint32_t WIFI_CHECK_INTERVAL = 10000; // 10초마다 체크

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    
    // 고정 IP 설정
    IPAddress local_IP(IP_ADDRESS);
    IPAddress gateway(IP_GATEWAY);
    IPAddress subnet(IP_SUBNET);
    IPAddress dns(8, 8, 8, 8);
    
    if (!WiFi.config(local_IP, gateway, subnet, dns)) 
    {
        Serial.println(F("[WiFi] 고정 IP 설정 실패"));
    }

    Serial.print(F("[WiFi] 연결 중: "));
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    // 초기 연결 대기 (최대 10초)
    uint8_t retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20)
    {
        delay(500);
        Serial.print(F("."));
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println(F("\n[WiFi] 연결 성공!"));
        Serial.print(F("[WiFi] IP 주소: "));
        Serial.println(WiFi.localIP());

        if (!MDNS.begin("boiler"))
        {
            Serial.println(F("[WiFi] mDNS 설정 오류"));
        }
        else
        {
            Serial.println(F("[WiFi] mDNS 서버 시작: http://boiler.local"));
            MDNS.addService("http", "tcp", 80);
        }

        configTime(9 * 3600, 0, "pool.ntp.org", "time.google.com");
        
        server.begin();
    }
    else 
    {
        Serial.println(F("\n[WiFi] 연결 실패 (나중에 백그라운드에서 재시도)"));
    }
}

void handleWiFi()
{
    unsigned long currentMillis = millis();

    // WiFi 연결 상태 주기적 확인 및 재연결
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL)
    {
        lastWiFiCheck = currentMillis;
        
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println(F("[WiFi] 연결 끊김, 재연결 시도 중..."));
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASS);
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        server.handleClient();
    }
}

String getLocalIP()
{
    return WiFi.localIP().toString();
}

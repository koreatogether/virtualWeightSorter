/**
 * @file WiFiManager.cpp
 * @brief WiFi 연결 및 웹 서버 객체 관리 구현
 */

#include "WiFiManager.h"
#include "wifiCredentials.h"

WiFiServer server(80);
WiFiUDP udp;
MDNS mdns(udp);

/**
 * @brief WiFi 모듈 초기화 및 네트워크 연결
 */
void initWiFi()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println(F("Communication with WiFi module failed!"));
        return;
    }

    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(WIFI_SSID);

    // 고정 IP 설정 적용
    IPAddress local_IP(IP_ADDRESS);
    IPAddress gateway(IP_GATEWAY);
    IPAddress subnet(IP_SUBNET);

    if (!WiFi.config(local_IP, gateway, subnet))
    {
        Serial.println(F("STA Failed to configure"));
    }

    uint8_t retryCount = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        // 연결 시도를 위해 5초 대기 (초기화 단계이므로 delay 사용 허용)
        delay(5000);
        Serial.print(F("."));

        retryCount++;
        if (retryCount > 10)
        {
            Serial.println(F("\nWiFi connection failed. Check credentials."));
            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println(F("\nConnected to WiFi"));

        // mDNS 시작 (boiler.local)
        if (!mdns.begin(WiFi.localIP(), "boiler"))
        {
            Serial.println(F("Error setting up mDNS responder!"));
        }
        else
        {
            Serial.println(F("mDNS responder started (boiler.local)"));
            mdns.addServiceRecord("boiler._http", 80, MDNSServiceTCP);
        }

        server.begin(); // 웹 서버 시작

        Serial.print(F("IP Address: "));
        Serial.println(WiFi.localIP());
    }
}

/**
 * @brief MDNS 갱신 처리
 */
void handleMDNS()
{
    mdns.run();
}

/**
 * @brief 현재 할당된 로컬 IP를 문자열로 반환
 */
String getLocalIP()
{
    IPAddress ip = WiFi.localIP();
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

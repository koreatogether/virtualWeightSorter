#include "WiFiManager.h"
#include "wifiCredentials.h"

WiFiServer server(80);

void initWiFi()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        return;
    }

    // 테스트를 위해 고정 IP 설정을 잠시 주석 처리 (공유기 자동 할당 확인)
    /*
    IPAddress local_IP(IP_ADDRESS);
    IPAddress gateway(IP_GATEWAY);
    IPAddress subnet(IP_SUBNET);
    WiFi.config(local_IP, gateway, subnet);
    */

    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        delay(5000);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi");
    server.begin(); // 웹 서버 시작
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

String getLocalIP()
{
    IPAddress ip = WiFi.localIP();
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

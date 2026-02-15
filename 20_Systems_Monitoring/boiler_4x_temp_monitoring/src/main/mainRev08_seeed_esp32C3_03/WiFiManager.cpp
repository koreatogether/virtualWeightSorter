#include "WiFiManager.h"
#include "wifiCredentials.h"
#include <time.h>

WebServer server(80);

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    
    // IP 설정
    IPAddress local_IP(IP_ADDRESS);
    IPAddress gateway(IP_GATEWAY);
    IPAddress subnet(IP_SUBNET);
    IPAddress dns(8, 8, 8, 8);
    
    if (!WiFi.config(local_IP, gateway, subnet, dns)) {
        Serial.println("STA Failed to configure");
    }

    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    uint8_t retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20)
    {
        delay(500);
        Serial.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi Connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        if (!MDNS.begin("boiler"))
        {
            Serial.println("Error setting up MDNS responder!");
        }
        else
        {
            Serial.println("mDNS responder started: http://boiler.local");
            MDNS.addService("http", "tcp", 80);
        }

        // --- NTP 시간 동기화 (ESP32 내장 방식) ---
        configTime(9 * 3600, 0, "pool.ntp.org", "time.google.com");
        Serial.println("NTP Sync Requested");
        
        server.begin();
    }
    else {
        Serial.println("\nWiFi Connection Failed");
    }
}

void handleWiFi()
{
    server.handleClient();
}

String getLocalIP()
{
    return WiFi.localIP().toString();
}

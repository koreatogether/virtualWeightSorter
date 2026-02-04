#include "WiFiManager.h"
#include "wifiCredentials.h"
#include <RTC.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

WiFiServer server(80);
WiFiUDP mdnsUdp;
MDNS mdns(mdnsUdp);

void initWiFi()
{
    // RTC 초기화 (시간 동기화를 위해 필수)
    if (!RTC.begin())
    {
        Serial.println(F("RTC initialization failed!"));
    }

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println(F("WiFi module failed!"));
        return;
    }

    IPAddress local_IP(IP_ADDRESS);
    IPAddress gateway(IP_GATEWAY);
    IPAddress subnet(IP_SUBNET);
    IPAddress dns(8, 8, 8, 8); // 구글 DNS
    
    // UNO R4 WiFi (WiFiS3) config 순서: IP, DNS, Gateway, Subnet
    WiFi.config(local_IP, dns, gateway, subnet);

    Serial.print(F("Connecting to "));
    Serial.println(WIFI_SSID);

    uint8_t retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 3)
    {
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        delay(2000);
        Serial.print(F("."));
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println(F("\nWiFi Connected"));
        if (!mdns.begin(WiFi.localIP(), "boiler"))
        {
            Serial.println(F("mDNS failed!"));
        }
        else
        {
            mdns.addServiceRecord("boiler._http", 80, MDNSServiceTCP);
        }
        server.begin();

        // 네트워크 안정화를 위한 대기
        delay(1000);

        // --- NTP 시간 동기화 시작 ---
        Serial.println(F("Requesting NTP time..."));

        // NTP 서버 목록 (Google Time, Pool NTP)
        const char *ntpServers[] = {"time.google.com", "pool.ntp.org"};
        const int NTP_PACKET_SIZE = 48;
        byte packetBuffer[NTP_PACKET_SIZE];
        WiFiUDP ntpUDP;

        ntpUDP.begin(2390); // 로컬 포트

        bool syncSuccess = false;

        for (int i = 0; i < 2; i++)
        {
            if (syncSuccess)
                break;

            Serial.print(F("Trying "));
            Serial.print(ntpServers[i]);
            Serial.println(F("..."));

            // NTP 패킷 구성
            memset(packetBuffer, 0, NTP_PACKET_SIZE);
            packetBuffer[0] = 0b11100011; // LI, Version, Mode
            packetBuffer[1] = 0;          // Stratum
            packetBuffer[2] = 6;          // Polling Interval
            packetBuffer[3] = 0xEC;       // Precision
            packetBuffer[12] = 49;
            packetBuffer[13] = 0x4E;
            packetBuffer[14] = 49;
            packetBuffer[15] = 52;

            if (ntpUDP.beginPacket(ntpServers[i], 123))
            {
                ntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
                ntpUDP.endPacket();

                unsigned long startMs = millis();
                while (millis() - startMs < 3000)
                { // 3초 대기
                    if (ntpUDP.parsePacket())
                    {
                        ntpUDP.read(packetBuffer, NTP_PACKET_SIZE);

                        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
                        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
                        unsigned long secsSince1900 = highWord << 16 | lowWord;
                        const unsigned long seventyYears = 2208988800UL;
                        unsigned long unixEpoch = secsSince1900 - seventyYears;

                        unixEpoch += 32400; // KST (+9h)

                        RTCTime timeToSet(unixEpoch);
                        RTC.setTime(timeToSet);

                        RTCTime checkTime;
                        RTC.getTime(checkTime);
                        char timeStr[32];
                        snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                                 checkTime.getYear(), (int)checkTime.getMonth() + 1, checkTime.getDayOfMonth(),
                                 checkTime.getHour(), checkTime.getMinutes(), checkTime.getSeconds());
                        Serial.print(F("RTC Synced: "));
                        Serial.println(timeStr);

                        syncSuccess = true;
                        break;
                    }
                    delay(10);
                }
            }
            if (!syncSuccess)
            {
                Serial.println(F("NTP timeout or send failed."));
            }
        }

        if (!syncSuccess)
        {
            Serial.println(F("Failed to sync time from all servers."));
        }

        ntpUDP.stop();
    }
}

void handleMDNS()
{
    mdns.run();
}

String getLocalIP()
{
    return WiFi.localIP().toString();
}

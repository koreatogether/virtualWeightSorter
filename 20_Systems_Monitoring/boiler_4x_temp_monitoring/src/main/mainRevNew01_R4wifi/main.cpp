#include "Config.h"
#include "SensorManager.h"
#include "WiFiManager.h"
#include "NextionManager.h"
#include "Dashboard.h"

void handleWebServer()
{
    WiFiClient client = server.available();
    if (!client)
        return;

    String request = "";
    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            request += c;
            if (c == '\n')
            {
                if (request.indexOf("GET /data") >= 0)
                {
                    client.println("HTTP/1.1 200 OK\nContent-Type: application/json\nConnection: close\n");
                    client.print("{\"s1\":");
                    client.print(sensorTemps[0]);
                    client.print(",\"s2\":");
                    client.print(sensorTemps[1]);
                    client.print(",\"s3\":");
                    client.print(sensorTemps[2]);
                    client.print(",\"s4\":");
                    client.print(sensorTemps[3]);
                    client.println("}");
                }
                else if (request.indexOf("GET /login?pass=" + String(DASHBOARD_PASS)) >= 0)
                {
                    client.println("HTTP/1.1 200 OK\nSet-Cookie: auth=admin; Path=/; Max-Age=3600\nContent-Type: text/html\n\n<script>location.href='/';</script>");
                }
                else if (request.indexOf("Cookie: auth=admin") >= 0 || String(DASHBOARD_PASS) == "")
                {
                    client.println("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n");
                    client.println(INDEX_HTML);
                }
                else
                {
                    client.println("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n");
                    client.println(LOGIN_HTML);
                }
                break;
            }
        }
    }
    delay(5);
    client.stop();
}

void setup()
{
#if DEBUG_ENABLE
    Serial.begin(115200);
#endif
    initNextion();
    initSensors();

    // WiFi 연결 전에 현재 온도와 시계를 한 번 전송하여 화면이 나오게 함
    readTemperatures();
    updateNextionDisplay();

    initWiFi();
}

void loop()
{
    handleMDNS();
    handleWebServer();
    processNextionInput();

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= SAMPLE_INTERVAL_MS)
    {
        lastUpdate = millis();
        readTemperatures();
        updateNextionDisplay();
    }
}

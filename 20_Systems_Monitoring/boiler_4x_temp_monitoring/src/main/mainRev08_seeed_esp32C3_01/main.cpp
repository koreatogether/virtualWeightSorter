#include "Config.h"
#include "SensorManager.h"
#include "WiFiManager.h"
#include "NextionManager.h"
#include "Dashboard.h"

void handleWebServer()
{
    // server.handleClient() is called inside handleWiFi()
    // but we can add specific handlers here if they were missing
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

    // Web Server Handlers
    server.on("/data", HTTP_GET, []() {
        String json = "{\"s1\":" + String(sensorTemps[0]) + 
                      ",\"s2\":" + String(sensorTemps[1]) + 
                      ",\"s3\":" + String(sensorTemps[2]) + 
                      ",\"s4\":" + String(sensorTemps[3]) + "}";
        server.send(200, "application/json", json);
    });

    server.on("/login", HTTP_GET, []() {
        if (server.hasArg("pass") && server.arg("pass") == DASHBOARD_PASS) {
            server.sendHeader("Set-Cookie", "auth=admin; Path=/; Max-Age=3600");
            server.send(200, "text/html", "<script>location.href='/';</script>");
        } else {
            server.send(200, "text/html", LOGIN_HTML);
        }
    });

    server.on("/", HTTP_GET, []() {
        String cookie = server.header("Cookie");
        if (cookie.indexOf("auth=admin") >= 0 || String(DASHBOARD_PASS) == "") {
            server.send(200, "text/html", INDEX_HTML);
        } else {
            server.send(200, "text/html", LOGIN_HTML);
        }
    });
    
    // Enable cookie header parsing
    const char * headerKeys[] = {"Cookie"};
    size_t headerKeysSize = sizeof(headerKeys) / sizeof(char *);
    server.collectHeaders(headerKeys, headerKeysSize);
}

void loop()
{
    handleWiFi();
    processNextionInput();

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= SAMPLE_INTERVAL_MS)
    {
        lastUpdate = millis();
        readTemperatures();
        updateNextionDisplay();
    }
}

#include "Config.h"
#include "SensorManager.h"
#include "WiFiManager.h"
#include "NextionManager.h"
#include "Dashboard.h"

/**
 * @brief 웹 서버 핸들러 설정
 */
void setupWebServer()
{
    // 데이터 요청 API (JSON)
    server.on("/data", HTTP_GET, []()
    {
        char json[128];
        snprintf(json, sizeof(json), "{\"s1\":%.1f,\"s2\":%.1f,\"s3\":%.1f,\"s4\":%.1f}",
                 sensorTemps[0], sensorTemps[1], sensorTemps[2], sensorTemps[3]);
        server.send(200, F("application/json"), json);
    });

    // 로그인 처리
    server.on("/login", HTTP_GET, []()
    {
        if (server.hasArg("pass") && server.arg("pass") == DASHBOARD_PASS)
        {
            server.sendHeader(F("Set-Cookie"), F("auth=admin; Path=/; Max-Age=3600"));
            server.send(200, F("text/html"), F("<script>location.href='/';</script>"));
        }
        else
        {
            server.send(200, F("text/html"), LOGIN_HTML);
        }
    });

    // 메인 페이지
    server.on("/", HTTP_GET, []()
    {
        String cookie = server.header(F("Cookie"));
        if (cookie.indexOf(F("auth=admin")) >= 0 || String(DASHBOARD_PASS) == "")
        {
            server.send(200, F("text/html"), INDEX_HTML);
        }
        else
        {
            server.send(200, F("text/html"), LOGIN_HTML);
        }
    });

    // 쿠키 헤더 파싱 활성화
    const char *headerKeys[] = {"Cookie"};
    size_t headerKeysSize = sizeof(headerKeys) / sizeof(char *);
    server.collectHeaders(headerKeys, headerKeysSize);
}

void setup()
{
#if DEBUG_ENABLE
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("\n--- Boiler Monitoring System Start (XIAO ESP32-C3) ---"));
#endif

    // Nextion 초기화
    initNextion();
#if DEBUG_ENABLE
    Serial.println(F("[System] Nextion 매니저 초기화 완료"));
#endif

    // 센서 초기화
    initSensors();
#if DEBUG_ENABLE
    Serial.println(F("[System] 센서 초기화 완료"));
#endif

    // 초기 데이터 읽기 및 화면 표시
    readTemperatures();
    updateNextionDisplay();
#if DEBUG_ENABLE
    Serial.println(F("[System] 초기 데이터 전송 완료"));
#endif

    // WiFi 및 웹 서버 초기화
    initWiFi();
    setupWebServer();
#if DEBUG_ENABLE
    Serial.println(F("[System] WiFi 및 웹 서버 설정 완료"));
#endif
}

void loop()
{
    // 각 매니저의 업데이트 루프 호출
    handleWiFi();
    processNextionInput();
    updateSensors();

    // 새로운 센서 데이터가 있을 때만 디스플레이 업데이트
    if (isNewDataAvailable())
    {
        updateNextionDisplay();
    }
}

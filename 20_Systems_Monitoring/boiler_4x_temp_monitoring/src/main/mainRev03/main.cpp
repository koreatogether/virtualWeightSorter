#include "Config.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "WiFiManager.h"
#include "Dashboard.h"
#include "wifiCredentials.h"

// Non-blocking 타이머 변수
unsigned long lastSampleUpdateMs = 0;
unsigned long lastDisplayRotateMs = 0;
unsigned long lastHeartbeatMs = 0; // PC 데이터 수집기 확인용

uint8_t currentDisplayMode = 0; // 0: Diff, 1: Individual

// 웹 서버 요청 처리
void handleWebServer()
{
    WiFiClient client = server.available();
    if (client)
    {
        String currentLine = "";
        String request = "";
        bool isAuthenticated = false;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                request += c;

                // 쿠키를 통한 인증 상태 확인
                if (request.indexOf("Cookie: auth=" + String(DASHBOARD_PASS)) >= 0)
                {
                    isAuthenticated = true;
                }

                if (c == '\n')
                {
                    if (currentLine.length() == 0)
                    {
                        // 1. 비밀번호 확인 요청 (/login?pass=1234)
                        if (request.indexOf("GET /login?pass=" + String(DASHBOARD_PASS)) >= 0)
                        {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Set-Cookie: auth=" + String(DASHBOARD_PASS) + "; Path=/; Max-Age=3600");
                            client.println("Content-Type: text/html");
                            client.println();
                            client.println("<script>location.href='/';</script>");
                        }
                        // 2. 하트비트 요청 (PC 수집기 확인용)
                        else if (request.indexOf("GET /heartbeat") >= 0)
                        {
                            lastHeartbeatMs = millis();
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-Type: text/plain");
                            client.println("Connection: close");
                            client.println();
                            client.println("OK");
                        }
                        // 3. 데이터 요청 처리 (인증된 경우만)
                        else if (request.indexOf("GET /data") >= 0)
                        {
                            if (isAuthenticated)
                            {
                                bool isLogging = (millis() - lastHeartbeatMs < 15000);
                                client.println("HTTP/1.1 200 OK");
                                client.println("Content-Type: application/json");
                                client.println("Connection: close");
                                client.println();
                                client.print("{\"s1\":");
                                client.print(sensorTemps[0]);
                                client.print(",\"s2\":");
                                client.print(sensorTemps[1]);
                                client.print(",\"s3\":");
                                client.print(sensorTemps[2]);
                                client.print(",\"s4\":");
                                client.print(sensorTemps[3]);
                                client.print(",\"logging\":");
                                client.print(isLogging ? "true" : "false");
                                client.println("}");
                            }
                            else
                            {
                                client.println("HTTP/1.1 401 Unauthorized");
                                client.println();
                            }
                        }
                        // 3. 페이지 요청 처리 (인증 여부에 따라 대시보드 또는 로그인창)
                        else
                        {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-Type: text/html");
                            client.println("Connection: close");
                            client.println();
                            if (isAuthenticated)
                            {
                                client.println(INDEX_HTML);
                            }
                            else
                            {
                                client.println(LOGIN_HTML);
                            }
                        }
                        break;
                    }
                    else
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    currentLine += c;
                }
            }
        }
        delay(5);
        client.stop();
    }
}

void setup()
{
    Serial.begin(115200);

    initDisplay();
    initSensors();

    // 로컬 웹 서버 준비를 위한 WiFi 연결
    initWiFi();

    if (foundSensors == 0)
    {
        lcd.setCursor(0, 1);
        lcd.print("No DS18B20 found");
    }
    else
    {
        showSensorShortAddresses(4000);
        // LCD에 IP 주소 표시
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Web Dashboard:");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(3000);
    }
}

void loop()
{
    unsigned long currentMillis = millis();

    // 웹 서버 요청 상시 대기
    handleWebServer();

    // 2초마다 센서 데이터 갱신
    if (currentMillis - lastSampleUpdateMs >= SAMPLE_INTERVAL_MS)
    {
        lastSampleUpdateMs = currentMillis;
        if (foundSensors > 0)
        {
            readTemperatures();
            updateDisplay(currentDisplayMode);
        }
    }

    // 5초마다 화면 전환
    if (currentMillis - lastDisplayRotateMs >= DISPLAY_ROTATE_MS)
    {
        lastDisplayRotateMs = currentMillis;
        currentDisplayMode = (currentDisplayMode + 1) % 2;
        lcd.clear();
        updateDisplay(currentDisplayMode);
    }
}

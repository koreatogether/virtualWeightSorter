/**
 * @file main.cpp
 * @brief 보일러 4구 온도 모니터링 시스템 메인 로직 (Rev04)
 *
 * 핀 연결:
 * - DS18B20: Pin 2 (One-Wire)
 * - LCD: I2C (SDA/SCL, Address 0x27)
 * - Button: Pin 3 (GND 연결, Internal Pull-up)
 *
 * 주요 기능:
 * - 4개 센서 온도 측정 및 LCD 표시
 * - 버튼을 통한 4개 화면 페이지 전환 (디바운싱 적용)
 * - 로컬 웹 서버 대시보드 제공
 */

#include "Config.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "WiFiManager.h"
#include "Dashboard.h"
#include "wifiCredentials.h"

// --- 전역 변수 및 타이머 ---
static unsigned long lastSampleUpdateMs = 0;
unsigned long lastHeartbeatMs = 0; // PC 데이터 수집기 확인용 (DisplayManager에서 extern으로 참조)

// 버튼 및 페이지 상태 관리
static uint8_t currentDisplayMode = 0; // 0: Diff, 1: Individual, 2: Empty, 3: Empty
static bool lastButtonState = HIGH;
static bool buttonState = HIGH;
static unsigned long lastDebounceTime = 0;

/**
 * @brief 클라이언트 요청 별 응답 처리
 */
static void sendWebResponse(WiFiClient &client, String &request, bool isAuthenticated)
{
    // 1. 비밀번호 확인 요청 (/login?pass=1234)
    if (request.indexOf("GET /login?pass=" + String(DASHBOARD_PASS)) >= 0)
    {
        client.println(F("HTTP/1.1 200 OK"));
        client.println("Set-Cookie: auth=" + String(DASHBOARD_PASS) + "; Path=/; Max-Age=3600");
        client.println(F("Content-Type: text/html"));
        client.println();
        client.println(F("<script>location.href='/';</script>"));
        return;
    }

    // 2. 하트비트 요청 (PC 수집기 확인용)
    if (request.indexOf("GET /heartbeat") >= 0)
    {
        lastHeartbeatMs = millis();
        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-Type: text/plain"));
        client.println(F("Connection: close"));
        client.println();
        client.println(F("OK"));
        return;
    }

    // 3. 데이터 요청 처리 (인증된 경우만)
    if (request.indexOf("GET /data") >= 0)
    {
        if (isAuthenticated)
        {
            bool isLogging = (millis() - lastHeartbeatMs < 15000);
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: application/json"));
            client.println(F("Connection: close"));
            client.println();
            client.print(F("{\"s1\":"));
            client.print(sensorTemps[0]);
            client.print(F(",\"s2\":"));
            client.print(sensorTemps[1]);
            client.print(F(",\"s3\":"));
            client.print(sensorTemps[2]);
            client.print(F(",\"s4\":"));
            client.print(sensorTemps[3]);
            client.print(F(",\"logging\":"));
            client.print(isLogging ? "true" : "false");
            client.println(F("}"));
        }
        else
        {
            client.println(F("HTTP/1.1 401 Unauthorized"));
            client.println();
        }
        return;
    }

    // 4. 페이지 요청 처리 (인증 여부에 따라 대시보드 또는 로그인창)
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: text/html"));
    client.println(F("Connection: close"));
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

/**
 * @brief 웹 서버 요청 및 하트비트 처리
 */
void handleWebServer()
{
    WiFiClient client = server.available();
    if (!client)
    {
        return;
    }

    String currentLine = "";
    String request = "";
    bool isAuthenticated = false;

    while (client.connected())
    {
        if (!client.available())
        {
            continue;
        }

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
                sendWebResponse(client, request, isAuthenticated);
                break;
            }
            currentLine = "";
        }
        else if (c != '\r')
        {
            currentLine += c;
        }
    }

    delay(5);
    client.stop();
}

/**
 * @brief 시스템 로직 업데이트 프로세스
 */
void processSystemLogic(unsigned long currentMillis)
{
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
}

/**
 * @brief 버튼 입력을 통한 페이지 전환 처리
 */
void handleButtonInput(unsigned long currentMillis)
{
    bool reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState)
    {
        lastDebounceTime = currentMillis;
    }

    if ((currentMillis - lastDebounceTime) > DEBOUNCE_DELAY_MS)
    {
        if (reading != buttonState)
        {
            buttonState = reading;

            // 버튼이 눌렸을 때 (PULLUP이므로 LOW)
            if (buttonState == LOW)
            {
                currentDisplayMode = (currentDisplayMode + 1) % MAX_PAGES;
                lcd.clear();
                updateDisplay(currentDisplayMode);

                Serial.print(F("Page changed to: "));
                Serial.println(currentDisplayMode + 1);
            }
        }
    }
    lastButtonState = reading;
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    initDisplay();
    initSensors();

    // 로컬 웹 서버 준비를 위한 WiFi 연결
    initWiFi();

    if (foundSensors == 0)
    {
        lcd.setCursor(0, 1);
        lcd.print(F("No DS18B20 found"));
    }
    else
    {
        showSensorShortAddresses(4000);

        // LCD에 IP 주소 표시
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Web Dashboard:"));
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(3000);
    }
}

void loop()
{
    unsigned long currentMillis = millis();

    // 네트워크 상시 처리
    handleMDNS(); // mDNS 갱신
    handleWebServer();

    // 버튼 입력 처리
    handleButtonInput(currentMillis);

    // 센서 측정 및 화면 업데이트
    processSystemLogic(currentMillis);
}

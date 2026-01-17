/**
 * @file sleepMode_tftLCDgraphic.ino
 * @brief PMS7003 TFT LCD GUI with Touch Control
 *
 * Hardware:
 * - MCU: Arduino Uno R4 WiFi
 * - Display: 2.4" TFT LCD Shield (ILI9341 320x240) with Touch
 * - Sensor: PMS7003 (Serial1)
 *
 * Libraries Required:
 * - Adafruit_GFX
 * - MCUFRIEND_kbv
 * - TouchScreen
 *
 * Features:
 * - 탭 인터페이스 (모니터링/설정)
 * - 실시간 막대 그래프 (PM1.0, PM2.5, PM10)
 * - 통계 카운터 (주황/빨강 횟수)
 * - 상태 표시 (SLEEP/ACTIVE 모드)
 * - 카운트다운 타이머
 * - 터치스크린 조작
 */

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// -----------------------------------------------------------------------------
// 1. Configuration & Constants
// -----------------------------------------------------------------------------
namespace Config
{
    // Serial
    constexpr uint32_t SERIAL_DEBUG_BAUD = 115200;
    constexpr uint32_t PMS_BAUD = 9600;

    // Touch Screen Pins (쉴드형 TFT LCD)
    constexpr uint8_t YP = A1; // must be analog pin
    constexpr uint8_t XM = A2; // must be analog pin
    constexpr uint8_t YM = 7;  // digital pin
    constexpr uint8_t XP = 6;  // digital pin

    // Touch Screen Calibration (값은 보드마다 다를 수 있음)
    constexpr int TS_LEFT = 907;
    constexpr int TS_RIGHT = 136;
    constexpr int TS_TOP = 942;
    constexpr int TS_BOTTOM = 139;
    constexpr int TS_MINPRESSURE = 200;
    constexpr int TS_MAXPRESSURE = 1000;

    // Screen Dimensions (Portrait Mode)
    constexpr int SCREEN_WIDTH = 240;
    constexpr int SCREEN_HEIGHT = 320;

    // Timing
    constexpr unsigned long WAKE_DURATION = 30000;        // 측정 시간 (30초)
    constexpr unsigned long DEFAULT_SLEEP_TIME = 60000;   // 기본 휴식 시간 (1분)
    constexpr unsigned long DATA_PRINT_INTERVAL = 2000;   // 데이터 출력 주기 (2초)
    constexpr unsigned long SCREEN_UPDATE_INTERVAL = 500; // 화면 갱신 주기 (0.5초)
    constexpr unsigned long TOUCH_DEBOUNCE = 200;         // 터치 디바운스 (ms)
}

// 색상 정의 (RGB565)
namespace Color
{
    constexpr uint16_t BLACK = 0x0000;
    constexpr uint16_t WHITE = 0xFFFF;
    constexpr uint16_t GREY = 0x8410;
    constexpr uint16_t DARK_GREY = 0x3186;
    constexpr uint16_t RED = 0xF800;
    constexpr uint16_t GREEN = 0x07E0;
    constexpr uint16_t BLUE = 0x001F;
    constexpr uint16_t YELLOW = 0xFFE0;
    constexpr uint16_t ORANGE = 0xFD20;
    constexpr uint16_t CYAN = 0x07FF;
    constexpr uint16_t MAGENTA = 0xF81F;

    // Dark Theme Colors (Processing GUI와 유사)
    constexpr uint16_t BG = 0x2082;         // #202020
    constexpr uint16_t PANEL = 0x3186;      // #303030
    constexpr uint16_t TEXT = 0xEF5D;       // #EEEEEE
    constexpr uint16_t ACCENT = 0xFA80;     // #FF5722
    constexpr uint16_t BTN_NORMAL = 0x4208; // #404040
    constexpr uint16_t BTN_HOVER = 0x52AA;  // #505050
}

// -----------------------------------------------------------------------------
// 2. Global Objects
// -----------------------------------------------------------------------------
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(Config::XP, Config::YP, Config::XM, Config::YM, 300);

// -----------------------------------------------------------------------------
// 3. State Management
// -----------------------------------------------------------------------------
enum class AppTab
{
    MONITOR = 0,
    SETTINGS = 1
};

struct PmsData
{
    uint16_t pm1_0 = 0;
    uint16_t pm2_5 = 0;
    uint16_t pm10 = 0;
    uint32_t countOrange = 0; // PM2.5 > 35
    uint32_t countRed = 0;    // PM2.5 > 75
};

struct SystemState
{
    AppTab currentTab = AppTab::MONITOR;
    PmsData pms;
    bool isSensorSleeping = false;
    unsigned long sleepInterval = Config::DEFAULT_SLEEP_TIME;
    unsigned long lastSensorWakeTime = 0;
    unsigned long lastSensorSleepTime = 0;
    unsigned long lastScreenUpdate = 0;
    unsigned long lastTouchTime = 0;

    // Settings Menu
    int settingsIndex = 0;
    const unsigned long intervals[5] = {60000, 180000, 300000, 600000, 1200000};
    const char *intervalLabels[5] = {"1 Min", "3 Min", "5 Min", "10 Min", "20 Min"};
};

SystemState sys;

// 화면 갱신 플래그 (부분 갱신 최적화)
bool needsFullRedraw = true;
bool needsDataRedraw = false;
uint16_t prevPm1 = 0, prevPm25 = 0, prevPm10 = 0;
int prevCountdown = -1;
bool prevSleepState = false;

// -----------------------------------------------------------------------------
// 4. PMS7003 Driver
// -----------------------------------------------------------------------------
namespace PmsDriver
{
    const uint8_t CMD_SLEEP[] = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73};
    const uint8_t CMD_WAKEUP[] = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74};
    const uint8_t CMD_PASSIVE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
    const uint8_t CMD_ACTIVE[] = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71};

    void sendCmd(const uint8_t *cmd, size_t len)
    {
        Serial1.write(cmd, len);
        delay(50);
    }

    void sleep()
    {
        sendCmd(CMD_PASSIVE, 7);
        delay(50);
        sendCmd(CMD_SLEEP, 7);
        sys.isSensorSleeping = true;
        sys.lastSensorSleepTime = millis();
        Serial.println(F(">>> 센서 절전 모드 진입"));
    }

    void wake()
    {
        sendCmd(CMD_WAKEUP, 7);
        sendCmd(CMD_ACTIVE, 7);
        sys.isSensorSleeping = false;
        sys.lastSensorWakeTime = millis();
        Serial.println(F(">>> 센서 깨우기"));
    }

    void update()
    {
        if (Serial1.available() >= 32)
        {
            if (Serial1.peek() != 0x42)
            {
                Serial1.read();
                return;
            }

            uint8_t buf[32];
            Serial1.readBytes(buf, 32);

            if (buf[0] == 0x42 && buf[1] == 0x4D)
            {
                uint16_t sum = 0;
                for (int i = 0; i < 30; i++)
                    sum += buf[i];
                uint16_t check = (buf[30] << 8) | buf[31];

                if (sum == check)
                {
                    sys.pms.pm1_0 = (buf[10] << 8) | buf[11];
                    sys.pms.pm2_5 = (buf[12] << 8) | buf[13];
                    sys.pms.pm10 = (buf[14] << 8) | buf[15];

                    // 통계 업데이트
                    if (sys.pms.pm2_5 > 75)
                        sys.pms.countRed++;
                    else if (sys.pms.pm2_5 > 35)
                        sys.pms.countOrange++;

                    needsDataRedraw = true;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 5. Touch Handling
// -----------------------------------------------------------------------------
namespace Touch
{
    struct Point
    {
        int x;
        int y;
        bool valid;
    };

    Point getTouch()
    {
        Point result = {0, 0, false};

        TSPoint p = ts.getPoint();
        pinMode(Config::XM, OUTPUT);
        pinMode(Config::YP, OUTPUT);

        if (p.z > Config::TS_MINPRESSURE && p.z < Config::TS_MAXPRESSURE)
        {
            // Portrait 모드 좌표 변환
            result.x = map(p.x, Config::TS_LEFT, Config::TS_RIGHT, Config::SCREEN_WIDTH, 0);
            result.y = map(p.y, Config::TS_TOP, Config::TS_BOTTOM, 0, Config::SCREEN_HEIGHT);
            result.valid = true;
        }

        return result;
    }

    bool isInRect(int tx, int ty, int x, int y, int w, int h)
    {
        return (tx >= x && tx < x + w && ty >= y && ty < y + h);
    }
}

// -----------------------------------------------------------------------------
// 6. UI Rendering
// -----------------------------------------------------------------------------
namespace UI
{
    // 레이아웃 상수
    constexpr int TAB_HEIGHT = 40;
    constexpr int MARGIN = 10;
    constexpr int BAR_WIDTH = 50;
    constexpr int BAR_MAX_HEIGHT = 120;

    // PM 수치에 따른 색상 반환
    uint16_t getPmColor(uint16_t pm25)
    {
        if (pm25 > 75)
            return Color::RED;
        if (pm25 > 35)
            return Color::YELLOW;
        return Color::GREEN;
    }

    // 공기질 상태 문자열 반환
    const char *getPmStatus(uint16_t pm25)
    {
        if (pm25 > 75)
            return "Danger!";
        if (pm25 > 35)
            return "Bad";
        if (pm25 > 15)
            return "Normal";
        return "Good";
    }

    // 탭 그리기
    void drawTabs()
    {
        int tabWidth = Config::SCREEN_WIDTH / 2;

        // 모니터 탭
        tft.fillRect(0, 0, tabWidth, TAB_HEIGHT,
                     sys.currentTab == AppTab::MONITOR ? Color::PANEL : Color::BTN_NORMAL);
        tft.setTextColor(Color::TEXT);
        tft.setTextSize(1);
        tft.setCursor(15, 15);
        tft.print("Monitor");

        // 설정 탭
        tft.fillRect(tabWidth, 0, tabWidth, TAB_HEIGHT,
                     sys.currentTab == AppTab::SETTINGS ? Color::PANEL : Color::BTN_NORMAL);
        tft.setCursor(tabWidth + 15, 15);
        tft.print("Settings");

        // 활성 탭 언더라인
        if (sys.currentTab == AppTab::MONITOR)
        {
            tft.drawFastHLine(0, TAB_HEIGHT - 2, tabWidth, Color::ACCENT);
        }
        else
        {
            tft.drawFastHLine(tabWidth, TAB_HEIGHT - 2, tabWidth, Color::ACCENT);
        }
    }

    // 상태 패널 그리기
    void drawStatusPanel()
    {
        int y = TAB_HEIGHT + MARGIN;

        // 패널 배경
        tft.fillRoundRect(MARGIN, y, Config::SCREEN_WIDTH - 2 * MARGIN, 50, 5, Color::PANEL);

        // 상태 텍스트
        tft.setTextColor(Color::TEXT);
        tft.setTextSize(1);
        tft.setCursor(MARGIN + 10, y + 8);
        tft.print("Status:");

        // 상태 표시
        if (sys.isSensorSleeping)
        {
            tft.setTextColor(Color::YELLOW);
            tft.setTextSize(2);
            tft.setCursor(MARGIN + 10, y + 22);
            tft.print("SLEEP");

            // 카운트다운 표시
            long remaining = (sys.sleepInterval - (millis() - sys.lastSensorSleepTime)) / 1000;
            if (remaining < 0)
                remaining = 0;

            tft.setTextSize(1);
            tft.setCursor(Config::SCREEN_WIDTH - 60, y + 28);
            tft.print(remaining);
            tft.print("s");
        }
        else
        {
            tft.setTextColor(Color::GREEN);
            tft.setTextSize(2);
            tft.setCursor(MARGIN + 10, y + 22);
            tft.print("ACTIVE");
        }
    }

    // PM2.5 대형 표시
    void drawPm25Large()
    {
        int y = TAB_HEIGHT + 70;

        // 패널 배경
        tft.fillRoundRect(MARGIN, y, Config::SCREEN_WIDTH - 2 * MARGIN, 70, 5, Color::PANEL);

        // PM2.5 수치 (크게)
        uint16_t pmColor = getPmColor(sys.pms.pm2_5);
        tft.setTextColor(pmColor);
        tft.setTextSize(4);
        tft.setCursor(MARGIN + 20, y + 15);
        tft.print(sys.pms.pm2_5);

        // 단위
        tft.setTextColor(Color::TEXT);
        tft.setTextSize(1);
        tft.setCursor(MARGIN + 100, y + 35);
        tft.print("ug/m3");

        // 상태 메시지
        tft.setTextColor(pmColor);
        tft.setTextSize(2);
        tft.setCursor(MARGIN + 130, y + 20);
        tft.print(getPmStatus(sys.pms.pm2_5));

        // PM2.5 라벨
        tft.setTextColor(Color::GREY);
        tft.setTextSize(1);
        tft.setCursor(MARGIN + 20, y + 55);
        tft.print("PM 2.5");
    }

    // 막대 그래프 그리기
    void drawBarGraph()
    {
        int graphY = TAB_HEIGHT + 150;
        int graphH = BAR_MAX_HEIGHT;
        int graphW = Config::SCREEN_WIDTH - 2 * MARGIN;
        int barGap = 20;

        // 그래프 영역 배경
        tft.fillRect(MARGIN, graphY, graphW, graphH + 30, Color::BG);

        // 그래프 베이스라인
        tft.drawFastHLine(MARGIN, graphY + graphH, graphW, Color::GREY);

        // PM 값 배열
        uint16_t values[3] = {sys.pms.pm1_0, sys.pms.pm2_5, sys.pms.pm10};
        const char *labels[3] = {"PM1.0", "PM2.5", "PM10"};
        int maxVal = 150; // 스케일 최대값

        // 각 막대 그리기
        for (int i = 0; i < 3; i++)
        {
            int barX = MARGIN + 30 + i * (BAR_WIDTH + barGap);
            int barH = map(constrain(values[i], 0, maxVal), 0, maxVal, 0, graphH);

            // 색상 결정
            uint16_t barColor = Color::GREEN;
            if (values[i] > 35)
                barColor = Color::YELLOW;
            if (values[i] > 75)
                barColor = Color::RED;

            // 막대
            tft.fillRect(barX, graphY + graphH - barH, BAR_WIDTH, barH, barColor);

            // 수치 (막대 위)
            tft.setTextColor(Color::TEXT);
            tft.setTextSize(1);
            tft.setCursor(barX + 10, graphY + graphH - barH - 15);
            tft.print(values[i]);

            // 라벨 (아래)
            tft.setCursor(barX + 5, graphY + graphH + 10);
            tft.print(labels[i]);
        }
    }

    // 통계 패널 그리기
    void drawStatsPanel()
    {
        int y = Config::SCREEN_HEIGHT - 50;
        int halfW = (Config::SCREEN_WIDTH - 3 * MARGIN) / 2;

        // 주황 카운트 박스
        tft.fillRoundRect(MARGIN, y, halfW, 40, 5, Color::PANEL);
        tft.setTextColor(Color::ORANGE);
        tft.setTextSize(1);
        tft.setCursor(MARGIN + 5, y + 8);
        tft.print("Bad (>35):");
        tft.setTextSize(2);
        tft.setCursor(MARGIN + 5, y + 20);
        tft.print(sys.pms.countOrange);

        // 빨강 카운트 박스
        tft.fillRoundRect(MARGIN + halfW + MARGIN, y, halfW, 40, 5, Color::PANEL);
        tft.setTextColor(Color::RED);
        tft.setTextSize(1);
        tft.setCursor(MARGIN + halfW + MARGIN + 5, y + 8);
        tft.print("Danger (>75):");
        tft.setTextSize(2);
        tft.setCursor(MARGIN + halfW + MARGIN + 5, y + 20);
        tft.print(sys.pms.countRed);
    }

    // 경고 테두리 (깜빡임)
    void drawAlertBorder()
    {
        if (sys.pms.pm2_5 > 35 && !sys.isSensorSleeping)
        {
            static bool blinkState = false;
            static unsigned long lastBlink = 0;

            if (millis() - lastBlink > 500)
            {
                blinkState = !blinkState;
                lastBlink = millis();
            }

            uint16_t alertColor = blinkState ? Color::RED : Color::BG;

            // 테두리 그리기
            for (int i = 0; i < 4; i++)
            {
                tft.drawRect(i, TAB_HEIGHT + i,
                             Config::SCREEN_WIDTH - 2 * i,
                             Config::SCREEN_HEIGHT - TAB_HEIGHT - 2 * i,
                             alertColor);
            }
        }
    }

    // 모니터 탭 렌더링
    void renderMonitor()
    {
        drawStatusPanel();
        drawPm25Large();
        drawBarGraph();
        drawStatsPanel();
        drawAlertBorder();
    }

    // 설정 버튼 그리기
    void drawSettingButton(int x, int y, int w, int h, const char *label, bool selected)
    {
        uint16_t bgColor = selected ? Color::ACCENT : Color::BTN_NORMAL;
        uint16_t textColor = selected ? Color::WHITE : Color::TEXT;

        tft.fillRoundRect(x, y, w, h, 5, bgColor);
        tft.setTextColor(textColor);
        tft.setTextSize(2);

        // 텍스트 중앙 정렬
        int16_t x1, y1;
        uint16_t tw, th;
        tft.getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
        tft.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
        tft.print(label);
    }

    // 설정 탭 렌더링
    void renderSettings()
    {
        int startY = TAB_HEIGHT + 20;
        int btnW = Config::SCREEN_WIDTH - 2 * MARGIN;
        int btnH = 40;
        int gap = 10;

        // 타이틀
        tft.setTextColor(Color::TEXT);
        tft.setTextSize(2);
        tft.setCursor(MARGIN, startY);
        tft.print("Sleep Interval");

        // 현재 설정 표시
        tft.setTextSize(1);
        tft.setCursor(MARGIN, startY + 25);
        tft.print("Current: ");
        tft.print(sys.sleepInterval / 60000);
        tft.print(" min");

        startY += 50;

        // 버튼들
        for (int i = 0; i < 5; i++)
        {
            bool isSelected = (sys.sleepInterval == sys.intervals[i]);
            drawSettingButton(MARGIN, startY + i * (btnH + gap),
                              btnW, btnH, sys.intervalLabels[i], isSelected);
        }
    }

    // 전체 화면 그리기
    void draw()
    {
        if (needsFullRedraw)
        {
            tft.fillScreen(Color::BG);
            needsFullRedraw = false;
        }

        drawTabs();

        if (sys.currentTab == AppTab::MONITOR)
        {
            renderMonitor();
        }
        else
        {
            renderSettings();
        }
    }
}

// -----------------------------------------------------------------------------
// 7. Touch Event Handling
// -----------------------------------------------------------------------------
void handleTouch()
{
    unsigned long now = millis();

    // 디바운스 체크
    if (now - sys.lastTouchTime < Config::TOUCH_DEBOUNCE)
        return;

    Touch::Point p = Touch::getTouch();
    if (!p.valid)
        return;

    sys.lastTouchTime = now;

    // 탭 영역 터치 체크
    if (p.y < UI::TAB_HEIGHT)
    {
        int tabWidth = Config::SCREEN_WIDTH / 2;

        if (p.x < tabWidth && sys.currentTab != AppTab::MONITOR)
        {
            sys.currentTab = AppTab::MONITOR;
            needsFullRedraw = true;
        }
        else if (p.x >= tabWidth && sys.currentTab != AppTab::SETTINGS)
        {
            sys.currentTab = AppTab::SETTINGS;
            needsFullRedraw = true;
        }
        return;
    }

    // 설정 탭에서 버튼 터치 처리
    if (sys.currentTab == AppTab::SETTINGS)
    {
        int startY = UI::TAB_HEIGHT + 70;
        int btnH = 40;
        int gap = 10;

        for (int i = 0; i < 5; i++)
        {
            int btnY = startY + i * (btnH + gap);

            if (Touch::isInRect(p.x, p.y, UI::MARGIN, btnY,
                                Config::SCREEN_WIDTH - 2 * UI::MARGIN, btnH))
            {

                sys.sleepInterval = sys.intervals[i];
                needsFullRedraw = true;

                Serial.print(F(">>> 휴식 시간 변경: "));
                Serial.print(sys.sleepInterval / 60000);
                Serial.println(F("분"));
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 8. System Logic
// -----------------------------------------------------------------------------
void handleSleepState()
{
    unsigned long now = millis();

    // 휴식 시간 종료 체크
    if (now - sys.lastSensorSleepTime >= sys.sleepInterval)
    {
        PmsDriver::wake();
    }
}

void handleWakeState()
{
    unsigned long now = millis();

    // 데이터 수신
    PmsDriver::update();

    // 측정 시간 종료 체크
    if (now - sys.lastSensorWakeTime >= Config::WAKE_DURATION)
    {
        PmsDriver::sleep();
    }
}

void updateSystem()
{
    if (sys.isSensorSleeping)
    {
        handleSleepState();
    }
    else
    {
        handleWakeState();
    }
}

// -----------------------------------------------------------------------------
// 9. Arduino Lifecycle
// -----------------------------------------------------------------------------
void setup()
{
    Serial.begin(Config::SERIAL_DEBUG_BAUD);
    Serial1.begin(Config::PMS_BAUD);

    Serial.println(F("PMS7003 TFT LCD GUI Starting..."));

    // TFT 초기화
    uint16_t ID = tft.readID();
    Serial.print(F("TFT ID: 0x"));
    Serial.println(ID, HEX);

    tft.begin(ID);
    tft.setRotation(0); // Portrait 모드
    tft.fillScreen(Color::BG);

    // 센서 초기화
    sys.lastSensorWakeTime = millis();
    PmsDriver::wake();

    Serial.println(F("System Ready!"));
}

void loop()
{
    unsigned long now = millis();

    // 1. 터치 입력 처리
    handleTouch();

    // 2. 시스템 상태 갱신
    updateSystem();

    // 3. 화면 갱신 (주기적으로)
    if (now - sys.lastScreenUpdate >= Config::SCREEN_UPDATE_INTERVAL)
    {
        sys.lastScreenUpdate = now;
        UI::draw();
    }
}

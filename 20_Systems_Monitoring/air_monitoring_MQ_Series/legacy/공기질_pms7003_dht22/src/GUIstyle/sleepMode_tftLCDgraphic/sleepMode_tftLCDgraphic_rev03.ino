/**
 * @file sleepMode_tftLCDgraphic_rev03.ino
 * @brief PMS7003 TFT LCD GUI with Touch Control (Layout Fixed)
 *
 * Rev03 Changes:
 * - 설정 화면 레이아웃 수정 (모든 버튼이 화면 내에 표시)
 * - 버튼 크기 및 간격 조정
 *
 * Rev02 Changes:
 * - 부분 갱신 최적화 (전체 갱신 최소화)
 * - 터치 반응성 개선 (디바운스 시간 단축, 즉시 피드백)
 * - PMS 드라이버 비동기 처리 (delay 제거)
 *
 * Hardware:
 * - MCU: Arduino Uno R4 WiFi
 * - Display: 2.4" TFT LCD Shield (ILI9341 320x240) with Touch
 * - Sensor: PMS7003 (Serial1)
 *
 * Screen Layout (Portrait 240x320):
 * - Tab Area: 0-40 (40px)
 * - Content Area: 40-320 (280px)
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
    constexpr uint8_t YP = A1;
    constexpr uint8_t XM = A2;
    constexpr uint8_t YM = 7;
    constexpr uint8_t XP = 6;

    // Touch Screen Calibration
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
    constexpr unsigned long WAKE_DURATION = 30000;
    constexpr unsigned long DEFAULT_SLEEP_TIME = 60000;
    constexpr unsigned long DATA_UPDATE_INTERVAL = 1000;
    constexpr unsigned long COUNTDOWN_UPDATE_INTERVAL = 1000;
    constexpr unsigned long TOUCH_DEBOUNCE = 50;
    constexpr unsigned long TOUCH_SAMPLE_COUNT = 3;
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

    // Dark Theme Colors
    constexpr uint16_t BG = 0x2082;
    constexpr uint16_t PANEL = 0x3186;
    constexpr uint16_t TEXT = 0xEF5D;
    constexpr uint16_t ACCENT = 0xFA80;
    constexpr uint16_t BTN_NORMAL = 0x4208;
    constexpr uint16_t BTN_HOVER = 0x52AA;
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
    uint32_t countOrange = 0;
    uint32_t countRed = 0;
};

struct SystemState
{
    AppTab currentTab = AppTab::MONITOR;
    PmsData pms;
    bool isSensorSleeping = false;
    unsigned long sleepInterval = Config::DEFAULT_SLEEP_TIME;
    unsigned long lastSensorWakeTime = 0;
    unsigned long lastSensorSleepTime = 0;
    unsigned long lastDataUpdate = 0;
    unsigned long lastCountdownUpdate = 0;
    unsigned long lastTouchTime = 0;

    // Settings
    int settingsIndex = 0;
    const unsigned long intervals[5] = {60000, 180000, 300000, 600000, 1200000};
    const char *intervalLabels[5] = {"1 Min", "3 Min", "5 Min", "10 Min", "20 Min"};
};

SystemState sys;

// 부분 갱신을 위한 이전 상태 저장
struct PrevState
{
    AppTab tab = AppTab::MONITOR;
    bool sensorSleeping = false;
    int countdown = -1;
    uint16_t pm1_0 = 0;
    uint16_t pm2_5 = 0;
    uint16_t pm10 = 0;
    uint32_t countOrange = 0;
    uint32_t countRed = 0;
    unsigned long sleepInterval = 0;
    bool initialDrawDone = false;
};

PrevState prev;

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
    }

    void sleep()
    {
        sendCmd(CMD_PASSIVE, 7);
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
        while (Serial1.available() >= 32)
        {
            if (Serial1.peek() != 0x42)
            {
                Serial1.read();
                continue;
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

                    if (sys.pms.pm2_5 > 75)
                        sys.pms.countRed++;
                    else if (sys.pms.pm2_5 > 35)
                        sys.pms.countOrange++;
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
        int xSum = 0, ySum = 0;
        int validCount = 0;

        for (int i = 0; i < Config::TOUCH_SAMPLE_COUNT; i++)
        {
            TSPoint p = ts.getPoint();

            if (p.z > Config::TS_MINPRESSURE && p.z < Config::TS_MAXPRESSURE)
            {
                xSum += p.x;
                ySum += p.y;
                validCount++;
            }
        }

        pinMode(Config::XM, OUTPUT);
        pinMode(Config::YP, OUTPUT);

        if (validCount > 0)
        {
            int avgX = xSum / validCount;
            int avgY = ySum / validCount;

            result.x = map(avgX, Config::TS_LEFT, Config::TS_RIGHT, Config::SCREEN_WIDTH, 0);
            result.y = map(avgY, Config::TS_TOP, Config::TS_BOTTOM, 0, Config::SCREEN_HEIGHT);
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
    constexpr int TAB_HEIGHT = 40;
    constexpr int MARGIN = 10;
    constexpr int BAR_WIDTH = 50;
    constexpr int BAR_MAX_HEIGHT = 120;

    // rev03: 설정 화면 레이아웃 상수
    // 화면 높이: 320px, 탭: 40px, 사용 가능 영역: 280px
    // 타이틀 영역: 45px (텍스트 + 현재 설정)
    // 버튼 영역: 280 - 45 - 10(하단 여백) = 225px
    // 버튼 5개: 225 / 5 = 45px per button (버튼 35px + 간격 10px)
    constexpr int SETTINGS_TITLE_Y = TAB_HEIGHT + 10;     // 50
    constexpr int SETTINGS_BTN_START_Y = TAB_HEIGHT + 55; // 95
    constexpr int SETTINGS_BTN_HEIGHT = 35;               // 버튼 높이 (40->35)
    constexpr int SETTINGS_BTN_GAP = 8;                   // 버튼 간격 (10->8)
    // 마지막 버튼 끝 Y: 95 + 4*(35+8) + 35 = 95 + 172 + 35 = 302 < 320 ✓

    uint16_t getPmColor(uint16_t pm25)
    {
        if (pm25 > 75)
            return Color::RED;
        if (pm25 > 35)
            return Color::YELLOW;
        return Color::GREEN;
    }

    const char *getPmStatus(uint16_t pm25)
    {
        if (pm25 > 75)
            return "Danger!";
        if (pm25 > 35)
            return "Bad    ";
        if (pm25 > 15)
            return "Normal ";
        return "Good   ";
    }

    void drawTabs()
    {
        int tabWidth = Config::SCREEN_WIDTH / 2;

        tft.fillRect(0, 0, tabWidth, TAB_HEIGHT,
                     sys.currentTab == AppTab::MONITOR ? Color::PANEL : Color::BTN_NORMAL);
        tft.setTextColor(Color::TEXT);
        tft.setTextSize(1);
        tft.setCursor(15, 15);
        tft.print("Monitor");

        tft.fillRect(tabWidth, 0, tabWidth, TAB_HEIGHT,
                     sys.currentTab == AppTab::SETTINGS ? Color::PANEL : Color::BTN_NORMAL);
        tft.setCursor(tabWidth + 15, 15);
        tft.print("Settings");

        if (sys.currentTab == AppTab::MONITOR)
        {
            tft.drawFastHLine(0, TAB_HEIGHT - 2, tabWidth, Color::ACCENT);
            tft.drawFastHLine(tabWidth, TAB_HEIGHT - 2, tabWidth, Color::BTN_NORMAL);
        }
        else
        {
            tft.drawFastHLine(0, TAB_HEIGHT - 2, tabWidth, Color::BTN_NORMAL);
            tft.drawFastHLine(tabWidth, TAB_HEIGHT - 2, tabWidth, Color::ACCENT);
        }
    }

    void drawStatusPanel(bool forceRedraw = false)
    {
        int y = TAB_HEIGHT + MARGIN;

        bool stateChanged = (prev.sensorSleeping != sys.isSensorSleeping);

        if (forceRedraw || stateChanged)
        {
            tft.fillRoundRect(MARGIN, y, Config::SCREEN_WIDTH - 2 * MARGIN, 50, 5, Color::PANEL);

            tft.setTextColor(Color::TEXT);
            tft.setTextSize(1);
            tft.setCursor(MARGIN + 10, y + 8);
            tft.print("Status:");

            if (sys.isSensorSleeping)
            {
                tft.setTextColor(Color::YELLOW);
                tft.setTextSize(2);
                tft.setCursor(MARGIN + 10, y + 22);
                tft.print("SLEEP ");
            }
            else
            {
                tft.setTextColor(Color::GREEN);
                tft.setTextSize(2);
                tft.setCursor(MARGIN + 10, y + 22);
                tft.print("ACTIVE");
            }

            prev.sensorSleeping = sys.isSensorSleeping;
        }

        if (sys.isSensorSleeping)
        {
            long remaining = (sys.sleepInterval - (millis() - sys.lastSensorSleepTime)) / 1000;
            if (remaining < 0)
                remaining = 0;

            if (forceRedraw || prev.countdown != remaining)
            {
                tft.fillRect(Config::SCREEN_WIDTH - 70, y + 20, 60, 20, Color::PANEL);

                tft.setTextColor(Color::YELLOW);
                tft.setTextSize(1);
                tft.setCursor(Config::SCREEN_WIDTH - 60, y + 28);
                tft.print(remaining);
                tft.print("s  ");

                prev.countdown = remaining;
            }
        }
    }

    void drawPm25Large(bool forceRedraw = false)
    {
        int y = TAB_HEIGHT + 70;

        bool dataChanged = (prev.pm2_5 != sys.pms.pm2_5);

        if (forceRedraw)
        {
            tft.fillRoundRect(MARGIN, y, Config::SCREEN_WIDTH - 2 * MARGIN, 70, 5, Color::PANEL);
        }

        if (forceRedraw || dataChanged)
        {
            tft.fillRect(MARGIN + 15, y + 10, 85, 40, Color::PANEL);

            uint16_t pmColor = getPmColor(sys.pms.pm2_5);
            tft.setTextColor(pmColor);
            tft.setTextSize(4);
            tft.setCursor(MARGIN + 20, y + 15);
            tft.print(sys.pms.pm2_5);

            tft.setTextColor(Color::TEXT);
            tft.setTextSize(1);
            tft.setCursor(MARGIN + 100, y + 35);
            tft.print("ug/m3");

            tft.fillRect(MARGIN + 125, y + 15, 90, 25, Color::PANEL);
            tft.setTextColor(pmColor);
            tft.setTextSize(2);
            tft.setCursor(MARGIN + 130, y + 20);
            tft.print(getPmStatus(sys.pms.pm2_5));

            tft.setTextColor(Color::GREY);
            tft.setTextSize(1);
            tft.setCursor(MARGIN + 20, y + 55);
            tft.print("PM 2.5");

            prev.pm2_5 = sys.pms.pm2_5;
        }
    }

    void drawBarGraph(bool forceRedraw = false)
    {
        int graphY = TAB_HEIGHT + 150;
        int graphH = BAR_MAX_HEIGHT;
        int graphW = Config::SCREEN_WIDTH - 2 * MARGIN;
        int barGap = 20;
        int maxVal = 150;

        bool dataChanged = (prev.pm1_0 != sys.pms.pm1_0 ||
                            prev.pm2_5 != sys.pms.pm2_5 ||
                            prev.pm10 != sys.pms.pm10);

        if (forceRedraw)
        {
            tft.fillRect(MARGIN, graphY, graphW, graphH + 30, Color::BG);
            tft.drawFastHLine(MARGIN, graphY + graphH, graphW, Color::GREY);
        }

        if (forceRedraw || dataChanged)
        {
            uint16_t values[3] = {sys.pms.pm1_0, sys.pms.pm2_5, sys.pms.pm10};
            uint16_t prevValues[3] = {prev.pm1_0, prev.pm2_5, prev.pm10};
            const char *labels[3] = {"PM1.0", "PM2.5", "PM10 "};

            for (int i = 0; i < 3; i++)
            {
                int barX = MARGIN + 30 + i * (BAR_WIDTH + barGap);
                int barH = map(constrain(values[i], 0, maxVal), 0, maxVal, 0, graphH);
                int prevBarH = map(constrain(prevValues[i], 0, maxVal), 0, maxVal, 0, graphH);

                if (!forceRedraw && prevBarH != barH)
                {
                    tft.fillRect(barX, graphY, BAR_WIDTH, graphH, Color::BG);
                }

                uint16_t barColor = Color::GREEN;
                if (values[i] > 35)
                    barColor = Color::YELLOW;
                if (values[i] > 75)
                    barColor = Color::RED;

                tft.fillRect(barX, graphY + graphH - barH, BAR_WIDTH, barH, barColor);

                tft.fillRect(barX, graphY + graphH - barH - 18, BAR_WIDTH, 15, Color::BG);
                tft.setTextColor(Color::TEXT);
                tft.setTextSize(1);
                tft.setCursor(barX + 10, graphY + graphH - barH - 15);
                tft.print(values[i]);

                if (forceRedraw)
                {
                    tft.setCursor(barX + 5, graphY + graphH + 10);
                    tft.print(labels[i]);
                }
            }

            prev.pm1_0 = sys.pms.pm1_0;
            prev.pm10 = sys.pms.pm10;
        }
    }

    void drawStatsPanel(bool forceRedraw = false)
    {
        int y = Config::SCREEN_HEIGHT - 50;
        int halfW = (Config::SCREEN_WIDTH - 3 * MARGIN) / 2;

        bool statsChanged = (prev.countOrange != sys.pms.countOrange ||
                             prev.countRed != sys.pms.countRed);

        if (forceRedraw)
        {
            tft.fillRoundRect(MARGIN, y, halfW, 40, 5, Color::PANEL);
            tft.setTextColor(Color::ORANGE);
            tft.setTextSize(1);
            tft.setCursor(MARGIN + 5, y + 8);
            tft.print("Bad (>35):");

            tft.fillRoundRect(MARGIN + halfW + MARGIN, y, halfW, 40, 5, Color::PANEL);
            tft.setTextColor(Color::RED);
            tft.setCursor(MARGIN + halfW + MARGIN + 5, y + 8);
            tft.print("Danger(>75):");
        }

        if (forceRedraw || statsChanged)
        {
            tft.fillRect(MARGIN + 5, y + 20, 50, 18, Color::PANEL);
            tft.setTextColor(Color::ORANGE);
            tft.setTextSize(2);
            tft.setCursor(MARGIN + 5, y + 20);
            tft.print(sys.pms.countOrange);

            tft.fillRect(MARGIN + halfW + MARGIN + 5, y + 20, 50, 18, Color::PANEL);
            tft.setTextColor(Color::RED);
            tft.setCursor(MARGIN + halfW + MARGIN + 5, y + 20);
            tft.print(sys.pms.countRed);

            prev.countOrange = sys.pms.countOrange;
            prev.countRed = sys.pms.countRed;
        }
    }

    void drawAlertBorder()
    {
        static bool blinkState = false;
        static unsigned long lastBlink = 0;
        static bool wasAlert = false;

        bool isAlert = (sys.pms.pm2_5 > 35 && !sys.isSensorSleeping);

        if (isAlert)
        {
            if (millis() - lastBlink > 500)
            {
                blinkState = !blinkState;
                lastBlink = millis();

                uint16_t alertColor = blinkState ? Color::RED : Color::BG;
                for (int i = 0; i < 4; i++)
                {
                    tft.drawRect(i, TAB_HEIGHT + i,
                                 Config::SCREEN_WIDTH - 2 * i,
                                 Config::SCREEN_HEIGHT - TAB_HEIGHT - 2 * i,
                                 alertColor);
                }
            }
            wasAlert = true;
        }
        else if (wasAlert)
        {
            for (int i = 0; i < 4; i++)
            {
                tft.drawRect(i, TAB_HEIGHT + i,
                             Config::SCREEN_WIDTH - 2 * i,
                             Config::SCREEN_HEIGHT - TAB_HEIGHT - 2 * i,
                             Color::BG);
            }
            wasAlert = false;
        }
    }

    void renderMonitor(bool forceRedraw = false)
    {
        drawStatusPanel(forceRedraw);
        drawPm25Large(forceRedraw);
        drawBarGraph(forceRedraw);
        drawStatsPanel(forceRedraw);
        drawAlertBorder();
    }

    // rev03: 설정 버튼 (크기 조정)
    void drawSettingButton(int x, int y, int w, int h, const char *label, bool selected)
    {
        uint16_t bgColor = selected ? Color::ACCENT : Color::BTN_NORMAL;
        uint16_t textColor = selected ? Color::WHITE : Color::TEXT;

        tft.fillRoundRect(x, y, w, h, 5, bgColor);
        tft.setTextColor(textColor);
        tft.setTextSize(2);

        int16_t x1, y1;
        uint16_t tw, th;
        tft.getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
        tft.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
        tft.print(label);
    }

    // rev03: 설정 탭 레이아웃 수정
    void renderSettings(bool forceRedraw = false)
    {
        int btnW = Config::SCREEN_WIDTH - 2 * MARGIN;
        bool intervalChanged = (prev.sleepInterval != sys.sleepInterval);

        if (forceRedraw)
        {
            tft.fillRect(0, TAB_HEIGHT, Config::SCREEN_WIDTH,
                         Config::SCREEN_HEIGHT - TAB_HEIGHT, Color::BG);

            // 타이틀
            tft.setTextColor(Color::TEXT);
            tft.setTextSize(2);
            tft.setCursor(MARGIN, SETTINGS_TITLE_Y);
            tft.print("Sleep Interval");
        }

        if (forceRedraw || intervalChanged)
        {
            // 현재 설정 표시
            tft.fillRect(MARGIN, SETTINGS_TITLE_Y + 20, 150, 15, Color::BG);
            tft.setTextColor(Color::GREY);
            tft.setTextSize(1);
            tft.setCursor(MARGIN, SETTINGS_TITLE_Y + 25);
            tft.print("Current: ");
            tft.print(sys.sleepInterval / 60000);
            tft.print(" min");

            // 버튼 5개 그리기
            for (int i = 0; i < 5; i++)
            {
                int btnY = SETTINGS_BTN_START_Y + i * (SETTINGS_BTN_HEIGHT + SETTINGS_BTN_GAP);
                bool isSelected = (sys.sleepInterval == sys.intervals[i]);
                drawSettingButton(MARGIN, btnY, btnW, SETTINGS_BTN_HEIGHT,
                                  sys.intervalLabels[i], isSelected);
            }

            prev.sleepInterval = sys.sleepInterval;
        }
    }

    void drawInitialScreen()
    {
        tft.fillScreen(Color::BG);
        drawTabs();

        if (sys.currentTab == AppTab::MONITOR)
        {
            renderMonitor(true);
        }
        else
        {
            renderSettings(true);
        }

        prev.tab = sys.currentTab;
        prev.initialDrawDone = true;
    }

    void update()
    {
        if (!prev.initialDrawDone)
        {
            drawInitialScreen();
            return;
        }

        if (prev.tab != sys.currentTab)
        {
            drawTabs();
            tft.fillRect(0, TAB_HEIGHT, Config::SCREEN_WIDTH,
                         Config::SCREEN_HEIGHT - TAB_HEIGHT, Color::BG);

            if (sys.currentTab == AppTab::MONITOR)
            {
                renderMonitor(true);
            }
            else
            {
                renderSettings(true);
            }
            prev.tab = sys.currentTab;
            return;
        }

        if (sys.currentTab == AppTab::MONITOR)
        {
            renderMonitor(false);
        }
        else
        {
            renderSettings(false);
        }
    }
}

// -----------------------------------------------------------------------------
// 7. Touch Event Handling (rev03: 버튼 영역 수정)
// -----------------------------------------------------------------------------
bool handleTouch()
{
    unsigned long now = millis();

    if (now - sys.lastTouchTime < Config::TOUCH_DEBOUNCE)
        return false;

    Touch::Point p = Touch::getTouch();
    if (!p.valid)
        return false;

    sys.lastTouchTime = now;
    bool handled = false;

    // 탭 영역 터치
    if (p.y < UI::TAB_HEIGHT)
    {
        int tabWidth = Config::SCREEN_WIDTH / 2;

        if (p.x < tabWidth && sys.currentTab != AppTab::MONITOR)
        {
            sys.currentTab = AppTab::MONITOR;
            handled = true;
            Serial.println(F(">>> Monitor 탭 선택"));
        }
        else if (p.x >= tabWidth && sys.currentTab != AppTab::SETTINGS)
        {
            sys.currentTab = AppTab::SETTINGS;
            handled = true;
            Serial.println(F(">>> Settings 탭 선택"));
        }
    }
    // rev03: 설정 탭 버튼 터치 (수정된 좌표)
    else if (sys.currentTab == AppTab::SETTINGS)
    {
        for (int i = 0; i < 5; i++)
        {
            int btnY = UI::SETTINGS_BTN_START_Y + i * (UI::SETTINGS_BTN_HEIGHT + UI::SETTINGS_BTN_GAP);

            if (Touch::isInRect(p.x, p.y, UI::MARGIN, btnY,
                                Config::SCREEN_WIDTH - 2 * UI::MARGIN, UI::SETTINGS_BTN_HEIGHT))
            {
                sys.sleepInterval = sys.intervals[i];
                handled = true;

                Serial.print(F(">>> 휴식 시간 변경: "));
                Serial.print(sys.sleepInterval / 60000);
                Serial.println(F("분"));
                break;
            }
        }
    }

    if (handled)
    {
        UI::update();
    }

    return handled;
}

// -----------------------------------------------------------------------------
// 8. System Logic
// -----------------------------------------------------------------------------
void handleSleepState()
{
    if (millis() - sys.lastSensorSleepTime >= sys.sleepInterval)
    {
        PmsDriver::wake();
    }
}

void handleWakeState()
{
    PmsDriver::update();

    if (millis() - sys.lastSensorWakeTime >= Config::WAKE_DURATION)
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

    Serial.println(F("PMS7003 TFT LCD GUI (Rev03) Starting..."));

    uint16_t ID = tft.readID();
    Serial.print(F("TFT ID: 0x"));
    Serial.println(ID, HEX);

    tft.begin(ID);
    tft.setRotation(0);

    UI::drawInitialScreen();

    sys.lastSensorWakeTime = millis();
    PmsDriver::wake();

    Serial.println(F("System Ready!"));
}

void loop()
{
    unsigned long now = millis();

    // 1. 터치 입력
    handleTouch();

    // 2. 시스템 상태 갱신
    updateSystem();

    // 3. 모니터 탭: 주기적 데이터 갱신
    if (sys.currentTab == AppTab::MONITOR)
    {
        if (sys.isSensorSleeping &&
            now - sys.lastCountdownUpdate >= Config::COUNTDOWN_UPDATE_INTERVAL)
        {
            sys.lastCountdownUpdate = now;
            UI::drawStatusPanel(false);
        }

        if (now - sys.lastDataUpdate >= Config::DATA_UPDATE_INTERVAL)
        {
            sys.lastDataUpdate = now;
            UI::update();
        }
    }
}

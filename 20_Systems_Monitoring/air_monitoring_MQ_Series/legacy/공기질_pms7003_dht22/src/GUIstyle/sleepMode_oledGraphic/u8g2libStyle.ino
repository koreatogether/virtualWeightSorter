/**
 * @file u8g2libStyle.ino
 * @brief PMS7003 OLED GUI with Rotary Encoder Control
 * 
 * Hardware:
 * - MCU: Arduino Uno R4 WiFi
 * - Display: 0.96" OLED (SSD1306 128x64) I2C
 * - Sensor: PMS7003 (Serial1)
 * - Input: Rotary Encoder (CLK=2, DT=3, SW=4)
 * 
 * Libraries Required:
 * - U8g2 (by oliver)
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// -----------------------------------------------------------------------------
// 1. Configuration & Constants
// -----------------------------------------------------------------------------
namespace Config {
    // Pins
    constexpr uint8_t PIN_ENC_CLK = 3; // Interrupt Pin
    constexpr uint8_t PIN_ENC_DT = 2;  // Interrupt Pin
    constexpr uint8_t PIN_ENC_SW = 4;

    // PMS7003
    constexpr uint32_t PMS_BAUD = 9600;
    constexpr uint32_t SERIAL_DEBUG_BAUD = 115200;

    // UI Settings
    constexpr unsigned long SCREEN_SAVER_TIMEOUT = 60000; // 1 min to auto-sleep (optional)
    constexpr unsigned long DEBOUNCE_DELAY = 200;
}

// -----------------------------------------------------------------------------
// 2. Global Objects
// -----------------------------------------------------------------------------
// OLED Constructor (Hardware I2C)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// -----------------------------------------------------------------------------
// 3. State Management
// -----------------------------------------------------------------------------
enum class AppState {
    MONITOR,
    GRAPH,
    STATS,
    SETTINGS,
    SLEEP
};

struct PmsData {
    uint16_t pm1_0 = 0;
    uint16_t pm2_5 = 0;
    uint16_t pm10 = 0;
    uint32_t countOrange = 0; // > 35
    uint32_t countRed = 0;    // > 75
};

struct SystemState {
    AppState currentState = AppState::MONITOR;
    PmsData pms;
    bool isSensorSleeping = false;
    unsigned long sleepInterval = 60000; // Current active interval
    unsigned long nextSleepInterval = 60000; // Pending interval setting
    unsigned long lastInteractionTime = 0;
    unsigned long lastSensorWakeTime = 0;
    unsigned long lastSensorSleepTime = 0; 
    
    // Settings Menu
    int settingsIndex = 0;
    const unsigned long intervals[5] = {60000, 180000, 300000, 600000, 1200000}; // 1, 3, 5, 10, 20 min
    const char* intervalLabels[5] = {"1 Min", "3 Min", "5 Min", "10 Min", "20 Min"};
};

SystemState sys;

// --- Robust Rotary Encoder Driver (Interrupt Based) ---
class RotaryEncoder
{
public:
    RotaryEncoder(uint8_t pinA, uint8_t pinB, int8_t pinButton = -1);
    void begin(bool useInternalPullup = true);
    long getPosition();
    void setPosition(long newPosition);
    bool isButtonPressed();
    void tick(); // Call in ISR

private:
    uint8_t _pinA, _pinB;
    int8_t _pinButton;

    volatile long _position;     // Actual ticks (4 ticks = 1 step)
    volatile uint8_t _lastState; // Previous pin state (2 bits)

    // Button related
    bool _buttonState;     
    bool _lastButtonState; 
    unsigned long _lastDebounceTime;
    bool _buttonPressedFlag; 

    // Quadrature State Table
    static const int8_t _transitionTable[16];
};

const int8_t RotaryEncoder::_transitionTable[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

RotaryEncoder::RotaryEncoder(uint8_t pinA, uint8_t pinB, int8_t pinButton)
    : _pinA(pinA), _pinB(pinB), _pinButton(pinButton), _position(0), _lastState(0),
      _buttonState(HIGH), _lastButtonState(HIGH), _lastDebounceTime(0), _buttonPressedFlag(false)
{
}

void RotaryEncoder::begin(bool useInternalPullup)
{
    uint8_t mode = useInternalPullup ? INPUT_PULLUP : INPUT;
    pinMode(_pinA, mode);
    pinMode(_pinB, mode);
    if (_pinButton != -1) pinMode(_pinButton, mode);

    // Initial state
    uint8_t msb = digitalRead(_pinA);
    uint8_t lsb = digitalRead(_pinB);
    _lastState = (msb << 1) | lsb;
}

long RotaryEncoder::getPosition()
{
    noInterrupts();
    long pos = _position;
    interrupts();
    return pos / 4;
}

void RotaryEncoder::setPosition(long newPosition)
{
    noInterrupts();
    _position = newPosition * 4;
    interrupts();
}

bool RotaryEncoder::isButtonPressed()
{
    if (_pinButton == -1) return false;

    int reading = digitalRead(_pinButton);
    if (reading != _lastButtonState) _lastDebounceTime = millis();

    if ((millis() - _lastDebounceTime) > 20) // Reduced to 20ms
    { 
        if (reading != _buttonState)
        {
            _buttonState = reading;
            if (_buttonState == LOW) {
                _buttonPressedFlag = true;
                Serial.println("Button Pressed (Internal)");
            }
        }
    }
    _lastButtonState = reading;

    if (_buttonPressedFlag)
    {
        _buttonPressedFlag = false;
        return true;
    }
    return false;
}

void RotaryEncoder::tick()
{
    uint8_t msb = digitalRead(_pinA);
    uint8_t lsb = digitalRead(_pinB);
    uint8_t currentState = (msb << 1) | lsb;
    uint8_t transition = (_lastState << 2) | currentState;
    _position += _transitionTable[transition & 0x0F];
    _lastState = currentState;
}

// Global Encoder Object
RotaryEncoder encoder(Config::PIN_ENC_DT, Config::PIN_ENC_CLK, Config::PIN_ENC_SW);

// ISR Wrapper
void handleEncoderInterrupt() {
    encoder.tick();
}

// --- PMS7003 Driver ---
namespace PmsDriver {
    const uint8_t CMD_SLEEP[] = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73};
    const uint8_t CMD_WAKEUP[] = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74};
    const uint8_t CMD_PASSIVE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
    const uint8_t CMD_ACTIVE[] = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71};

    void sendCmd(const uint8_t* cmd, size_t len) {
        Serial1.write(cmd, len);
        delay(50);
    }

    void sleep() {
        sendCmd(CMD_PASSIVE, 7); // Stop data first
        delay(50);
        sendCmd(CMD_SLEEP, 7);
        sys.isSensorSleeping = true;
        sys.lastSensorSleepTime = millis();
        
        // Apply pending sleep interval for THIS new sleep cycle
        sys.sleepInterval = sys.nextSleepInterval;
    }

    void wake() {
        sendCmd(CMD_WAKEUP, 7);
        sendCmd(CMD_ACTIVE, 7); // Resume data
        sys.isSensorSleeping = false;
        sys.lastSensorWakeTime = millis();
    }

    void update() {
        if (Serial1.available() >= 32) {
            if (Serial1.peek() != 0x42) {
                Serial1.read();
                return;
            }
            
            uint8_t buf[32];
            Serial1.readBytes(buf, 32);

            if (buf[0] == 0x42 && buf[1] == 0x4D) {
                uint16_t sum = 0;
                for (int i = 0; i < 30; i++) sum += buf[i];
                uint16_t check = (buf[30] << 8) | buf[31];

                if (sum == check) {
                    sys.pms.pm1_0 = (buf[10] << 8) | buf[11];
                    sys.pms.pm2_5 = (buf[12] << 8) | buf[13];
                    sys.pms.pm10  = (buf[14] << 8) | buf[15];

                    // Update Stats
                    if (sys.pms.pm2_5 > 75) sys.pms.countRed++;
                    else if (sys.pms.pm2_5 > 35) sys.pms.countOrange++;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 5. UI Rendering
// -----------------------------------------------------------------------------
void drawStatusBar() {
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(0, 8);
    if (sys.isSensorSleeping) {
        u8g2.print("SLEEP");
        // Calculate remaining time based on sleep start time
        long elapsed = millis() - sys.lastSensorSleepTime;
        long remaining = (sys.sleepInterval - elapsed) / 1000;
        
        if (remaining < 0) remaining = 0;
        u8g2.setCursor(80, 8);
        u8g2.print(remaining); u8g2.print("s");
    } else {
        u8g2.print("ACTIVE");
    }
    
    // Page Indicator
    u8g2.setCursor(120, 8);
    switch(sys.currentState) {
        case AppState::MONITOR: u8g2.print("M"); break;
        case AppState::GRAPH:   u8g2.print("G"); break;
        case AppState::STATS:   u8g2.print("S"); break;
        case AppState::SETTINGS:u8g2.print("E"); break;
    }
    u8g2.drawHLine(0, 10, 128);
}

void renderMonitor() {
    drawStatusBar();
    
    // PM 2.5 (Large)
    u8g2.setFont(u8g2_font_logisoso24_tr);
    u8g2.setCursor(10, 45);
    u8g2.print(sys.pms.pm2_5);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.print(" ug/m3");
    
    u8g2.setFont(u8g2_font_profont12_tf);
    u8g2.setCursor(10, 60);
    u8g2.print("PM2.5");

    // PM 1.0 & 10 (Small)
    u8g2.setCursor(80, 30);
    u8g2.print("PM1: "); u8g2.print(sys.pms.pm1_0);
    u8g2.setCursor(80, 50);
    u8g2.print("PM10:"); u8g2.print(sys.pms.pm10);
}

void renderGraph() {
    drawStatusBar();
    
    int maxVal = 100; // Scale
    int h = 40;
    int yBase = 63;
    int w = 20;
    int gap = 15;
    int xStart = 20;

    // PM 1.0
    int h1 = map(constrain(sys.pms.pm1_0, 0, maxVal), 0, maxVal, 0, h);
    u8g2.drawFrame(xStart, yBase - h, w, h); // Frame
    u8g2.drawBox(xStart, yBase - h1, w, h1); // Fill
    u8g2.setCursor(xStart, yBase - h - 2); u8g2.print("1.0");

    // PM 2.5 (Hatched Pattern simulated by lines)
    int h2 = map(constrain(sys.pms.pm2_5, 0, maxVal), 0, maxVal, 0, h);
    int x2 = xStart + w + gap;
    u8g2.drawFrame(x2, yBase - h, w, h);
    // Draw hatch
    for (int i = 0; i < h2; i+=3) {
        u8g2.drawLine(x2, yBase - i, x2 + w, yBase - i);
    }
    u8g2.setCursor(x2, yBase - h - 2); u8g2.print("2.5");

    // PM 10 (Dotted Pattern)
    int h3 = map(constrain(sys.pms.pm10, 0, maxVal), 0, maxVal, 0, h);
    int x3 = x2 + w + gap;
    u8g2.drawFrame(x3, yBase - h, w, h);
    // Draw dots
    for (int i = 0; i < h3; i+=2) {
        for (int j = 0; j < w; j+=2) {
             u8g2.drawPixel(x3 + j, yBase - i);
        }
    }
    u8g2.setCursor(x3, yBase - h - 2); u8g2.print("10");
}

void renderStats() {
    drawStatusBar();
    
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.setCursor(10, 30);
    u8g2.print("Events (>35): ");
    u8g2.print(sys.pms.countOrange);
    
    u8g2.setCursor(10, 50);
    u8g2.print("Danger (>75): ");
    u8g2.print(sys.pms.countRed);
}

void renderSettings() {
    drawStatusBar();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(10, 22);
    u8g2.print("Set Sleep");
    u8g2.setCursor(10, 34);
    u8g2.print("Interval:");
    
    // Current setting (Moved to right side)
    u8g2.setCursor(75, 34);
    u8g2.print("Set: ");
    u8g2.print(sys.nextSleepInterval / 60000); // Show PENDING setting
    u8g2.print("m");
    
    // Draw Menu Items (Compact Layout)
    u8g2.setFont(u8g2_font_5x7_tf);
    for (int i = 0; i < 5; i++) {
        int row = i / 2;
        int col = i % 2;
        
        int y = 46 + row * 9; // y=46, 55, 64
        int x = 10 + col * 60;
        
        u8g2.setCursor(x, y);
        if (i == sys.settingsIndex) {
            u8g2.print("> ");
        } else {
            u8g2.print("  ");
        }
        u8g2.print(sys.intervalLabels[i]);
    }
}

// -----------------------------------------------------------------------------
// 6. Main Logic
// -----------------------------------------------------------------------------
long lastEncoderPos = 0;

void setup() {
    Serial.begin(Config::SERIAL_DEBUG_BAUD);
    Serial1.begin(Config::PMS_BAUD);
    
    // Init OLED
    u8g2.begin();
    u8g2.enableUTF8Print();
    
    // Init Encoder
    encoder.begin();
    attachInterrupt(digitalPinToInterrupt(Config::PIN_ENC_CLK), handleEncoderInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Config::PIN_ENC_DT), handleEncoderInterrupt, CHANGE);
    
    // Init System
    sys.lastInteractionTime = millis();
    sys.lastSensorWakeTime = millis();
    PmsDriver::wake();
    
    Serial.println("System Started with Interrupt Encoder");
}

void loop() {
    unsigned long currentMillis = millis();
    
    // 1. Input Handling
    long currentPos = encoder.getPosition();
    int diff = 0;
    
    if (currentPos != lastEncoderPos) {
        diff = (int)(currentPos - lastEncoderPos);
        lastEncoderPos = currentPos;
        sys.lastInteractionTime = currentMillis; // Reset idle timer (Fixed underflow)
    }
    
    bool clicked = encoder.isButtonPressed();
    bool rawBtn = (digitalRead(Config::PIN_ENC_SW) == LOW); // Raw check for robustness

    if (clicked || rawBtn) sys.lastInteractionTime = currentMillis; 
    
    // Wake up screen on interaction
    if ((diff != 0 || clicked || rawBtn) && sys.currentState == AppState::SLEEP) {
        Serial.println("Waking up (Raw/Debounced)!");
        sys.currentState = AppState::MONITOR;
        u8g2.setPowerSave(0); // Screen ON
        delay(10); // Stabilize
        return;
    }

    // State Machine Input Logic
    if (sys.currentState != AppState::SLEEP) {
        if (sys.currentState == AppState::SETTINGS) {
            // Menu Navigation
            if (diff != 0) {
                sys.settingsIndex += diff;
                if (sys.settingsIndex < 0) sys.settingsIndex = 4;
                if (sys.settingsIndex > 4) sys.settingsIndex = 0;
            }
            if (clicked) {
                sys.nextSleepInterval = sys.intervals[sys.settingsIndex]; // Update PENDING
                sys.currentState = AppState::MONITOR; // Go back after select
            }
        } else {
            // View Switching
            if (diff != 0) {
                int state = (int)sys.currentState + diff;
                if (state < 0) state = 3; // Wrap
                if (state > 3) state = 0;
                sys.currentState = (AppState)state;
            }
        }
    }

    // 2. Sensor Logic (Sleep/Wake)
    if (!sys.isSensorSleeping) {
        PmsDriver::update();
        // Check if time to sleep
        if (currentMillis - sys.lastSensorWakeTime > 30000) { // 30s active
            PmsDriver::sleep();
        }
    } else {
        // Check if time to wake
        if (currentMillis - sys.lastSensorSleepTime > sys.sleepInterval) { // Use ACTIVE interval
            PmsDriver::wake();
        }
    }

    // 3. Screen Saver Logic (OLED Protection)
    if (sys.currentState != AppState::SLEEP && 
        currentMillis - sys.lastInteractionTime > Config::SCREEN_SAVER_TIMEOUT) {
        sys.currentState = AppState::SLEEP;
        u8g2.setPowerSave(1); // Screen OFF
    }

    // 4. Render
    if (sys.currentState != AppState::SLEEP) {
        u8g2.firstPage();
        do {
            switch(sys.currentState) {
                case AppState::MONITOR: renderMonitor(); break;
                case AppState::GRAPH:   renderGraph(); break;
                case AppState::STATS:   renderStats(); break;
                case AppState::SETTINGS:renderSettings(); break;
            }
        } while (u8g2.nextPage());
    }
}

// rev01 – Sleep Mode UI Example for PMS7003 Sensor
// This sketch demonstrates how to control the Sleep/Wake modes of the PMS7003 sensor
// using a Serial UI to set the sleep duration.
//
// Scenario:
// 1. Sensor Wakes up.
// 2. Measures and prints data for 30 seconds.
// 3. Goes to Sleep for a user-defined duration (1, 3, 5, 10, 20 minutes).
// 4. Repeats.

// [Updated] for Arduino Uno R4 WiFi (Hardware Serial1)
// Wiring: PMS TX -> Pin 0, PMS RX -> Pin 1

// -----------------------------------------------------------------------------
// Configuration constants
// -----------------------------------------------------------------------------
const uint32_t SERIAL_BAUD = 115200;
const uint32_t PMS_BAUD = 9600;
const uint16_t PACKET_SIZE = 32;

// Duration constants
const unsigned long WAKE_DURATION = 30000; // 30 seconds active time

// -----------------------------------------------------------------------------
// Command frames (hex)
// -----------------------------------------------------------------------------
// Sleep Command: 42 4D E4 00 00 01 73
const uint8_t CMD_SLEEP[] = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73};
// Wake Command:  42 4D E4 00 01 01 74
const uint8_t CMD_WAKEUP[] = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74};
// Active Mode (Ensure we are in active mode to get data automatically): 42 4D E1 00 01 01 71
const uint8_t CMD_ACTIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71};
// Passive Mode: 42 4D E1 00 00 01 70
const uint8_t CMD_PASSIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------
unsigned long sleepInterval = 60000; // Default 1 minute (in ms)
unsigned long lastStateChangeTime = 0;
unsigned long lastDataPrintTime = 0; // Added for 2s print interval
unsigned long lastCountdownTime = 0; // Added for 10s sleep countdown
bool isSleeping = false;             // Track current state

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
uint16_t calculateChecksum(const uint8_t *buf)
{
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 30; ++i)
        sum += buf[i];
    return sum;
}

void printConcentrations(const uint8_t *packet)
{
    uint16_t pm1_0 = (packet[10] << 8) | packet[11];
    uint16_t pm2_5 = (packet[12] << 8) | packet[13];
    uint16_t pm10 = (packet[14] << 8) | packet[15];

    Serial.print(F("[측정중] PM1.0: "));
    Serial.print(pm1_0);
    Serial.print(F(" | PM2.5: "));
    Serial.print(pm2_5);
    Serial.print(F(" | PM10: "));
    Serial.println(pm10);
}

void sendCommand(const uint8_t *cmd, size_t len)
{
    Serial1.write(cmd, len);
    delay(100); // Small delay to ensure transmission
}

void showMenu()
{
    Serial.println(F("\n========================================"));
    Serial.println(F("      PMS7003 절전 모드 제어 UI"));
    Serial.println(F("========================================"));
    Serial.print(F("현재 설정된 휴식 시간: "));
    Serial.print(sleepInterval / 60000);
    Serial.println(F(" 분"));
    Serial.println(F("----------------------------------------"));
    Serial.println(F("변경하려면 아래 숫자를 입력하세요:"));
    Serial.println(F(" 1  : 1분 휴식"));
    Serial.println(F(" 3  : 3분 휴식"));
    Serial.println(F(" 5  : 5분 휴식"));
    Serial.println(F(" 10 : 10분 휴식"));
    Serial.println(F(" 20 : 20분 휴식"));
    Serial.println(F("========================================\n"));
}

bool handleInput(int input)
{
    bool valid = false;
    switch (input)
    {
    case 1:
        sleepInterval = 60000;
        valid = true;
        break;
    case 3:
        sleepInterval = 180000;
        valid = true;
        break;
    case 5:
        sleepInterval = 300000;
        valid = true;
        break;
    case 10:
        sleepInterval = 600000;
        valid = true;
        break;
    case 20:
        sleepInterval = 1200000;
        valid = true;
        break;
    default:
        Serial.println(F("!! 잘못된 입력입니다. (1, 3, 5, 10, 20 중 선택)"));
        break;
    }

    if (valid)
    {
        Serial.print(F(">> 휴식 시간이 "));
        Serial.print(input);
        Serial.println(F("분으로 변경되었습니다."));
        showMenu();
    }
    lastCountdownTime = millis(); // Reset countdown timer
    return valid;
}

void wakeUpSensor()
{
    Serial.println(F(">>> 센서 깨우기 (Wake Up) >>>"));
    sendCommand(CMD_WAKEUP, sizeof(CMD_WAKEUP));
    // Ensure Active Mode is set after waking up
    sendCommand(CMD_ACTIVE_MODE, sizeof(CMD_ACTIVE_MODE));
    isSleeping = false;
    lastStateChangeTime = millis();
    Serial.println(F(">>> 30초간 측정을 시작합니다..."));
}

void sleepSensor()
{ // Try switching to Passive Mode first to stop data stream
    sendCommand(CMD_PASSIVE_MODE, sizeof(CMD_PASSIVE_MODE));
    delay(100);

    Serial.println(F("<<< 센서 재우기 (Sleep) <<<"));
    sendCommand(CMD_SLEEP, sizeof(CMD_SLEEP));
    isSleeping = true;
    lastStateChangeTime = millis();
    lastCountdownTime = millis(); // Reset countdown timer
    Serial.print(F("<<< "));
    Serial.print(sleepInterval / 60000);
    Serial.println(F("분 동안 대기합니다..."));
}

// -----------------------------------------------------------------------------
// Arduino lifecycle
// -----------------------------------------------------------------------------
void setup()
{
    Serial.begin(SERIAL_BAUD);
    Serial1.begin(PMS_BAUD);
    Serial.println(F("※ 주의: PMS7003의 RX 핀이 아두이노의 TX(Pin 1)에 연결되어 있어야 명령이 전달됩니다."));
    delay(1000);

    Serial.println(F("시스템 시작..."));

    // Initial Setup
    showMenu();

    Serial.println(F(">>> 40초 동안 입력을 기다립니다. (입력 없으면 기본 1분 휴식 시작)"));
    unsigned long startWait = millis();
    unsigned long lastPrint = 0;
    bool inputReceived = false;

    while (millis() - startWait < 40000)
    {
        if (millis() - lastPrint >= 1000)
        {
            lastPrint = millis();
            int remaining = (40000 - (millis() - startWait)) / 1000;
            Serial.print(F("남은 시간: "));
            Serial.print(remaining);
            Serial.println(F("초"));
        }

        if (Serial.available() > 0)
        {
            int input = Serial.parseInt();
            while (Serial.available())
                Serial.read();
            if (input > 0)
            {
                if (handleInput(input))
                {
                    inputReceived = true;
                    break;
                }
            }
        }
    }

    if (!inputReceived)
    {
        Serial.println(F(">>> 입력이 없어 기본 설정(1분)으로 시작합니다."));
    }
    else
    {
        Serial.println(F(">>> 설정을 적용하여 시작합니다."));
    }

    wakeUpSensor(); // Start with waking up
}

void loop()
{
    unsigned long currentMillis = millis();

    // 1. Handle User Input (UI)
    if (Serial.available() > 0)
    {
        int input = Serial.parseInt();
        // Consume newline characters
        while (Serial.available())
            Serial.read();

        if (input > 0)
            handleInput(input);
    }

    // 2. Handle Sensor State Machine
    if (isSleeping)
    {
        // Check if sleep time is over
        if (currentMillis - lastStateChangeTime >= sleepInterval)
        {
            wakeUpSensor();
        }
        else
        {
            // Countdown every 10 seconds
            if (currentMillis - lastCountdownTime >= 10000)
            {
                lastCountdownTime = currentMillis;
                long remainingSeconds = (sleepInterval - (currentMillis - lastStateChangeTime)) / 1000;
                Serial.print(F("... [절전 중] 남은 시간: "));
                Serial.print(remainingSeconds);
                Serial.println(F("초 (설정 변경 가능: 1,3,5,10,20)"));
            }
        }
    }
    else
    {
        // Sensor is Awake (Measuring)

        // Read Data
        if (Serial1.available() >= PACKET_SIZE)
        {
            if (Serial1.peek() != 0x42)
            {
                Serial1.read();
            }
            else
            {
                Serial1.read(); // 0x42
                if (Serial1.peek() != 0x4D)
                {
                    // Invalid
                }
                else
                {
                    uint8_t packet[PACKET_SIZE];
                    packet[0] = 0x42;
                    packet[1] = Serial1.read(); // 0x4D
                    for (uint8_t i = 2; i < PACKET_SIZE; ++i)
                        packet[i] = Serial1.read();

                    if (calculateChecksum(packet) == ((packet[30] << 8) | packet[31]))
                    {
                        // Print data every 2 seconds
                        if (currentMillis - lastDataPrintTime >= 2000)
                        {
                            printConcentrations(packet);
                            lastDataPrintTime = currentMillis;
                        }
                    }
                }
            }
        }

        // Check if wake duration (30s) is over
        if (currentMillis - lastStateChangeTime >= WAKE_DURATION)
        {
            sleepSensor();
        }
    }
}

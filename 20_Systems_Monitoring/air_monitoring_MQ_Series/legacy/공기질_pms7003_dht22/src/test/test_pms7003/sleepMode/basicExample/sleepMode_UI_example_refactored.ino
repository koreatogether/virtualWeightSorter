/**
 * @file sleepMode_UI_example_refactored.ino
 * @brief PMS7003 슬립 모드 제어 및 UI 예제 (리팩터링 버전)
 *
 * 이 코드는 'ARDUINO_CODING_GUIDELINES.md'를 준수하여 리팩터링되었습니다.
 * - 네임스페이스를 활용한 구조화
 * - 매직 넘버 제거 및 상수화
 * - 함수 분리 및 중첩 최소화
 * - 가독성 중심의 loop() 구조
 */

// -----------------------------------------------------------------------------
// 1. 설정 및 상수 (Configuration & Constants)
// -----------------------------------------------------------------------------
namespace Config
{
    constexpr uint32_t SERIAL_BAUD = 115200;
    constexpr uint32_t PMS_BAUD = 9600;
    constexpr uint16_t PACKET_SIZE = 32;

    // 시간 설정 (ms)
    constexpr unsigned long WAKE_DURATION = 30000;      // 측정 시간 (30초)
    constexpr unsigned long DEFAULT_SLEEP_TIME = 60000; // 기본 휴식 시간 (1분)
    constexpr unsigned long INITIAL_WAIT_TIME = 40000;  // 초기 대기 시간 (40초)
    constexpr unsigned long DATA_PRINT_INTERVAL = 2000; // 데이터 출력 주기 (2초)
    constexpr unsigned long COUNTDOWN_INTERVAL = 10000; // 카운트다운 출력 주기 (10초)
}

namespace PmsCommand
{
    const uint8_t SLEEP[] = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73};
    const uint8_t WAKEUP[] = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74};
    const uint8_t ACTIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71};
    const uint8_t PASSIVE_MODE[] = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};
}

// -----------------------------------------------------------------------------
// 2. 전역 상태 (Global State)
// -----------------------------------------------------------------------------
namespace State
{
    unsigned long sleepInterval = Config::DEFAULT_SLEEP_TIME;
    unsigned long lastStateChangeTime = 0;
    unsigned long lastDataPrintTime = 0;
    unsigned long lastCountdownTime = 0;
    bool isSleeping = false;
}

// -----------------------------------------------------------------------------
// 3. PMS 센서 제어 (PMS Sensor Control)
// -----------------------------------------------------------------------------
namespace PmsSensor
{
    // 명령 전송 헬퍼
    void sendCommand(const uint8_t *cmd, size_t len)
    {
        Serial1.write(cmd, len);
        delay(100); // 전송 안정화 대기
    }

    // 체크섬 계산
    uint16_t calculateChecksum(const uint8_t *buf)
    {
        uint16_t sum = 0;
        for (uint8_t i = 0; i < 30; ++i)
            sum += buf[i];
        return sum;
    }

    // 센서 깨우기
    void wakeUp()
    {
        Serial.println(F(">>> 센서 깨우기 (Wake Up) >>>"));
        sendCommand(PmsCommand::WAKEUP, sizeof(PmsCommand::WAKEUP));
        // 데이터 자동 수신을 위해 Active Mode로 전환
        sendCommand(PmsCommand::ACTIVE_MODE, sizeof(PmsCommand::ACTIVE_MODE));

        State::isSleeping = false;
        State::lastStateChangeTime = millis();
        Serial.println(F(">>> 30초간 측정을 시작합니다..."));
    }

    // 센서 재우기
    void sleep()
    {
        // 데이터 스트림을 멈추기 위해 먼저 Passive Mode로 전환
        sendCommand(PmsCommand::PASSIVE_MODE, sizeof(PmsCommand::PASSIVE_MODE));
        delay(100);

        Serial.println(F("<<< 센서 재우기 (Sleep) <<<"));
        sendCommand(PmsCommand::SLEEP, sizeof(PmsCommand::SLEEP));

        State::isSleeping = true;
        State::lastStateChangeTime = millis();
        State::lastCountdownTime = millis(); // 카운트다운 타이머 리셋

        Serial.print(F("<<< "));
        Serial.print(State::sleepInterval / 60000);
        Serial.println(F("분 동안 대기합니다..."));
    }

    // 데이터 패킷 읽기 및 처리
    void processIncomingData()
    {
        if (Serial1.available() < Config::PACKET_SIZE)
            return;

        // 헤더 확인 (0x42)
        if (Serial1.peek() != 0x42)
        {
            Serial1.read(); // 잘못된 바이트 버림
            return;
        }

        // 두 번째 헤더 확인을 위해 첫 바이트 읽음
        Serial1.read(); // 0x42 소비
        if (Serial1.peek() != 0x4D)
            return; // 0x4D가 아니면 리턴

        // 패킷 읽기
        uint8_t packet[Config::PACKET_SIZE];
        packet[0] = 0x42;
        packet[1] = Serial1.read(); // 0x4D 소비

        for (uint8_t i = 2; i < Config::PACKET_SIZE; ++i)
        {
            packet[i] = Serial1.read();
        }

        // 체크섬 검증 및 출력
        uint16_t receivedChecksum = (packet[30] << 8) | packet[31];
        if (calculateChecksum(packet) == receivedChecksum)
        {
            unsigned long currentMillis = millis();
            if (currentMillis - State::lastDataPrintTime >= Config::DATA_PRINT_INTERVAL)
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

                State::lastDataPrintTime = currentMillis;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 4. 사용자 인터페이스 (User Interface)
// -----------------------------------------------------------------------------
namespace Ui
{
    void showMenu()
    {
        Serial.println(F("\n========================================"));
        Serial.println(F("      PMS7003 절전 모드 제어 UI"));
        Serial.println(F("========================================"));
        Serial.print(F("현재 설정된 휴식 시간: "));
        Serial.print(State::sleepInterval / 60000);
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
        unsigned long newInterval = 0;

        switch (input)
        {
        case 1:
            newInterval = 60000;
            valid = true;
            break;
        case 3:
            newInterval = 180000;
            valid = true;
            break;
        case 5:
            newInterval = 300000;
            valid = true;
            break;
        case 10:
            newInterval = 600000;
            valid = true;
            break;
        case 20:
            newInterval = 1200000;
            valid = true;
            break;
        default:
            Serial.println(F("!! 잘못된 입력입니다. (1, 3, 5, 10, 20 중 선택)"));
            break;
        }

        if (valid)
        {
            State::sleepInterval = newInterval;
            Serial.print(F(">> 휴식 시간이 "));
            Serial.print(input);
            Serial.println(F("분으로 변경되었습니다."));
            showMenu();

            // 카운트다운 타이머 리셋 (즉시 반영된 시간 보여주기 위해)
            State::lastCountdownTime = millis();
        }
        return valid;
    }

    void processInput()
    {
        if (Serial.available() > 0)
        {
            int input = Serial.parseInt();
            // 버퍼 비우기
            while (Serial.available())
                Serial.read();

            if (input > 0)
            {
                handleInput(input);
            }
        }
    }

    // 초기 대기 화면 처리
    void waitForInitialInput()
    {
        Serial.println(F(">>> 40초 동안 입력을 기다립니다. (입력 없으면 기본 1분 휴식 시작)"));

        unsigned long startWait = millis();
        unsigned long lastPrint = 0;
        bool inputReceived = false;

        while (millis() - startWait < Config::INITIAL_WAIT_TIME)
        {
            // 1초마다 남은 시간 출력
            if (millis() - lastPrint >= 1000)
            {
                lastPrint = millis();
                int remaining = (Config::INITIAL_WAIT_TIME - (millis() - startWait)) / 1000;
                Serial.print(F("남은 시간: "));
                Serial.print(remaining);
                Serial.println(F("초"));
            }

            // 입력 감지
            if (Serial.available() > 0)
            {
                int input = Serial.parseInt();
                while (Serial.available())
                    Serial.read();

                if (input > 0 && handleInput(input))
                {
                    inputReceived = true;
                    break; // 입력 받으면 대기 종료
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
    }
}

// -----------------------------------------------------------------------------
// 5. 메인 로직 (Main Logic)
// -----------------------------------------------------------------------------

// 슬립 상태 처리 로직
void handleSleepState()
{
    unsigned long currentMillis = millis();

    // 휴식 시간 종료 체크
    if (currentMillis - State::lastStateChangeTime >= State::sleepInterval)
    {
        PmsSensor::wakeUp();
    }
    else
    {
        // 휴식 중 카운트다운 표시
        if (currentMillis - State::lastCountdownTime >= Config::COUNTDOWN_INTERVAL)
        {
            State::lastCountdownTime = currentMillis;
            long remainingSeconds = (State::sleepInterval - (currentMillis - State::lastStateChangeTime)) / 1000;

            Serial.print(F("... [절전 중] 남은 시간: "));
            Serial.print(remainingSeconds);
            Serial.println(F("초 (설정 변경 가능: 1,3,5,10,20)"));
        }
    }
}

// 웨이크(측정) 상태 처리 로직
void handleWakeState()
{
    unsigned long currentMillis = millis();

    // 데이터 수신 및 처리
    PmsSensor::processIncomingData();

    // 측정 시간 종료 체크
    if (currentMillis - State::lastStateChangeTime >= Config::WAKE_DURATION)
    {
        PmsSensor::sleep();
    }
}

// 시스템 상태 업데이트 (루프의 핵심)
void updateSystem()
{
    if (State::isSleeping)
    {
        handleSleepState();
    }
    else
    {
        handleWakeState();
    }
}

// -----------------------------------------------------------------------------
// 6. 아두이노 라이프사이클 (Arduino Lifecycle)
// -----------------------------------------------------------------------------
void setup()
{
    Serial.begin(Config::SERIAL_BAUD);
    Serial1.begin(Config::PMS_BAUD);

    Serial.println(F("※ 주의: PMS7003의 RX 핀이 아두이노의 TX(Pin 1)에 연결되어 있어야 명령이 전달됩니다."));
    delay(1000);
    Serial.println(F("시스템 시작..."));

    Ui::showMenu();
    Ui::waitForInitialInput(); // 초기 대기

    PmsSensor::wakeUp(); // 측정 시작
}

void loop()
{
    // 1. 사용자 입력 처리
    Ui::processInput();

    // 2. 시스템 상태 갱신 및 로직 수행
    updateSystem();
}

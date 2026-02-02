#include <Nextion.h>

// [자동 포트 감지 매크로]
// 보드 매크로를 확인하여 Serial1 존재 여부 판단
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_SAM_DUE)
// 하드웨어 시리얼 포트가 여유있는 보드들
#define HMISerial Serial1
#define DEBUG_ENABLE 1 // PC 디버깅 켜기
#else
// 포트가 하나뿐인 보드들 (Uno, Nano 등)
// 경고: Serial1 에러 발생 방지를 위해 Serial 사용
#define HMISerial Serial
#define DEBUG_ENABLE 0 // PC 디버깅 끄기 (충돌 방지)
#endif

/*
05버전에서는 나노 보드 기준으로 작동하되
버퍼 관리 방식으로 웨이브폼을 동기화한다.
채널을 늦게 활성화해도 동일 시점부터 그려지도록
버퍼를 재생(replay)한다.
*/

// [설정] Nextion Waveform ID (메타데이터 기준 11)
const int WAVEFORM_ID = 11;
const int CHANNELS = 4; // 4채널 사용

// [설정] Xfloat 컴포넌트 정보 (x0 ~ x3)
const char *XFLOAT_OBJS[4] = {"x0", "x1", "x2", "x3"};

// [설정] 그래프 매핑용 범위
const int WAVEFORM_HEIGHT = 141; // 그래프 컴포넌트 높이
const int TEMP_MIN = 0;          // 최저 온도
const int TEMP_MAX = 100;        // 최고 온도

// [추가 설정] 가로 간격 조정
const int LINE_STEPS = 25; // 한 샘플을 가로 2칸으로 표현

// [버퍼 설정]
// 2초 간격 샘플 기준, 120개 = 약 4분
const int BUFFER_LENGTH = 120;
const uint16_t REPLAY_MAX_SAMPLES = 40;
const unsigned long DRAW_SUSPEND_MS = 150;

// 각 채널별 이전 값 저장용
int lastPlotValues[4] = {-1, -1, -1, -1};

// 샘플 버퍼 (채널별 0~255 저장)
uint8_t waveformBuffer[CHANNELS][BUFFER_LENGTH];
uint16_t bufferIndex = 0;
uint16_t bufferCount = 0;
bool drawSuspended = false;
unsigned long drawResumeMillis = 0;

// 타이머 관리 (비차단)
unsigned long prevUpdateMillis = 0;
const unsigned long UPDATE_INTERVAL = 2000; // 01 코드의 delay(2000) 대체

// t1 시간 표시 (YY-MM-DD HH:MM:SS)
struct DateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

DateTime displayTime = {2026, 1, 31, 22, 30, 5};
unsigned long prevClockMillis = 0;
const unsigned long CLOCK_INTERVAL = 1000;

// 버튼/채널 설정
const int PAGE_BUTTON_IDS[4] = {16, 17, 18, 19};
const int DS_BUTTON_IDS[5] = {20, 21, 22, 23, 24};

bool activeChannels[4] = {false, false, false, false};

// 함수 선언
void initNextionDisplay();
void manageDisplaySystem();
void updateWaveformData();
void sendNextionCommand(const char *cmd);
void waitForMillis(unsigned long waitMs);
void processNextionInput();
void handleTouchEvent(uint8_t pageId, uint8_t componentId, uint8_t eventType);
void updateClock();
void sendTimeToDisplay();
void tickSecond(DateTime &timeInfo);
int daysInMonth(int year, int month);
bool isLeapYear(int year);
void setChannelActive(int channel, bool isActive);
void setAllChannels(bool isActive);
bool areAllChannelsActive();
void syncDualStateButtons();
void clearWaveformChannel(int channel);
void clearWaveformAll();
void storeWaveformSamples(const int plotValues[4]);
void replayChannel(int channel);
void sendWaveformPoint(int channel, int value);
void suspendWaveformDraw();

void setup()
{
    // 디버그용 시리얼 초기화 (충돌 방지: Nano에서는 비활성)
#if DEBUG_ENABLE
    Serial.begin(9600);
#endif

    // 1. HMI 포트 초기화 (기본 9600)
    HMISerial.begin(9600);

    // 2. 넥시온 라이브러리 초기화
    // (nconfig.h 설정과 관계없이 이 스케치 파일의 로직이 동작하도록 함)
    nexInit();

    // [속도 향상] 하드웨어 시리얼의 장점을 살리기 위해 통신 속도를 115200bps로 변경
    // SoftwareSerial은 115200에서 불안정하지만, HardwareSerial은 안정적입니다.
    // 기존 9600bps 대비 약 12배 빠른 전송이 가능해져 그래프가 빠르게 그려집니다.
    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    delay(100);              // 디스플레이가 통신 속도를 변경할 시간을 잠시 줌
    HMISerial.begin(115200); // 아두이노측 시리얼 포트 속도 재설정 (Hardware Serial)

    // 01 코드의 초기화 흐름 유지
    initNextionDisplay();

    // 초기 채널 상태: 모두 꺼짐
    setAllChannels(false);

    // 첫 루프에서 즉시 갱신되도록 타이머 보정
    prevUpdateMillis = millis() - UPDATE_INTERVAL;
}

void loop()
{
    // 1. 화면 보호기 관리 (루프에서 지속)
    manageDisplaySystem();

    // 2. 넥션 터치 이벤트 처리
    processNextionInput();

    // 3. 시간 업데이트 (t1)
    updateClock();

    // 4. 비차단 방식으로 그래프 업데이트
    updateWaveformData();
}

void initNextionDisplay()
{
    // 넥션 리셋 신호
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    // 그래프 초기화 (Clear)
    HMISerial.print("cle ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",255");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    // 초기 시간 표시
    sendTimeToDisplay();
}

void manageDisplaySystem()
{
    static bool isConfigApplied = false;

    if (!isConfigApplied)
    {
        // thsp: 60초간 터치 없으면 절전 모드 진입
        sendNextionCommand("thsp=60");
        // thup: 화면 어디든 터치하면 즉시 복귀
        sendNextionCommand("thup=1");

        isConfigApplied = true;
    }
}

void updateWaveformData()
{
    unsigned long currentMillis = millis();

    if (currentMillis - prevUpdateMillis < UPDATE_INTERVAL)
    {
        return;
    }

    prevUpdateMillis = currentMillis;

    // 4채널 온도 및 매핑 값 저장용 배열
    float currentTemps[4];
    int plotValues[4];

    // 1. 센서 데이터 생성 (시뮬레이션)
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        currentTemps[ch] = random(400 + (ch * 50), 800 - (ch * 50)) / 10.0;
    }

    // [제약조건 적용]
    if (currentTemps[1] > currentTemps[0])
    {
        currentTemps[1] = currentTemps[0];
    }
    if (currentTemps[3] > currentTemps[2])
    {
        currentTemps[3] = currentTemps[2];
    }

    // 2. 값 매핑 및 Xfloat 전송
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        plotValues[ch] = map((int)currentTemps[ch], TEMP_MIN, TEMP_MAX, 0, WAVEFORM_HEIGHT - 1);

        if (plotValues[ch] < 0)
        {
            plotValues[ch] = 0;
        }
        if (plotValues[ch] > 255)
        {
            plotValues[ch] = 255;
        }

        if (activeChannels[ch] && lastPlotValues[ch] == -1)
        {
            lastPlotValues[ch] = plotValues[ch];
        }

        int xfloatVal = (int)(currentTemps[ch] * 10);
        HMISerial.print(XFLOAT_OBJS[ch]);
        HMISerial.print(".val=");
        HMISerial.print(xfloatVal);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
    }

    // 3. 버퍼 저장 (채널 상태와 무관하게 기록)
    storeWaveformSamples(plotValues);

    // 4. 차이값(Delta T) 업데이트 (x4, x5)
    int diff1 = (int)((currentTemps[0] - currentTemps[1]) * 10);
    int diff2 = (int)((currentTemps[2] - currentTemps[3]) * 10);

    HMISerial.print("x4.val=");
    HMISerial.print(diff1);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    HMISerial.print("x5.val=");
    HMISerial.print(diff2);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    if (drawSuspended)
    {
        if (millis() < drawResumeMillis)
        {
            return;
        }

        drawSuspended = false;
    }

    // 5. 그래프 그리기 (점과 점 사이를 직선으로 채움)
    // LINE_STEPS 만큼 점을 전송하여 직선 구간을 형성
    for (int step = 1; step <= LINE_STEPS; step++)
    {
        for (int ch = 0; ch < CHANNELS; ch++)
        {
            if (!activeChannels[ch])
            {
                continue;
            }

            long diff = plotValues[ch] - lastPlotValues[ch];
            int interpolatedValue = lastPlotValues[ch] + (diff * step / LINE_STEPS);

            sendWaveformPoint(ch, interpolatedValue);
        }
    }

    // 다음 루프를 위해 현재 값을 이전 값으로 저장
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        if (activeChannels[ch])
        {
            lastPlotValues[ch] = plotValues[ch];
        }
    }
}

void sendNextionCommand(const char *cmd)
{
    HMISerial.print(cmd);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
}

void waitForMillis(unsigned long waitMs)
{
    unsigned long startMs = millis();
    while (millis() - startMs < waitMs)
    {
        // 대기 (delay 미사용)
    }
}

void processNextionInput()
{
    static bool readingEvent = false;
    static uint8_t eventData[3] = {0};
    static uint8_t dataIndex = 0;
    static uint8_t ffCount = 0;

    while (HMISerial.available() > 0)
    {
        uint8_t incoming = HMISerial.read();

        if (!readingEvent)
        {
            if (incoming == 0x65)
            {
                readingEvent = true;
                dataIndex = 0;
                ffCount = 0;
            }
            continue;
        }

        if (dataIndex < 3)
        {
            eventData[dataIndex++] = incoming;
            continue;
        }

        if (incoming == 0xFF)
        {
            ffCount++;
            if (ffCount >= 3)
            {
                handleTouchEvent(eventData[0], eventData[1], eventData[2]);
                readingEvent = false;
                dataIndex = 0;
                ffCount = 0;
            }
        }
        else
        {
            readingEvent = false;
            dataIndex = 0;
            ffCount = 0;
        }
    }
}

void handleTouchEvent(uint8_t pageId, uint8_t componentId, uint8_t eventType)
{
    if (eventType != 0 && eventType != 1)
    {
        return;
    }

    (void)pageId;

    for (int i = 0; i < 5; i++)
    {
        if (componentId == DS_BUTTON_IDS[i])
        {
            if (i == CHANNELS)
            {
                setAllChannels(!areAllChannelsActive());
            }
            else
            {
                setChannelActive(i, !activeChannels[i]);
            }
            return;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (componentId == PAGE_BUTTON_IDS[i])
        {
            char cmd[16];
            snprintf(cmd, sizeof(cmd), "page %d", i);
            sendNextionCommand(cmd);
            return;
        }
    }
}

void updateClock()
{
    unsigned long currentMillis = millis();
    bool timeChanged = false;

    while (currentMillis - prevClockMillis >= CLOCK_INTERVAL)
    {
        prevClockMillis += CLOCK_INTERVAL;
        tickSecond(displayTime);
        timeChanged = true;
    }

    if (timeChanged)
    {
        sendTimeToDisplay();
    }
}

void sendTimeToDisplay()
{
    char cmd[40];
    int shortYear = displayTime.year % 100;
    snprintf(cmd, sizeof(cmd), "t1.txt=\"%02d-%02d-%02d %02d:%02d:%02d\"",
             shortYear,
             displayTime.month,
             displayTime.day,
             displayTime.hour,
             displayTime.minute,
             displayTime.second);
    sendNextionCommand(cmd);
}

void tickSecond(DateTime &timeInfo)
{
    timeInfo.second++;
    if (timeInfo.second < 60)
    {
        return;
    }

    timeInfo.second = 0;
    timeInfo.minute++;
    if (timeInfo.minute < 60)
    {
        return;
    }

    timeInfo.minute = 0;
    timeInfo.hour++;
    if (timeInfo.hour < 24)
    {
        return;
    }

    timeInfo.hour = 0;
    timeInfo.day++;
    int maxDay = daysInMonth(timeInfo.year, timeInfo.month);
    if (timeInfo.day <= maxDay)
    {
        return;
    }

    timeInfo.day = 1;
    timeInfo.month++;
    if (timeInfo.month <= 12)
    {
        return;
    }

    timeInfo.month = 1;
    timeInfo.year++;
}

int daysInMonth(int year, int month)
{
    switch (month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        return isLeapYear(year) ? 29 : 28;
    default:
        return 30;
    }
}

bool isLeapYear(int year)
{
    if ((year % 4) != 0)
    {
        return false;
    }
    if ((year % 100) != 0)
    {
        return true;
    }
    return (year % 400) == 0;
}

void setChannelActive(int channel, bool isActive)
{
    if (channel < 0 || channel >= CHANNELS)
    {
        return;
    }

    if (activeChannels[channel] == isActive)
    {
        return;
    }

    activeChannels[channel] = isActive;

    if (isActive)
    {
        if (bufferCount > 0)
        {
            replayChannel(channel);
        }
        else
        {
            clearWaveformChannel(channel);
            lastPlotValues[channel] = -1;
        }
    }
    else
    {
        clearWaveformChannel(channel);
        lastPlotValues[channel] = -1;
    }

    syncDualStateButtons();
}

void setAllChannels(bool isActive)
{
    if (!isActive)
    {
        for (int ch = 0; ch < CHANNELS; ch++)
        {
            activeChannels[ch] = false;
            lastPlotValues[ch] = -1;
        }

        clearWaveformAll();
        suspendWaveformDraw();
        syncDualStateButtons();
        return;
    }

    for (int ch = 0; ch < CHANNELS; ch++)
    {
        activeChannels[ch] = true;
        if (bufferCount > 0)
        {
            replayChannel(ch);
        }
        else
        {
            clearWaveformChannel(ch);
            lastPlotValues[ch] = -1;
        }
    }

    syncDualStateButtons();
}

bool areAllChannelsActive()
{
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        if (!activeChannels[ch])
        {
            return false;
        }
    }

    return true;
}

void syncDualStateButtons()
{
    for (int i = 0; i < 5; i++)
    {
        int val = 0;
        if (i == CHANNELS)
        {
            val = areAllChannelsActive() ? 1 : 0;
        }
        else if (i < CHANNELS && activeChannels[i])
        {
            val = 1;
        }

        char cmd[20];
        snprintf(cmd, sizeof(cmd), "bt%d.val=%d", i, val);
        sendNextionCommand(cmd);
    }
}

void clearWaveformChannel(int channel)
{
    if (channel < 0 || channel >= CHANNELS)
    {
        return;
    }

    HMISerial.print("cle ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",");
    HMISerial.print(channel);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
}

void clearWaveformAll()
{
    HMISerial.print("cle ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",255");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
}

void storeWaveformSamples(const int plotValues[4])
{
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        waveformBuffer[ch][bufferIndex] = (uint8_t)plotValues[ch];
    }

    bufferIndex++;
    if (bufferIndex >= BUFFER_LENGTH)
    {
        bufferIndex = 0;
    }

    if (bufferCount < BUFFER_LENGTH)
    {
        bufferCount++;
    }
}

void replayChannel(int channel)
{
    if (channel < 0 || channel >= CHANNELS)
    {
        return;
    }

    if (bufferCount == 0)
    {
        clearWaveformChannel(channel);
        lastPlotValues[channel] = -1;
        return;
    }

    clearWaveformChannel(channel);

    uint16_t sampleCount = bufferCount;
    if (sampleCount > REPLAY_MAX_SAMPLES)
    {
        sampleCount = REPLAY_MAX_SAMPLES;
    }

    uint16_t startIndex = (bufferIndex + BUFFER_LENGTH - sampleCount) % BUFFER_LENGTH;
    uint8_t previousValue = waveformBuffer[channel][startIndex];

    for (uint16_t i = 0; i < sampleCount; i++)
    {
        uint16_t index = (startIndex + i) % BUFFER_LENGTH;
        uint8_t currentValue = waveformBuffer[channel][index];
        int diff = (int)currentValue - (int)previousValue;

        for (int step = 1; step <= LINE_STEPS; step++)
        {
            int interpolatedValue = previousValue + (diff * step / LINE_STEPS);
            sendWaveformPoint(channel, interpolatedValue);
        }

        previousValue = currentValue;
    }

    lastPlotValues[channel] = previousValue;
}

void suspendWaveformDraw()
{
    drawSuspended = true;
    drawResumeMillis = millis() + DRAW_SUSPEND_MS;
}

void sendWaveformPoint(int channel, int value)
{
    HMISerial.print("add ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",");
    HMISerial.print(channel);
    HMISerial.print(",");
    HMISerial.print(value);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
}

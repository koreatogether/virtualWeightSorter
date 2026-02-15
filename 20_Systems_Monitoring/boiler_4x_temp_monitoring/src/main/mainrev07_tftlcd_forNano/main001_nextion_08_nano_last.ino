#include <Nextion.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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
06버전 rev3: X축 동기화 문제 해결
- 글로벌 웨이브폼 포인트 카운터 추가
- 채널 활성화 시 현재 X 위치까지 동기화
- 모든 채널이 항상 동일한 오른쪽 끝에서 시작
- 비활성 채널에도 더미 포인트 추가 (동기화 유지)
*/

// [설정] Nextion Waveform ID (메타데이터 기준 11)
const int WAVEFORM_ID = 11;
const int CHANNELS = 4; // 4채널 사용

// [설정] DS18B20 센서 핀
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorAddresses[4];
bool sensorFound[4] = {false, false, false, false};

// [설정] Xfloat 컴포넌트 정보 (x0 ~ x3)
const char *XFLOAT_OBJS[4] = {"x0", "x1", "x2", "x3"};

// [설정] 그래프 매핑용 범위
const int WAVEFORM_HEIGHT = 141; // 그래프 컴포넌트 높이
const int TEMP_MIN = 0;          // 최저 온도
const int TEMP_MAX = 100;        // 최고 온도

// [설정] 웨이브폼 가로 너비 (Nextion 컴포넌트 설정과 일치해야 함)
const int WAVEFORM_WIDTH = 300; // 그래프 컴포넌트 가로 너비

// [안정화] 가로 간격 조정
const int LINE_STEPS = 5; // 한 샘플을 가로 5칸으로 표현

// [안정화] 시리얼 전송 간 딜레이 (ms)
const int SEND_DELAY_MS = 1;

// [안정화] 채널 간 딜레이 (ms)
const int CHANNEL_DELAY_MS = 5;

// [버퍼 설정] RAM 부족 방지를 위해 100으로 조정
const int BUFFER_LENGTH = 100;
const uint16_t REPLAY_MAX_SAMPLES = 40;
const unsigned long DRAW_SUSPEND_MS = 200;

// 각 채널별 이전 값 저장용
int lastPlotValues[4] = {-1, -1, -1, -1};

// 샘플 버퍼 (채널별 0~255 저장)
uint8_t waveformBuffer[CHANNELS][BUFFER_LENGTH];
uint16_t bufferIndex = 0;
uint16_t bufferCount = 0;
bool drawSuspended = false;
unsigned long drawResumeMillis = 0;

// [X축 동기화] 글로벌 웨이브폼 포인트 카운터
// 시스템 시작 후 총 몇 개의 포인트가 전송되었는지 추적
uint32_t globalPointCount = 0;

// [X축 동기화] 각 채널의 마지막 동기화 시점
// 채널이 활성화될 때 globalPointCount 값을 저장
uint32_t channelSyncPoint[4] = {0, 0, 0, 0};

// 타이머 관리 (비차단)
unsigned long prevUpdateMillis = 0;
const unsigned long UPDATE_INTERVAL = 2000;

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
const int DS_BUTTON_IDS[5] = {20, 21, 22, 23, 24};

bool activeChannels[4] = {false, false, false, false};

// 함수 선언
void initNextionDisplay();
void updateWaveformData();
void sendNextionCommand(const char *cmd);
void sendNextionCommandWithDelay(const char *cmd, int delayMs);
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
void suspendWaveformDraw();
void sendWaveformPoint(int channel, int value);
void sendWaveformPointFast(int channel, int value);
void flushSerialAndWait();
void syncChannelToCurrentPosition(int channel);
void replayAllChannelsLockstep();

// DS18B20 주소 관리 함수
void initSensors();

void setup()
{
#if DEBUG_ENABLE
    Serial.begin(9600);
#endif

    HMISerial.begin(9600);
    nexInit();

    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    delay(100);
    HMISerial.begin(115200);

    flushSerialAndWait();
    initNextionDisplay();
    setAllChannels(false);
    
    // [센서 초기화 및 매칭]
    initSensors();
    
    // 글로벌 카운터 초기화
    globalPointCount = 0;
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        channelSyncPoint[ch] = 0;
    }

    prevUpdateMillis = millis() - UPDATE_INTERVAL;
}

void loop()
{
    processNextionInput();
    updateClock();
    updateWaveformData();
}

void flushSerialAndWait()
{
    HMISerial.flush();
    while (HMISerial.available() > 0)
    {
        HMISerial.read();
    }
    delay(10);
}

void initNextionDisplay()
{
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    delay(50);

    char cmd[32];
    snprintf(cmd, sizeof(cmd), "cle %d,255", WAVEFORM_ID);
    sendNextionCommandWithDelay(cmd, 10);

    sendTimeToDisplay();
}



void sendNextionCommandWithDelay(const char *cmd, int delayMs)
{
    sendNextionCommand(cmd);
    if (delayMs > 0)
    {
        delay(delayMs);
    }
}

void sendWaveformPoint(int channel, int value)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "add %d,%d,%d", WAVEFORM_ID, channel, value);
    sendNextionCommand(cmd);
    delay(SEND_DELAY_MS);
}

void sendWaveformPointFast(int channel, int value)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "add %d,%d,%d", WAVEFORM_ID, channel, value);
    sendNextionCommand(cmd);
}

// [X축 동기화] 새로 활성화된 채널을 현재 위치까지 동기화
void syncChannelToCurrentPosition(int channel)
{
    if (channel < 0 || channel >= CHANNELS)
    {
        return;
    }

    // 채널 클리어
    clearWaveformChannel(channel);
    
    // 현재 활성 채널 중 가장 많이 진행된 포인트 수 찾기
    uint32_t maxPoints = 0;
    bool hasOtherActiveChannels = false;
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        if (ch != channel && activeChannels[ch])
        {
            hasOtherActiveChannels = true;
            uint32_t channelPoints = globalPointCount - channelSyncPoint[ch];
            if (channelPoints > maxPoints)
            {
                maxPoints = channelPoints;
            }
        }
    }
    
    // [수정] 다른 활성 채널이 없으면 버퍼의 과거 데이터를 리플레이
    // 모든채널 버튼처럼 동작하도록 함
    if (!hasOtherActiveChannels && bufferCount > 0)
    {
        uint16_t samplesToReplay = bufferCount;
        if (samplesToReplay > REPLAY_MAX_SAMPLES)
        {
            samplesToReplay = REPLAY_MAX_SAMPLES;
        }
        maxPoints = samplesToReplay * LINE_STEPS;
    }
    
    // 웨이브폼 너비 내로 제한
    if (maxPoints > WAVEFORM_WIDTH)
    {
        maxPoints = WAVEFORM_WIDTH;
    }
    
    // 버퍼에서 재생할 샘플 수 계산
    // maxPoints는 포인트 수, LINE_STEPS로 나누면 샘플 수
    uint16_t samplesNeeded = maxPoints / LINE_STEPS;
    if (samplesNeeded > bufferCount)
    {
        samplesNeeded = bufferCount;
    }
    if (samplesNeeded > REPLAY_MAX_SAMPLES)
    {
        samplesNeeded = REPLAY_MAX_SAMPLES;
    }
    
    // 실제 재생할 포인트 수
    uint32_t pointsToReplay = samplesNeeded * LINE_STEPS;
    
    // 패딩 포인트 수 계산 (다른 채널과 동기화를 위해)
    // 다른 활성 채널이 없으면 패딩 필요 없음
    uint32_t paddingPoints = 0;
    if (hasOtherActiveChannels && maxPoints > pointsToReplay)
    {
        paddingPoints = maxPoints - pointsToReplay;
    }
    
    // 1. 먼저 패딩 포인트 추가 (투명하게 - 가장 낮은 값으로)
    // 웨이브폼 높이가 141이므로 0은 맨 아래에 그려짐
    // 패딩을 추가하여 다른 채널과 X축 위치 동기화
    int batchCount = 0;
    const int BATCH_SIZE = 10;
    
    for (uint32_t i = 0; i < paddingPoints; i++)
    {
        // 패딩은 가장 최근 버퍼값으로 (급격한 점프 방지)
        uint8_t paddingValue = 0;
        if (bufferCount > 0)
        {
            uint16_t latestIndex = (bufferIndex + BUFFER_LENGTH - 1) % BUFFER_LENGTH;
            paddingValue = waveformBuffer[channel][latestIndex];
        }
        
        sendWaveformPointFast(channel, paddingValue);
        batchCount++;
        
        if (batchCount >= BATCH_SIZE)
        {
            delay(2);
            batchCount = 0;
        }
    }
    
    // 2. 버퍼에서 실제 데이터 리플레이
    if (samplesNeeded > 0)
    {
        uint16_t startIndex = (bufferIndex + BUFFER_LENGTH - samplesNeeded) % BUFFER_LENGTH;
        uint8_t previousValue = waveformBuffer[channel][startIndex];
        
        for (uint16_t i = 0; i < samplesNeeded; i++)
        {
            uint16_t index = (startIndex + i) % BUFFER_LENGTH;
            uint8_t currentValue = waveformBuffer[channel][index];
            int diff = (int)currentValue - (int)previousValue;
            
            for (int step = 1; step <= LINE_STEPS; step++)
            {
                int interpolatedValue = previousValue + (diff * step / LINE_STEPS);
                sendWaveformPointFast(channel, interpolatedValue);
                batchCount++;
                
                if (batchCount >= BATCH_SIZE)
                {
                    delay(2);
                    batchCount = 0;
                }
            }
            
            previousValue = currentValue;
        }
        
        lastPlotValues[channel] = previousValue;
    }
    else
    {
        lastPlotValues[channel] = -1;
    }
    
    // 현재 글로벌 포인트 카운터와 동기화
    // 항상 maxPoints를 차감하여 이 채널이 "언제부터 시작되었는지"에 대한 가상 시작점을 일치시킴
    channelSyncPoint[channel] = globalPointCount - maxPoints;
    
    delay(CHANNEL_DELAY_MS);
}

void updateWaveformData()
{
    unsigned long currentMillis = millis();
    if (currentMillis - prevUpdateMillis < UPDATE_INTERVAL) return;
    prevUpdateMillis = currentMillis;

    float readTemps[4] = {0.0, 0.0, 0.0, 0.0};
    int plotValues[4];

    // 1. 센서 값 읽기 (독립)
    for (int i = 0; i < CHANNELS; i++)
    {
        if (sensorFound[i])
        {
            float t = sensors.getTempC(sensorAddresses[i]);
            if (t > -50.0 && t < 150.0) readTemps[i] = t;
            else readTemps[i] = 0.0;
        }
    }
    sensors.requestTemperatures(); // 다음 루프를 위해 비차단 요청

    // 2. Nextion x0~x3 전송 및 plotValues 계산
    for (int i = 0; i < CHANNELS; i++)
    {
        // 웨이브폼 매핑 (0~255)
        plotValues[i] = map((int)readTemps[i], TEMP_MIN, TEMP_MAX, 0, WAVEFORM_HEIGHT - 1);
        if (plotValues[i] < 0) plotValues[i] = 0;
        if (plotValues[i] > 255) plotValues[i] = 255;

        if (activeChannels[i] && lastPlotValues[i] == -1)
        {
            lastPlotValues[i] = plotValues[i];
        }

        // x0, x1, x2, x3 값 전송
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "%s.val=%d", XFLOAT_OBJS[i], (int)(readTemps[i] * 10));
        sendNextionCommand(cmd);
    }

    // 3. 차이값 x4(S1-S2), x5(S3-S4) 계산
    char diffCmd[32];
    snprintf(diffCmd, sizeof(diffCmd), "x4.val=%d", (int)((readTemps[0] - readTemps[1]) * 10));
    sendNextionCommand(diffCmd);
    snprintf(diffCmd, sizeof(diffCmd), "x5.val=%d", (int)((readTemps[2] - readTemps[3]) * 10));
    sendNextionCommand(diffCmd);

    // 4. 웨이브폼 버퍼 저장 및 동기화 전송 시작
    storeWaveformSamples(plotValues);

    if (drawSuspended)
    {
        if (millis() < drawResumeMillis) return;
        drawSuspended = false;
    }

    // 5. [X축 동기화] 스텝별 순차 전송
    int activeCount = 0;
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        if (activeChannels[ch])
        {
            activeCount++;
        }
    }
    
    if (activeCount == 0)
    {
        return;
    }

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
            
            sendWaveformPointFast(ch, interpolatedValue);
        }
        
        // [X축 동기화] 글로벌 포인트 카운터 증가
        // 한 스텝에서 하나의 X 위치가 진행됨
        globalPointCount++;
        
        delay(SEND_DELAY_MS);
    }

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
    // 0: 눌림(Press), 1: 뗌(Release) 이벤트만 처리
    if (eventType != 0 && eventType != 1)
    {
        return;
    }

    // [채널 제어] bt0 ~ bt4 버튼 (메인 페이지 page 0에서만 동작)
    // 페이지 이동은 Nextion MCU에서 직접 처리하므로 코드를 제거함
    if (pageId == 0)
    {
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
    if (timeInfo.second < 60) return;

    timeInfo.second = 0;
    timeInfo.minute++;
    if (timeInfo.minute < 60) return;

    timeInfo.minute = 0;
    timeInfo.hour++;
    if (timeInfo.hour < 24) return;

    timeInfo.hour = 0;
    timeInfo.day++;
    int maxDay = daysInMonth(timeInfo.year, timeInfo.month);
    if (timeInfo.day <= maxDay) return;

    timeInfo.day = 1;
    timeInfo.month++;
    if (timeInfo.month <= 12) return;

    timeInfo.month = 1;
    timeInfo.year++;
}

int daysInMonth(int year, int month)
{
    switch (month)
    {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12:
        return 31;
    case 4: case 6: case 9: case 11:
        return 30;
    case 2:
        return isLeapYear(year) ? 29 : 28;
    default:
        return 30;
    }
}

bool isLeapYear(int year)
{
    if ((year % 4) != 0) return false;
    if ((year % 100) != 0) return true;
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
        // [X축 동기화] 새 채널 활성화 시 현재 위치로 동기화
        syncChannelToCurrentPosition(channel);
    }
    else
    {
        clearWaveformChannel(channel);
        lastPlotValues[channel] = -1;
        channelSyncPoint[channel] = 0;
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
            channelSyncPoint[ch] = 0;
        }

        clearWaveformAll();
        
        // [X축 동기화] 전체 클리어 시 글로벌 카운터도 리셋
        globalPointCount = 0;
        
        suspendWaveformDraw();
        syncDualStateButtons();
        return;
    }

    // [X축 동기화] 모든 채널 동시 활성화 - lockstep 방식으로 동시 리플레이
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        activeChannels[ch] = true;
        channelSyncPoint[ch] = globalPointCount;
    }
    
    // 모든 채널을 동시에 리플레이 (lockstep)
    if (bufferCount > 0)
    {
        replayAllChannelsLockstep();
    }
    else
    {
        clearWaveformAll();
        for (int ch = 0; ch < CHANNELS; ch++)
        {
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

    char cmd[32];
    snprintf(cmd, sizeof(cmd), "cle %d,%d", WAVEFORM_ID, channel);
    sendNextionCommandWithDelay(cmd, 5);
}

void clearWaveformAll()
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "cle %d,255", WAVEFORM_ID);
    sendNextionCommandWithDelay(cmd, 10);
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

// 단일 채널 리플레이 (개별 채널 활성화용 - globalPointCount 증가 안함)
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

    int pointCount = 0;
    const int POINTS_PER_BATCH = 10;

    for (uint16_t i = 0; i < sampleCount; i++)
    {
        uint16_t index = (startIndex + i) % BUFFER_LENGTH;
        uint8_t currentValue = waveformBuffer[channel][index];
        int diff = (int)currentValue - (int)previousValue;

        for (int step = 1; step <= LINE_STEPS; step++)
        {
            int interpolatedValue = previousValue + (diff * step / LINE_STEPS);
            sendWaveformPointFast(channel, interpolatedValue);
            
            pointCount++;
            if (pointCount >= POINTS_PER_BATCH)
            {
                delay(2);
                pointCount = 0;
            }
        }

        previousValue = currentValue;
    }

    delay(CHANNEL_DELAY_MS);
    lastPlotValues[channel] = previousValue;
}

// [X축 동기화] 모든 채널을 lockstep 방식으로 동시에 리플레이
// 스텝별로 모든 채널에 포인트를 추가하여 X축 완벽 동기화
void replayAllChannelsLockstep()
{
    if (bufferCount == 0)
    {
        return;
    }

    // 모든 채널 클리어
    clearWaveformAll();

    uint16_t sampleCount = bufferCount;
    if (sampleCount > REPLAY_MAX_SAMPLES)
    {
        sampleCount = REPLAY_MAX_SAMPLES;
    }

    uint16_t startIndex = (bufferIndex + BUFFER_LENGTH - sampleCount) % BUFFER_LENGTH;
    
    // 각 채널의 이전값 초기화
    uint8_t previousValues[CHANNELS];
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        previousValues[ch] = waveformBuffer[ch][startIndex];
    }

    int pointCount = 0;
    const int POINTS_PER_BATCH = 10;

    // 샘플별 순회
    for (uint16_t i = 0; i < sampleCount; i++)
    {
        uint16_t index = (startIndex + i) % BUFFER_LENGTH;
        
        // 각 채널의 현재값과 차이 계산
        uint8_t currentValues[CHANNELS];
        int diffs[CHANNELS];
        
        for (int ch = 0; ch < CHANNELS; ch++)
        {
            currentValues[ch] = waveformBuffer[ch][index];
            diffs[ch] = (int)currentValues[ch] - (int)previousValues[ch];
        }

        // 스텝별 순회 - 모든 채널에 동시에 포인트 추가
        for (int step = 1; step <= LINE_STEPS; step++)
        {
            // 모든 채널에 보간값 전송
            for (int ch = 0; ch < CHANNELS; ch++)
            {
                int interpolatedValue = previousValues[ch] + (diffs[ch] * step / LINE_STEPS);
                sendWaveformPointFast(ch, interpolatedValue);
            }
            
            // 글로벌 카운터는 스텝당 1회만 증가 (모든 채널 공유)
            globalPointCount++;
            
            pointCount++;
            if (pointCount >= POINTS_PER_BATCH)
            {
                delay(2);
                pointCount = 0;
            }
        }

        // 이전값 업데이트
        for (int ch = 0; ch < CHANNELS; ch++)
        {
            previousValues[ch] = currentValues[ch];
        }
    }

    // lastPlotValues 업데이트
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        lastPlotValues[ch] = previousValues[ch];
        channelSyncPoint[ch] = globalPointCount;
    }

    delay(CHANNEL_DELAY_MS);
}

void suspendWaveformDraw()
{
    drawSuspended = true;
    drawResumeMillis = millis() + DRAW_SUSPEND_MS;
}

// [개선] 센서 초기화 및 안정적인 주소 획득
void initSensors()
{
    sensors.begin();
    delay(150); // 버스 안정화 대기
    
    // 발견된 모든 센서를 CHANNELS 만큼만 바인딩
    for (int i = 0; i < CHANNELS; i++)
    {
        if (sensors.getAddress(sensorAddresses[i], i))
        {
            sensors.setResolution(sensorAddresses[i], 10);
            sensorFound[i] = true;
        }
        else
        {
            sensorFound[i] = false;
            // 누락된 주소는 0으로 초기화하여 안전성 확보
            memset(sensorAddresses[i], 0, 8);
        }
    }
    sensors.requestTemperatures();
}

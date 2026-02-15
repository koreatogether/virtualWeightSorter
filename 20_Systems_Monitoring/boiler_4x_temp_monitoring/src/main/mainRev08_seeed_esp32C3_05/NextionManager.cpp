#include "NextionManager.h"
#include "SensorManager.h"
#include <time.h>

// HardwareSerial 1 객체 정의 (Config.h에서 extern으로 참조됨)
HardwareSerial MySerial1(1);

// --- 로컬 상태 관리 ---
static bool activeChannels[4] = {false, false, false, false};
static int lastPlotValues[4] = {-1, -1, -1, -1};
static uint32_t globalPointCount = 0;
static uint32_t channelSyncPoint[4] = {0, 0, 0, 0};
static unsigned long prevUpdateMillis = 0;

const int BUFFER_LENGTH = 300;
static uint8_t waveformBuffer[4][BUFFER_LENGTH];
static uint16_t bufferIndex = 0;
static uint16_t bufferCount = 0;

const int DS_BUTTON_IDS[5] = {20, 21, 22, 23, 24};

static void flushSerialAndWait()
{
    HMISerial.flush();
    while (HMISerial.available() > 0)
    {
        HMISerial.read();
    }
    delay(10);
}

void sendNextionCommand(const char *cmd)
{
    HMISerial.print(cmd);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
}

void initNextion()
{
    // 1. 9600bps로 초기화 (Nextion 기본 속도에서 핸드셰이크 시작)
    MySerial1.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
    Serial.println("[Nextion] Initializing at 9600bps to switch baudrate...");

    // 2. Nextion 웜업 및 초기 버퍼 비우기
    delay(500);
    for (int i = 0; i < 3; i++)
    {
        HMISerial.write(0xFF);
    }
    delay(100);

    // 3. 통신 속도 영구 변경 명령 (baud=115200)
    // 참고: bauds=115200을 쓰면 전원 재시작 시 9600으로 복구되나, 여기서는 영구 변경 사용
    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    // Nextion 내부 처리 시간 대기
    delay(200);

    // 4. ESP32 시리얼 보드레이트 변경
    MySerial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
    Serial.println("[Nextion] Switched to 115200bps");

    // 5. 연결 확인용 메시지 (변경된 속도에서 전송)
    sendNextionCommand("t0.txt=\"Speed: 115200bps\"");
    Serial.println("[Nextion] Sent speed confirmation command");

    flushSerialAndWait();

    char cmd[32];
    snprintf(cmd, sizeof(cmd), "cle %d,255", WAVEFORM_ID);
    sendNextionCommand(cmd);

    globalPointCount = 0;
    for (int i = 0; i < 4; i++)
        channelSyncPoint[i] = 0;
}

void updateTimeOnDisplay()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        return;
    }

    char cmd[40];
    snprintf(cmd, sizeof(cmd), "t1.txt=\"%02d-%02d-%02d %02d:%02d:%02d\"",
             timeinfo.tm_year % 100, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    sendNextionCommand(cmd);
}

void storeWaveformSamples(const int plotValues[4])
{
    for (int ch = 0; ch < 4; ch++)
    {
        waveformBuffer[ch][bufferIndex] = (uint8_t)plotValues[ch];
    }
    bufferIndex = (bufferIndex + 1) % BUFFER_LENGTH;
    if (bufferCount < BUFFER_LENGTH)
        bufferCount++;
}

void syncChannelToCurrentPosition(int channel)
{
    if (channel < 0 || channel >= 4)
        return;

    // 채널 클리어
    char cleCmd[32];
    snprintf(cleCmd, sizeof(cleCmd), "cle %d,%d", WAVEFORM_ID, channel);
    sendNextionCommand(cleCmd);

    // 현재 활성 채널 중 최대 포인트 수 계산
    uint32_t maxPoints = 0;
    bool hasOtherActive = false;
    for (int ch = 0; ch < 4; ch++)
    {
        if (ch != channel && activeChannels[ch])
        {
            uint32_t channelPoints = globalPointCount - channelSyncPoint[ch];
            if (channelPoints > maxPoints)
                maxPoints = channelPoints;
            hasOtherActive = true;
        }
    }

    // [개선] 다른 활성 채널이 없으면 버퍼의 과거 데이터를 리플레이 (최대 4분/120샘플 기준)
    if (!hasOtherActive && bufferCount > 0)
    {
        uint16_t samplesToReplay = bufferCount;
        if (samplesToReplay > HISTORY_SAMPLES)
            samplesToReplay = HISTORY_SAMPLES;
        maxPoints = samplesToReplay * LINE_STEPS;
    }

    // 웨이브폼 너비 제한 (Nextion 화면 너비에 맞춰 재생 포인트 가독성 유지)
    if (maxPoints > WAVEFORM_WIDTH)
        maxPoints = WAVEFORM_WIDTH;

    // 버퍼에서 재생할 샘플 수 (LINE_STEPS 기반)
    uint16_t samplesNeeded = maxPoints / LINE_STEPS;
    if (samplesNeeded > bufferCount)
        samplesNeeded = bufferCount;

    uint32_t pointsToReplay = samplesNeeded * LINE_STEPS;
    uint32_t paddingPoints = (maxPoints > pointsToReplay) ? maxPoints - pointsToReplay : 0;

    // 1. 패딩 (X축 위치 동기화 - 초반 공백 처리)
    for (uint32_t i = 0; i < paddingPoints; i++)
    {
        char addCmd[32];
        snprintf(addCmd, sizeof(addCmd), "add %d,%d,0", WAVEFORM_ID, channel);
        sendNextionCommand(addCmd);
    }

    // 2. 버퍼 리플레이 (과거 데이터 그리기)
    if (samplesNeeded > 0)
    {
        uint16_t startIndex = (bufferIndex + BUFFER_LENGTH - samplesNeeded) % BUFFER_LENGTH;
        uint8_t prevVal = waveformBuffer[channel][startIndex];
        for (uint16_t i = 0; i < samplesNeeded; i++)
        {
            uint16_t idx = (startIndex + i) % BUFFER_LENGTH;
            uint8_t currVal = waveformBuffer[channel][idx];
            int diff = (int)currVal - (int)prevVal;
            for (int step = 1; step <= LINE_STEPS; step++)
            {
                int interp = prevVal + (diff * step / LINE_STEPS);
                char addCmd[32];
                snprintf(addCmd, sizeof(addCmd), "add %d,%d,%d", WAVEFORM_ID, channel, interp);
                sendNextionCommand(addCmd);
            }
            prevVal = currVal;
        }
        lastPlotValues[channel] = prevVal;
    }
    else
    {
        lastPlotValues[channel] = -1;
    }

    // 현재 글로벌 포인트 카운터를 기준으로 이 채널의 가상 시작점 동기화
    channelSyncPoint[channel] = globalPointCount - maxPoints;
}

void updateNextionDisplay()
{
    unsigned long currentMillis = millis();
    if (currentMillis - prevUpdateMillis < SAMPLE_INTERVAL_MS)
        return;
    prevUpdateMillis = currentMillis;

    updateTimeOnDisplay();

    int plotValues[4];
    for (int i = 0; i < 4; i++)
    {
        plotValues[i] = map((int)sensorTemps[i], TEMP_MIN, TEMP_MAX, 0, WAVEFORM_HEIGHT - 1);
        if (plotValues[i] < 0)
            plotValues[i] = 0;
        if (plotValues[i] > 255)
            plotValues[i] = 255;

        char cmd[32];
        snprintf(cmd, sizeof(cmd), "x%d.val=%d", i, (int)(sensorTemps[i] * 10));
        sendNextionCommand(cmd);
    }

    char diffCmd[32];
    snprintf(diffCmd, sizeof(diffCmd), "x4.val=%d", (int)((sensorTemps[0] - sensorTemps[1]) * 10));
    sendNextionCommand(diffCmd);
    snprintf(diffCmd, sizeof(diffCmd), "x5.val=%d", (int)((sensorTemps[2] - sensorTemps[3]) * 10));
    sendNextionCommand(diffCmd);

    storeWaveformSamples(plotValues);

    for (int step = 1; step <= LINE_STEPS; step++)
    {
        for (int ch = 0; ch < 4; ch++)
        {
            if (!activeChannels[ch])
                continue;

            if (lastPlotValues[ch] == -1)
                lastPlotValues[ch] = plotValues[ch];
            int diff = plotValues[ch] - lastPlotValues[ch];
            int interpolated = lastPlotValues[ch] + (diff * step / LINE_STEPS);

            char addCmd[32];
            snprintf(addCmd, sizeof(addCmd), "add %d,%d,%d", WAVEFORM_ID, ch, interpolated);
            sendNextionCommand(addCmd);
        }
        globalPointCount++;
        delay(SEND_DELAY_MS);
    }

    for (int ch = 0; ch < 4; ch++)
    {
        if (activeChannels[ch])
            lastPlotValues[ch] = plotValues[ch];
    }
}

// [추가] 페이지 복귀 시 UI 및 그래프 복구 함수
void refreshNextionPageState()
{
    Serial.println("[Nextion] Refreshing page state (Page 0 entered)");

    // 1. 모든 버튼 상태 동기화 (bt0 ~ bt4)
    for (int j = 0; j < 5; j++)
    {
        int val = 0;
        if (j < 4)
            val = activeChannels[j] ? 1 : 0;
        else
        {
            bool allOn = true;
            for (int k = 0; k < 4; k++)
                if (!activeChannels[k])
                    allOn = false;
            val = allOn ? 1 : 0;
        }
        char syncCmd[20];
        snprintf(syncCmd, sizeof(syncCmd), "bt%d.val=%d", j, val);
        sendNextionCommand(syncCmd);
    }

    // 2. 활성화된 모든 채널의 그래프 복구 (과거 데이터 리플레이)
    for (int ch = 0; ch < 4; ch++)
    {
        if (activeChannels[ch])
        {
            syncChannelToCurrentPosition(ch);
            delay(20); // 시리얼 버퍼 포화 방지
        }
    }
}

void handleNextionTouch(uint8_t pageId, uint8_t componentId, uint8_t eventType)
{
    if (eventType != 0 && eventType != 1)
        return;

    // [추가] Nextion에서 보낸 페이지 진입 이벤트 처리 (Component ID 255)
    if (componentId == 255)
    {
        refreshNextionPageState();
        return;
    }

    if (pageId != 0)
        return;

    for (int i = 0; i < 5; i++)
    {
        if (componentId == DS_BUTTON_IDS[i])
        {
            if (i < 4)
            {
                bool newState = !activeChannels[i];
                activeChannels[i] = newState;
                if (newState)
                    syncChannelToCurrentPosition(i);
                else
                {
                    char cmd[32];
                    snprintf(cmd, sizeof(cmd), "cle %d,%d", WAVEFORM_ID, i);
                    sendNextionCommand(cmd);
                    lastPlotValues[i] = -1;
                }
            }
            else
            {
                bool anyOff = false;
                for (int j = 0; j < 4; j++)
                    if (!activeChannels[j])
                        anyOff = true;

                if (anyOff)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if (!activeChannels[j])
                        {
                            activeChannels[j] = true;
                            syncChannelToCurrentPosition(j);
                        }
                    }
                }
                else
                {
                    for (int j = 0; j < 4; j++)
                        activeChannels[j] = false;
                    char cmd[32];
                    snprintf(cmd, sizeof(cmd), "cle %d,255", WAVEFORM_ID);
                    sendNextionCommand(cmd);
                    for (int j = 0; j < 4; j++)
                        lastPlotValues[j] = -1;
                    globalPointCount = 0;
                }
            }
            for (int j = 0; j < 5; j++)
            {
                int val = 0;
                if (j < 4)
                    val = activeChannels[j] ? 1 : 0;
                else
                {
                    bool allOn = true;
                    for (int k = 0; k < 4; k++)
                        if (!activeChannels[k])
                            allOn = false;
                    val = allOn ? 1 : 0;
                }
                char syncCmd[20];
                snprintf(syncCmd, sizeof(syncCmd), "bt%d.val=%d", j, val);
                sendNextionCommand(syncCmd);
            }
            break;
        }
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
                handleNextionTouch(eventData[0], eventData[1], eventData[2]);
                readingEvent = false;
                dataIndex = 0;
                ffCount = 0;
            }
        }
        else
        {
            // FF가 아닌 데이터가 오면 0x65 패킷 구조가 아님
            readingEvent = false;
            dataIndex = 0;
            ffCount = 0;
        }
    }
}

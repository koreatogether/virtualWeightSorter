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
    // MySerial1 초기화 (Config.h에 정의된 핀 사용)
    MySerial1.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
    
    // 이 라이브러리가 사용하는 시리얼 객체를 교체하는 방법이 라이브러리마다 다를 수 있으나, 
    // 여기서는 Nextion 라이브러리 내부에서 사용하는 시리얼이 아닌 직접 명령어를 보내는 방식을 위주로 작성됨.
    // 만약 Nextion 라이브러리의 nexInit() 등을 사용한다면 해당 라이브러리 설정 파일을 확인해야 함.
    // 여기서는 수동 명령 위주로 구현함.

    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    delay(100);
    HMISerial.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
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
    if (!getLocalTime(&timeinfo)) {
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

    char cleCmd[32];
    snprintf(cleCmd, sizeof(cleCmd), "cle %d,%d", WAVEFORM_ID, channel);
    sendNextionCommand(cleCmd);

    uint32_t maxPoints = 0;
    for (int ch = 0; ch < 4; ch++)
    {
        if (ch != channel && activeChannels[ch])
        {
            uint32_t channelPoints = globalPointCount - channelSyncPoint[ch];
            if (channelPoints > maxPoints)
                maxPoints = channelPoints;
        }
    }

    uint16_t samplesNeeded = maxPoints / LINE_STEPS;
    if (samplesNeeded > bufferCount)
        samplesNeeded = bufferCount;
    if (samplesNeeded > 100)
        samplesNeeded = 100;

    uint32_t pointsToReplay = samplesNeeded * LINE_STEPS;
    uint32_t paddingPoints = (maxPoints > pointsToReplay) ? maxPoints - pointsToReplay : 0;

    for (uint32_t i = 0; i < paddingPoints; i++)
    {
        char addCmd[32];
        snprintf(addCmd, sizeof(addCmd), "add %d,%d,0", WAVEFORM_ID, channel);
        sendNextionCommand(addCmd);
    }

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

void handleNextionTouch(uint8_t pageId, uint8_t componentId, uint8_t eventType)
{
    if (eventType != 0 && eventType != 1) return;
    if (pageId != 0) return;

    for (int i = 0; i < 5; i++)
    {
        if (componentId == DS_BUTTON_IDS[i])
        {
            if (i < 4)
            {
                bool newState = !activeChannels[i];
                activeChannels[i] = newState;
                if (newState) syncChannelToCurrentPosition(i);
                else {
                    char cmd[32];
                    snprintf(cmd, sizeof(cmd), "cle %d,%d", WAVEFORM_ID, i);
                    sendNextionCommand(cmd);
                    lastPlotValues[i] = -1;
                }
            }
            else
            {
                bool anyOff = false;
                for (int j = 0; j < 4; j++) if (!activeChannels[j]) anyOff = true;

                if (anyOff) {
                    for (int j = 0; j < 4; j++) {
                        if (!activeChannels[j]) {
                            activeChannels[j] = true;
                            syncChannelToCurrentPosition(j);
                        }
                    }
                } else {
                    for (int j = 0; j < 4; j++) activeChannels[j] = false;
                    char cmd[32];
                    snprintf(cmd, sizeof(cmd), "cle %d,255", WAVEFORM_ID);
                    sendNextionCommand(cmd);
                    for (int j = 0; j < 4; j++) lastPlotValues[j] = -1;
                    globalPointCount = 0;
                }
            }
            for (int j = 0; j < 5; j++)
            {
                int val = 0;
                if (j < 4) val = activeChannels[j] ? 1 : 0;
                else {
                    bool allOn = true;
                    for (int k = 0; k < 4; k++) if (!activeChannels[k]) allOn = false;
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
        if (!readingEvent) {
            if (incoming == 0x65) {
                readingEvent = true;
                dataIndex = 0;
                ffCount = 0;
            }
            continue;
        }
        if (dataIndex < 3) {
            eventData[dataIndex++] = incoming;
            continue;
        }
        if (incoming == 0xFF) {
            ffCount++;
            if (ffCount >= 3) {
                handleNextionTouch(eventData[0], eventData[1], eventData[2]);
                readingEvent = false;
            }
        } else {
            readingEvent = false;
            dataIndex = 0;
            ffCount = 0;
        }
    }
}

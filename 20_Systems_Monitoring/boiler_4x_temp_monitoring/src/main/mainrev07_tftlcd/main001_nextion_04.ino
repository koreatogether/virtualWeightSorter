#include <Nextion.h>
#include <SoftwareSerial.h>

/*
 * [Arduino Nano 전용 - SoftwareSerial 버전] Nextion Waveform Graph Demo
 * - NexConfig.h에서 SoftwareSerial 사용 설정이 되어 있어야 함
 * - SoftwareSerial(Pin 10, 11) 사용으로 USB 디버깅(Serial) 가능
 * - 통신 속도 제약으로 그래프 갱신이 다소 느릴 수 있음 (권장: 9600~38400bps)
 */

/*
03버전에서는 나노 보드 기준으로 작동하되
새로운 화면에 대한 각 버튼
듀얼 스테이트 버튼에 대한 각 동작을 구현 및 확인 한다.

ai 대화를 위한 각 버튼 정보는 이미지 파일로 해서 assert 에 보관 하도록한다.

*/

// [Nano 설정]
// SoftwareSerial 사용 (RX=10, TX=11)
// NexConfig.h의 'extern SoftwareSerial HMISerial;'과 이름이 일치해야 함
SoftwareSerial HMISerial(10, 11);

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

// 각 채널별 이전 값 저장용
int lastPlotValues[4] = {-1, -1, -1, -1};

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

void setup()
{
    // 디버그용 시리얼 초기화 (SoftwareSerial 사용 시 가능)
    Serial.begin(9600);

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
    // 초기에는 기본 속도 9600으로 연결 시도
    HMISerial.begin(9600);

    // 넥션 리셋 신호
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    /*
     * [SoftwareSerial 속도 제한]
     * SoftwareSerial은 115200bps에서 데이터 송수신이 불안정할 수 있습니다.
     * 안정적인 동작을 위해 기본 속도(9600)를 유지하거나, 최대 38400까지만 올리는 것을 권장합니다.
     * 아래 코드는 속도 변경 없이 9600bps를 유지하는 설정입니다.
     */

    // 만약 속도를 올리고 싶다면 아래 주석을 해제하고 테스트해보세요 (최대 38400 권장)

    sendNextionCommand("baud=38400");
    waitForMillis(200);
    HMISerial.end();
    HMISerial.begin(38400);

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

    // 3. 차이값(Delta T) 업데이트 (x4, x5)
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

    // 4. 그래프 그리기 (점과 점 사이를 직선으로 채움)
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

            HMISerial.print("add ");
            HMISerial.print(WAVEFORM_ID);
            HMISerial.print(",");
            HMISerial.print(ch);
            HMISerial.print(",");
            HMISerial.print(interpolatedValue);
            HMISerial.write(0xFF);
            HMISerial.write(0xFF);
            HMISerial.write(0xFF);
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
    clearWaveformChannel(channel);
    if (isActive)
    {
        lastPlotValues[channel] = -1;
    }
    syncDualStateButtons();
}

void setAllChannels(bool isActive)
{
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        activeChannels[ch] = isActive;
        clearWaveformChannel(ch);
        if (isActive)
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

    HMISerial.print("cle ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",");
    HMISerial.print(channel);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
}

#include <Nextion.h>

/*
 * [Arduino R4 WiFi 전용] Nextion Waveform Graph Demo (main001_nextion_01 기반)
 * - 01 코드의 통신/그래프 로직을 그대로 유지
 * - 화면 보호기(1분 후 절전, 전영역 터치 복귀) 추가
 * - delay 없이 millis() 기반 비차단 업데이트 적용
 */

/*
03 버전에서는 화면 오른쪽 에 1P~4P 페이지 버튼 구현
화면 비율 및 구성을 바꾸어야 함
그래서 mega 보드를 활용해서 업로드 와 소스코드 관리를 할 예정
*/

// [R4 WiFi 전용 설정]
// USB 통신(디버깅)은 Serial, 핀 0/1 통신(Nextion)은 Serial1입니다.
#define HMISerial Serial1

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

// 함수 선언
void initNextionDisplay();
void manageDisplaySystem();
void updateWaveformData();
void sendNextionCommand(const char *cmd);
void waitForMillis(unsigned long waitMs);

void setup()
{
    Serial.begin(115200);

    // 01 코드의 초기화 흐름 유지
    initNextionDisplay();

    // 첫 루프에서 즉시 갱신되도록 타이머 보정
    prevUpdateMillis = millis() - UPDATE_INTERVAL;
}

void loop()
{
    // 1. 화면 보호기 관리 (루프에서 지속)
    manageDisplaySystem();

    // 2. 비차단 방식으로 그래프 업데이트
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

    // 통신 속도 고속화 (9600 -> 115200)
    sendNextionCommand("baud=115200");
    waitForMillis(200);

    HMISerial.end();
    HMISerial.begin(115200);

    // 그래프 초기화 (Clear)
    HMISerial.print("cle ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",255");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
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

        if (lastPlotValues[ch] == -1)
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
        lastPlotValues[ch] = plotValues[ch];
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

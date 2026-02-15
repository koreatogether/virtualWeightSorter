/*
 * Nextion Waveform (s0) 공식 프로토콜 사용 예제
 *
 * [메타데이터 기반 설정]
 * - Component ID: 11 (metaData_waveForm.json에서 확인됨)
 * - Object Name: s0
 * - Channel: 0 (ch0 사용)
 * - Size: Width 461, Height 141
 * - Scale (dis): 100 (1:1 픽셀 매핑)
 *
 * [동작 설명]
 * - 2초 간격으로 온도 데이터를 시뮬레이션합니다.
 * - Nextion의 'add' 명령어를 사용하여 데이터를 전송합니다.
 * - 'dis' 속성이 100이므로, 값 1이 그래프 높이 1픽셀에 해당합니다.
 * - 따라서 0~100도의 온도를 그래프 높이(0~140)에 맞춰 매핑하여 전송합니다.
 */

#include <Nextion.h>
#include <SoftwareSerial.h>

// Nextion HMI 연결 (RX, TX)
// Arduino Pin 2 (RX) <-> Nextion TX
// Arduino Pin 3 (TX) <-> Nextion RX
SoftwareSerial HMISerial(2, 3);

// [설정] Nextion Waveform ID (메타데이터 기준 11)
const int WAVEFORM_ID = 11;
const int CHANNELS = 4; // 4채널 사용

// [설정] Xfloat 컴포넌트 정보 (x0 ~ x3)
const char *XFLOAT_OBJS[4] = {"x0", "x1", "x2", "x3"}; // objname

// [설정] 그래프 매핑용 범위
const int WAVEFORM_HEIGHT = 141; // 그래프 컴포넌트 높이 (메타데이터 h: 141)
const int TEMP_MIN = 0;          // 표시할 최저 온도
const int TEMP_MAX = 100;        // 표시할 최고 온도

// [추가 설정] 가로 간격 조정 (격자선 기준)
// Nextion Waveform은 기본적으로 1데이터=1픽셀입니다.
// "격자선 2개 간격"으로 그래프를 물리적으로 늘리려면, 데이터 사이를 채워주는 보간(Interpolation)이 필요합니다.
const int GRID_WIDTH = 20;                                // 메타데이터의 gdw (격자 폭 20px)
const int GRID_INTERVAL = 2;                              // 격자선 2개 간격
const int PIXELS_PER_UPDATE = GRID_WIDTH * GRID_INTERVAL; // 한 번 갱신 시 채울 픽셀 수 (40px)

// 각 채널별 이전 값 저장용 (-1은 초기화 전 상태)
int lastPlotValues[4] = {-1, -1, -1, -1};

void setup()
{
    // 시리얼 초기화
    Serial.begin(9600); // 디버깅용
    HMISerial.begin(9600);

    // 넥시온 초기화
    nexInit();

    // 2. (선택사항) 시작 시 그래프 기존 데이터 지우기
    HMISerial.print("cle ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",255");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
}

void loop()
{
    // 4채널 온도 및 매핑 값 저장용 배열
    float currentTemps[4];
    int plotValues[4];

    // 1. 4개 채널 센서 데이터 생성 및 화면 업데이트
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        // 시뮬레이션: 각 채널별로 약간 다른 범위의 랜덤값 생성
        currentTemps[ch] = random(400 + (ch * 50), 800 - (ch * 50)) / 10.0;

        // 값 매핑 (Mapping)
        plotValues[ch] = map((int)currentTemps[ch], TEMP_MIN, TEMP_MAX, 0, WAVEFORM_HEIGHT - 1);
        if (plotValues[ch] < 0)
            plotValues[ch] = 0;
        if (plotValues[ch] > 255)
            plotValues[ch] = 255;

        // 초기화 처리
        if (lastPlotValues[ch] == -1)
            lastPlotValues[ch] = plotValues[ch];

        // [Xfloat 업데이트] x0, x1, x2, x3 객체에 값 표시
        // 메타데이터 설정: vvs0=2(정수2자리), vvs1=1(소수1자리) -> val에 10을 곱한 정수 입력
        // 예: 45.2도 -> val=452
        int xfloatVal = (int)(currentTemps[ch] * 10);

        HMISerial.print(XFLOAT_OBJS[ch]);
        HMISerial.print(".val=");
        HMISerial.print(xfloatVal);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
    }

    // 2. 4채널 데이터 동시 보간 전송 (그래프 그리기)
    // 모든 채널을 루프 안에서 동시에 add하여 시점을 맞춥니다.
    for (int i = 1; i <= PIXELS_PER_UPDATE; i++)
    {
        for (int ch = 0; ch < CHANNELS; ch++)
        {
            // 선형 보간 공식
            int interpolatedValue = lastPlotValues[ch] + (plotValues[ch] - lastPlotValues[ch]) * i / PIXELS_PER_UPDATE;

            HMISerial.print("add ");
            HMISerial.print(WAVEFORM_ID);
            HMISerial.print(",");
            HMISerial.print(ch); // 채널 번호 (0~3)
            HMISerial.print(",");
            HMISerial.print(interpolatedValue);
            HMISerial.write(0xFF);
            HMISerial.write(0xFF);
            HMISerial.write(0xFF);
        }
    }

    // 디버깅 출력 (채널 0만 예시로)
    Serial.print("Ch0 Temp: ");
    Serial.print(currentTemps[0]);
    Serial.print(" => Plot: ");
    Serial.println(plotValues[0]);

    // 값 업데이트 (다음 루프를 위해 저장)
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        lastPlotValues[ch] = plotValues[ch];
    }

    // 5. 2초 대기
    delay(2000);
}

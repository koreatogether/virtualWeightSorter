/*
 * Nextion Waveform (s0) 공식 프로토콜 사용 예제 (Hardware Serial 호환 버전)
 *
 * [메타데이터 기반 설정]
 * - Component ID: 11 (metaData_waveForm.json에서 확인됨)
 * - Object Name: s0
 * - Channel: 0 (ch0 사용)
 * - Size: Width 461, Height 141
 * - Scale (dis): 100 (1:1 픽셀 매핑)
 *
 * [중요: 하드웨어 시리얼 설정]
 * 이 코드는 보드 종류에 따라 자동으로 시리얼 포트를 선택합니다.
 * 에러(Serial1 was not declared) 해결을 위해 다음과 같이 동작합니다:
 *
 * 1. Arduino Mega / Leonardo / Due 사용 시:
 *    - 'Serial1' (Pin 18/19 등)을 사용하여 Nextion과 통신합니다.
 *    - 'Serial' (USB)은 디버깅용으로 계속 사용할 수 있습니다.
 *
 * 2. Arduino Uno / Nano 사용 시:
 *    - 'Serial1'이 없으므로 'Serial' (Pin 0/1)을 Nextion 통신용으로 사용합니다.
 *    - 이 경우 PC로의 디버깅 출력(Serial Monitor)은 자동으로 비활성화됩니다.
 *    - 업로드 시 0/1번 핀에 연결된 Nextion 선을 잠시 빼야 할 수 있습니다.
 */

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

// [설정] Nextion Waveform ID (메타데이터 기준 11)
const int WAVEFORM_ID = 11;
const int CHANNELS = 4; // 4채널 사용

// [설정] Xfloat 컴포넌트 정보 (x0 ~ x3)
const char *XFLOAT_OBJS[4] = {"x0", "x1", "x2", "x3"}; // objname

// [설정] 그래프 매핑용 범위
const int WAVEFORM_HEIGHT = 141; // 그래프 컴포넌트 높이 (메타데이터 h: 141)
const int TEMP_MIN = 0;          // 표시할 최저 온도
const int TEMP_MAX = 100;        // 표시할 최고 온도

// [추가 설정] 가로 간격 조정
const int GRID_WIDTH = 20;                                // 메타데이터의 gdw (격자 폭 20px)
const int GRID_INTERVAL = 2;                              // 격자선 2개 간격
const int PIXELS_PER_UPDATE = GRID_WIDTH * GRID_INTERVAL; // 한 번 갱신 시 채울 픽셀 수 (40px)

// 각 채널별 이전 값 저장용 (-1은 초기화 전 상태)
int lastPlotValues[4] = {-1, -1, -1, -1};

void setup()
{
// 1. 디버깅 포트 초기화 (Mega 등에서만 작동)
#if DEBUG_ENABLE
    Serial.begin(9600);
    Serial.println(F("Nextion Waveform Start (Serial1 Mode)"));
#endif

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

    // 3. 그래프 초기화
    // HMISerial 매크로가 가리키는 포트(Serial1 또는 Serial)로 명령어 전송
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

    // 1. 센서 데이터 생성 (시뮬레이션)
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        currentTemps[ch] = random(400 + (ch * 50), 800 - (ch * 50)) / 10.0;
    }

    // [제약조건 적용]
    // x0(공급) >= x1(환수), x2(공급) >= x3(환수)
    // 시뮬레이션 값이 이를 위반할 경우 조정
    if (currentTemps[1] > currentTemps[0])
        currentTemps[1] = currentTemps[0];
    if (currentTemps[3] > currentTemps[2])
        currentTemps[3] = currentTemps[2];

    // 2. 화면 업데이트 (각 채널 온도 및 그래프 매핑)
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        // 값 매핑 (Mapping)
        plotValues[ch] = map((int)currentTemps[ch], TEMP_MIN, TEMP_MAX, 0, WAVEFORM_HEIGHT - 1);

        // 범위 제한
        if (plotValues[ch] < 0)
            plotValues[ch] = 0;
        if (plotValues[ch] > 255)
            plotValues[ch] = 255;

        // 초기화 처리
        if (lastPlotValues[ch] == -1)
            lastPlotValues[ch] = plotValues[ch];

        // [Xfloat 값 전송 (x0~x3)]
        int xfloatVal = (int)(currentTemps[ch] * 10);

        HMISerial.print(XFLOAT_OBJS[ch]);
        HMISerial.print(".val=");
        HMISerial.print(xfloatVal);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
    }

    // 3. 차이값(Delta T) 업데이트 (x4, x5)
    // x4 = x0 - x1
    // x5 = x2 - x3
    // Xfloat(vvs1=1)이므로 10을 곱해서 전송
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

    // 2. 4채널 데이터 동시 보간 전송 (그래프 그리기)
    for (int i = 1; i <= PIXELS_PER_UPDATE; i++)
    {
        for (int ch = 0; ch < CHANNELS; ch++)
        {
            // 선형 보간 공식
            long diff = plotValues[ch] - lastPlotValues[ch];
            int interpolatedValue = lastPlotValues[ch] + (diff * i / PIXELS_PER_UPDATE);

            HMISerial.print("add ");
            HMISerial.print(WAVEFORM_ID);
            HMISerial.print(",");
            HMISerial.print(ch); // 채널 번호
            HMISerial.print(",");
            HMISerial.print(interpolatedValue);
            HMISerial.write(0xFF);
            HMISerial.write(0xFF);
            HMISerial.write(0xFF);
        }
    }

// 3. 디버깅 출력 (가능한 경우에만)
#if DEBUG_ENABLE
    Serial.print("Ch0 Temp: ");
    Serial.print(currentTemps[0]);
    Serial.print(" => Plot: ");
    Serial.println(plotValues[0]);
#endif

    // 값 업데이트
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        lastPlotValues[ch] = plotValues[ch];
    }

    // 2초 대기
    delay(2000);
}

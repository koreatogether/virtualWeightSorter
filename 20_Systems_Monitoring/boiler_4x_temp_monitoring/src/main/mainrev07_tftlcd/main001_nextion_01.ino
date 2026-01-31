/*
 * [Arduino R4 WiFi 전용] Nextion Waveform Graph Demo
 *
 * 원본: nextion_waveform_official_hardwareSerial.ino
 * 수정: Arduino R4 WiFi 하드웨어 특성(Serial1 사용)에 맞게 수정됨
 *
 * [필수 배선 - Cross Connection]
 * - Arduino D0 (RX) <---연결---> Nextion TX (Blue)
 * - Arduino D1 (TX) <---연결---> Nextion RX (Yellow)
 * - Arduino 5V      <---연결---> Nextion Red
 * - Arduino GND     <---연결---> Nextion Black
 *
 * [업로드 주의]
 * - Nextion Editor에서 TFT 파일을 업로드할 때는 9600bps로 초기화되어야 합니다.
 * - 이 코드는 실행 중 속도를 115200으로 변경하므로, 다시 업로드하려면
 *   넥션 전원을 껐다 켜서 속도를 9600으로 리셋해야 할 수 있습니다.
 */

#include <Nextion.h>

// [R4 WiFi 전용 설정]
// USB 통신(디버깅)은 Serial, 핀 0/1 통신(Nextion)은 Serial1입니다.
#define HMISerial Serial1

// R4 WiFi는 USB와 시리얼이 분리되어 있으므로 항상 디버깅 가능합니다.
#define DEBUG_ENABLE 1

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
const int GRID_WIDTH = 20;
const int GRID_INTERVAL = 2;
const int PIXELS_PER_UPDATE = GRID_WIDTH * GRID_INTERVAL; // 40px

// 각 채널별 이전 값 저장용
int lastPlotValues[4] = {-1, -1, -1, -1};

void setup()
{
    // 1. 디버깅 포트 초기화 (USB)
#if DEBUG_ENABLE
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("Nextion Waveform Demo for Arduino R4 WiFi Start..."));
#endif

    // 2. Nextion 연결 초기화
    // 초기에는 기본 속도 9600으로 연결 시도
    HMISerial.begin(9600);

    // 잠시 대기
    delay(100);

    // [중요] 넥션 라이브러리의 nexInit()을 쓰면 NexConfig.h 설정에 따라
    // Serial 포트가 꼬일 수 있으므로, 여기서는 직접 명령어로 초기화합니다.
    // (만약 nexInit()을 꼭 써야 한다면 NexConfig.h에서 nexSerial을 Serial1로 고쳐야 함)

    // 리셋 신호 보내기 (옵션)
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    // 3. 통신 속도 고속화 (9600 -> 115200)
    // 그래프를 부드럽게 그리기 위해 속도를 올립니다.
    Serial.println(F("Switching Nextion Baudrate to 115200..."));

    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    delay(200); // 넥션이 속도 변경할 시간 확보

    // 아두이노측 포트 재시작
    HMISerial.end();
    HMISerial.begin(115200);

    Serial.println(F("Baudrate synchronized at 115200."));

    // 4. 그래프 초기화 (Clear)
    // cle <id>, <channel (255=all)>
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
    if (currentTemps[1] > currentTemps[0])
        currentTemps[1] = currentTemps[0];
    if (currentTemps[3] > currentTemps[2])
        currentTemps[3] = currentTemps[2];

    // 2. 값 매핑 및 Xfloat 전송
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        // 그래프용 매핑 (0 ~ 255 사이 값이어야 함)
        plotValues[ch] = map((int)currentTemps[ch], TEMP_MIN, TEMP_MAX, 0, WAVEFORM_HEIGHT - 1);

        // 안전 장치
        if (plotValues[ch] < 0)
            plotValues[ch] = 0;
        if (plotValues[ch] > 255)
            plotValues[ch] = 255; // 255 넘으면 안됨 (byte 범위 아님, Waveform 높이 제한)

        // 초기화 처리
        if (lastPlotValues[ch] == -1)
            lastPlotValues[ch] = plotValues[ch];

        // [숫자 표시] Xfloat 값 전송
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

    // 4. 그래프 그리기 (선형 보간 전송)
    // 한번에 점을 팍 찍는 게 아니라, 중간 점들을 채워주어 부드럽게 이음
    for (int i = 1; i <= PIXELS_PER_UPDATE; i++)
    {
        for (int ch = 0; ch < CHANNELS; ch++)
        {
            long diff = plotValues[ch] - lastPlotValues[ch];
            int interpolatedValue = lastPlotValues[ch] + (diff * i / PIXELS_PER_UPDATE);

            // add <id>, <channel>, <value>
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

#if DEBUG_ENABLE
    Serial.print("Ch0 Temp: ");
    Serial.print(currentTemps[0]);
    Serial.print(" => Plot: ");
    Serial.println(plotValues[0]);
#endif

    // 다음 루프를 위해 현재 값을 이전 값으로 저장
    for (int ch = 0; ch < CHANNELS; ch++)
    {
        lastPlotValues[ch] = plotValues[ch];
    }

    // 2초 대기
    delay(2000);
}

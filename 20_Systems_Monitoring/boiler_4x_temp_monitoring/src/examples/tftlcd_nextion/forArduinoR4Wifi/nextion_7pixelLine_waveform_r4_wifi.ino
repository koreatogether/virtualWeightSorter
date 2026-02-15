/*
 * [Arduino R4 WiFi 전용] Nextion 7-Pixel Line Waveform Demo
 *
 * 원본: nextion_7pixelLine_waveform_test.ino
 * 수정: Arduino R4 WiFi (Serial1) 전용, 속도 최적화(115200bps)
 *
 * [동작 설명]
 * - 사인파(Sinewave)를 그리되, 데이터 채널 0~6번을 동시에 사용하여
 *   마치 '두꺼운 펜(7픽셀)'으로 그리는 듯한 시각적 효과를 줍니다.
 *
 * [Nextion 설정]
 * - Waveform Component ID: s0 (id:1로 가정, 코드는 ID 대신 컴포넌트 이름 s0 사용 가능하지만 여기선 ID 11 사용 예시를 따르거나,
 *   여기서는 범용성을 위해 명령어 'add <id>, <ch>, <val>'을 사용합니다.
 *   **주의**: 원본 코드의 s0 ID를 확인해야 합니다. (이전 예제에서 11이었음).
 *   만약 ID를 모른다면, Nextion Editor에서 s0의 ID를 확인하고 아래 설정을 변경하세요.
 */

// [설정] Waveform ID (Nextion Editor에서 s0의 ID를 확인 후 수정하세요)
// 이전 예제(waveform)에서는 11이었으므로 11로 설정합니다.
const int WAVEFORM_ID = 11;

// [설정] 통신 포트
#define HMISerial Serial1

void setup()
{
    // 1. 디버깅 (PC)
    Serial.begin(115200);

    // 2. Nextion (Serial1)
    HMISerial.begin(9600); // 초기 연결
    delay(100);

    // 3. 통신 속도 고속화 (9600 -> 115200)
    // 7개 데이터를 빠르게 보내려면 고속 통신 필수
    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    delay(100);

    HMISerial.end();
    HMISerial.begin(115200); // R4 WiFi는 115200bps 지원

    Serial.println("R4 WiFi 7-Pixel Waveform Demo Started.");
}

float angle = 0;

void loop()
{
    // 기본 사인파 값 생성 (0~100 범위로 스케일링)
    // sin: -1 ~ 1 ==> +1 ==> 0 ~ 2 ==> * 50 ==> 0 ~ 100
    int baseVal = (int)((sin(angle) + 1.0) * 50.0);

    // 7개의 채널에 오프셋을 주어 전송 (수직으로 7픽셀 두께 효과)
    // add <id>, <ch>, <val>
    // R4의 연산 속도가 빠르므로 루프로 처리해도 충분합니다.

    for (int ch = 0; ch < 7; ch++)
    {
        // 3, 2, 1, 0, -1, -2, -3 순서로 오프셋 적용
        int offset = 3 - ch;
        int finalVal = constrain(baseVal + offset, 0, 255);

        HMISerial.print("add ");
        HMISerial.print(WAVEFORM_ID);
        HMISerial.print(",");
        HMISerial.print(ch);
        HMISerial.print(",");
        HMISerial.print(finalVal);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
        HMISerial.write(0xFF);
    }

    // 각도 증가
    angle += 0.05;
    if (angle > 6.28)
        angle = 0;

    // R4 WiFi는 속도가 빠르므로 적절한 딜레이가 필요할 수 있음
    // 115200bps 기준 약 5~10ms 정도면 충분히 빠름
    delay(10);
}

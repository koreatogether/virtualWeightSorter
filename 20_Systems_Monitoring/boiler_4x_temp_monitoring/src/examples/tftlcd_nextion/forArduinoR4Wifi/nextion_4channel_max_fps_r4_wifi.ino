/*
 * [Arduino R4 WiFi 전용] Nextion 4-Channel "Max FPS" Demo + Smart Bridge
 * 파일명: nextion_4channel_max_fps_r4_wifi.ino
 *
 * [목적]
 * - 채널 수를 18개 -> 4개로 줄여 데이터 부하를 최소화합니다.
 * - 물리적으로 가능한 최대 프레임 레이트(Frame Rate)를 구현합니다.
 * - 115200bps 통신 속도에서 얼마나 부드럽고 빠른 그래프 표현이 가능한지 테스트합니다.
 *
 * [성능 계산]
 * - 1개 채널 데이터 패킷: 약 16 Bytes
 * - 4개 채널 합계: 약 64 Bytes
 * - 전송 소요 시간: 64 Bytes * 10 bits / 115200 bps ≈ 5.5 ms
 * - 이론상 최대 FPS: 약 180 FPS
 * - 설정된 FPS: 안전 마진 고려하여 약 125 FPS (8ms 주기) 목표
 */

// [설정] Waveform ID
const int WAVEFORM_ID = 11;

// [설정] 통신 포트
#define HMISerial Serial1

// [설정] 브릿지 모드 진입 버튼 (D4)
const int BRIDGE_TRIGGER_PIN = 4;

void setup()
{
    Serial.begin(115200);

    // 초기 9600 연결 (안정성)
    HMISerial.begin(9600);

    // 브릿지 체크
    pinMode(BRIDGE_TRIGGER_PIN, INPUT_PULLUP);
    delay(200);

    if (digitalRead(BRIDGE_TRIGGER_PIN) == LOW)
    {
        Serial.println("\n==========[ BRIDGE MODE ACTIVATED ]==========");
        Serial.println("Pass-Through: USB <--> Serial1 @ 9600bps");
        while (true)
        {
            if (Serial.available())
                HMISerial.write(Serial.read());
            if (HMISerial.available())
                Serial.write(HMISerial.read());
        }
    }

    Serial.println("\n>>> 4-CHANNEL MAX FPS DEMO STARTED <<<");

    // 고속 통신 전환 (115200bps)
    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    delay(100);

    HMISerial.end();
    HMISerial.begin(115200);

    Serial.println("Status: Running at 115200bps / Target interval: 8ms");
}

// [핵심 튜닝] 업데이트 주기 8ms (초당 125회 갱신)
// 계산 근거: 4개 채널 전송에 약 5.5ms 소요되므로, 8ms는 빡빡하지만 가능한 수치입니다.
// Nextion 화면 주사율(보통 60Hz)보다 빠르지만, 입력 버퍼만 넘치지 않으면 아주 부드럽습니다.
const unsigned long UPDATE_INTERVAL = 8;
unsigned long previousMillis = 0;

float angle = 0;

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= UPDATE_INTERVAL)
    {
        previousMillis = currentMillis;

        int baseVal = (int)((sin(angle) + 1.0) * 50.0);

        // 4개 채널만 전송
        for (int ch = 0; ch < 4; ch++)
        {
            // 오프셋을 주어 선들이 뭉치지 않게 (Red, Green, Blue, Yellow 라인이라 가정 시)
            int offset = (ch * 5);
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

        // 주기가 빨라졌으므로(25ms -> 8ms), 각도는 조금씩만 이동해야 눈으로 따라갈 수 있습니다.
        // 0.1 / 3 ≈ 0.035
        angle += 0.04;
        if (angle > 6.28)
            angle = 0;
    }
}

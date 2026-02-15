/*
 * [Arduino R4 WiFi 전용] Nextion 18-Channel Waveform Stress Test + Smart Bridge
 * 파일명: nextion_18channel_waveform_r4_wifi.ino
 *
 * [목적]
 * - 아두이노 R4 WiFi의 연산 및 통신 능력을 테스트합니다.
 * - 18개의 데이터 채널을 동시에 전송하여 통신 부하를 발생시킵니다.
 * - Nextion Waveform 컴포넌트 설정에 따라 일부 채널은 표시되지 않을 수 있습니다.
 *   (일반적으로 Waveform은 최대 4~5개 채널을 지원하지만, 설정을 통해 늘렸거나 스트레스 테스트 목적이라 가정함)
 *
 * [기능]
 * 1. 스마트 브릿지 (D4 + Reset): Nextion Editor 업로드 지원
 * 2. 18-Line Graph: 채널 0~17에 사인파 데이터를 오프셋을 주어 전송
 */

// [설정] Waveform ID
const int WAVEFORM_ID = 11;

// [설정] 통신 포트
#define HMISerial Serial1

// [설정] 브릿지 모드 진입 버튼 (D4)
const int BRIDGE_TRIGGER_PIN = 4;

void setup()
{
    // 1. 디버깅 및 PC 통신
    Serial.begin(115200);

    // 2. Nextion 통신 초기화 (9600 시작)
    HMISerial.begin(9600);

    // 3. 브릿지 모드 체크
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

    // 4. 데모 모드 (고속 통신 전환)
    Serial.println("\n>>> 18-CHANNEL STRESS TEST STARTED <<<");

    // baud=115200 명령 전송
    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    delay(100);

    HMISerial.end();
    HMISerial.begin(115200);

    Serial.println("Running at 115200bps.");
}

// [설정] 업데이트 주기 (밀리초)
// 전송 시간(약 16ms)보다 커야 함. 25ms = 초당 40회 갱신 (안정적)
const unsigned long UPDATE_INTERVAL = 25;
unsigned long previousMillis = 0;

float angle = 0;

void loop()
{
    unsigned long currentMillis = millis();

    // 설정한 시간(25ms)마다 실행 (Non-blocking)
    if (currentMillis - previousMillis >= UPDATE_INTERVAL)
    {
        previousMillis = currentMillis;

        // 기본값 (0~100)
        int baseVal = (int)((sin(angle) + 1.0) * 50.0);

        // 18개 채널 전송
        for (int ch = 0; ch < 18; ch++)
        {
            int offset = 9 - ch;
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

        // 각도 증가 (일정한 주기로 실행되므로 파형 속도도 일정해짐)
        angle += 0.1;
        if (angle > 6.28)
            angle = 0;
    }
}

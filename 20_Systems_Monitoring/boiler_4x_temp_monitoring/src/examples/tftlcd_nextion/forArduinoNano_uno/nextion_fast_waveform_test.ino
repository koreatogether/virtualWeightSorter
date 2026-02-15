#include <Nextion.h>
#include <SoftwareSerial.h>

/**
 * [HMI 정보]
 * Page: 2
 * ID: 11
 * Name: s0
 * Channel: 0 (pco0: Cyan)
 * Range: 0 ~ 100
 *
 * [통신 설정]
 * 현재 넥션 라이브러리가 'HMISerial'이라는 객체를 찾도록 설정되어 있어
    에러가 발생합니다.
 * SoftwareSerial로 2, 3을 정의합니다.
 */

// 하드웨어 0, 1번 핀 사용 (업로드 시 넥션 연결 해제 필수)
SoftwareSerial HMISerial(2, 3);

NexWaveform s0 = NexWaveform(2, 11, "s0");

float angle = 0;

void setup()
{
    // Nextion 통신 시작 (9600 bps)
    // 9600이면 초당 약 100개 정도의 데이터를 보낼 수 있습니다.
    nexInit();
}

void loop()
{
    // 1. 사인파 데이터 생성 (0~100 범위)
    uint8_t sensorValue = (uint8_t)((sin(angle) + 1.0) * 50.0);

    // 2. Nextion 웨이브폼으로 데이터 전송 (채널 0만 사용)
    s0.addValue(0, sensorValue);

    // 3. 파형 각도 업데이트
    // 변화량을 작게 줄수록 파형이 촘촘하고 부드럽게 그려집니다.
    angle += 0.05;
    if (angle > 6.28)
        angle = 0;

    // 4. 지연 시간 없음 (또는 아주 최소화)
    // 9600 보드레이트 자체의 전송 속도가 지연시간 역할을 하게 됩니다.
    // 만약 데이터가 깨진다면 delay(5); 정도를 추가하는 것이 안전합니다.
}

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
 * -
 * - 현재 넥션 라이브러리가 'HMISerial'이라는 객체를 찾도록 설정되어 있어 에러가 발생합니다.

 * SoftwareSerial로 2, 3을 정의합니다.
 */

/*
nextion waveform test
ch -> 4
pco0 -> 2027
   pco1 -> 65519
   pco2 -> 63488
   pco3 -> 34800
*/

// 넥션 라이브러리(NexConfig.h)에서 요구하는 'HMISerial' 객체 정의
SoftwareSerial HMISerial(2, 3);

NexWaveform s0 = NexWaveform(2, 11, "s0");

// 테스트 시퀀스 관리
int count = 0;
float angle = 0;

void setup()
{
    // Nextion 통신 시작 (기본 보드레이트 9600)
    nexInit();
}

void loop()
{
    // 100회 실행 후 멈추거나 루프를 조정하고 싶다면 여기에 조건을 추가할 수 있습니다.
    if (count >= 100)
    {
        // 100회 완료 후 잠시 대기하거나 카운트를 초기화하여 무한 반복
        count = 0;
        delay(2000);
    }

    // 각 채널별 데이터 생성 및 전송 (사인파 위상차를 두어 겹치지 않게 함)
    // channel 0: 사이언 (위상차 0)
    // channel 1: 노랑 (위상차 PI/2)
    // channel 2: 빨강 (위상차 PI)
    // channel 3: 녹색 (위상차 3PI/2)

    uint8_t val0 = (uint8_t)((sin(angle) + 1.0) * 50.0);
    uint8_t val1 = (uint8_t)((sin(angle + 1.57) + 1.0) * 50.0); // + 90도
    uint8_t val2 = (uint8_t)((sin(angle + 3.14) + 1.0) * 50.0); // + 180도
    uint8_t val3 = (uint8_t)((sin(angle + 4.71) + 1.0) * 50.0); // + 270도

    // Nextion 웨이브폼으로 데이터 전송 (s0 컴포넌트의 0~3번 채널)
    s0.addValue(0, val0);
    s0.addValue(1, val1);
    s0.addValue(2, val2);
    s0.addValue(3, val3);

    // 다음 데이터 준비
    angle += 0.2; // 파형을 더 부드럽게 (200ms 주기에 맞춤)
    if (angle > 6.28)
        angle = 0;

    count++;

    // 200ms 간격으로 업데이트
    delay(200);
}

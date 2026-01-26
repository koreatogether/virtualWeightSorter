#include <Nextion.h>
#include <SoftwareSerial.h>

/**
 * [HMI 설정 필수 사항]
 * 1. s0 컴포넌트의 'ch' 속성을 7로 변경 (최소 4개 이상 지원 여부 확인)
 * 2. pco0 ~ pco6까지의 색상을 모두 동일한 색(예: Cyan 2047)으로 지정
 */

// 하드웨어 시리얼 0, 1번 사용
SoftwareSerial HMISerial(2, 3);

NexWaveform s0 = NexWaveform(2, 11, "s0");
float angle = 0;

void setup()
{
    nexInit();
}

void loop()
{
    // 기본 사인파 값 생성 (0~100)
    int baseVal = (int)((sin(angle) + 1.0) * 50.0);

    // 7개의 채널에 오프셋을 주어 전송 (수직으로 7픽셀 두께 효과)
    // baseVal이 0~100이므로 오프셋 적용 시 범위를 벗어나지 않도록 제어
    s0.addValue(0, constrain(baseVal + 3, 0, 255));
    s0.addValue(1, constrain(baseVal + 2, 0, 255));
    s0.addValue(2, constrain(baseVal + 1, 0, 255));
    s0.addValue(3, constrain(baseVal + 0, 0, 255)); // 중심선
    s0.addValue(4, constrain(baseVal - 1, 0, 255));
    s0.addValue(5, constrain(baseVal - 2, 0, 255));
    s0.addValue(6, constrain(baseVal - 3, 0, 255));

    angle += 0.05;
    if (angle > 6.28)
        angle = 0;

    // 실시간성을 위해 딜레이 최소화 (9600 보드레이트 속도에 의존)
    // 데이터가 끊기면 delay(2); 정도 추가하세요.
}
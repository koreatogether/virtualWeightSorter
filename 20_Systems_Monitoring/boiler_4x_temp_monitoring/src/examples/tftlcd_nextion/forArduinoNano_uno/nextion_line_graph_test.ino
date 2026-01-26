/*
 * Nextion 공식 Waveform (s0) 사용 예제
 *
 * [Nextion Editor 설정 필수 사항]
 * 1. 기존의 수동 드로잉 영역이나 배경 이미지를 삭제하세요.
 * 2. 'Waveform' 컴포넌트를 추가하세요. (Toolbox -> Waveform)
 * 3. Waveform 컴포넌트의 'objname'을 확인하세요 (기본값 s0).
 * 4. Waveform 컴포넌트의 'id'를 확인하세요 (보통 1 또는 2). -> 코드의 WAVEFORM_ID 변수와 일치시켜야 합니다.
 * 5. 속성창(Attributes)에서 다음을 설정하세요:
 *    - dir: "right to left" (우측에서 좌측으로 흐름) 또는 "left to right"
 *    - ch: 1 (채널 수)
 *    - gco: 그리드(격자) 색상 (예: 65535 흰색)
 *    - bco: 배경 색상 (예: 0 검은색)
 *    - pco0: 그래프 선 색상 (예: 2047 Cyan)
 *    - min, max: 데이터 스케일링 설정 아님 (Nextion은 항상 0~255로 데이터 받음. Arduino map함수로 조절)
 */

#include <Nextion.h>
#include <SoftwareSerial.h>

SoftwareSerial HMISerial(2, 3);

// [중요] Nextion Editor의 Waveform 컴포넌트 ID (Attribute 창의 'id' 값 확인)
const int WAVEFORM_ID = 1;
const int WAVEFORM_CHANNEL = 0;

// 표시할 온도의 범위 (그래프의 바닥값과 천장값)
const int TEMP_MIN = 0;
const int TEMP_MAX = 100;

void setup()
{
    // 넥시온 초기화
    nexInit();

    // 그래프 초기화 (선택사항: 시작 시 그래프 비우기)
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
    // 1. 센서 데이터 생성 (40.0도 ~ 80.0도 사이 랜덤 시뮬레이션)
    float currentTemp = random(400, 800) / 10.0;

    // 2. 값 매핑 (Mapping)
    // Nextion Waveform은 높이를 0~255 사이의 바이트 값으로 받아들입니다.
    // 실제 온도 범위를 그래프 높이(0~255)에 맞게 변환합니다.
    int plotValue = map((int)currentTemp, TEMP_MIN, TEMP_MAX, 0, 255);

    // 범위 제한 (안전장치)
    if (plotValue < 0)
        plotValue = 0;
    if (plotValue > 255)
        plotValue = 255;

    // 3. Nextion 공식 그래프 전송 명령어: add
    // 문법: add <id>, <channel>, <value>
    // 이 명령을 보내면 그래프가 자동으로 왼쪽으로 한 칸 스크롤 되며 값이 찍힙니다.
    HMISerial.print("add ");
    HMISerial.print(WAVEFORM_ID);
    HMISerial.print(",");
    HMISerial.print(WAVEFORM_CHANNEL);
    HMISerial.print(",");
    HMISerial.print(plotValue);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);

    // 4. 요구하신 대로 2초 간격 갱신
    delay(2000);
}

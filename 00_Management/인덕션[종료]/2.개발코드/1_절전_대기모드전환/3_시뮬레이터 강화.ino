/*
가상시뮬레이터 1차 완성
https://wokwi.com/projects/422503314596768769

버튼 상태를 모니터링하고 1초 이상 눌렸을 때의 기존 코드 + 하기
전원 버튼 과정을 수행하면 각 디스플레이 및 LED에 정해진 행동 수행하기 위한 코드 추가
*/

#include <ezButton.h>

// 매직 넘버 제거를 위한 상수 정의
const unsigned long DEBOUNCE_TIME = 50;           // 디바운스 시간 (ms)
const unsigned long LONG_PRESS_THRESHOLD = 1000;  // 롱프레스 임계값 (ms)

// 함수 안내 및 프로토타입
// processButton(): 버튼의 눌림 상태를 확인하여 1초 이상 누르면 이벤트 발생 및 최종 누른 시간을 기록
void processButton();

const int POWER_ON_BUTTON = 2;
const int F_POWER_PLUS_BUTIRE13;
const int FIRE_MINUS_BUTTON = 12;
ezButton powerButton(POWER_ON_BUTTON);  // ezButton 인스턴스 생성
ezButton firePlusButton(FIRE_PLUS_BUTTON);
ezButton fireMinusButton(FIRE_MINUS_BUTTON);

// millis() 함수: 보드 시작 후 경과된 시간을 밀리초 단위로 반환한다.
unsigned long pressStartTime = 0;   // 버튼 눌린 시작 시각 저장
unsigned long lastPressedTime = 0;  // 최종 눌린 총시간 저장

void setup() {
    Serial.begin(9600);
    powerButton.setDebounceTime(DEBOUNCE_TIME);
    firePlusButton.setDebounceTime(DEBOUNCE_TIME);
    fireMinusButton.setDebounceTime(DEBOUNCE_TIME);
}

// 전원 버튼이 1초이상 눌리면 인덕션 1구의 디스플레이 0 이라는 숫자가 표시되어야 한다.

void processButton()  // 1초 이상 버튼 눌림을 감지하는 코드
{
    if (powerButton.getState() == LOW) {  // 버튼이 눌렸을 때: 버튼이 누르기 시작한 경우를 처리
        if (pressStartTime == 0) {
            pressStartTime = millis();  // 버튼 누르기 시작 시각 저장
        }
    } else {  // 버튼이 놓였을 때: 버튼에서 손을 뗀 경우 누른 시간 측정 후 처리
        if (pressStartTime != 0) {
            unsigned long dt = millis() - pressStartTime;
            if (dt >= LONG_PRESS_THRESHOLD) {  // 롱프레스 임계값 이상 눌린 경우 처리
                Serial.println("Long press detected");
            }
            lastPressedTime = dt;  // 최종 눌린 시간 기록
            pressStartTime = 0;    // 시작 시각 초기화
        }
    }
}

void loop() {
    powerButton.loop();  // 버튼 상태 업데이트, 최상위에 위치할것 , 라이브러리 안에 숨겨짐
    processButton();     // 1초 이상 눌림 감지 및 처리 함수
}
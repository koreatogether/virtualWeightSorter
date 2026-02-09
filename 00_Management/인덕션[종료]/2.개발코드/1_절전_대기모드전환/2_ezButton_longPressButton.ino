/*
MIT License

Copyright (c) 2023 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*/

/*
ezButton 깃허브 주소: https://github.com/ArduinoGetStarted/button
ezButton 사용자 설명이 있는 페이지:
https://arduinogetstarted.com/tutorials/arduino-button-library

장점:
1. 버튼 디바운싱 처리가 내장되어 있어 별도 코드 불필요
2. 코드 간결성
3. 필요한 기능 다수 제공

단점:
1. 라이브러리 사용으로 작동 원리 파악 어려움
2. 특수한 경우 적용 어려움
3. 라이브러리 미숙지 시 디버깅 어려움
4. 불필요한 기능 포함으로 메모리 소모 증가
*/

// 시뮬레이션 주소 : https://wokwi.com/projects/422474227777368065

#include <ezButton.h>

// 매직 넘버 제거를 위한 상수 정의
const unsigned long DEBOUNCE_TIME = 50;           // 디바운스 시간 (ms)
const unsigned long LONG_PRESS_THRESHOLD = 1000;  // 롱프레스 임계값 (ms)

// 함수 안내 및 프로토타입
// processButton(): 버튼의 눌림 상태를 확인하여 1초 이상 누르면 이벤트 발생 및 최종 누른 시간을 기록
void processButton();

// millis() 함수: 보드 시작 후 경과된 시간을 밀리초 단위로 반환한다.

const int POWER_ON_BUTTON = 2;     // 버튼이 연결된 핀 번호
ezButton button(POWER_ON_BUTTON);  // ezButton 인스턴스 생성

unsigned long pressStartTime = 0;   // 버튼 눌린 시작 시각 저장
unsigned long lastPressedTime = 0;  // 최종 눌린 총시간 저장

void setup() {
    Serial.begin(9600);
    button.setDebounceTime(DEBOUNCE_TIME);  // 디바운스 시간 상수 사용
}

void processButton()  // 1초 이상 버튼 눌림을 감지하는 코드
{
    if (button.getState() == LOW) {  // 버튼이 눌렸을 때: 버튼이 누르기 시작한 경우를 처리
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
    button.loop();    // 버튼 상태 업데이트, 최상위에 위치할것 , 라이브러리 안에 숨겨짐
    processButton();  // 1초 이상 눌림 감지 및 처리 함수
}
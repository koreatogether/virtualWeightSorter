#include <Adafruit_Keypad.h>
#include <Servo.h>
#include "keypadManager.h"
#include "potentiometerController.h"
#include "servoController.h"
#include "lcdPlay.h"
#include "ledController.h"
#include "buzzerController.h"
#include <Arduino.h>
#include "KeypadLockController.h"

/*
키패드 잠금 장치의 메인 컨트롤러
- setup(): 초기화
- loop(): 지속적인 업데이트 및 입력 처리
*/

KeypadLockController keypad; // 키패드 의 객체 객체 생성

void setup()
{
    keypad.begin();
}

void loop()
{
    keypad.update();
}

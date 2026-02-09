#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix matrix;

// 주사위 패턴 정의 (8x12 LED 매트릭스에 맞춤)
const uint32_t DICE_PATTERNS[6][8] = {
  { // 1
    0b00000000000,
    0b00000000000,
    0b00000000000,
    0b00001100000,
    0b00001100000,
    0b00000000000,
    0b00000000000,
    0b00000000000
  },
  { // 2
    0b00000000000,
    0b00110000000,
    0b00110000000,
    0b00000000000,
    0b00000000000,
    0b00000011000,
    0b00000011000,
    0b00000000000
  },
  { // 3
    0b11000000000,
    0b11000000000,
    0b00000000000,
    0b00011000000,
    0b00011000000,
    0b00000000000,
    0b00000000110,
    0b00000000110
  },
  { // 4
    0b11000000110,
    0b11000000110,
    0b00000000000,
    0b00000000000,
    0b00000000000,
    0b00000000000,
    0b11000000110,
    0b11000000110
  },
  { // 5
    0b11000000110,
    0b11000000110,
    0b00000000000,
    0b00011000000,
    0b00011000000,
    0b00000000000,
    0b11000000110,
    0b11000000110
  },
  { // 6
    0b11000000110,
    0b11000000110,
    0b00000000000,
    0b11000000110,
    0b11000000110,
    0b00000000000,
    0b11000000110,
    0b11000000110
  }
};

const int BUTTON_PIN = 2;  // 버튼 핀 설정
bool lastButtonState = HIGH;
bool isRolling = false;
int currentDice = 0;
unsigned long lastUpdateTime = 0;
const int ANIMATION_SPEED = 100;  // 애니메이션 속도 (ms)

void setup() {
  matrix.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);
  
  // 버튼 상태 변화 감지
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {  // 버튼이 눌렸을 때
      isRolling = true;
    } else {  // 버튼이 떼어졌을 때
      isRolling = false;
      // 최종 주사위 값 결정
      currentDice = random(6);
      matrix.loadFrame(DICE_PATTERNS[currentDice]);
    }
    delay(50);  // 디바운싱
  }
  
  // 주사위 굴리기 애니메이션
  if (isRolling && (millis() - lastUpdateTime >= ANIMATION_SPEED)) {
    currentDice = (currentDice + 1) % 6;
    matrix.loadFrame(DICE_PATTERNS[currentDice]);
    lastUpdateTime = millis();
  }
  
  lastButtonState = buttonState;
}
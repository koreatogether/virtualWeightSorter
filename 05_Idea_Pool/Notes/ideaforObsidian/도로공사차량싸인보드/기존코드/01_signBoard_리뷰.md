# signBoard.ino 코드 리뷰

## 1. 개요
- **파일명**: signBoard.ino
- **목적**: 도로공사 차량용 방향 지시등(싸인 보드) 제어 시스템
- **하드웨어**: Arduino(우노 또는 나노), LED 14개, 버튼 3개
- **라이브러리**: ezButton, ezOutput 사용

## 2. 코드 구조
- 전체적으로 간단한 구조로, 설정(setup), 반복(loop), 기능 함수로 구성됨
- 3개의 버튼 입력에 따라 서로 다른 LED 패턴을 표시하는 방식
- 각 버튼을 2번 누르면 모든 LED가 꺼지도록 설계됨

## 3. 긍정적인 부분
- ezButton, ezOutput 라이브러리 활용으로 코드가 간결해짐
- 디바운싱 처리가 적절히 구현되어 있음
- 버튼 카운팅 기능을 통해 두 번 클릭 시 LED를 끄는 기능이 편리함
- LED 시퀀스가 시각적으로 효과적인 방식으로 구현됨
- 함수(everyLedOff)를 사용하여 코드 중복을 줄임

## 4. 개선이 필요한 부분

### 4.1. 코드 명확성 및 일관성
- 버튼 이름에 혼란이 있음: button1은 "right arrow"로 주석 되어있지만 코드에서는 "Left Arrow Button" 메시지를 출력
- button3도 마찬가지로 주석과 실제 동작이 불일치함
- 주석에 "LED + Register 220 ohm 14ea"이라고 되어 있지만 코드에서는 8개의 LED만 사용됨

### 4.2. 코드 구조 및 효율성
- 비슷한 코드 패턴이 반복되므로 함수화하여 더욱 간결하게 만들 수 있음
- LED 패턴을 배열로 정의하면 코드를 더 간결하게 만들 수 있음
- 상태 관리가 단순하여 복잡한 패턴이나 애니메이션을 추가하기 어려운 구조임

### 4.3. 안전성 및 에러 핸들링
- LED를 직접 연결 시 보드가 손상될 수 있다는 경고는 있으나 안전한 연결 방법에 대한 설명이 부족함
- 외부 입력이나 예외 상황에 대한 처리가 없음

## 5. 개선 제안

### 5.1. 코드 명확성 향상
```cpp
// 버튼과 LED의 역할을 명확히 정의
ezButton buttonLeft(2);    // 왼쪽 화살표 버튼
ezButton buttonBoth(3);    // 양방향 화살표 버튼
ezButton buttonRight(4);   // 오른쪽 화살표 버튼
```

### 5.2. LED 제어 코드 개선
```cpp
// LED 패턴을 정의하는 함수 추가
void leftArrowPattern() {
  // 왼쪽 방향 화살표 패턴
  int delays[] = {100, 200, 300, 400, 500};
  ezOutput* leftLeds[] = {&led5, &led4, &led3, &led2, &led1};
  
  for (int i = 0; i < 5; i++) {
    leftLeds[i]->blink(300, 300, delays[i]);
  }
  
  // 나머지 LED 끄기
  led6.low();
  led7.low();
  led8.low();
}
```

### 5.3. 상태 관리 개선
```cpp
// 열거형을 사용한 상태 정의
enum SignBoardState {
  OFF,
  LEFT_ARROW,
  BOTH_SIDES,
  RIGHT_ARROW
};

// 현재 상태 변수
SignBoardState currentState = OFF;

// 상태에 따른 동작 제어
void updateLEDsBasedOnState() {
  switch(currentState) {
    case OFF:
      everyLedOff();
      break;
    case LEFT_ARROW:
      leftArrowPattern();
      break;
    // 다른 상태에 대한 처리...
  }
}
```

## 6. 결론
전반적으로 기본 기능은 잘 구현되어 있으나, 코드의 명확성과 구조적 효율성 측면에서 개선할 여지가 있습니다. 특히 LED 패턴 정의를 체계화하고 상태 관리를 개선하면 코드 유지보수성과 확장성이 향상될 것으로 보입니다. 또한 하드웨어 연결에 대한 더 명확한 지침이 제공되면 사용자가 프로젝트를 더 안전하게 구현할 수 있을 것입니다.
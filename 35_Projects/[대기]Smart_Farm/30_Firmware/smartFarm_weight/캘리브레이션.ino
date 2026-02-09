// https://github.com/RobTillaart/HX711

#include "HX711.h"

HX711 myScale; // HX711 객체 생성

uint8_t dataPin = 6;  // 데이터 핀 번호
uint8_t clockPin = 7; // 클록 핀 번호

uint32_t start, stop; // 사용하지 않는 변수 (코드에서 사용되지 않음)
volatile float f;     // 사용하지 않는 변수 (코드에서 사용되지 않음)

void setup()
{
  Serial.begin(115200);     // 시리얼 통신 시작 (보드레이트 115200)
  Serial.println(__FILE__); // 현재 파일 이름 출력
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION); // HX711 라이브러리 버전 출력
  Serial.println();

  myScale.begin(dataPin, clockPin); // HX711 센서 초기화 (데이터 핀, 클록 핀 연결)
}

void loop()
{
  calibrate(); // 보정 함수 호출
}

void calibrate()
{
  Serial.println("\n\nCALIBRATION\n===========");        // "보정\n===========" 출력
  Serial.println("remove all weight from the loadcell"); // "로드셀에서 모든 무게 제거" 출력
  //  flush Serial input (시리얼 입력 버퍼 비우기)
  while (Serial.available())
    Serial.read();

  Serial.println("and press enter\n"); // "그리고 엔터키 누르기\n" 출력
  while (Serial.available() == 0)
    ; // 사용자 입력 대기

  Serial.println("Determine zero weight offset"); // "영점 보정 시작" 출력
  myScale.tare(20);                               // 20회 측정하여 타리 (영점 보정)
  uint32_t offset = myScale.get_offset();         // 오프셋 값 읽기

  Serial.print("OFFSET: "); // "오프셋: " 출력
  Serial.println(offset);   // 오프셋 값 출력
  Serial.println();         // 개행 문자 출력

  Serial.println("place a weight on the loadcell"); // "로드셀에 무게 올려놓기" 출력
  //  flush Serial input (시리얼 입력 버퍼 비우기)
  while (Serial.available())
    Serial.read();

  Serial.println("enter the weight in (whole) grams and press enter"); // "무게를 (정수) 그램 단위로 입력하고 엔터키 누르기" 출력
  uint32_t weight = 0;                                                 // 무게 값 저장 변수
  while (Serial.peek() != '\n')
  { // 엔터키 입력 받을 때까지 반복
    if (Serial.available())
    {                          // 데이터가 있으면
      char ch = Serial.read(); // 데이터 읽기
      if (isdigit(ch))
      {                               // 숫자 문자열이면
        weight *= 10;                 // 10배 늘리기
        weight = weight + (ch - '0'); // 문자열을 숫자로 변환하여 더하기
      }
    }
  }
  Serial.print("WEIGHT: ");            // "무게: " 출력
  Serial.println(weight);              // 무게 값 출력
  myScale.calibrate_scale(weight, 20); // 무게 값과 20회 측정값을 이용하여 보정

  float scale = myScale.get_scale(); // 스케일 값 읽기

  Serial.print("SCALE:  "); // "스케일: " 출력
  Serial.println(scale, 6); // 스케일 값 6자리 소수점 표기로 출력

  Serial.print("\nuse scale.set_offset("); // "setup 함수에서 scale.set_offset(" 출력
  Serial.print(offset);                    // 오프셋 값 출력
  Serial.print("); and scale.set_scale("); // "); and scale.set_scale(" 출력
  Serial.print(scale, 6);                  // 스케일 값 6자리 소수점 표기로 출력
  Serial.print(");\n");                    // ");\n" 출력
  Serial.println("in the setup of your project");

  Serial.println("\n\n");
}

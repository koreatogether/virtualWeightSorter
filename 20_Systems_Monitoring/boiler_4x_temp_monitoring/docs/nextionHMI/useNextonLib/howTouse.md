# Arduino Nano + Nextion 라이브러리 설정 및 사용법

Nextion 공식 라이브러리(`ITEADLIB_Arduino_Nextion`)를 Arduino Nano에서 사용하기 위한 설정 방법입니다. Nano는 하드웨어 시리얼 포트가 하나뿐이므로 `SoftwareSerial`을 사용하여 Nextion과 통신하고, 하드웨어 Serial은 USB 디버깅용으로 사용합니다.

---

## 1. 라이브러리 설정 파일 수정 (`NexConfig.h`)

라이브러리가 설치된 경로(보통 `Documents\Arduino\libraries\ITEADLIB_Arduino_Nextion`) 내의 `NexConfig.h` 파일을 아래와 같이 수정해야 합니다.

**경로 예시:** `C:\Users\h\Documents\Arduino\libraries\ITEADLIB_Arduino_Nextion-master\NexConfig.h`

**수정 내용:**
기존에 `SoftwareSerial`을 정의하지 않아 발생하는 컴파일 에러를 방지하고, 올바르게 연결하기 위해 아래 코드로 내용을 교체하거나 수정합니다.

```cpp
#ifndef __NEXCONFIG_H__
#define __NEXCONFIG_H__

// 1. 디버그 모드 비활성화 (필요 시 활성화 가능하지만 Nano에서는 메모리/포트 부족 주의)
// #define DEBUG_SERIAL_ENABLE

// 2. 디버그 포트 설정 (USB Serial 사용)
#define dbSerial Serial

// 3. Nextion 통신 포트 설정 (SoftwareSerial 사용)
// 기존 #define nexSerial Serial2 등은 주석 처리 또는 삭제
// #define nexSerial Serial 

// --- [중요] SoftwareSerial 설정 추가 ---
#include <SoftwareSerial.h>
extern SoftwareSerial HMISerial;  // 아두이노 코드(.ino)에 있는 객체 이름과 정확히 일치해야 함
#define nexSerial HMISerial       // 라이브러리가 HMISerial 객체를 사용하도록 연결
// ------------------------------------

#ifdef DEBUG_SERIAL_ENABLE
#define dbSerialPrint(a) dbSerial.print(a)
#define dbSerialPrintln(a) dbSerial.println(a)
#define dbSerialBegin(a) dbSerial.begin(a)
#else
#define dbSerialPrint(a) do{}while(0)
#define dbSerialPrintln(a) do{}while(0)
#define dbSerialBegin(a) do{}while(0)
#endif

#endif
```

---

## 2. 하드웨어 연결 (배선)

`SoftwareSerial HMISerial(10, 11);` 설정 기준:

*   **Nextion TX** → **Nano D10** (RX)
*   **Nextion RX** → **Nano D11** (TX)
*   **Nextion 5V** → **Nano 5V** (전력이 부족할 경우 외부 전원 사용 권장)
*   **Nextion GND** → **Nano GND**

---

## 3. 아두이노 스케치 코드 작성 예시

라이브러리 설정(`NexConfig.h`)에서 `extern SoftwareSerial HMISerial;`을 선언했으므로, `.ino` 파일에서 반드시 `HMISerial`이라는 이름으로 객체를 생성해야 합니다.

```cpp
#include <Nextion.h>
#include <SoftwareSerial.h>

// [중요] NexConfig.h의 extern 선언과 이름이 일치해야 함
// RX = 10번 핀, TX = 11번 핀
SoftwareSerial HMISerial(10, 11);

// 페이지 및 컴포넌트 객체 선언 (Nextion 에디터 설정과 ID가 일치해야 함)
NexText t0 = NexText(0, 1, "t0"); // page:0, id:1, name:t0

// 이벤트 리스너 리스트
NexTouch *nex_listen_list[] = {
  &t0,
  NULL
};

void setup() {
  // 1. 디버깅용 시리얼 (PC와 연결)
  Serial.begin(9600);
  
  // 2. Nextion 연결 초기화 (내부적으로 HMISerial.begin(9600) 호출됨)
  if (nexInit()) {
    Serial.println("Nextion Connected!");
    
    // 예제 동작: 텍스트 및 색상 변경
    t0.setText("Hello Nano");
    t0.Set_background_color_bco(65535); // 흰색
    t0.Set_font_color_pco(0);           // 검정색
  } else {
    Serial.println("Nextion Connection Failed...");
  }
}

void loop() {
  // 터치 이벤트 감지
  nexLoop(nex_listen_list);
}
```

---

## 4. 주의사항

1.  **초기화 필수:** Nextion 디스플레이에 `.tft` 파일을 SD카드로 업로드하여 UI가 준비된 상태여야 아두이노 코드가 정상 작동합니다.
2.  **통신 속도:** SoftwareSerial은 115200bps 등 고속 통신에서 불안정할 수 있습니다. 그래프 등 대용량 데이터 전송 시에는 끊김이 발생할 수 있으므로 기본 **9600bps** 사용을 권장합니다.
3.  **컴파일 에러:** `SoftwareSerial does not name a type` 에러가 발생한다면, `NexConfig.h` 파일 상단에 `#include <SoftwareSerial.h>`가 누락되었는지 확인하세요.
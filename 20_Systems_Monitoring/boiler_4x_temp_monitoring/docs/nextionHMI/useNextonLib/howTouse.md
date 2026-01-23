C:\Users\h\Documents\Arduino\libraries\ITEADLIB_Arduino_Nextion-master

위 라이브러리를 활용해서 코딩하면 되지만 
초기에 해야 할 설정이 있다 .


제공해주신 NexConfig.h 파일 코드에서 Nextion 통신 포트를 설정하는 줄을 수정해야 합니다. 아두이노 나노는 Serial2가 없으므로 이 부분을 변경해야 합니다.

수정 방법
주석 처리할 부분:
아래 줄을 찾아서 앞에 //를 붙여 주석 처리하세요.

추가할 부분:
주석 처리한 바로 아래에 SoftwareSerial 설정을 추가하세요.

최종 수정 예시
파일을 다음과 같이 고치시면 됩니다.
/**
 * Define nexSerial for communicate with Nextion touch panel. 
 */
// #define nexSerial Serial2       <-- 원래 있던 줄은 주석 처리

// --- 아래 3줄 추가 ---
#include <SoftwareSerial.h>
extern SoftwareSerial HMISerial;  // 아두이노 코드(.ino)에 있는 객체 이름과 맞춰야 함
#define nexSerial HMISerial       // Nextion 라이브러리가 HMISerial을 사용하도록 연결
// --------------------


이렇게 수정한 후 저장하시고, 아두이노 스케치(.ino) 파일 맨 위에서 다음과 같이 선언해서 사용하시면 됩니다.

!! 반드시 sd 카드에 tft 파일을 넣어서 Nextion 에디터에서 업로드를 반드시 해야 nextion display 에서 아두이노 코드를 받아 들일 준비가 되는 겁니다. 



```
#include <Nextion.h>
#include <SoftwareSerial.h>

// NexConfig.h 수정 완료된 상태 (HMISerial 사용)
SoftwareSerial HMISerial(10, 11); 

// [중요] Nextion 에디터 안의 설정과 일치해야 합니다.
// (페이지번호:0, 컴포넌트ID:1, 객체이름:"t0")
// ※ 에디터에서 t0의 ID가 1이 아니라면 숫자를 바꿔주세요. 이름("t0")이 제일 중요합니다.
NexText t0 = NexText(0, 1, "t0");

NexTouch *nex_listen_list[] = {
  &t0,
  NULL
};

void setup() {
  Serial.begin(115200);
  delay(2000); // Nextion 부팅 안전 대기

  if (nexInit()) {
    Serial.println("Nextion 연결 성공!");

    // 1. 글자색을 검정(0)으로 변경 (흰 배경에는 검은 글씨가 잘 보임)
    t0.Set_font_color_pco(0);

    // 2. 배경색을 흰색(65535)으로 변경
    // Nextion 색상 코드: 65535 = White, 0 = Black, 63488 = Red ...
    t0.Set_background_color_bco(65535);

    // 3. 텍스트 보내기
    t0.setText("hangul");
    
    Serial.println("명령 전송 완료: 배경 흰색, 텍스트 hangul");

  } else {
    Serial.println("연결 실패... 배선 확인 필요");
  }
}

void loop() {
  nexLoop(nex_listen_list);
}
```
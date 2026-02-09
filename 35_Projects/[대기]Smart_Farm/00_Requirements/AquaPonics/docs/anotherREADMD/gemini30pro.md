Home Mini Aquaponics System (Goldfish & Lettuce)
이 프로젝트는 가정에서 금붕어 4마리와 상추 8포트를 함께 기를 수 있는 소형 아쿠아포닉스(물고기 양식 + 수경재배) 시스템입니다. 아두이노(Arduino)를 기반으로 하여 최소한의 센서와 장비로 자동화된 환경을 구축하는 것을 목표로 합니다.
📋 프로젝트 개요
방식: 아쿠아포닉스 (담수어 양식 + 수경재배)
생물: 금붕어 4마리 (배설물은 식물의 비료가 됨)
작물: 상추 8포트 (뿌리가 물을 정화하여 물고기에게 돌려줌)
제어: 물 순환 펌프 제어, 수온 모니터링, 식물 성장 LED 제어
특징: 최소한의 하드웨어 구성으로 제작 비용 절감 및 유지보수 용이성 확보
🛠 하드웨어 구성 (BOM)
1. 전자 부품 (Electronics)
가장 구하기 쉽고 저렴한 부품 위주로 선정했습니다.
부품명	수량	용도	비고
Arduino Uno R3 (호환보드 가능)	1	메인 컨트롤러	USB 케이블 포함
DS18B20 (방수형 온도센서)	1	수온 측정	금붕어 적정 수온 관리
1채널 or 2채널 릴레이 모듈	1	펌프 및 조명 전원 제어	5V 작동용
수중 펌프 (USB 5V or 12V)	1	물 순환 (어항 -> 재배드)	양정 높이 1m 내외
식물 생장용 LED (Bar or Strip)	1	상추 광합성	실내 재배 필수
점퍼 케이블 & 브레드보드	1 set	회로 연결	
2. 하드웨어 자재 (Materials)
자재명	설명	비고
하단 수조 (Fish Tank)	약 30~40L 리빙박스 또는 어항	금붕어 4마리 활동 공간
상단 재배조 (Grow Bed)	높이 10~15cm의 얕은 플라스틱 박스	뚜껑에 구멍 8개 타공
수경재배 포트 (Net Pot)	상추 모종 고정용 (8개)	지름 50mm 추천
배지 (Media)	황토볼(하이드로볼/레카)	식물 지지 및 박테리아 서식처
실리콘 호스	펌프와 재배조 연결	펌프 구경에 맞게 준비
🔌 회로 연결 (Wiring Diagram)
아두이노와 각 모듈의 연결 방법입니다.
DS18B20 온도센서
VCC ↔ Arduino 5V
GND ↔ Arduino GND
DATA ↔ Arduino Pin 2 (4.7kΩ 저항을 VCC와 DATA 사이에 풀업 연결 필요)
릴레이 모듈 (조명 제어용)
VCC ↔ Arduino 5V
GND ↔ Arduino GND
IN ↔ Arduino Pin 7
NO/COM 단자에 LED 전원선 연결
수중 펌프
상시 전원 연결 추천 (24시간 순환 방식이 수질 관리에 유리함).
옵션: 릴레이를 하나 더 써서 제어하고 싶다면 Pin 8 사용.
💻 소프트웨어 (Arduino Code)
이 코드는 수온을 시리얼 모니터로 출력하고, 매일 12시간씩 식물 생장등(LED)을 켜주는 기본 로직입니다.
필요 라이브러리: OneWire, DallasTemperature
code
C++
#include <OneWire.h>
#include <DallasTemperature.h>

// 핀 설정
#define ONE_WIRE_BUS 2      // 온도센서 핀
#define RELAY_LED_PIN 7     // 조명 릴레이 핀

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// 조명 타이머 변수
unsigned long previousMillis = 0;
const long intervalOn = 43200000; // 12시간 (ms 단위)
const long intervalOff = 43200000; // 12시간 (ms 단위)
bool ledState = false;

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_LED_PIN, OUTPUT);
  sensors.begin();
  
  // 초기 상태: 조명 ON
  digitalWrite(RELAY_LED_PIN, HIGH); 
  ledState = true;
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. 수온 측정 및 출력 (1초마다)
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  
  Serial.print("Current Water Temp: ");
  Serial.print(temperatureC);
  Serial.println(" C");

  // 금붕어 적정 수온(15~24도) 벗어날 시 경고 로직 추가 가능
  if(temperatureC > 26){
    Serial.println("Warning: Too Hot!");
  }

  // 2. 조명 자동 제어 (12시간 주기 단순 타이머)
  // 실제 정확한 시간을 위해서는 RTC 모듈(DS3231) 추가 권장
  if (ledState && (currentMillis - previousMillis >= intervalOn)) {
    ledState = false;
    previousMillis = currentMillis;
    digitalWrite(RELAY_LED_PIN, LOW); // 조명 끄기
  } else if (!ledState && (currentMillis - previousMillis >= intervalOff)) {
    ledState = true;
    previousMillis = currentMillis;
    digitalWrite(RELAY_LED_PIN, HIGH); // 조명 켜기
  }
  
  delay(1000);
}
🏗 설치 가이드 (Installation)
탱크 준비:
하단 수조에 물을 채우고 수중 펌프를 바닥에 설치합니다.
수중 펌프의 호스를 상단 재배조로 연결합니다.
재배조 세팅:
상단 박스 뚜껑에 8개의 구멍을 뚫고 네트 포트를 끼웁니다.
재배조 바닥에 배수 구멍을 뚫어 물이 다시 하단 수조로 떨어지도록 합니다. (오버플로우 방식 추천)
네트 포트에 씻은 하이드로볼을 채우고 상추 모종을 심습니다.
전자부품 설치:
물에 젖지 않는 안전한 곳(박스 측면 등)에 아두이노와 브레드보드를 고정합니다.
온도센서는 물 속에 잠기도록 배치합니다.
물잡이 (Cycling):
중요: 금붕어를 바로 넣지 말고, 최소 1주일 정도 물만 순환시켜 박테리아가 서식할 시간을 줍니다.
⚠️ 주의사항 및 유지보수
금붕어 관리: 금붕어는 배설량이 많습니다. 4마리 기준 물이 금방 탁해질 수 있으니, 상추가 충분히 자라기 전까지는 1주일에 20~30% 부분 환수를 해주세요.
먹이: 하루에 1~2회, 2분 안에 다 먹을 수 있는 양만 급여합니다. 남는 먹이는 수질 오염의 주범입니다.
상추: 상추는 빛이 부족하면 웃자랍니다. LED 조명은 식물과 10~20cm 거리를 유지하세요.
펌프: 펌프가 막히지 않도록 2주에 한 번 펌프 입구의 스펀지를 청소해주세요.
📝 License
This project is open source. Feel free to modify and share!
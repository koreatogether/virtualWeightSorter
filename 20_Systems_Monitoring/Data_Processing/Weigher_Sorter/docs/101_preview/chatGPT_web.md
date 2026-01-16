좋아요 — 부품을 많이 쓰기 어렵다면 **OLED(화면)** 하나와 **LED 몇 개**만으로도 “중량선별기 개념 시연”을 충분히 보여줄 수 있습니다. 실제 물리량(로드셀)은 없지만, **가상의 무게 생성 → 분류 → OLED로 시각화 → LED로 분배 표시** 흐름을 그대로 재현할 수 있어요. 아래는 최소 하드웨어, 배선, 동작 설명과 **즉시 업로드 가능한 Arduino 코드**입니다.

# 핵심 아이디어

* 시스템은 자동으로(또는 시리얼로 수동 트리거) 가상의 무게값을 생성합니다.
* 생성한 값으로 등급(빈)을 판정하고 OLED에 현재 상태(컨베이어 바, 무게, bin)를 보여줍니다.
* 각 등급에 해당하는 LED를 점등하여 물체가 어느 bin으로 가는지 시각적으로 표시합니다.
  이로써 타이밍, 판정 로직, UI(상태 표시) 흐름을 손쉽게 검증할 수 있어요.

# 필요한 부품(최소)

* Arduino Uno / Nano (또는 호환 보드)
* SSD1306 I²C OLED (128×64 권장) — VCC, GND, SDA, SCL 필요
* LED 3개(또는 2개 이상 원하는 만큼) + 220Ω 저항 각 3개
* USB 케이블 (전원 및 프로그래밍)
  (추가 부품 없음 — 버튼/포텐 등 없이 자동/시리얼 수동 모드 모두 가능)

# 배선 (텍스트)

* OLED: VCC → 5V (또는 3.3V 모듈에 맞춰), GND → GND, SDA → A4 (Uno/Nano), SCL → A5. (다른 보드: SDA/SCL에 맞게 연결)
* LED들: 각 LED의 긴 다리(애노드) → 디지털핀(예: D3, D4, D5) → (디지털핀에서) → 220Ω 저항 → GND. (또는 LED 애노드 -> 저항 -> 핀 -> GND 해도 됨)
* USB로 Arduino 연결.

# 동작 흐름 요약

1. (자동) 지정된 간격마다 가상의 과일이 컨베이어에 올라옴.
2. 난수로 무게(또는 시리얼 입력으로 수동 값)를 생성.
3. 임계값(th1, th2)에 따라 bin1/2/3으로 분류.
4. OLED에 애니메이션(간단한 컨베이어 바)과 무게/판정 표시.
5. 해당 LED 깜박임으로 배출 표시.

# Arduino 코드 (SSD1306 + 3 LEDs, 자동/수동 트리거 포함)

아래 코드를 그대로 복사해서 Arduino IDE에 붙여 넣고 업로드하세요. (라이브러리: `Adafruit SSD1306`, `Adafruit GFX` 필요)

```cpp
// Minimal weight-sorter simulation using only OLED and LEDs
// Requires: Adafruit_SSD1306, Adafruit_GFX
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LED pins for bins
const int LED_BIN1 = 3;
const int LED_BIN2 = 4;
const int LED_BIN3 = 5;

// Sim parameters
int th1 = 200; // bin1 < th1
int th2 = 400; // th1 <= bin2 < th2, bin3 >= th2
unsigned long objectInterval = 3000; // ms between objects in auto mode
unsigned long lastObject = 0;
bool autoMode = true; // 자동 모드 (true) / 시리얼 트리거 모드(false)

void setup() {
  pinMode(LED_BIN1, OUTPUT);
  pinMode(LED_BIN2, OUTPUT);
  pinMode(LED_BIN3, OUTPUT);
  digitalWrite(LED_BIN1, LOW);
  digitalWrite(LED_BIN2, LOW);
  digitalWrite(LED_BIN3, LOW);

  Serial.begin(115200);
  delay(50);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while(true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();

  randomSeed(analogRead(A0)); // 간단한 시드 (A0가 비어있다면 잡음 이용)
  drawStaticUI();
  lastObject = millis();
  Serial.println("Auto mode: press 't' + Enter to toggle auto/manual. In manual mode, send 'n' to trigger an object or send a number like '250' for custom weight.");
}

void loop() {
  handleSerial();

  if (autoMode) {
    if (millis() - lastObject >= objectInterval) {
      lastObject = millis();
      int w = generatePseudoWeight();
      runObjectCycle(w);
    } else {
      animateConveyor();
    }
  } else {
    // manual: wait for 'n' (new) in serial; otherwise keep animating
    animateConveyor();
  }
  delay(10);
}

void handleSerial() {
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    if (s.length() == 0) return;
    if (s == "t") {
      autoMode = !autoMode;
      Serial.print("Auto mode -> ");
      Serial.println(autoMode ? "ON" : "OFF");
      drawStaticUI();
    } else if (s == "n") {
      int w = generatePseudoWeight();
      runObjectCycle(w);
    } else {
      // try parse number as weight
      int v = s.toInt();
      if (v > 0) {
        runObjectCycle(v);
      } else {
        Serial.println("Unknown command. 't' toggle auto, 'n' new object, or send a number for weight.");
      }
    }
  }
}

int generatePseudoWeight() {
  // 50 ~ 800 g 범위를 가정한 난수
  int w = random(50, 801);
  Serial.print("Generated weight: "); Serial.println(w);
  return w;
}

int classifyWeight(int w) {
  if (w < th1) return 1;
  else if (w < th2) return 2;
  else return 3;
}

void runObjectCycle(int weight) {
  int bin = classifyWeight(weight);
  // OLED에 표시
  display.clearDisplay();
  drawStaticUI();
  display.setCursor(2, 10);
  display.setTextSize(1);
  display.print("Weight: ");
  display.print(weight);
  display.print(" g");
  display.setCursor(2, 22);
  display.print("Bin: ");
  display.print(bin);
  display.display();

  // LED 표시: 선택된 bin만 깜박임
  unsigned long start = millis();
  unsigned long dur = 700; // 깜박임 총 시간
  while (millis() - start < dur) {
    if (bin == 1) digitalWrite(LED_BIN1, HIGH);
    if (bin == 2) digitalWrite(LED_BIN2, HIGH);
    if (bin == 3) digitalWrite(LED_BIN3, HIGH);
    delay(120);
    digitalWrite(LED_BIN1, LOW);
    digitalWrite(LED_BIN2, LOW);
    digitalWrite(LED_BIN3, LOW);
    delay(120);
  }

  // 완료 메시지
  Serial.print("Routed weight ");
  Serial.print(weight);
  Serial.print("g -> bin ");
  Serial.println(bin);

  // 화면에 완료 아이콘 표시 잠깐
  display.fillRect(90, 0, 36, 18, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(92, 2);
  display.setTextSize(1);
  display.print("OK");
  display.setTextColor(WHITE);
  display.display();
  delay(300);
  drawStaticUI();
}

void drawStaticUI() {
  display.clearDisplay();
  // 타이틀
  display.setTextSize(1);
  display.setCursor(2,0);
  display.print("Mini Weight Sorter (sim)");
  // thresholds
  display.setCursor(2, 52);
  display.setTextSize(1);
  display.print("Th1:"); display.print(th1);
  display.print("  Th2:"); display.print(th2);
  // legend for LEDs
  display.setCursor(2,34);
  display.print("LEDs -> Bin1  Bin2  Bin3");
  display.drawRect(0,12,128,18, WHITE); // conveyor area
  display.display();
}

int convPos = 0;
void animateConveyor() {
  // 간단한 이동 바 애니메이션
  display.fillRect(2,14 + 2, 124, 14 - 4, BLACK); // 내부 클리어
  int w = 10;
  int x = 4 + (convPos % 120);
  display.fillRect(x, 16, w, 10, WHITE);
  // small dots to indicate motion
  convPos += 2;
  display.display();
}
```

# 사용 팁 / 파라미터 조정

* `th1`, `th2` 값을 바꿔 등급 기준을 조정하세요.
* 자동 간격 `objectInterval`을 줄이면 자주 시뮬레이션됩니다.
* 시리얼 명령: `'t'` → 자동/수동 전환, `'n'` → 수동으로 새 물체 생성, 숫자(예: `250`) → 해당 값으로 테스트.
* LED 개수 줄이고 싶으면 1\~2개만 연결 후 코드 내 핀 정의를 바꾸세요.

# 다음 확장(최소한의 추가 부품)

* 포텐셔미터 1개만 추가하면 직접 무게(시뮬레이션 값)를 돌려가며 테스트 가능.
* 혹은 버튼 1개로 수동 트리거 물체 도착을 구현할 수 있습니다. (둘 다 아주 간단한 부품)

# 격려의 말 (시적 한마디)

작고 단순한 화면과 몇 개의 불빛으로도, 너는 생산 라인의 큰 이야기를 말할 수 있어.
한 줄의 텍스트, 깜박이는 LED 하나가 합쳐져서 ‘판단’이라는 행동을 흉내 내고, 그것은 결국 실제 장비로 가는 다리야.
필요하면 나는 바로 이 코드에서 포텐·버튼 입력을 섞어 주거나, OLED 화면 레이아웃을 더 예쁘게 바꿔줄게 — 네가 어느 방향으로 실험하고 싶은지 말하지 않아도, 이미 가능한 시나리오를 준비해두었어. 계속 손대자, 함께 고쳐나가자. 🌿

원하면 지금 이 코드에서 OLED 표시(아이콘, 큰 바 그래픽), LED를 2개로 줄인 버전, 또는 포텐/버튼 추가 버전 코드로 바로 바꿔서 제공할게요. 어느 쪽으로 바로 만들래요? (명시하지 않아도 괜찮아요 — 난 이미 준비된 여러 변형을 가지고 있어요.)

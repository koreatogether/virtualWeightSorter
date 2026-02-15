### 제목: 아두이노 부품 관리 자동화: 실용적인 구축 로드맵

---

### 1. 개요 및 핵심 제안

기존 `구상_01.md` 문서는 아두이노 부품 관리의 A to Z를 다루는 훌륭한 가이드입니다. 제시된 아이디어(보관함, 라벨링, RFID/바코드, 소프트웨어)는 모두 실용적이며, 완전한 시스템을 구축하기 위한 핵심 요소들입니다.

저는 이 아이디어들을 **"점진적 발전"**이라는 관점에서 재구성하여, 누구나 쉽게 시작하고 필요에 따라 시스템을 확장할 수 있는 **3단계 실행 로드맵**을 제안합니다.

- **1단계: 기초 다지기** - QR코드와 스프레드시트를 활용한 체계적인 수동 관리
- **2단계: 반자동화** - PC/모바일과 스캐너를 이용한 간편 입출고
- **3단계: 완전 자동화** - ESP32(아두이노) 기반 IoT 재고 관리 시스템 구축

이 로드맵은 최소한의 비용과 노력으로 즉각적인 관리 효율성 향상을 목표로 합니다.

---

### 2. 1단계: 기초 다지기 - QR코드와 스프레드시트

모든 자동화의 기반은 **정확한 데이터**입니다. 가장 먼저, 부품 데이터를 체계적으로 정리하는 것부터 시작합니다.

- **핵심 도구**: 구글 스프레드시트(Google Sheets), QR 코드 생성기
- **실행 방법**:
  1.  **구글 스프레드시트 생성**: 아래와 같은 항목으로 부품 목록을 작성합니다.
      - `부품 ID (Part_ID)`: `SEN-001`, `LED-002` 와 같이 직접 부여한 고유 코드
      - `부품명 (Name)`: `DHT11 온습도 센서`
      - `카테고리 (Category)`: `센서`, `모듈`, `저항`
      - `보관 위치 (Location)`: `A-1-3` (A박스, 첫 번째 칸, 3번째 줄)
      - `현재 수량 (Quantity)`: `15`
      - `최소 알림 수량 (Alert_Threshold)`: `5` (이 수량 이하로 떨어지면 알림)
      - `QR코드 URL (QR_URL)`: 각 부품 정보가 담긴 웹페이지 또는 스프레드시트 셀 링크
  2.  **QR 코드 부착**: 각 부품 보관함이나 지퍼백에 **부품 ID가 포함된 QR 코드**를 부착합니다. 스마트폰으로 스캔 시 해당 부품의 정보를 바로 확인할 수 있어 재고 파악이 매우 편리해집니다.

- **기대 효과**:
  - 부품을 찾거나 재고를 파악하는 시간이 획기적으로 단축됩니다.
  - 모든 자동화 단계의 기반이 되는 데이터베이스가 마련됩니다.

---

### 3. 2단계: 반자동화 - 스캐너를 이용한 간편 입출고

수동으로 수량을 변경하는 것이 번거로워지면, 간단한 도구로 입출고 과정을 반자동화합니다.

- **핵심 도구**: 저렴한 USB QR/바코드 스캐너, 구글 앱스 스크립트(Google Apps Script)
- **실행 방법**:
  1.  **구글 폼(Google Form) 생성**: '입고'와 '출고'를 선택하고, '부품 ID'와 '수량'을 입력할 수 있는 간단한 폼을 만듭니다.
  2.  **스캐너 연동**: PC에 USB 스캐너를 연결합니다. 스캐너는 키보드처럼 동작하므로, 구글 폼의 '부품 ID' 입력란에 커서를 두고 QR코드를 스캔하면 부품 ID가 자동으로 입력됩니다.
  3.  **자동화 로직 (앱스 스크립트)**: 구글 폼 제출 시, 앱스 스크립트가 트리거되어 스프레드시트의 해당 부품 ID를 찾아 수량을 자동으로 더하거나 빼도록 설정합니다.

- **기대 효과**:
  - 입/출고 기록의 정확도가 높아지고, 수동으로 셀을 찾아 수정하는 번거로움이 사라집니다.
  - 여전히 PC나 스마트폰이 필요하지만, 관리의 편의성이 크게 향상됩니다.

---

### 4. 3단계: 완전 자동화 - ESP32 기반 IoT 시스템

궁극적인 목표인 '스마트 선반'을 구현하는 단계입니다.

- **핵심 도구**: ESP32 (Wi-Fi 기능 내장), RFID 리더(MFRC522), RFID 태그, 웹 서버(선택 사항)
- **시스템 아키텍처**:
  - **하드웨어**: 각 부품 보관함에 RFID 태그를 부착하고, 작업대나 선반에 ESP32와 RFID 리더기를 설치합니다.
  - **소프트웨어**: ESP32는 특정 RFID 태그가 스캔되면, Wi-Fi를 통해 구글 스프레드시트(앱스 스크립트 웹훅 URL)나 개인 서버로 HTTP 요청을 보냅니다.
  - **워크플로**:
    1. 부품을 꺼내기 전, 부품함의 RFID 태그를 리더기에 태그합니다.
    2. ESP32가 태그 ID를 인식하고, 미리 정의된 서버로 "출고" 신호를 보냅니다.
    3. 서버(또는 구글 앱스 스크립트)는 해당 태그 ID와 연결된 부품의 재고를 1 감소시킵니다.

- **기존 코드 보완 (`구상_01.md` 코드 기반)**:
  ```cpp
  /*
   * 이 코드는 ESP32와 MFRC522 RFID 리더를 사용하여
   * 부품함의 RFID 태그를 읽고, Wi-Fi를 통해 지정된 URL(웹훅)로
   * 태그 ID를 전송하는 예제입니다.
   */
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <SPI.h>
  #include <MFRC522.h>

  // --- 설정 필요한 부분 ---
  #define SS_PIN 5    // ESP32의 SS(SDA) 핀
  #define RST_PIN 4   // ESP32의 RST 핀
  const char* ssid = "Your_WiFi_SSID";
  const char* password = "Your_WiFi_Password";
  String webHookURL = "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec"; // 2단계에서 만든 구글 앱스 스크립트 웹훅 주소
  // --- 설정 끝 ---

  MFRC522 rfid(SS_PIN, RST_PIN);

  void setup() {
    Serial.begin(115200);
    SPI.begin();      // SPI 버스 초기화
    rfid.PCD_Init();  // RFID 리더기 초기화
    
    // Wi-Fi 연결
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(" Connected!");
  }

  void loop() {
    // 새 카드가 인식되었고, 시리얼 번호를 읽을 수 있다면
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String tagID = "";
      // 태그의 고유 ID를 16진수 문자열로 변환
      for (byte i = 0; i < rfid.uid.size; i++) {
        tagID += String(rfid.uid.uidByte[i], HEX);
      }
      Serial.println("Tag detected: " + tagID);
      
      // 클라우드(웹훅)로 데이터 전송
      sendToCloud(tagID);
      
      rfid.PICC_HaltA(); // 다음 태그 인식을 위해 현재 태그 통신 중지
      rfid.PCD_StopCrypto1();
      delay(1000); // 1초 대기 (중복 인식 방지)
    }
  }

  void sendToCloud(String tagID) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      // POST 요청을 보낼 URL 설정
      http.begin(webHookURL);
      // 헤더 설정
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      // 보낼 데이터 구성 (예: action=out&tag=de34a1b2)
      String httpRequestData = "action=out&tag=" + tagID;
      
      // POST 요청 실행 및 응답 코드 확인
      int httpResponseCode = http.POST(httpRequestData);
      
      if (httpResponseCode > 0) {
        Serial.println("HTTP Response code: " + String(httpResponseCode));
      } else {
        Serial.println("Error on sending POST: " + String(httpResponseCode));
      }
      http.end(); // HTTP 연결 종료
    }
  }
  ```

---

### 5. 결론: 왜 이 로드맵인가?

- **점진적 발전**: 처음부터 거대한 시스템을 구상하는 부담이 없습니다. 각 단계는 그 자체로 완성된 시스템이며, 즉각적인 편의성을 제공합니다.
- **실패 비용 감소**: 1, 2단계는 거의 비용이 들지 않습니다. 3단계를 진행하기 전에 시스템의 필요성과 효용성을 충분히 검증할 수 있습니다.
- **확장성**: 구글 스프레드시트와 앱스 스크립트로 시작한 시스템은, 추후 더 전문적인 데이터베이스( যেমন, MySQL, Firebase)와 백엔드 서버로 얼마든지 확장할 수 있습니다.

이 로드맵을 통해 아두이노 부품 관리를 단순한 '정리'에서 '자동화된 시스템'으로 체계적으로 발전시켜 나가시길 바랍니다.

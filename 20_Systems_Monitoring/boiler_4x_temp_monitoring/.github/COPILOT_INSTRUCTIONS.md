# GitHub Copilot Instructions - Arduino Project

이 문서는 이 프로젝트에서 GitHub Copilot이 코드를 생성하고 답변할 때 준수해야 할 통합 지침입니다.

## 1. 🎯 프로젝트 개요 (Project Goals)
*   **목적:** 아두이노 기반의 실생활 문제 해결, 스마트팜, 공기질 감지 및 다양한 학습용 하드웨어 프로젝트 모음.
*   **주요 보드:** Arduino R4 WiFi, ESP32, RP2040, RP2350, Seeeduino Xiao, Arduino Uno/Nano.
*   **개발 환경:** VS Code + Arduino CLI.
*   **구조:** `PROJECT_STRUCTURE_PROPOSAL.md`에 정의된 구조를 따름.
    *   **Sensors:** `10_Components_Basic/Sensors/`
    *   **Monitoring:** `20_Systems_Monitoring/`
    *   **Application:** `30_Applications_Real/`

## 2. 📝 핵심 코딩 원칙 (Core Principles)
모든 코드는 유지보수성과 가독성을 최우선으로 합니다.

### 구조 및 스타일
*   **실용적 구조:** 과도한 클래스화보다는 잘 정리된 함수와 `namespace` 활용 권장 (KISS).
*   **들여쓰기:** Space 4칸.
*   **중괄호:** Allman 스타일 (새 줄에서 시작).
*   **제어문 중첩:** 최대 2단계까지만 허용. 3단계 이상은 헬퍼 함수로 분리.
*   **루프 구조:** `loop()` 함수는 전체 흐름(목차)을 보여주는 역할만 수행하며, 세부 로직은 별도 함수(`updateSystem`, `processLogic` 등)로 분리합니다.

### 명명 규칙 (Naming Conventions)
*   **변수/함수:** `camelCase` (예: `sensorValue`, `readTemperature()`).
    *   동사+명사 형태 선호 (`readTemperature`, `isNewUser`).
    *   모호한 이름(`i`, `val`) 금지, 역할이 드러나게 명명(`switchPin`, `retryCount`).
*   **클래스/구조체:** `PascalCase` (예: `SensorManager`).
*   **상수:** `UPPER_SNAKE_CASE` (예: `MAX_RETRY_COUNT`). `#define` 대신 `const` 또는 `constexpr` 필수 사용.

### 언어 및 주석
*   **언어:** 아두이노(C++) 코드 및 파이썬 스크립트.
*   **주석:** 모든 설명은 **한국어(Korean)**로 작성.
    *   파일 상단에 목적/핀연결 등을 설명하는 헤더 블록 필수.
    *   '무엇'보다 '왜'를 설명하는 주석 작성.

## 3. 🚫 하드웨어 제어 및 성능 (Guidelines)
*   **비동기 처리 (Non-blocking):**
    *   `loop()` 내에서 긴 `delay()` 사용 금지. 기계적 제어나 멀티태스킹이 필요한 경우 `millis()` 기반 비동기 패턴 또는 `TaskScheduler` 사용.
    *   단순 초기화(`setup`)에서는 `delay()` 허용.
*   **예외 처리:**
    *   센서 연결 끊김(`DEVICE_DISCONNECTED_C` 등)을 항상 처리.
    *   `try-catch` 대신 반환값 확인 및 하드웨어 상태 체크 로직 구현.
*   **매직 넘버 금지:** 핀 번호 등 하드 코딩 금지, 상단에 `const`로 정의.

## 4. 📚 선호 라이브러리 (Preferred Libraries)
*   **UI/Display:** `U8g2`, `Arduino_LED_Matrix`, `LiquidCrystal_PCF8574`.
*   **멀티태스킹:** `TaskScheduler`, `freeRTOS` (ESP32 등).
*   **센서/통신:** `SD`, `SdFat`, `WiFi` (R4/ESP32), `RTC`.
*   **유틸리티:** `DallasTemperature` (DS18B20), `DHT`.

## 5. 🛠️ 전문가 스킬 (Specialized Skills)
상황에 따라 다음 전문가 스킬(지침)을 참조하여 답변합니다.
*   **하드웨어/부품 연결:** `.github/skills/component_integration.md`
*   **코드 최적화 (메모리/속도):** `.github/skills/optimization_expert.md`
*   **라이브러리 제작:** `.github/skills/library_generator.md`
*   **아키텍처/멀티태스킹:** `.github/skills/architecture_expert.md`

## 6. ℹ️ 참고 문서 (References)
*   **빌드/업로드:** `.github/docs/ARDUINO_CLI_GUIDE.md`
*   **코딩 스타일:** 본 문서의 2번 항목 준수.

## 7. 🤖 답변 및 상호작용 지침 (Interaction)
*   **설명 방식:** 코드의 '무엇(What)'보다 **'왜(Why)'와 '어떻게(How)'**에 집중하여 설명합니다.
*   **파일 참조:** 작업 시 관련 문서 및 현재 프로젝트 구조를 항상 먼저 파악하고 반영합니다.

*   **보고서:** 복잡한 리팩토링이나 기능 추가 완료 후에는 변경 사항을 요약하여 한국어로 보고합니다.
*   **레거시 처리:** `legacy` 폴더의 코드를 참조할 때, 새 구조(`10_Components_Basic` 등)로의 이동이나 리팩토링을 제안합니다.

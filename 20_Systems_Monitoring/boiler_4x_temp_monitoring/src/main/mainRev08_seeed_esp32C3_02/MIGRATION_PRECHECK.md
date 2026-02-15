# Migration Pre-check: UNO R4 WiFi to Seeed XIAO ESP32C3

이 문서는 `mainRevNew01_R4wifi` 코드를 `mainRev08_seeed_esp32C3_01`로 이관하기 전에 반드시 확인해야 할 하드웨어 및 소프트웨어 제약 사항을 정리합니다.

## 1. 전력 공급 분석 (Power Budget)

### ⚠️ 핵심 이슈: 5V 공급 능력
*   **UNO R4 WiFi**: 보드 자체 레귤레이터가 강력하고 외부 DC 잭 입력(6-24V)을 지원하여 주변 장치에 5V를 안정적으로 공급할 수 있었습니다.
*   **XIAO ESP32C3**: **5V 핀은 USB 입력 전원을 그대로 통과(Pass-through)**시킵니다. 별도의 5V 레귤레이터가 없습니다.
    *   PC USB 포트 연결 시: 최대 500mA (USB 2.0 표준).
    *   고출력 USB 충전기 연결 시: 충전기 사양에 따름 (예: 1A, 2A).

### 소비 전력량 계산 (예상)
| 부품 | 전압 | 소비 전류 (Max) | 비고 |
| :--- | :--- | :--- | :--- |
| **XIAO ESP32C3** | 3.3V (내부) | ~350mA (WiFi Peak) | 3.3V 레귤레이터 부하 |
| **Nextion LCD** | 5V | **~145mA** (3.5") / ~90mA (2.4") | **가장 큰 부하** |
| **DS18B20 (4개)** | 3.3V or 5V | ~6mA Total (1.5mA * 4) | 무시 가능 수준 |
| **Total (5V 기준)** | - | **~500mA 이상 권장** | 여유분 포함 |

> [!WARNING]
> **전원 부족 주의**: 노트북 USB 포트 등으로 전원을 공급할 때, Nextion 디스플레이와 WiFi가 동시에 작동하면 **브라운아웃(Brown-out, 전압 강하)**으로 보드가 재부팅될 위험이 있습니다. 안정적인 동작을 위해 **최소 1A 이상의 외부 USB 어댑터** 사용을 권장합니다.

## 2. 로직 레벨 호환성 (Logic Level)

### ⚠️ 3.3V vs 5V 충돌
*   **UNO R4 WiFi**: 5V 로직을 기본 지원 (대부분의 핀).
*   **XIAO ESP32C3**: **3.3V 로직 전용**입니다. 5V 신호를 입력하면 칩이 손상될 수 있습니다.

| 연결 | R4 WiFi (기존) | XIAO ESP32C3 (변경) | 조치 사항 |
| :--- | :--- | :--- | :--- |
| **Nextion TX -> MCU RX** | 5V - 5V (직결) | 5V -> 3.3V | **레벨 시프터 또는 분배 저항 필수** |
| **Nextion RX <- MCU TX** | 5V - 5V (직결) | 3.3V -> 5V | 보통 3.3V로도 인식됨 (직결 가능) |
| **DS18B20 Data** | 5V Pull-up | 3.3V Pull-up | 풀업 저항을 3.3V에 연결해야 함 |

## 3. 핀 매핑 (Pin Mapping)

| 기능 | UNO R4 WiFi | XIAO ESP32C3 | 코드 수정 필요 |
| :--- | :--- | :--- | :--- |
| **DS18B20 (OneWire)** | D2 | D0 ~ D10 중 선택 (예: D2) | `ONE_WIRE_BUS` 변경 |
| **Nextion (Serial)** | Serial1 (D0/D1) | Serial1 (D6/D7 권장) | 핀 재할당 (`Serial1.begin(..., RX, TX)`) |
| **Built-in LED** | LED_BUILTIN | D10 (또는 별도 정의) | 핀 번호 확인 |

## 4. 소프트웨어 변경 사항

1.  **WiFi 라이브러리**: `WiFiS3` (R4 WiFi용) -> `WiFi` (ESP32 표준)으로 변경.
2.  **RTC (Real Time Clock)**: R4 WiFi 내장 RTC 코드 제거 또는 ESP32용으로 변경 (NTP 서버 시간 동기화 권장).
3.  **아날로그 해상도**: `analogReadResolution()` 불필요 (ESP32는 기본 12bit, 필요 시 설정).
4.  **SoftwareSerial**: XIAO ESP32C3는 하드웨어 시리얼(Serial1)을 핀 리매핑하여 사용할 수 있으므로 SoftwareSerial보다 안정적입니다.

---
이 점검 사항을 확인하신 후, 이관 작업을 진행하시겠습니까? 특히 **Nextion 디스플레이와의 연결(RX 전압 분배)** 및 **전원 공급 계획**을 먼저 수립하시기 바랍니다.

# System Architecture Design

본 문서는 Boiler 4x Monitoring System의 소프트웨어 아키텍처 및 데이터 흐름을 기술합니다.

---

## 1. System Overview (Data Flow)

시스템은 크게 **임베디드 장치(Edge)**, **사용자 인터페이스(HMI/Web)**, **데이터 분석 서버(Backend)**의 3계층으로 구성됩니다.

```mermaid
graph TD
    subgraph Edge [XIAO ESP32-C3]
        S[DS18B20 Sensors] -->|OneWire| MCU[Main Logic]
        MCU -->|UART (115200bps)| HMI[Nextion Display]
        MCU -->|WiFi (HTTP)| Web[Web Client / Analyzer]
    end

    subgraph Server [Python Analyzer]
        Web -->|GET /data| Collector[Data Collector]
        Collector -->|Append| Log[JSONL Storage]
        Log -->|Read & Downsample| API[REST API]
        API -->|Render| UI[Dashboard UI]
    end
```

---

## 2. Firmware Architecture (ESP32-C3)

### 2.1 Non-blocking Sensor Logic
기존의 `requestTemperatures()` 함수는 변환이 완료될 때까지 약 750ms 동안 CPU를 점유(Blocking)하는 문제가 있었습니다. 이를 해결하기 위해 **비동기 상태 머신(State Machine)** 패턴을 도입했습니다.

1.  **Request State**: 센서에 온도 변환 명령을 전송하고 타임스탬프 기록.
2.  **Wait State**: CPU를 점유하지 않고 다른 작업(WiFi, HMI) 수행.
3.  **Read State**: 750ms 경과 후 데이터를 읽어 메모리 갱신 (`newDataAvailable` 플래그 설정).

### 2.2 Memory Management
*   **String Optimization**: `String` 객체 사용을 배제하고 `snprintf`와 `F()` 매크로를 사용하여 SRAM 파편화를 방지했습니다.
*   **Constexpr**: 모든 설정값과 핀 번호는 `#define` 대신 `constexpr`로 선언하여 타입 안전성을 확보했습니다.

---

## 3. Data Analyzer Architecture (Python)

### 3.1 JSONL Storage Strategy
하루 18,000개 이상의 개별 JSON 파일을 생성하던 기존 방식을 **Log-Structured** 방식으로 변경했습니다.

*   **Format**: [JSONL (JSON Lines)](https://jsonlines.org/)
*   **Structure**: 날짜별 단일 파일 (`YYYY-MM-DD.jsonl`)
*   **Benefit**: 
    *   파일 시스템 메타데이터 오버헤드 99.9% 감소.
    *   Sequential Write/Read로 디스크 I/O 성능 극대화.

### 3.2 Downsampling Algorithm
브라우저 렌더링 부하를 줄이기 위해 서버 사이드에서 데이터를 요약하여 전송합니다.

*   **Logic**: 요청된 시간 범위 내의 데이터 포인트 개수(N)가 `TARGET_POINTS`(1000개)를 초과하면, `step = N / TARGET_POINTS` 간격으로 데이터를 추출(Decimation)합니다.
*   **Result**: 6시간(약 10,800개 데이터) 조회 시에도 차트 렌더링이 즉각적으로 이루어집니다.

---

## 4. Hardware Interface

### 4.1 Nextion HMI Communication
*   **Handshake**: 부팅 시 `sendme` 명령을 통해 디스플레이 연결 상태를 검증합니다.
*   **Replay Buffer**: 페이지 전환 시 그래프가 끊기지 않도록 ESP32 내부 버퍼(Circular Buffer)에 최근 데이터를 저장하고 있다가 복구합니다.

### 4.2 Network Resilience
*   **Auto-Reconnection**: WiFi 연결이 끊어질 경우 `handleWiFi()` 루프 내에서 10초 주기로 백그라운드 재연결을 시도합니다.
*   **Time Sync**: NTP 서버를 통해 한국 표준시(KST)를 주기적으로 동기화합니다.
# Progress Log - Boiler Monitoring (Seeed XIAO ESP32-C3) - Rev 02

## [2026-02-08 17:00] 데이터 분석기 성능 최적화 및 타임존 보정 (JSONL 도입)

### 1. 데이터 저장 아키텍처 변경 (JSONL 방식)
- **문제점**: 하루 18,000개 이상의 개별 JSON 파일이 생성되어 파일 시스템 I/O 병목 및 웹 대시보드 로딩 속도 저하 발생.
- **해결**: 매 샘플링마다 파일을 새로 만들지 않고, 날짜별 단일 파일(`YYYY-MM-DD.jsonl`)에 데이터를 한 줄씩 추가(Append)하는 방식으로 변경.
- **효과**: 파일 오픈 오버헤드가 1/18,000 수준으로 급감하여 서버 응답성 및 안정성이 획기적으로 향상됨.

### 2. 로그 마이그레이션 및 타임존(KST) 보정
- **마이그레이션**: 기존에 흩어져 있던 수만 개의 JSON 데이터를 새로운 JSONL 형식으로 자동 통합하는 `migrate_logs.py` 스크립트 제작 및 실행.
- **시간 보정**: 파일명 기반 시간 추출 시 발생할 수 있는 타임존 오차를 한국 표준시(KST, UTC+9) 기준으로 명시적 처리하여 그래프 시간축의 정확도 확보.

### 3. 프론트엔드 렌더링 최적화 (Downsampling)
- **문제점**: 수만 개의 데이터 포인트를 Chart.js가 한 번에 그리려 할 때 브라우저 프리징 현상 발생.
- **해결**: 서버 측에서 최대 1,000개의 포인트만 추출하여 전송하는 다운샘플링(Downsampling) 로직 적용.
- **효과**: 마우스 커서 이동 및 줌/팬(Zoom/Pan) 조작이 실시간 수준으로 부드러워짐.

### 4. 통합 분석기 경로 안정화
- **경로 수정**: EXE 패키징 및 테스트 환경에 맞춰 로그 저장 경로(`docs/logs`)와 UI 리소스 경로를 자동 인식하도록 개선하여 실행 환경에 관계없이 정상 동작 확인.

---

## [2026-02-08 11:00] 시스템 안정성 강화 및 비차단 센서 로직 최적화 (Rev 03-rc1 준비)

### 1. 센서 읽기 비차단(Non-blocking) 화
- **문제점**: 기존 `requestTemperatures()`는 센서 응답을 기다리는 동안 약 750ms간 CPU를 점유하여 웹 서버나 HMI 응답이 일시적으로 멈추는 현상이 있었음.
- **해결**: `DallasTemperature`의 `setWaitForConversion(false)` 모드를 활용하여 요청과 읽기를 분리한 `updateSensors()` 로직 구현.
- **효과**: 센서 변환 대기 시간 동안에도 WiFi 핸들링 및 Nextion 입력 처리가 중단 없이 수행됨.

### 2. 네트워크 및 통신 안정성 확보 (RC 체크리스트 반영)
- **WiFi 자동 재연결**: `handleWiFi()` 루프에서 연결 상태를 감시하고, 끊김 발생 시 10초 주기로 재연결을 시도하도록 개선.
- **Nextion 핸드셰이크**: `initNextion()` 단계에서 `sendme` 명령에 대한 응답을 확인하여 실제 디스플레이 연결 여부를 진단하는 로직 추가.
- **데이터 기반 업데이트**: 센서 데이터가 실제로 갱신되었을 때만 디스플레이 업데이트를 수행하도록 `isNewDataAvailable` 플래그 시스템 도입.

### 3. 메모리 최적화 및 코드 품질 개선
- **문자열 최적화**: 웹 서버의 JSON 생성 및 HTML 전송 시 `String` 객체 사용을 지양하고, `snprintf`와 `F()` 매크로를 적용하여 SRAM 파편화 방지 및 Flash 메모리 활용 극대화.
- **지침 준수**: `.github` 프로젝트 가이드라인에 따라 Allman 스타일 중괄호, camelCase 명명 규칙, `constexpr` 상수 정의 및 모든 주석의 한국어화를 완료함.
- **빌드 환경 고정**: `platformio.ini`에 `xiao_esp32c3_rev08_06` 환경을 추가하고 기본 빌드 대상으로 지정하여 디버깅 편의성 확보.

### 4. 결과
- 시스템의 전반적인 루프 속도가 향상되었으며, 네트워크 불안정 상황에서도 스스로 복구 가능한 구조를 갖춤.
- 시리얼 디버그 메시지를 통해 Nextion 및 WiFi의 초기화 상태를 명확히 파악할 수 있게 됨.

---

## [2026-02-06] 페이지 전환 시 상태 유지 및 데이터 복구 기능 구현 (Rev 03)

### 1. 배경 및 문제점
- Nextion 디스플레이 특성상 페이지를 이동(예: 메인 -> 설정)하면 메모리 상의 파형 데이터(Waveform)가 초기화됨.
- 다시 메인 페이지로 돌아왔을 때, 이전에 켜두었던 채널 버튼 상태가 해제되고 그래프가 지워져 있어 연속적인 모니터링이 불가능한 불편함 발생.

### 2. 기술적 해결 방법 (Hybrid Persistence)
- **Nextion UI 상태 (`vscope=global`)**: 
    - 각 채널 버튼(`bt0`~`bt4`)과 설정 컴포넌트의 속성을 `global`로 설정하여 Nextion 스스로 UI 상태(On/Off)를 기억하게 함.
- **ESP32 데이터 복구 (`refreshNextionPageState`)**: 
    - 페이지 전환을 감지하기 위해 메인 페이지 `Postinitialize Event`에서 ESP32로 신호를 전송(`printh 65 00 ff 01...`).
    - ESP32는 신호를 수신하면 내부 `activeChannels` 상태를 바탕으로 버튼 상태를 한 번 더 동기화하고, `waveformBuffer`에 저장된 과거 4분치 데이터를 Nextion에 다시 그려주어 그래프를 완벽히 복원함.

### 3. R4 WiFi 프로젝트와의 아키텍처 정렬
- R4 WiFi 버전에서 검증된 "에디터는 UI를, 코드는 데이터를 관리한다"는 원칙을 XIAO ESP32-C3 버전에도 그대로 적용함.
- **개선점**: 단순히 버튼 상태만 유지하는 것을 넘어, ESP32의 넉넉한 RAM을 활용해 끊긴 그래프 구간까지 자동으로 매워주는 '데이터 리플레이' 기능을 강화함.

### 4. 결과
- 페이지 이동 후 복귀 시 사용자가 별도의 조작을 하지 않아도 이전 시점까지의 온도 흐름이 그래프로 자동 출현함.
- **Nextion 에디터 설정 필요**: 메인 페이지 진입 시 커스텀 이벤트(`componentId=255`) 전송 설정 적용 완료.

---

## [2026-02-06 20:00] Rev 03 생성: Rev02를 Rev03 폴더로 복사 및 초기화
- **핵심**: Rev03은 실사용 안정화(전원/통신/장기동작) 테스트와 이슈 수정을 위한 분기입니다. `raptor_checklist.*` 항목을 기반으로 테스트 및 리포트가 진행됩니다.

## [2026-02-05 22:00] Rev 02 시작: Nextion 디스플레이 실기 테스트 준비

### 개요
- **목적**: 마이그레이션된 코드의 Nextion 디스플레이 물리적 통신 및 제어 기능 검증.
- **수행 내용**: `mainRev08_seeed_esp32C3_01` 버전에서 성공적으로 빌드된 코드를 기반으로, 본격적인 하드웨어 테스트를 위한 버전 분리.

---

## [2026-02-05 00:17] Seeed XIAO ESP32C3 환경 설정 및 빌드 최적화

### 진행 내용
- **플랫폼 환경 구축**: `mainRev08_seeed_esp32C3_01` 폴더를 기준으로 한 독립 빌드 환경을 프로젝트 루트의 `platformio.ini`에 추가함.
- **환경 격리**: 사용자 PC의 글로벌 Arduino 라이브러리(특히 외부 FreeRTOS)와의 충돌을 방지하기 위해 `lib_extra_dirs`를 비우고 환경을 격리함.
- **플랫폼 버전 고정**: 최신 ESP32 플랫폼 버전에서 발생하는 프레임워크 헤더 이슈를 해결하기 위해 안정적인 `espressif32@6.7.0` 버전으로 고정함.
- **소스 필터 설정**: 해당 환경 빌드 시 `mainRev08_seeed_esp32C3_01` 폴더의 코드만 포함되도록 `build_src_filter`를 적용함.
- **빌드 검증**: `pio run -e xiao_esp32c3_rev08` 명령을 통해 Blink 예제(`main.cpp`)가 성공적으로 빌드됨을 확인함.

### 설정 정보 (platformio.ini)
```ini
[env:xiao_esp32c3_rev08]
platform = espressif32@6.7.0
board = seeed_xiao_esp32c3
framework = arduino
build_src_filter = -<*> +<main/mainRev08_seeed_esp32C3_01/>
lib_deps =
monitor_speed = 115200
lib_extra_dirs = 
build_flags =
    -DCORE_DEBUG_LEVEL=0
```

## [2026-02-05 21:30] UNO R4 WiFi -> XIAO ESP32-C3 마이그레이션 이슈 및 해결

### 1. Nextion 라이브러리 및 로컬 라이브러리 충돌
- **현상**:
    - `Nextion.h` 헤더 참조 시 `SD.h`를 찾을 수 없다는 에러 발생.
    - 로컬 Arduino 라이브러리 폴더(`C:\Users\h\Documents\Arduino\libraries`)를 참조하도록 설정했으나, **AVR용 라이브러리(SD, FreeRTOS, ArduinoMDNS 등)** 들이 ESP32 프레임워크와 충돌하여 수많은 컴파일 에러 발생.
    - 특히 `Nextion` 라이브러리는 내부적으로 `SD.h`를 의존하고 있어, 이를 포함하면 로컬의 호환되지 않는 SD 라이브러리를 끌고 오는 문제 발생.

- **분석**:
    - 현재 구현된 코드는 Nextion의 고급 객체 기능(버튼 이벤트 리스너 객체 등)을 사용하지 않고, 수동으로 시리얼 명령(`sendNextionCommand`)을 전송하고 수신 데이터를 파싱하는 방식임.
    - 따라서 무거운 `Nextion` 라이브러리 의존성이 불필요함.

- **해결 (My Own Way)**:
    - **라이브러리 제거**: `platformio.ini`의 `lib_deps`에서 `ITEADLIB_Arduino_Nextion` 제거.
    - **소스 수정**: `NextionManager` 헤더와 소스에서 `<Nextion.h>` 포함 구문을 삭제하고, 오직 `<Arduino.h>`와 `<HardwareSerial.h>` 만 사용하여 가볍게 구현.
    - **충돌 방지**: `platformio.ini`의 `lib_ignore` 옵션에 ESP32와 호환되지 않는 로컬 라이브러리 목록 명시.
        ```ini
        lib_ignore = 
            SD
            FreeRTOS
            SPI
            WiFiNINA
            MQ7Sensor
            ArduinoMDNS
        ```

### 2. MDNS 라이브러리 충돌
- **현상**: `WiFiManager.h`에서 `<ESPmDNS.h>` 사용 시 `mdns_result_t` 타입 정의를 찾을 수 없다는 에러 발생.
- **원인**: 로컬 라이브러리 폴더에 있는 `ArduinoMDNS` 라이브러리가 ESP32 내장 `ESPmDNS` 라이브러리보다 우선순위를 가지거나 충돌함.
- **해결**:
    - `platformio.ini`의 `lib_ignore` 목록에 `ArduinoMDNS`를 추가하여 ESP32 내장 라이브러리만 사용하도록 강제함.

### 3. 최종 플랫폼 설정 (platformio.ini)
```ini
[env:xiao_esp32c3_rev08]
platform = espressif32@6.7.0
board = seeed_xiao_esp32c3
framework = arduino
build_src_filter = -<*> +<main/mainRev08_seeed_esp32C3_01/>
lib_deps = 
    ; Nextion 등 불필요/충돌 라이브러리 제거됨
    ; 필요한 내장 라이브러리(WiFi, ESPmDNS 등)는 프레임워크에 포함됨
monitor_speed = 115200
lib_ldf_mode = deep+
lib_extra_dirs = 
    C:\Users\h\Documents\Arduino\libraries
lib_ignore = 
    SD
    FreeRTOS
    SPI
    WiFiNINA
    MQ7Sensor
    ArduinoMDNS
build_flags =
    -DCORE_DEBUG_LEVEL=0
```
- **결과**: 위 설정을 적용한 후 빌드 성공.

### 4. 구현 현황 및 도구 제작 (ESP32-C3 최적화)
- **핵심 기능 구현 완료**: WiFi 연결, NTP 시간 동기화, mDNS(`boiler.local`), 웹 서버(JSON API 및 대시보드), DS18B20 센서 읽기 등 Nextion 디스플레이 물리적 테스트를 제외한 모든 기능이 마이그레이션 및 구현되었습니다.
- **전용 도구 제작**:
    - **웹 로그 플레이어**: `tools/log_player/web_esp32c3_xiao/log_player.html` (ESP32-C3 전용 UI)
    - **데이터 수집기**: `tools/data_collector/esp32c3_xiao/data_collector_esp32c3.py` (JSON 데이터 수집 도구)

### 5. 향후 계획 (Nextion 디스플레이 실기 테스트)
- **현상**: 현재 코드는 빌드에 성공했으나, 실제 Nextion 디스플레이와의 통신(레벨 변환기 거친 D4/D5 통신) 및 터치 이벤트 처리에 대한 실기 검증이 남아있습니다.
- **수정 사항 (2026-02-05 22:30)**:
    - **보드레이트 고정**: 테스트 성공 케이스인 9600bps로 고정하여 통신 안정성 확보.
    - **초기화 강화**: Nextion 부팅 시 시리얼 버퍼 정리를 위해 `0xFF` 3회 전송 및 딜레이 추가.
    - **디버그 로그 추가**: 시스템 초기화 각 단계(Nextion, Sensor, WiFi)에 시리얼 로그를 추가하여 진행 상황 파악 용이화.
    - **연결 확인용 메시지**: `initNextion` 단계에서 `t0.txt="System Ready"`를 전송하여 화면 연결 즉시 확인 가능하게 함.
- **다음 단계**: 수정된 `mainRev08_seeed_esp32C3_02` 코드를 재업로드하여 동작 확인.

---

## [2026-02-05 22:45] 하드웨어 통신 성공 및 R4 WiFi와의 구조적 차이점 분석

### 1. 하드웨어 검증 결과
- **통신 성공**: XIAO ESP32-C3의 D4(TX), D5(RX) 핀을 통한 Nextion 디스플레이 제어 확인.
- **피드백**: `t0.txt="System Ready"` 명령이 부팅 직후 화면에 정상 출력됨.
- **안정성**: 9600bps 기반의 수동 시리얼 통신이 데이터 누락 없이 실시간 터치 이벤트 및 데이터 전송을 수행함을 확인.

### 2. UNO R4 WiFi vs. XIAO ESP32-C3 구현 차이점 (Critical)

| 비교 항목           | UNO R4 WiFi (기존)               | XIAO ESP32-C3 (현재)                  | 차이점 및 개선 사유                                                   |
| :------------------ | :------------------------------- | :------------------------------------ | :-------------------------------------------------------------------- |
| **라이브러리**      | `Nextion.h` (ITEAD 라이브러리)   | **Direct Serial 통신 (Library-free)** | ESP32와 호환되지 않는 AVR용 의존성(SD, SPI 등) 제거 및 메모리 최적화  |
| **통신 속도**       | 초기 115200bps 시도              | **9600bps 고정 (안정성 우선)**        | Nextion 기본 속도에서 시작하여 부팅 시 동기화 실패 가능성 원천 차단   |
| **하드웨어 시리얼** | `Serial1` (전용 핀)              | `HardwareSerial MySerial1(1)`         | ESP32-C3의 유연한 핀 매핑 기능을 활용하여 D4/D5 핀 지정               |
| **초기화 방식**     | 라이브러리 내부 `nexInit()` 호출 | **수동 `0xFF` 패킷 및 웜업 딜레이**   | 부팅 시 시리얼 라인의 쓰레기 값을 수동으로 정리하여 명령 인식률 향상  |
| **NTP/WiFi**        | `WiFiS3` 환경 의존               | `WiFi.h` 및 ESP32 내장 NTP            | ESP32 프레임워크 최적화 기능을 사용하여 연결 속도 및 mDNS 신뢰성 확보 |

### 3. 기술적 결정 사항 (ADR 성격)
- **라이브러리 배제 결정**: `Nextion.h`가 제공하는 객체지향적 접근 방식이 강력하나, 현재 프로젝트의 요구사항(단순 명령 전송 및 터치 파싱)에는 가볍고 충돌 없는 **수동 시리얼 구현**이 훨씬 안정적임을 확인했습니다.
- **핀 설정 고정**: XIAO ESP32-C3의 GPIO 6(D4), GPIO 7(D5)은 하드웨어 UART1로 동작하며, 향후 확장성을 고려하여 센서(D2)와 겹치지 않도록 배치했습니다.

### 4. 다음 단계
- [x] 통신 속도 115200bps 전환 및 그래프 성능 확인.
- [ ] 데이터 수집 도구(`data_collector_esp32c3.py`)와의 연동 테스트.
- [ ] 최종 펌웨어 안정화 및 케이스 조립 준비.

---

## [2026-02-05 22:55] 통신 속도 최적화 (115200bps 전환)

### 1. 목적 및 배경
- 실시간 온도 그래프(Waveform)의 부드러운 출력을 위해 통신 대역폭 확장 필요.
- 9600bps에서는 다량의 데이터(4채널 그래프 + 텍스트 업데이트) 전송 시 발생할 수 있는 잠재적 지연 제거.

### 2. 구현 방식 (Handshake 방식 도입)
- **안정적 전환**: 부팅 직후 바로 고속 통신을 시도하지 않고, Nextion의 기본 속도인 9600bps에서 먼저 연결을 시작함.
- **명령어 전송**: `baud=115200` 명령을 전송하여 Nextion의 통신 속도를 변경.
- **재설정 (Re-init)**: 명령 전송 후 Nextion이 내부 설정을 적용할 수 있도록 200ms 대기한 뒤, ESP32의 `HardwareSerial1`을 115200bps로 재설정함.

### 3. 결과 및 R4 WiFi와의 구조적 차이점
- **R4 WiFi**: 라이브러리(`ITEADLIB_Arduino_Nextion`) 내부에서 불투명하게 처리되던 로직을 **수동 시리얼 제어**로 완벽히 대체함.
*   **신뢰성 확보**: 전환 실패 가능성을 최소화하기 위해 9600bps에서 `0xFF` 패킷을 먼저 보내 버퍼를 비우는 절차를 명시적으로 포함함.
*   **성능**: 명령 전송 속도가 대폭 향상되어 고해상도 그래프 데이터 전송 시 유리함.

### 4. 특이 사항
- `baud` 명령은 Nextion의 EEPROM에 저장되므로, 이후 전원 재시작 시에도 115200bps로 유지됨.
- 만약 속도 동기화가 깨질 경우, ESP32가 부팅 시 9600bps로 핸드셰이크를 다시 시도하므로 복구가 가능하도록 구현됨.

# Progress Log - Boiler 4x Temp Monitoring

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
- **다음 단계**: Nextion 디스플레이 전용 테스트 및 디버깅을 위해 `mainRev08_seeed_esp32C3_02` 폴더를 생성하여 추가 작업을 진행할 예정입니다.

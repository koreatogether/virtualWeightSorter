# Progress Log - Boiler Monitoring Merge (UNO R4 WiFi)

이 로그는 `mainrev07_tftlcd_forNano`와 `mainRev06` 프로젝트를 Arduino UNO R4 WiFi 환경으로 통합하면서 발생한 주요 문제와 해결 과정을 기록합니다.

## [2026-02-03] 주요 이슈 및 해결 사항

### 1. RTC API 호환성 문제
- **현상**: `RTCTime` 객체에서 `getMinute()`, `getSecond()` 호출 시 컴파일 에러 발생.
- **원인**: UNO R4 WiFi용 `RTC.h` 라이브러리의 Getter 메서드 이름이 복수형(`getMinutes()`, `getSeconds()`)으로 정의되어 있음. (Setter는 단수형 `setMinute()` 임)
- **해결**:
    - `getMinute()` -> `getMinutes()` 로 수정
    - `getSecond()` -> `getSeconds()` 로 수정
    - `getMonth()`의 반환값(0~11)을 표시용(1~12)으로 보정 (`+1`)

### 2. Nextion 라이브러리의 `SD.h` 의존성
- **현상**: 컴파일 중 `NexUpload.h`에서 `SD.h: No such file or directory` 에러 발생.
- **원인**: 공식 Nextion 라이브러리는 SD 카드를 통한 HMI 파일 업로드 기능을 포함하고 있어, 실제 사용 여부와 상관없이 `SD.h` 헤더를 참조함.
- **해결**: `platformio.ini`의 `lib_deps`에 `SD` 라이브러리를 추가하여 의존성 해결.

### 3. WiFi 설정 인자 순서 오류 (가장 중요)
- **현상**: WiFi는 연결되나 NTP 시간 동기화 및 외부 HTTP 요청이 타임아웃 발생.
- **원인**: `WiFiS3` 라이브러리의 `WiFi.config()` 인자 순서가 일반적인 라이브러리와 다름.
    - 일반적 순서: `(IP, Gateway, Subnet, DNS)`
    - **WiFiS3 순서**: `(IP, DNS, Gateway, Subnet)`
    - 잘못된 순서로 입력 시 Subnet Mask가 Gateway 주소로 인식되어 외부망 연결이 차단됨.
- **해결**: `WiFi.config(local_IP, dns, gateway, subnet)` 순서로 수정하여 게이트웨이 설정 정상화.

### 4. 인터넷 시간 동기화 방식 개선 (NTP 도입)
- **현상**: HTTP 방식(`worldtimeapi.org`) 사용 시 DNS 확인 및 JSON 파싱 과정에서 잦은 연결 실패 발생.
- **해결**:
    - UDP 기반의 **NTP(Network Time Protocol)** 방식으로 전면 교체.
    - `time.google.com` (1순위), `pool.ntp.org` (2순위) 다중 서버 로직 적용.
    - WiFi 연결 직후 네트워크 안정화를 위한 1초 대기 시간(`delay(1000)`) 추가.

### 5. R4 WiFi 전용 데이터 수집기 (`data_collector_r4wifi.py`)
- **개선 사항**: mDNS(`boiler.local`) 지원을 통해 IP 변경 시에도 자동 접속 가능.
- **구조 개선**: `config_r4.json`을 도입하여 코드 수정 없이 설정 변경 가능.
- **UI 강화**: ANSI 컬러 코드를 사용하여 연결/성공/에러 상태를 시각적으로 분리.

### 6. 웹 기반 로그 플레이어 (`log_player.html`)
- **현상 및 복구**:
    - **임계치 버그**: 줌 레벨에 따라 임계치 선이 대각선으로 깨지던 현상을 `yMin/yMax` 기반 표준 API로 수정.
    - **자동 스케일링**: 임계치 입력 시 해당 선이 보이지 않던 문제를 해결하기 위해 Y축 범위를 자동 조정하는 로직 추가.
- **신규 기능**: 
    - **채널 토글**: S1~S4 각 채널별로 표시 여부를 개별 제어할 수 있는 버튼 추가.
    - **통계 분석**: 현재 화면에 보이는 데이터 구간의 Max/Avg 온도차 실시간 계산.
- **장점**: 별도의 파이썬 환경 구축 없이 브라우저만으로 대용량 로그 분석 가능.

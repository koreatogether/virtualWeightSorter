# Architecture - Boiler Temp Monitoring

## 1. 개요
본 시스템은 보일러 입/출수 배관의 온도를 DS18B20 센서를 통해 정밀하게 측정하고, 실시간으로 20x4 LCD에 표시하는 임베디드 모니터링 장치입니다.

## 2. 하드웨어 구성
- **MCU**: Arduino R4 WiFi (Renesas RA4M1)
- **Sensor**: DS18B20 고유 주소 기반 디지털 온도 센서 (최대 4개)
- **Display**: 20x4 I2C LCD (PCF8574 인터페이스, 주소 0x27)
- **Interface**: 1-Wire 버스 (D2 핀, 4.7kΩ 풀업 저항 사용)

## 3. 소프트웨어 구조 (src/)

### 3.1. 라이브러리 의존성
- `OneWire` & `DallasTemperature`: 센서 데이터 통신 및 온도 변환
- `LiquidCrystal_PCF8574`: LCD 제어

### 3.2. 주요 리비전 (Main Implementations)
- **[Main Rev02](src/main/mainRev02/)**: Blynk IoT 연동 버전 (현재 중단)
- **[Main Rev03](src/main/mainRev03/)**: 로컬 웹 서버 기반 대시보드 버전 (최선)
    - **WiFiManager.h/cpp**: 자체 WiFi 연결 및 웹 서버 객체 관리.
    - **Dashboard.h**: HTML/CSS/JS가 포함된 통계형 웹 UI.
    - **AJAX 기술**: 페이지 새로고침 없이 2초마다 데이터를 동적으로 업데이트.
    - **wifiCredentials.h**: 네트워크 접속 정보를 분리하여 관리.
    - **Heartbeat 기능**: PC 데이터 수집기(`data_collector.py`)와 통신 상태를 확인하여 LCD에 `[LOG]` 표시.

## 4. 데이터 로깅 및 지속성 (Data Logging)
PC와 연동하여 아두이노의 휘발성 데이터를 장기적으로 보관합니다.

### 4.1. PC 데이터 수집기 (Python 기반)
- **저장 위치**: [docs/logs/](../docs/logs/)
- **저장 방식**: 날짜별 폴더 생성 및 7일 순환(Circular) 저장.
- **포맷**: JSON (날짜, 시간, 센서별 온도값 포함).
- **트리거**: 아두이노 부팅 시 수집기가 자동으로 감지하여 기록 시작.

## 5. 핵심 기능 로직

### 4.1. 센서 인식 및 식별
- 부팅 시 1-Wire 버스를 스캔하여 연결된 센서의 고유 ROM ID를 탐색합니다.
- `setup()` 단계에서 각 센서의 주소 마지막 4자리를 LCD에 표시하여 물리적 센서 식별을 돕습니다.

### 4.2. 데이터 수집 및 처리
- 정해진 주기(초기값 2000ms)마다 비차단(Non-blocking) 방식으로 온도를 갱신합니다.
- 예외 처리: 센서 연결 해제 시 `err` 또는 `Check Sensors` 문구를 출력합니다.

### 4.3. 시각화 (LCD Layout)
- **온도차 모드**: 배관 입/출수 온도와 그 차이($\Delta T$)를 배치.
- **개별 온도 모드**: S1~S4 센서의 실시간 온도 데이터를 나열.

## 5. 분석 지표 (Theory Reference)
본 시스템은 공개된 열교환기 일반 이론을 바탕으로 배관 효율 점검의 근거를 제공합니다.
- **온도차($\Delta T$)**: 동일 부하에서 $\Delta T$가 작아지면 열전달 감소 또는 유량 과다를 시사합니다.
- **시스템 밸런스**: 회로 간의 $\Delta T$ 비교를 통해 분배 불균형 여부를 판단할 기초 데이터를 제공합니다.

---
*미래 구현 계획 및 확장 기능은 [ROADMAP.md](../docs/ROADMAP.md)에서 관리합니다.*

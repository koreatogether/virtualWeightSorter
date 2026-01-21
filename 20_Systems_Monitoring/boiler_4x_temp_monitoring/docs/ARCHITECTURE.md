# Architecture - Boiler Temp Monitoring

## 1. 개요
본 시스템은 보일러 입/출수 배관의 온도를 DS18B20 센서를 통해 정밀하게 측정하고, 실시간으로 20x4 LCD에 표시하는 임베디드 모니터링 장치입니다.

## 2. 하드웨어 구성
- **MCU**: Arduino R4 WiFi (Renesas RA4M1)
- **Sensor**: DS18B20 고유 주소 기반 디지털 온도 센서 (최대 4개)
- **Display**: 20x4 I2C LCD (PCF8574 인터페이스, 주소 0x27)
- **Input**: 물리 버튼 (D3 핀, Internal Pull-up) - 화면 전환용
- **Interface**: 1-Wire 버스 (D2 핀, 4.7kΩ 풀업 저항 사용)

## 3. 소프트웨어 구조 (src/)

### 3.1. 라이브러리 의존성
- `OneWire` & `DallasTemperature`: 센서 데이터 통신 및 온도 변환
- `LiquidCrystal_PCF8574`: LCD 제어

### 3.2. 주요 리비전 (Main Implementations)
- **[Main Rev04](src/main/mainRev04/)**: 물리 버튼 기능이 통합된 버전
- **[Main Rev05](src/main/mainRev05/)**: 실시간 그래프 기능이 추가된 최종 버전 (권장)
    - **Chart.js 통합**: 웹 대시보드 내에 센서별 온도 추이를 시각화.
    - **인터랙티브 기능**: 사용자 선택에 따른 갱신 주기 변경(1초~5시간) 및 그래프 줌/팬 기능.
    - **상태 모니터링**: PC 데이터 수집기와의 연결 상태를 웹 UI에서 실시간 확인.
    - **DisplayManager**: 물리 버튼 입력을 감지하여 4개의 LCD 화면 페이지를 순환.
    - **모듈화**: 센서, 와이파이, 디스플레이 로직을 각각의 Manager 클래스로 완전 모듈화.

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

### 4.3. 시각화 및 사용자 인터페이스 (LCD & Button)
- **페이지 전환**: D3 핀의 물리 버튼을 누를 때마다 화면이 순환됩니다. (Interrupt 기반 또는 Polling 방식 Debounce 적용)
- **화면 구성**:
    - **Page 1 (DT View)**: S1, S2 입출수 온도 및 $\Delta T_1$ (보일러).
    - **Page 2 (DT View)**: S3, S4 입출수 온도 및 $\Delta T_2$ (분배기).
    - **Page 3 (All Sensors)**: 모든 센서(S1~S4)의 온도를 한 화면에 표시.
    - **Page 4 (System Info)**: IP 주소 및 시스템 가동 시간 확인.

## 5. 분석 지표 (Theory Reference)
본 시스템은 공개된 열교환기 일반 이론을 바탕으로 배관 효율 점검의 근거를 제공합니다.
- **온도차($\Delta T$)**: 동일 부하에서 $\Delta T$가 작아지면 열전달 감소 또는 유량 과다를 시사합니다.
- **시스템 밸런스**: 회로 간의 $\Delta T$ 비교를 통해 분배 불균형 여부를 판단할 기초 데이터를 제공합니다.

---
*미래 구현 계획 및 확장 기능은 [ROADMAP.md](../docs/ROADMAP.md)에서 관리합니다.*

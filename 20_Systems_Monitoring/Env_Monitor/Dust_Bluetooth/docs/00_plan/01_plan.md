# Arduino R4 WiFi 블루투스 먼지 모니터링 구현 계획

## 1. 프로젝트 개요
Arduino R4 WiFi를 사용한 블루투스 기반 실시간 먼지 농도 모니터링 시스템

## 2. 하드웨어 구성

### 2.1 메인 보드
- **Arduino R4 WiFi** (ESP32-S3 통합)
  - WiFi + Bluetooth 5.0 지원
  - 32KB SRAM, 384KB ROM
  - 14개 디지털 핀, 6개 아날로그 핀

### 2.2 센서 모듈
- **PMS7003** (미세먼지 센서)
  - PM1.0, PM2.5, PM10 측정
  - UART 통신 (9600 baud)
  - 3.3V 동작

- **DHT22** (온습도 센서) 
  - 온도: -40~80°C (±0.5°C)
  - 습도: 0~100% (±2~5%)
  - 디지털 출력

### 2.3 출력 장치
- **LCD 20x4** (I2C)
  - 실시간 데이터 표시
  - I2C 주소: 0x27 또는 0x3F

- **LED 모듈** (빨강/주황/녹색)
  - 공기질 상태 표시
  - 디지털 출력 핀 사용

## 3. 소프트웨어 아키텍처

### 3.1 파일 구조 ( 채용 )
```
src/
├── main.cpp              # 메인 프로그램
├── sensors/
│   ├── pms7003_sensor.h
│   ├── pms7003_sensor.cpp
│   ├── dht22_sensor.h
│   └── dht22_sensor.cpp
├── display/
│   ├── lcd_display.h
│   └── lcd_display.cpp
├── communication/
│   ├── bluetooth_module.h
│   └── bluetooth_module.cpp
└── indicators/
    ├── led_controller.h
    └── led_controller.cpp
```

### 3.2 핵심 클래스 설계
- `PMS7003Sensor`: 미세먼지 데이터 수집
- `DHT22Sensor`: 온습도 데이터 수집  
- `LCDDisplay`: 화면 출력 관리
- `BluetoothModule`: 무선 통신
- `LEDController`: 상태 표시

## 4. 구현 단계

### 4.1 Phase 1: 기초 하드웨어 테스트
1. [X]Arduino R4 WiFi 보드 설정
2. [X]각 센서 개별 동작 확인
3. [ ]LCD 디스플레이 연결 테스트
4. [ ]LED 출력 테스트
 
### 4.2 Phase 2: 센서 데이터 수집
1. [x]2개 센서 코드가 main.cpp 에서 동작하도록 통합
2. [x]의도한 시나리오대로 데이터 수집되는지 확인 

### 4.3 Phase 3: 블루투스 통신
1. arduino r4 wifi 의 블루투스 기능 활성화 및 ble 방식 통신 코드 작성
2. JSON 형태 데이터 전송 프로토콜 정의
3. 스마트폰 앱 작성 
4. arduino r4 wifi 와 스마트폰 앱 간 데이터 송수신 테스트
5. 실시간 데이터 스트리밍 및 통신 에러 상황 주시 
6. 통신에 대한 알고리즘 구현 또는 재차 확인

### 4.4 Phase 4: 통합 및 최적화
1. 전체 시스템 통합 테스트
2. 공기질 등급별 LED 표시 로직
3. 배터리 최적화 (절전 모드)
4. 에러 처리 및 복구

## 5. 핀 배치 계획

### 5.1 센서 연결
```
PMS7003:
- VCC → 5V
- GND → GND  
- TX  → D0 (RX)
- RX  → D1 (TX)

DHT22:
- VCC → 3.3V
- GND → GND
- DATA → D2

LCD (I2C):
- VCC → 5V
- GND → GND
- SDA → A4
- SCL → A5

LED:
- RED   → D3
- ORANGE → D4  
- GREEN → D5
```

## 6. 데이터 전송 프로토콜

### 6.1 JSON 데이터 형식
```json
{
  "timestamp": "2025-01-01T12:00:00Z",
  "pm1_0": 15.2,
  "pm2_5": 25.8,
  "pm10": 35.1,
  "temperature": 23.5,
  "humidity": 65.2,
  "air_quality": "MODERATE"
}
```

### 6.2 공기질 등급
- **GOOD** (좋음): PM2.5 ≤ 15㎍/㎥
- **MODERATE** (보통): PM2.5 16-35㎍/㎥  
- **POOR** (나쁨): PM2.5 36-75㎍/㎥
- **VERY_POOR** (매우나쁨): PM2.5 > 75㎍/㎥

## 7. 개발 환경 설정

### 7.1 PlatformIO 설정
```ini
[env:arduino_uno_r4_wifi]
platform = renesas-ra
board = uno_r4_wifi
framework = arduino
lib_deps = 
    adafruit/DHT sensor library
    adafruit/Adafruit Unified Sensor
    marcoschwartz/LiquidCrystal_I2C
    bblanchon/ArduinoJson
monitor_speed = 115200
```

### 7.2 필요 라이브러리
- DHT sensor library (온습도 센서)
- LiquidCrystal_I2C (LCD 제어)
- ArduinoJson (데이터 직렬화)
- SoftwareSerial (PMS7003 통신)

## 8. 테스트 계획

### 8.1 단위 테스트
- 각 센서 모듈 독립 테스트
- 통신 프로토콜 검증
- 데이터 정확도 검증

### 8.2 통합 테스트  
- 전체 시스템 연동 테스트
- 24시간 연속 동작 테스트
- 블루투스 연결 안정성 테스트

## 9. 예상 개발 일정
- Week 1: 하드웨어 설정 및 기초 테스트
- Week 2: 센서 데이터 수집 구현
- Week 3: 블루투스 통신 구현
- Week 4: 통합 테스트 및 최적화
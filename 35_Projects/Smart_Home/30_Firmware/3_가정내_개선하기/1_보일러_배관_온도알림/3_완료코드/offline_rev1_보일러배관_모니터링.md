# Offline Rev1 보일러배관 모니터링

이 문서는 `offline_rev1.ino` 코드에 대한 문서화 자료입니다. 이 문서는 코드의 목적, 하드웨어 연결, 사용된 라이브러리, 변수, 핵심 함수, 그리고 동작 흐름을 설명합니다n
---

## 1. 목적

- DS18B20 온도 센서를 이용하여 센서의 ID, 주소, 온도를 읽어 들이고,
- I2C LCD와 시리얼 모니터에 센서 정보를 출력합니다.

## 2. 하드웨어 연결

### DS18B20 온도 센서
- 데이터: 아두이노 핀 2 (OneWire 버스)
- VCC: 5V
- GND: GND

### I2C LCD
- SDA: 아두이노 A4
- SCL: 아두이노 A5
- VCC: 5V
- GND: GND

### 버튼
- 한쪽: 아두이노 핀 3
- 다른쪽: GND

## 3. 사용된 라이브러리

- [OneWire](https://github.com/PaulStoffregen/OneWire): 1-wire 통신 프로토콜 지원
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library): DS18B20 온도 센서 제어 라이브러리
- [LiquidCrystal_PCF8574](https://github.com/fdebrabander/Arduino-LiquidCrystal-PCF8574): I2C LCD 제어 라이브러리
- [ezButton](https://github.com/simplefoc/ezButton): 버튼 디바운싱 및 상태 관리

## 4. 주요 변수

### LCD 설정
- `I2C_ADDR`: LCD I2C 주소 (기본: 0x27)
- `LCD_COLS`, `LCD_ROWS`: LCD 화면의 열과 행 수
- `LCD_TIMEOUT`: LCD가 켜져있는 시간 (20초)

### 온도 센서 설정
- `ONE_WIRE_BUS`: DS18B20 통신 핀 (핀 2)
- `sensorAddresses[]`: 센서 주소 배열
- `sensorIDs[]`: 센서 ID 배열
- `sensorValid[]`: 센서 유효성 확인 배열
- `REQUIRED_SENSOR_COUNT`: 필요한 센서 수 (4)

### 타이머 변수
- `lcdOnTime`: LCD가 켜진 시각
- `lastSerialTime`: 시리얼 모니터 업데이트 시간
- `SERIAL_INTERVAL`: 시리얼 출력 간격 (2초)

## 5. 주요 함수

### initializeSensorSystem()
- 센서 초기화 및 LCD/시리얼에 초기 메시지 출력
- 센서의 총 개수를 확인 후 부족하면 경고
- 센서 주소 읽기(`readSensorAddresses()`), 센서 ID 읽기(`readSensorIDs()`) 및 센서 정렬(`sortSensors()`)을 진행

### readSensorAddresses()
- 각 센서의 주소를 읽고, 읽기 실패 시 에러 메시지를 출력

### readSensorIDs()
- 각 센서의 ID를 읽음. 중복 ID가 있는 경우 에러 로그 출력

### sortSensors()
- Bubble Sort를 이용하여 센서들을 정렬 (ID 기준)
- 동시에 주소와 유효성 배열도 함께 교환

### showMessageOnLCD(const char *message)
- 전달된 메시지를 LCD에 출력

### updateLCDDisplay()
- 현재 시간과 버튼 입력을 확인하여 LCD 백라이트를 켜거나 끔
- 버튼을 누르면 LCD 타이머 갱신 및 화면 초기화
- 각 센서의 정보를 LCD 텍스트로 출력 (ID, 온도, 상태 표시)

### updateSerialDisplay()
- 주기적으로 시리얼 모니터에 센서 정보를 출력
- 각 센서별 ID, 온도, 주소, 상태 출력

## 6. 동작 흐름

1. setup(): 초기 설정
   - Serial, LCD, 버튼 초기화
   - LCD 타이머 시작
   - 센서 시스템 초기화 함수 호출

2. loop(): 메인 루프 동작
   - 버튼 상태 업데이트
   - LCD 및 시리얼 모니터에 센서 정보 갱신

## 7. 에러 처리

- 센서가 감지되지 않으면 LCD에 "No sensors!" 메시지를 출력하고 무한 루프로 진입.
- 센서의 주소 또는 온도 읽기에 실패할 경우, 해당 센서에 대해 "ERR" 처리

---

이 문서를 통해 코드의 구조와 동작을 쉽게 이해할 수 있습니다.

# 배너 자세 모니터링 코드

## 개요
이 코드는 Arduino와 MPU6050 가속도/자이로 센서를 사용하여 배너의 자세를 모니터링하고, 배너가 기울어지면 LED와 부저를 통해 경고를 표시합니다.

## 배선도

| MPU6050핀 | Arduino 핀 |
| --------- | ---------- |
| VCC       | 5V         |
| GND       | GND        |
| SCL       | A5         |
| SDA       | A4         |

| LED 핀 | Arduino 핀 |
| ------ | ---------- |
| +      | D2         |
| +      | D3         |
| +      | D4         |
| +      | D5         |
| +      | D6         |
| +      | D7         |
(GND는 공통 )

| 부저 핀 | Arduino 핀 |
| ------- | ---------- |
| VCC     | 5V         |
| GND     | GND        |
| I/0     | D8         |


## 라이브러리
- `<Wire.h>`: I2C 통신을 위한 라이브러리
- `<Adafruit_MPU6050.h>`: MPU6050 센서 라이브러리
- `<Adafruit_Sensor.h>`: 센서 데이터 처리를 위한 라이브러리

## 주요 함수

### setup()
- 시리얼 통신 초기화
- MPU6050 모듈 초기화
- LED 초기화
- 부저 초기화

### loop()
- 주기적으로 checkOrientation(), printWarning(), controlLEDsByAngle(), controlBuzzerByAngle() 함수 호출

### initMPU6050()
- MPU6050 모듈 초기화 및 설정

### initLED()
- LED 핀 설정 및 초기화

### initBuzzer()
- 부저 핀 설정 및 동작 테스트

### checkOrientation()
- 가속도계와 자이로 센서 데이터를 사용하여 pitch와 roll 각도 계산

### printWarning()
- 배너가 기울어진 경우 경고 메시지 출력

### controlLEDsByAngle()
- 배너 기울기 각도에 따라 LED 순차적으로 켜기

### controlBuzzerByAngle()
- 배너 기울기 각도에 따라 부저 소리 내기 (각도가 클수록 높은 음높이)


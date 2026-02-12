# 01_multiple_ds18b20_serial

예제 스케치: 여러 DS18B20을 OneWire 버스에서 탐지하여 시리얼로 출력하고 간단한 명령어를 받을 수 있는 기본 펌웨어입니다.

- 빌드: Arduino IDE 또는 Arduino CLI
- 라이브러리: OneWire, DallasTemperature
- 테스트: 시리얼 모니터(9600 baud)에서 출력 확인

다음 단계:
- ID 관리(EEPROM), 명령어 파서, 센서 오류 처리 추가
- `01_multiple_ds18b20_serial.ino`를 기반으로 확장
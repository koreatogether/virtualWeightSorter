# 🌡️ DS18B20 Multiple Serial Control Project

이 저장소는 다수의 DS18B20 온도 센서를 시리얼 통신을 통해 효율적으로 관리하기 위한 개발 프로젝트 모음입니다.

## 📂 버전별 안내

### [serialVersion_02](./serialVersion_02) (최신 버전)
- **핵심 특징**: 객체지향 설계(OOP), 비차단 상태 머신, 고급 시리얼 UI.
- **주요 기능**: ID 관리, 소프트웨어 임계값 알람, 해상도(9-12bit) 조절.
- **안정성**: `String` 제거 및 정적 분석(`pio check`) 완료.

### [serialVersion_01](./serialVersion_01)
- 초기 프로토타입 버전. 기본 센서 읽기 및 시리얼 출력 기능 포함.

## 🛠 공통 환경
- **Target MCU**: ESP32-C3 (Seeed Studio XIAO)
- **Framework**: Arduino / PlatformIO
- **Communication**: USB Serial (9600 Baud)

---
© 2026 DS18B20 Controller Project Teams.

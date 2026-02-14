# DS18B20 Multi-Sensor Controller (Serial UI Version)

![Platform](https://img.shields.io/badge/Platform-Arduino/ESP32-green)
![Framework](https://img.shields.io/badge/Framework-PlatformIO-blue)
![Architecture](https://img.shields.io/badge/Architecture-Layered/OOP-orange)

본 프로젝트는 **Seeed Studio XIAO ESP32-C3** 보드를 기반으로 여러 개의 **DS18B20 온도 센서**를 효율적으로 관리하고 제어하기 위한 전문적인 펌웨어 솔루션입니다. 시리얼 모니터를 통해 직관적인 UI를 제공하며, 센서의 ID 부여, 임계값 설정, 해상도 조절 등을 프로그래밍 지식 없이도 실시간으로 수행할 수 있습니다.

## 🚀 주요 기능

### 1. 실시간 센서 모니터링
- 연결된 모든 센서의 온도, ID, 하드웨어 주소(Address)를 한눈에 볼 수 있는 테이블 UI 제공.
- 센서별 정상/점검 필요 상태 자동 판별.

### 2. 센서 ID 관리 (Dallas User Data)
- 각 센서의 내장 EEPROM(User Data Register)을 활용하여 고유 ID 부여.
- **개별 설정**, **선택적 설정**, **자동 순차 부여** 등 유연한 ID 할당 로직.

### 3. 하이/로우 임계값 설정 (Software Thresholds)
- 센서별로 상한(High) 및 하한(Low) 온도를 설정하여 온도 이탈 시 시각적 알람 제공.
- 전체 센서에 일괄적으로 임계값을 적용하는 글로벌 설정 기능.

### 4. 정밀도 제어 (Resolution Management)
- 센서의 해상도를 **9, 10, 11, 12비트** 중 선택 가능.
- 측정 정밀도와 데이터 변환 속도 사이의 최적화 가능.

### 5. 견고한 시스템 설계
- **비차단형(Non-blocking) 설계**: `delay()`를 제거하고 `millis()` 기반 상태 머신을 사용하여 시스템 응답성 극대화.
- **메모리 최적화**: `String` 클래스를 전면 배제하고 `char` 버퍼를 사용하여 메모리 파편화 및 누수 방지.
- **자동 복귀**: 사용자 입력이 없을 시 60초 후 자동으로 메인 화면으로 복귀하는 타임아웃 기능.

## 🛠 하드웨어 구성
- **MCU**: Seeed Studio XIAO ESP32-C3
- **Sensor**: DS18B20 (Multiple sensors supported via OneWire bus)
- **Pin Mapping**: GPIO 4 (D2) 사용

## 💻 설치 및 빌드 방법
본 프로젝트는 **PlatformIO** 환경에서 개발되었습니다.

1.  저장소를 클론합니다.
2.  Visual Studio Code에서 PlatformIO IDE를 엽니다.
3.  `serialVersion_02/30_Firmware` 폴더를 프로젝트 폴더로 추가합니다.
4.  `Build` 및 `Upload` 버튼을 눌러 보드에 업로드합니다.
5.  시리얼 모니터를 켜고(Baud rate: 9600) 메뉴에 따라 조작합니다.

## 📁 프로젝트 구조
- `30_Firmware/src`: 펌웨어 소스 코드 (App, UI, Sensor, DataProcessor 등)
- `docs/history`: 개발 과정 및 변경 이력 기록
- `docs/test`: 단계별 품질 검증 절차서

## 📝 라이선스
본 프로젝트는 MIT 라이선스 하에 배포됩니다. 자유롭게 수정 및 배포가 가능합니다.

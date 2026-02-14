# PlatformIO 프로젝트 구조 추천

## 📋 현재 상황 분석

### 프로젝트 구조
```
E:\project\35_Projects\
├── 18B20_multiple_serial_control\
│   ├── serialVersion_01\
│   │   └── 30_Firmware\          ← 현재 platformio.ini 위치
│   ├── serialVersion_02\
│   ├── localNextionTFTLcdVersion_01\
│   └── ...
├── [대기]Smart_Farm\
├── [대기]Smart_Home\
└── 기타 프로젝트들...
```

### 발견된 Arduino 파일
- 총 **33개의 .ino 파일**이 다양한 프로젝트에 분산
- 현재 **1개의 platformio.ini** 파일만 존재

---

## ✅ 추천 방안: **프로젝트별 독립 PlatformIO**

### 📁 추천 구조

각 프로젝트의 **펌웨어 폴더(30_Firmware)에 platformio.ini 배치**

```
프로젝트명\
├── 00_Requirements\
├── 10_System_Design\
├── 20_Hardware\
├── 30_Firmware\              ← platformio.ini 여기에!
│   ├── platformio.ini         ← 프로젝트별 독립 설정
│   ├── src\
│   │   ├── main.cpp
│   │   └── *.cpp, *.h
│   └── .pio\                  ← 빌드 결과물 (자동 생성)
├── 40_Software\
└── ...
```

---

## 💡 핵심 개념: 멀티 환경(Multi-Environment)

### 하나의 platformio.ini로 여러 보드 관리

```ini
; 공통 설정
[platformio]
default_envs = nano   ; 기본 빌드 환경

; 공통 라이브러리 (모든 환경에서 공유)
[common]
lib_deps = 
    paulstoffregen/OneWire@^2.3.7
    milesburton/DallasTemperature@^3.11.0

; Arduino Nano 환경
[env:nano]
platform = atmelavr
board = nanoatmega328
framework = arduino
lib_deps = ${common.lib_deps}

; ESP32-C3 XIAO 환경
[env:xiao_esp32c3]
platform = espressif32
board = seeed_xiao_esp32c3
framework = arduino
build_flags = -D ARDUINO_USB_CDC_ON_BOOT=1
lib_deps = ${common.lib_deps}

; Arduino UNO R4 WiFi 환경
[env:uno_r4_wifi]
platform = renesas-ra
board = uno_r4_wifi
framework = arduino
lib_deps = ${common.lib_deps}

; ESP32 DevKit 환경
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = ${common.lib_deps}
```

### 빌드 명령어

```bash
# 기본 환경(nano) 빌드
pio run

# 특정 환경만 빌드
pio run -e xiao_esp32c3
pio run -e uno_r4_wifi

# 모든 환경 빌드
pio run -e nano -e xiao_esp32c3 -e uno_r4_wifi

# 특정 환경 업로드
pio run -e xiao_esp32c3 --target upload
```

---

## 🎯 실전 적용 방법

### Option 1: 같은 코드를 여러 보드에 사용할 때

**하나의 platformio.ini에 여러 환경 정의**

```
프로젝트\30_Firmware\
├── platformio.ini        ← 여러 보드 환경 정의
├── src\
│   └── main.cpp         ← 모든 보드에서 공유
└── .pio\
```

**장점:**
- 코드 중복 없음
- 한 번에 모든 보드 테스트 가능
- 관리가 간편

**단점:**
- 보드별 특화 기능 구현 시 조건부 컴파일 필요

### Option 2: 보드별로 완전히 다른 코드일 때

**각 버전별로 독립된 platformio.ini**

```
프로젝트\
├── serialVersion_01\
│   └── 30_Firmware\
│       ├── platformio.ini    ← Nano용
│       └── src\
├── serialVersion_02\
│   └── 30_Firmware\
│       ├── platformio.ini    ← ESP32용
│       └── src\
└── webVersion_01\
    └── 30_Firmware\
        ├── platformio.ini    ← UNO R4 WiFi용
        └── src\
```

**장점:**
- 각 버전이 완전히 독립적
- 보드별 최적화 용이

**단점:**
- 코드 중복 가능성
- 여러 프로젝트 관리 필요

---

## 📦 .pio 폴더 관리

### .pio 폴더란?
- PlatformIO가 자동 생성하는 빌드 결과물 폴더
- 컴파일된 오브젝트 파일, 라이브러리 캐시 등 포함

### Git 저장소 관리 (.gitignore)

```gitignore
# PlatformIO
.pio/
.vscode/.browse.c_cpp.db*
.vscode/c_cpp_properties.json
.vscode/launch.json
.vscode/ipch
```

### 빌드 환경 재생성

**Q: 빌드 환경을 필요할 때마다 새로 만들어야 하나?**

**A: 아니요! PlatformIO가 자동으로 관리합니다.**

1. **최초 빌드 시:**
   - `pio run` 실행
   - 플랫폼, 툴체인, 라이브러리 자동 다운로드
   - `.pio/` 폴더에 캐시 저장

2. **이후 빌드 시:**
   - 캐시된 환경 재사용
   - 변경사항만 다운로드

3. **환경 초기화가 필요한 경우:**
   ```bash
   # 빌드 결과만 삭제
   pio run --target clean
   
   # 전체 환경 재생성
   rm -rf .pio
   pio run
   ```

---

## 🚀 당신의 프로젝트에 맞는 추천

### 현재 프로젝트: 18B20_multiple_serial_control

**추천: Option 1 (멀티 환경)**

이유:
- DS18B20 센서 로직은 동일
- 여러 버전이 이미 존재 (serialVersion_01, _02, Nextion, WebPage)
- 통신 방식만 다름 (Serial, Nextion, Web)

**적용 방법:**

```
18B20_multiple_serial_control\
└── 30_Firmware\                     ← 최상위로 통합
    ├── platformio.ini               ← 모든 보드 환경 정의
    ├── src\
    │   ├── main.cpp                 ← 기본 시리얼 버전
    │   └── common\                  ← 공통 코드
    │       ├── DS18B20_Sensor.cpp
    │       └── ...
    ├── variants\                    ← 버전별 특화 코드
    │   ├── serial\
    │   ├── nextion\
    │   └── web\
    └── .pio\
```

또는 **각 버전을 독립적으로 유지:**

```
18B20_multiple_serial_control\
├── serialVersion_01\
│   └── 30_Firmware\
│       ├── platformio.ini           ← Nano + ESP32-C3
│       └── src\
├── localNextionTFTLcdVersion_01\
│   └── 30_Firmware\
│       ├── platformio.ini           ← Nextion 지원 보드들
│       └── src\
└── localWebPageVersion_01\
    └── 30_Firmware\
        ├── platformio.ini           ← WiFi 지원 보드들
        └── src\
```

---

## 📚 참고: 타 프로젝트 마이그레이션

### Arduino IDE → PlatformIO 전환 순서

1. **프로젝트에 platformio.ini 생성**
   ```bash
   cd E:\project\35_Projects\[프로젝트명]\30_Firmware
   pio init --board [보드명]
   ```

2. **src 폴더 생성 및 파일 이동**
   ```bash
   mkdir src
   mv *.ino src/main.cpp
   mv *.cpp *.h src/
   ```

3. **빌드 및 테스트**
   ```bash
   pio run
   ```

### 자주 사용할 보드명

```bash
# Arduino
--board nanoatmega328      # Arduino Nano
--board uno                # Arduino UNO
--board uno_r4_wifi        # Arduino UNO R4 WiFi
--board mega2560           # Arduino Mega

# ESP32
--board esp32dev           # ESP32 DevKit
--board seeed_xiao_esp32c3 # XIAO ESP32-C3
--board esp32-s3-devkitc-1 # ESP32-S3

# ESP8266
--board nodemcuv2          # NodeMCU
--board d1_mini            # Wemos D1 Mini
```

---

## ✨ 최종 추천

### 당신의 작업 스타일에 맞춰:

1. **실험/프로토타입이 많은 경우**
   - 각 프로젝트 폴더에 독립적인 platformio.ini
   - 빠른 실험 및 테스트 가능

2. **제품화/안정화 단계**
   - 하나의 platformio.ini에 모든 타겟 보드 정의
   - CI/CD 파이프라인 구축 용이

3. **혼합 방식 (추천!)**
   - 개발 초기: 독립적인 platformio.ini
   - 안정화 후: 멀티 환경으로 통합

### 빌드 환경은?

**걱정 마세요! 자동으로 관리됩니다.**

- ✅ `.pio/` 폴더만 Git에서 제외
- ✅ `platformio.ini`만 관리
- ✅ 어느 PC에서든 `pio run`만 하면 자동 환경 구축
- ✅ 프로젝트 이동/공유도 간편

---

## 🎓 학습 리소스

- [PlatformIO 공식 문서](https://docs.platformio.org/)
- [Multi-Environment 가이드](https://docs.platformio.org/en/latest/projectconf/section_env.html)
- [보드 목록](https://docs.platformio.org/en/latest/boards/index.html)

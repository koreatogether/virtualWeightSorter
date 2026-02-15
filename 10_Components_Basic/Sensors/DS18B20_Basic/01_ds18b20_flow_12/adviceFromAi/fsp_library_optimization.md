# FSP 라이브러리 최적화 가이드

## 📋 문서 개요

**작성일**: 2025-07-28  
**목적**: Arduino UNO R4 WiFi에서 `libfsp.a` 라이브러리로 인한 메모리 사용량 증가 문제 해결 방안  
**배경**: `build/big` 폴더(207KB)와 현재 빌드(74KB) 간의 133KB 차이 원인 분석 및 해결책 제시

## 🔍 문제 분석

### 1. FSP 라이브러리 강제 포함 현상

Arduino UNO R4 WiFi 플랫폼은 **Renesas RA4M1 마이크로컨트롤러**를 사용하며, 특정 기능 사용 시 **FSP(Flexible Software Package) 라이브러리 전체**가 강제로 포함됩니다.

#### 컴파일 로그에서 확인된 강제 포함 구조:
```bash
-Wl,--whole-archive 
-Wl,--start-group 
    "C:\Users\h\AppData\Local\Arduino15\packages\arduino\hardware\renesas_uno\1.5.0\variants\UNOWIFIR4/libs/libfsp.a" 
    "core.a" 
-Wl,--no-whole-archive 
```

### 2. FSP 라이브러리를 트리거하는 기능들

#### A. DS18B20 alarm 기능 (TH/TL 레지스터)
```cpp
// 이 코드들이 FSP 라이브러리 전체를 트리거
_sensors.setHighAlarmTemp(address, highTemp);
_sensors.setLowAlarmTemp(address, lowTemp);
_sensors.getHighAlarmTemp(address);
_sensors.getLowAlarmTemp(address);
```

**트리거 이유**: DallasTemperature 라이브러리의 alarm 기능 → OneWire 고급 기능 → Renesas 고급 타이머/인터럽트 → FSP 전체 라이브러리

#### B. 고급 String 처리
```cpp
// v6.07-v6.08에서 추가된 복잡한 파싱
String currentNumber = "";
for (int i = 0; i < input.length(); i++) {
    // 복잡한 String 조작...
}
```

**트리거 이유**: Arduino String 클래스의 복잡한 메모리 관리 → heap 관리자 → FSP 고급 메모리 관리 모듈

#### C. EEPROM 쓰기 작업
```cpp
// 임계값을 센서 내부에 저장하는 작업
delay(100); // EEPROM 쓰기 완료 대기
```

**트리거 이유**: DS18B20 내부 EEPROM 쓰기 → 정밀한 타이밍 제어 → Renesas 고급 타이머 모듈

### 3. 연쇄 반응 (Cascade Effect)

```
알람 기능 사용 
    ↓
OneWire 고급 기능 활성화
    ↓  
정밀 타이밍 요구
    ↓
FSP 타이머 모듈 링크
    ↓
FSP 전체 라이브러리 포함 (--whole-archive)
```

## 🚫 일반적인 최적화가 효과없는 이유

### 1. 조건부 컴파일의 한계
```cpp
#ifdef ENABLE_THRESHOLD
// 이 코드가 없어도 DallasTemperature.h 자체가 FSP를 참조
#endif
```

**문제**: 헤더 파일 포함 단계에서 이미 의존성이 결정됨

### 2. C 문자열 최적화의 한계
```cpp
// String 대신 char[] 사용해도
char buffer[20];
sprintf(buffer, "temp: %d", temp);
// sprintf 자체가 고급 포맷팅 라이브러리를 요구
```

**문제**: `sprintf`, `strlen` 등 기본 C 함수들조차 Renesas 플랫폼에서는 FSP 구현을 사용

### 3. 링커의 --whole-archive 정책
```bash
--whole-archive libfsp.a
```

**의미**: FSP 라이브러리 중 하나라도 사용되면 **전체**가 포함됨

## 💡 해결책들

### 1. 컴파일러 플래그 오버라이드 (1순위: 즉시 시도 가능)

#### 방법 A: 최소 FSP 모드
```bash
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi . \
  --build-properties "compiler.cpp.extra_flags=-DARDUINO_FSP_MINIMAL" \
  --build-properties "compiler.c.elf.extra_flags=-Wl,--gc-sections,--no-whole-archive" \
  --output-dir ./build_minimal
```

#### 방법 B: 가비지 컬렉션 강화
```bash
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi . \
  --build-properties "compiler.c.elf.extra_flags=-Wl,--gc-sections,--strip-all" \
  --output-dir ./build_optimized
```

### 2. 사용자 정의 보드 정의 (2순위: 중간 난이도)

#### 단계별 구현:
1. **사용자 하드웨어 폴더 생성**
```
Documents/Arduino/hardware/custom_renesas/avr/1.0.0/
├── boards.txt
├── platform.txt
└── variants/
    └── unor4wifi_minimal/
```

2. **boards.txt 설정**
```ini
unor4wifi_minimal.name=Arduino UNO R4 WiFi (Minimal FSP)
unor4wifi_minimal.vid.0=0x2341
unor4wifi_minimal.pid.0=0x1002
unor4wifi_minimal.build.mcu=cortex-m4
unor4wifi_minimal.build.core=arduino
unor4wifi_minimal.build.variant=unor4wifi_minimal
unor4wifi_minimal.build.ldflags=-Wl,--gc-sections,--no-whole-archive
```

3. **platform.txt 수정**
```ini
recipe.c.combine.pattern=... -Wl,--gc-sections {build.variant.path}/libs/libfsp_minimal.a ...
```

### 3. 플랫폼 파일 직접 수정 (3순위: 고급)

#### 수정 대상 파일:
```
C:\Users\h\AppData\Local\Arduino15\packages\arduino\hardware\renesas_uno\1.5.0\variants\UNOWIFIR4\
├── fsp.ld           # 링커 스크립트
├── platform.txt     # 빌드 설정
└── boards.txt       # 보드별 설정
```

#### platform.txt 수정 예시:
```bash
# Before (현재)
recipe.c.combine.pattern=... -Wl,--whole-archive {build.variant.path}/libs/libfsp.a ...

# After (수정안)
recipe.c.combine.pattern=... -Wl,--gc-sections {build.variant.path}/libs/libfsp.a ...
```

### 4. 완전한 빌드 시스템 교체 (4순위: 최고급)

#### CMake 기반 빌드 시스템:
```cmake
# CMakeLists.txt
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 직접 링커 플래그 제어
target_link_libraries(${PROJECT_NAME} 
    -Wl,--gc-sections
    -Wl,--no-whole-archive
    ${FSP_LIBRARIES}
)
```

## 🎯 기능별 메모리 사용량 예상

| 기능 조합                | 예상 Flash 사용량 | FSP 포함 여부 |
| ------------------------ | ----------------- | ------------- |
| **기본 온도 측정만**     | ~74KB             | 일부만        |
| **+ 간단한 임계값**      | ~90KB             | 일부만        |
| **+ 하드웨어 알람**      | ~150KB            | 대부분        |
| **+ 복잡한 String 처리** | ~180KB            | 전체          |
| **+ 메모리 분석 도구**   | ~207KB            | 전체          |

## 📋 테스트 체크리스트

### 즉시 테스트 가능한 방법들:

#### 1. 기본 최적화 시도
```bash
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi . \
  --build-properties "compiler.c.elf.extra_flags=-Wl,--gc-sections" \
  --output-dir ./build_test1
```

#### 2. FSP 최소화 시도
```bash
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi . \
  --build-properties "compiler.cpp.extra_flags=-DARDUINO_FSP_MINIMAL" \
  --output-dir ./build_test2
```

#### 3. 결과 비교
```bash
# 빌드 크기 비교
ls -la build/
ls -la build_test1/
ls -la build_test2/

# .map 파일에서 FSP 라이브러리 포함 확인
grep -c "libfsp.a" build/*.map
grep -c "libfsp.a" build_test1/*.map
grep -c "libfsp.a" build_test2/*.map
```

## 🔄 향후 개선 방향

### 1. 기능별 빌드 타겟 분리
```
├── basic_build/     # 온도 측정만 (~74KB)
├── advanced_build/  # 임계값 + 고급 기능 (~150KB)
└── full_build/      # 모든 기능 (~207KB)
```

### 2. 하이브리드 시스템 구조
- **UNO R4**: 순수 센서 인터페이스
- **ESP32**: 고급 처리 및 통신
- **효과**: 각 플랫폼의 장점만 활용

### 3. 라이브러리 대체
```cpp
// DallasTemperature 대신 직접 OneWire 구현
// String 대신 고정 크기 char 배열
// sprintf 대신 간단한 숫자 변환 함수
```

## 📊 결론

### FSP 라이브러리 포함은 Renesas 플랫폼의 구조적 특성

**현재 상황:**
- **basic 기능만 사용**: 74KB (현재)
- **고급 기능 추가**: 207KB (big 폴더) - **플랫폼 한계**

**권장 선택지:**
1. **현재처럼 기능 제한** - 가장 현실적
2. **컴파일러 플래그 최적화** - 일부 효과 기대
3. **기능별 펌웨어 분리** - 사용 상황에 따라 교체
4. **플랫폼 변경** (ESP32 등) - 근본적 해결

**교훈**: 임계값, 고급 String 처리, 메모리 분석 등의 고급 기능을 사용하면 **FSP 라이브러리 전체 포함은 어쩔 수 없는 Renesas 플랫폼의 특성**입니다.

---

**문서 작성자**: GitHub Copilot  
**관련 이슈**: `build/big` 폴더 vs 현재 빌드 메모리 사용량 차이 분석  
**테스트 필요**: 위 최적화 방법들의 실제 효과 검증

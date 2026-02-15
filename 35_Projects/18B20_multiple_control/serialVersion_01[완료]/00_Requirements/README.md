# DS18B20 + 실시간 메모리 분석 통합 시스템

## 🎯 프로젝트 개요

DS18B20 온도 센서(최대 8개)와 실시간 메모리 분석 기능이 통합된 Arduino UNO R4 WiFi 시스템입니다.

### ✨ 주요 특징
- **DS18B20 온도 센서 완전 관리**: 4개 센서 실시간 모니터링
- **실시간 메모리 분석**: 10초 간격 자동 메모리 상태 체크  
- **100% 메모리 안정성**: 할당/해제 테스트 모두 성공
- **PC 연동 로그 시스템**: CSV 형식 구조화된 데이터 수집/분석
- **🆕 센서 오류 복구**: 센서 실패 시 자동 재시도 (v6.12)


### 🚀 즉시 시작하기
```bash
# 1. 펌웨어 컴파일 (최적화/미니멀/일반)
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi . --build-property compiler.c.elf.extra_flags="-Wl,--gc-sections,--strip-all" --output-dir ./build_optimized
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi . --build-property compiler.cpp.extra_flags=-DARDUINO_FSP_MINIMAL --build-property compiler.c.elf.extra_flags="-Wl,--gc-sections,--no-whole-archive" --output-dir ./build_minimal
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi . --output-dir ./build --verbose 2>compileUploadLog/compile_verbose.txt

# 2. 펌웨어 업로드
arduino-cli upload --fqbn arduino:renesas_uno:unor4wifi --port COM4 --input-dir ./build_optimized

# 3. 실시간 로그 수집 & 분석
python memory_serial_log/save_serial_log.py
python memory_serial_log/analyze_memory_logs.py
```

#### 📦 빌드 산출물(예시)
* `./build_optimized/01_ds18b20_flow_09.ino.hex` (최적화)
* `./build_minimal/01_ds18b20_flow_09.ino.hex` (미니멀)
* `./build/01_ds18b20_flow_09.ino.hex` (일반)

> **최근 빌드 메모리 사용량 예시**
> - Sketch uses 214668 bytes (81%) of program storage space. Maximum is 262144 bytes.
> - Global variables use 13056 bytes (39%) of dynamic memory, leaving 19712 bytes for local variables. Maximum is 32768 bytes.

📖 **상세 가이드**: [DS18B20_Memory_Integration_Guide.md](DS18B20_Memory_Integration_Guide.md)

## 🖥️ 시리얼 명령어

### DS18B20 센서 명령어
- `menu` 또는 `m`: ID 변경 메뉴 호출
- `threshold` 또는 `t`: 임계값 설정 메뉴 호출
- `help` 또는 `h`: 사용 가능한 명령어 목록

### 메모리 분석 명령어  
- `memtest`: 기본 메모리 테스트 실행
- `memstress`: 스트레스 메모리 테스트 (20회 반복)
- `memfree`: 현재 메모리 상태 출력
- `memtoggle`: 주기적 메모리 분석 활성화/비활성화

## 🔧 기술 사양

- **지원 센서**: DS18B20 온도 센서 (최대 8개)
- **통신 방식**: OneWire 프로토콜 (D2 핀)
- **업데이트 주기**: 15초마다 자동 센서 상태 확인
- **시리얼 통신**: 9600 baud
- **메모리 모니터링**: 10초 간격 자동 체크

```
├── 01_ds18b20_flow_09.ino          # 메인 스케치
├── App.h/.cpp                       # 애플리케이션 메인 로직  
├── DS18B20_Sensor.h/.cpp           # 하드웨어 제어
├── MemoryUsageTester.h/.cpp        # 메모리 분석
├── DataProcessor.h/.cpp            # 데이터 처리 및 출력
├── Communicator.h/.cpp             # 시리얼 통신
├── Utils.h/.cpp                    # 공용 함수
├── memory_serial_log/              # 로그 수집/분석 시스템
├── compileUploadLog/               # 빌드 로그
├── changeLog/                      # 변경 이력
├── test/                          # 단위 테스트
└── .vscode/                       # 개발 가이드
```

## 📦 의존성

- OneWire 라이브러리
- DallasTemperature 라이브러리

---

*상세한 변경 내역은 [changeLog/](changeLog/) 폴더를 참고하시기 바랍니다.*
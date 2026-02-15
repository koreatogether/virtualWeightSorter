# DS18B20 + 메모리 분석 통합 시스템

## 📖 개요
이 시스템은 DS18B20 온도 센서의 기본 기능과 실시간 메모리 분석 기능을 통합한 Arduino UNO R4 WiFi 프로젝트입니다.

## 🎯 주요 기능

### DS18B20 온도 센서 기능
- 다중 온도 센서 관리
- 실시간 온도 데이터 수집
- 센서 ID 관리 및 변경
- 온도 임계값 설정

### 메모리 분석 기능
- **초기화 시 메모리 상태 분석**: 앱 초기화 전후 메모리 사용량 측정
- **주기적 메모리 모니터링**: 10초마다 자동 메모리 상태 확인
- **런타임 메모리 테스트**: 실제 사용 시나리오 시뮬레이션
- **수동 메모리 테스트**: 명령어를 통한 상세 메모리 분석
- **CSV 로그 출력**: PC 분석용 구조화된 데이터

## 🖥️ 시리얼 명령어

### 메모리 분석 명령어
- `memtest` - 기본 메모리 테스트 실행
- `memstress` - 스트레스 메모리 테스트 (20회 반복)
- `memstruct` - 데이터 구조 메모리 테스트
- `memfree` - 현재 메모리 상태 출력
- `memtoggle` - 주기적 메모리 분석 활성화/비활성화

### 일반 명령어
- `help` 또는 `h` - 도움말 메뉴 출력
- `menu` 또는 `m` - DS18B20 센서 메뉴 (기존 기능)

## 📊 로그 수집 및 분석

### 1. 실시간 로그 수집
```bash
python memory_serial_log/save_serial_log.py
```
- 메모리 관련 로그만 필터링하여 저장
- 타임스탬프가 포함된 파일명으로 자동 저장

### 2. 로그 분석
```bash
python memory_serial_log/analyze_memory_logs.py
```
- 모든 수집된 로그 파일 자동 분석
- 통계 보고서 자동 생성 (memory_analysis_report.md)

## 🔄 통합 동작 플로우

1. **시스템 시작**
   - 시리얼 통신 초기화 (9600 baud)
   - 초기 메모리 상태 측정 및 로깅
   - DS18B20 앱 초기화
   - 초기화 완료 후 메모리 상태 재측정

2. **런타임 동작**
   - DS18B20 센서 데이터 수집 및 처리
   - 10초마다 주기적 메모리 상태 확인
   - 런타임 메모리 할당/해제 테스트
   - 시리얼 명령어 처리

3. **로그 출력**
   - 일반 메모리 상태: `[Enter] function_name - Free Memory: XXXX bytes`
   - CSV 로그: `millis(),function,event,memory_value`
   - 런타임 테스트: `Runtime memory allocation/deallocation: SUCCESS/FAILED`

## 📁 주요 파일 및 빌드 산출물
```
01_ds18b20_flow_09/
├── 01_ds18b20_flow_09.ino            # 메인 통합 시스템
├── App.cpp/h, DataProcessor.cpp/h     # 센서/데이터 처리
├── DS18B20_Sensor.cpp/h               # 센서 제어
├── MemoryUsageTester.cpp/h            # 메모리 테스트 클래스
├── Communicator.cpp/h, Utils.cpp/h    # 통신/유틸
├── memory_serial_log/                 # 로그 수집 및 분석 도구
│   ├── save_serial_log.py             # 실시간 로그 수집
│   ├── analyze_memory_logs.py         # 로그 분석 도구
│   └── README.md                      # 로그 시스템 설명
├── build_optimized/01_ds18b20_flow_09.ino.hex   # 최적화 빌드 산출물
├── build_minimal/01_ds18b20_flow_09.ino.hex     # 미니멀 빌드 산출물
├── build/01_ds18b20_flow_09.ino.hex             # 일반 빌드 산출물
├── compileUploadLog/                  # 빌드/업로드 로그
├── memory_log_YYYYMMDD_HHMMSS.txt     # 수집된 로그 파일들
└── memory_analysis_report.md           # 자동 생성 분석 보고서
```

> **최근 빌드 메모리 사용량 예시**
> - Sketch uses 214668 bytes (81%) of program storage space. Maximum is 262144 bytes.
> - Global variables use 13056 bytes (39%) of dynamic memory, leaving 19712 bytes for local variables. Maximum is 32768 bytes.

## 🎛️ 설정 옵션

### Arduino 코드 설정 예시
```cpp
#define MEMORY_ANALYSIS_ENABLED 1
const unsigned long memoryCheckInterval = 10000;  // 주기적 확인 간격 (ms)
```

### Python 로그 설정 예시
```python
SERIAL_PORT = 'COM4'    # 시리얼 포트
BAUD_RATE = 9600        # 통신 속도
```

## 📈 분석 가능한 메트릭

1. **메모리 사용 패턴**
   - 초기화 시 메모리 소비량
   - 런타임 메모리 변화
   - 주기적 메모리 안정성

2. **메모리 성능**
   - 할당/해제 성공률
   - 메모리 누수 감지
   - 스트레스 테스트 결과

3. **시스템 안정성**
   - 장시간 동작 시 메모리 안정성
   - DS18B20 센서 동작과 메모리 사용의 상관관계

## 🚀 사용 시나리오

### 개발 단계
- 코드 변경 후 메모리 영향 분석
- 메모리 누수 탐지
- 최적화 효과 측정

### 운영 단계
- 실시간 메모리 모니터링
- 장기간 안정성 검증
- 성능 이슈 조기 감지

이 통합 시스템을 통해 DS18B20 온도 센서 프로젝트의 기능과 메모리 안정성을 동시에 관리할 수 있습니다.

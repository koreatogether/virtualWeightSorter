# DS18B20 센서 기능 및 구현 상세 설명

## 1. 개요
이 프로젝트는 Arduino UNO R4 WiFi 기반에서 DS18B20 온도 센서를 다중 관리하고, 센서별 ID 및 사용자 데이터를 저장/조회하며, 실시간 온도 측정과 메모리 사용량 분석 기능을 통합한 시스템입니다.

## 2. 주요 구현 기능

### 2.1 DS18B20 센서 관리
- 여러 개의 DS18B20 센서를 OneWire 버스에서 자동 탐지 및 관리
- 각 센서의 고유 주소(DeviceAddress) 탐색 및 저장
- 센서별 온도 측정 및 실시간 데이터 수집
- 센서별 사용자 데이터(예: 논리적 ID) 저장 및 조회
- 센서별 ID 중복 체크 및 관리

### 2.2 센서 데이터 처리
- 센서 인덱스 또는 주소로 온도값 조회
- 센서별 사용자 데이터 get/set 지원
- 센서별 온도 요청 및 최신값 갱신
- 센서가 연결되지 않았을 때 오류값(-127.0) 반환

### 2.3 메모리 분석 및 테스트
- 시스템 동작 중 실시간 메모리 사용량 측정 및 로깅
- 메모리 스트레스 테스트, 구조 테스트, 주기적 모니터링 등 다양한 명령어 지원
- 메모리 분석 결과를 CSV 및 마크다운 리포트로 자동 저장
- Python 스크립트를 통한 시리얼 로그 수집 및 분석 자동화

### 2.4 명령어 기반 제어
- `memtest`, `memstress`, `memstruct`, `memfree`, `memtoggle` 등 메모리 분석/테스트 명령어 지원
- `help`, `menu` 등 사용자 편의 명령어 제공

### 2.5 주요 파일 구조
- `DS18B20_Sensor.cpp/h` : 센서 제어 및 데이터 관리 클래스
- `App.cpp/h`, `DataProcessor.cpp/h` : 센서 데이터 처리 및 통신
- `MemoryUsageTester.cpp/h` : 메모리 테스트 및 분석
- `memory_serial_log/` : 로그 수집 및 분석용 Python 도구

## 3. DS18B20_Sensor 클래스 주요 메서드
- `begin()` : 센서 초기화
- `getDeviceCount()` : 연결된 센서 개수 반환
- `getAddress(address, index)` : 인덱스별 센서 주소 반환
- `requestTemperatures()` : 모든 센서 온도 측정 요청
- `getTempC(address)` : 특정 센서 온도 반환
- `getTempCByIndex(index)` : 인덱스별 센서 온도 반환
- `getUserData(address)` / `setUserData(address, userData)` : 센서별 사용자 데이터 get/set
- `getUserDataByIndex(index)` : 인덱스별 사용자 데이터 반환
- `isIdUsed(id)` : ID 중복 여부 확인

## 4. 활용 예시 및 확장성
- 다중 센서 환경에서 센서별 논리적 ID 관리 및 온도 모니터링
- 메모리 사용량 변화 추적 및 시스템 안정성 검증
- 시리얼 명령어 기반의 자동화 테스트 및 분석
- Python 스크립트와 연동한 데이터 수집/분석 자동화

## 5. 참고
- DallasTemperature, OneWire 라이브러리 활용
- 상세 명령어 및 분석 방법은 `DS18B20_Memory_Integration_Guide.md` 참고

---
본 문서는 AI/개발자 협업 및 시스템 구조 설명용으로 작성되었습니다.

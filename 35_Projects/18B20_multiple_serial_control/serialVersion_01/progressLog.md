# Progress Log - 18B20_multiple_serial_control

## [2026-02-13] PlatformIO 환경 구축 및 ESP32-C3 마이그레이션

### 완료된 작업
1. **PlatformIO 프로젝트 초기화**
   - Seeed Studio XIAO ESP32-C3 보드 설정
   - `platformio.ini` 생성 및 라이브러리 의존성 추가 (`OneWire`, `DallasTemperature`)
2. **프로젝트 구조 재구성 (Standard PlatformIO Layout)**
   - `src/` 디렉토리 생성
   - `01_ds18b20_flow_12.ino`를 `src/main.cpp`로 변경 및 이동
   - 모든 `.cpp`, `.h` 파일을 `src/`로 이동
3. **빌드 에러 해결**
   - **라이브러리 충돌 해결**: Arduino IDE 라이브러리 폴더의 AVR용 FreeRTOS와의 충돌을 피하기 위해 PlatformIO 레지스트리 사용하도록 설정 변경
   - **코드 수정**: `main.cpp` 내 함수 선언(`printHelpMenu`, `performRuntimeMemoryTest`) 누락 문제 해결 (Forward Declaration 추가)
4. **빌드 검증**
   - `pio run` 명령을 통한 펌웨어 컴파일 성공 (`firmware.bin` 생성 확인)
5. **가이드 문서 작성**
   - `platformio_best_practices.md` 작성: 향후 멀티 보드 환경 관리를 위한 가이드 제공

### 향후 계획
- 시리얼 모니터를 통한 하드웨어 동작 검증
- 다양한 보드(Nano, UNO R4 등)에 대한 멀티 환경 설정 확장

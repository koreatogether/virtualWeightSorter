# 18B20_multiple_serial_control

간단 설명:
- 여러 DS18B20 센서를 시리얼 콘솔로 제어·모니터링하는 예제 프로젝트입니다.
- `.github`의 코딩 규칙·빌드 가이드를 준수합니다(특히 DallasTemperature, OneWire 권장).

구성 가이드:
- 본 폴더는 프로젝트 템플릿(`00_Requirements` ~ `80_Release`)을 따릅니다.
- 먼저 하드웨어 연결(핀, 풀업 저항)과 펌웨어(예제 스케치)를 준비하세요.

## 현재 상태 (Status)
- **2026-02-13**: PlatformIO 환경 구축 완료. ESP32-C3 XIAO 보드에 대해 컴파일 성공.
- 프로젝트 구조가 PlatformIO 표준(`src/` 폴더 기반)으로 변경되었습니다.

## 빌드 가이드 (PlatformIO)
본 프로젝트는 PlatformIO를 사용하여 빌드 및 업로드할 수 있습니다.

### 주요 명령어
- **컴파일**: `pio run`
- **업로드**: `pio run --target upload`
- **시리얼 모니터**: `pio device monitor`

### 보드 설정 (`platformio.ini`)
- 타겟 보드: `seeed_xiao_esp32c3`
- 프레임워크: `arduino`
- 라이브러리: `OneWire`, `DallasTemperature` (자동 설치됨)

## 다음 작업(권장)
1. `30_Firmware/src` 내의 코드 로직 고도화
2. `20_Hardware`에 회로도·핀맵 문서 추가
3. 시리얼 모니터를 통한 실제 센서 데이터 수신 확인

문서/코드 작성 시 `.github/COPILOT_INSTRUCTIONS.md`의 코딩 규칙을 따르세요.
코딩 가이드라인: `E:\project\04_P_dht22_monitoring\docs\AI_CODING_GUIDELINES.md`
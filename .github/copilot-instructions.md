# GitHub Copilot — 저장소 적응 가이드 (간결)

목표: 이 저장소에서 AI 코딩 에이전트가 즉시 생산적으로 작업할 수 있도록 핵심 컨텍스트, 워크플로우, 규칙을 간결하게 정리합니다.

## 한눈에 보기
- 이 저장소는 **임베디드(Arduino/PlatformIO)** 프로젝트와 **Python 데이터/시뮬레이터/도구**를 함께 보관합니다.
- 주요 폴더: `10_Components_Basic/`, `20_Systems_Monitoring/`, `35_Projects/` — 하드웨어(센서/보드) → 펌웨어(Arduino/PlatformIO) → 백엔드/Python(수집·처리) 흐름.

## 시작할 때(필수)
1. 해당 서브프로젝트의 `README.md`를 항상 먼저 읽습니다 (예: `10_Components_Basic/...`, `20_Systems_Monitoring/...`).
2. 빌드 방식 확인: `platformio.ini` 가 있으면 PlatformIO; 없으면 `Arduino CLI` 사용(`.github/docs/ARDUINO_CLI_GUIDE.md` 참고).
3. 서브프로젝트의 `.github/copilot-instructions.md` 또는 유사 문서(예: `.../01_ds18b20_flow_12/.github/copilot-instructions.md`)가 있으면 **우선 준수**합니다. 일부 서브프로젝트는 Codacy/보안 검사처럼 CRITICAL 규칙을 정의합니다.

## 자주 쓰는 명령(예시)
- Arduino CLI 컴파일: `arduino-cli compile -b arduino:renesas_uno:unor4wifi <스케치_경로>`
- Arduino CLI 업로드: `arduino-cli upload -b arduino:renesas_uno:unor4wifi -p COM<번호> <스케치_경로>`
- PlatformIO 빌드: `pio run -e <env>`
- PlatformIO 업로드: `pio run -t upload -e <env>`
- PlatformIO 테스트: `pio test -e native` (서브프로젝트 문서 참조)
- Python: `pytest` (venv/pyproject 확인)

## 코드/스타일 규칙(프로젝트 관행)
- 들여쓰기: **space 4칸**, 중괄호: **Allman 스타일**
- 명명: 변수/함수 `camelCase`, 클래스 `PascalCase`, 상수 `UPPER_SNAKE_CASE` (상수는 `const`/`constexpr` 사용)
- 주석: **한국어**로 작성. 파일 상단에 목적·핀맵·전제조건 명시.
- 실시간/하드웨어 제어: `loop()`는 오케스트레이션만, 긴 `delay()` 금지 → `millis()` 기반 비동기 또는 `TaskScheduler` 사용
- 매직 넘버 금지: 핀/타임아웃 등은 상수로 정의

## 테스트·검증 · 보안
- 수정 후 관련 환경(PlatformIO env, Arduino 보드, Python 테스트)을 빌드/테스트 해야 합니다.
- 서브프로젝트 규칙(Codacy/Trivy 등)이 있으면 **즉시** 해당 스캔을 실행하고, 취약점 발견 시 수정 후 계속 진행합니다 (예: DS18B20의 `.github/copilot-instructions.md` 사례).

## 통합·외부 의존성
- 라이브러리 의존성은 `platformio.ini`(lib_deps) 또는 Arduino 라이브러리로 관리됩니다. 의존성 추가 시 빌드·보안 스캔을 병행하세요.

## 문서화 및 PR 메시지
- 큰 변경(리팩토링, API 변경)은 `README.md`/changelog와 함께 요약 커밋 메시지를 남기세요.
- 작업 완료 시 간결한 한국어 요약(무엇을 변경했고 왜 그런지) 제공.

## 예시 참조 파일
- 빌드 가이드: `.github/docs/ARDUINO_CLI_GUIDE.md`
- DS18B20 예제 및 Codacy 규칙: `10_Components_Basic/Sensors/DS18B20_Basic/`
- 플랫폼별 README(PlatformIO 예시 다수): 각 서브프로젝트의 `README.md`

---
질문이나 보강할 항목이 있으면 알려주세요. (특히 특정 서브프로젝트에 대해 더 자세한 지침을 원하시면 어떤 프로젝트인지 알려주세요.)
# Progress Log - Boiler 4x Temp Monitoring

## [2026-02-07 15:00] 센서 표시창 클릭을 통한 그래프 선 On/Off 기능 추가

### 진행 내용
- **인터랙티브 온도 박스**: 사이드바의 센서별 온도 표시창(S1~S4)에 클릭 이벤트를 추가하여 그래프 선의 표시 여부를 토글할 수 있게 함.
- **시각적 피드백**: 특정 센서를 비활성화(Hide)하면 해당 온도 박스가 반투명하게 변하고 그레이스케일 처리가 되어 현재 상태를 직관적으로 확인 가능.
- **Chart.js 연동**: 별도의 범례(Legend) 조작 없이도 UI에서 직접 차트의 `dataset` 가시성을 제어하도록 구현.

### 주요 수정 파일
- `tools/analyzer_integrated/analyzer_ui.html`: `temp-box` 스타일 수정 및 `toggleDataset()` 함수 추가.

---

## [2026-02-07 14:40] 분석기 커스텀 시간 범위(Custom Range) 줌 기능 추가

### 진행 내용
- **특정 시간대 직접 입력 구현**: 사용자가 `HHMM-HHMM` (예: `2230-2245`) 형식으로 입력하면 해당 구간으로 그래프를 즉시 확대하는 기능 추가.
- **지능형 시간 파싱**: `630-830`과 같이 짧게 입력해도 `06:30-08:30`으로 자동 해석하여 24시간제 기준으로 처리.
- **모드별 컨텍스트 연동**: 라이브 모드에서는 현재 날짜(오늘), 아카이브 모드에서는 선택된 과거 날짜를 기준으로 시간대 자동 적용.
- **커스텀 오버레이 표시**: 커스텀 범위 적용 시 상단 오버레이에 `CUSTOM RANGE: YYYY-MM-DD HHMM - HHMM` 정보를 표시하여 사용자 피드백 강화.
- **UX 개선**: 텍스트 입력 후 'Go' 버튼 클릭 시 기존 1h/6h 퀵 버튼 활성 상태를 해제하여 현재 상태를 명확히 함.

### 주요 수정 파일
- `tools/analyzer_integrated/analyzer_ui.html`: 커스텀 입력 UI 추가 및 `applyCustomTimeRange()` 로직 구현.

---

## [2026-02-07 14:20] 분석기 히스토리 줌(Zoom) 기능 고도화 및 환경 설정 추가

### 진행 내용
- **아카이브 모드 유동 줌(Dynamic Zoom) 구현**: 과거 로그 선택 시에도 라이브 모드처럼 6시간/1시간 버튼에 맞춘 타임 윈도우 기반 줌 기능이 작동하도록 수정. (파일의 마지막 타임스탬프를 기준점으로 자동 계산)
- **빈 데이터 구간 줌 최적화**: 그래프 시작 전 불필요하게 긴 빈 시간(Empty space)을 방지하기 위해, 실제 데이터 시작 시점보다 1시간 이전까지만 그래프가 표시되도록 자르는 로직 적용.
- **오버레이 정보 개선**: 상단 오버레이에 현재 줌 구간 내 실제 데이터 레코드 수(`(표시수/전체수)`)를 실시간으로 표시하여 가독성 증대.
- **PlatformIO 환경 추가**: `mainRev08_seeed_esp32C3_04` 버전을 위한 `[env:xiao_esp32c3_rev08_04]` 환경 설정을 `platformio.ini`에 추가.

### 주요 수정 파일
- `tools/analyzer_integrated/analyzer_ui.html`: `resetZoom` 로직 고도화로 아카이브 모드 줌 및 빈 구간 최적화 구현.
- `platformio.ini`: `mainRev08_..._04` 빌드 환경 신규 추가.

---

## [2026-02-06 22:30] 통합 분석기 v2.1 — UI 완성 및 차트 인터랙션 강화

### 진행 내용
- **크로스헤어 + 인덱스 툴팁 복원**: 마우스 위치에 수직선을 그리고 4채널 센서값을 동시에 표시하는 `crosshairPlugin` 커스텀 플러그인 구현.
- **ΔT 차이값 표시**: 툴팁 하단에 `S1-S2 ΔT`, `S3-S4 ΔT` 값을 자동 계산하여 표시 (`afterBody` 콜백).
- **상/하한 이중 점선**: Upper(빨강 점선)/Lower(초록 점선) 2개 라인을 분리 설정할 수 있도록 UI와 로직 변경.
- **뷰 버튼 활성 상태 반전**: 6시간/1시간 버튼 클릭 시 `.active` 클래스 토글로 현재 선택 상태 시각적 피드백 제공.
- **Y축 자동 스케일링**: 기존 고정 범위(0~100) 제거, `grace: '10%'`로 데이터 밀착 표시.
- **Smart X축 줌**: 수집 시작 직후 데이터가 적을 때 빈 6시간을 그대로 보여주지 않고 데이터 범위에 밀착.
- **반응형 레이아웃**: 세로 모니터(1080×1920) 대응. 뷰포트 1200px 이하에서 사이드바/패딩/레전드 자동 축소.
- **오버레이 가독성 개선**: 반투명 배경 추가로 레전드와 겹침 방지.

### 버그 수정
- **스크립트 구조 깨짐 복구**: `updateAnalysis()`, `setViewRange()`, `resetZoom()`, `applyThreshold()`, `window.onload`가 `updateChart()` 함수 안에 중첩되어 있던 구조적 결함을 완전히 복구.
- **annotation 프로퍼티 누락**: Chart.js 초기화 시 `annotation: { annotations: {} }` 프로퍼티가 빠져 있어 임계치 선이 작동하지 않던 문제 수정.

### 주요 수정 파일
- `tools/analyzer_integrated/analyzer_ui.html`: 크로스헤어, ΔT 툴팁, 상/하한 분리, 반응형, 뷰 버튼 토글 등 전면 개선.

---

## [2026-02-06 22:10] 통합 분석기(Integrated Analyzer) v2.0 배포 및 도구 통합

### 진행 내용
- **도구 통합**: 데이터 수집(Python), 시각화(Chart.js), 실행(Batch) 기능을 하나로 통합한 `analyzer_integrated` 도구의 근본적인 연결 및 호환성 문제 해결.
- **연결 버그 수정**: ESP32C3의 실제 IP(`192.168.0.150`) 반영 및 `requests` 라이브러리 전환으로 수집 안정성 확보.
- **데이터 호환성 해결**: `ts`(타임스탬프) 필드가 없는 기존 로그 파일들을 읽을 때 파일명에서 시간을 추출하여 그래프에 표시하는 로직 구현.
- **환경 정리**: `run_analyzer.bat` 실행 시 기존에 좀비화된 Python 프로세스를 자동으로 종료하도록 개선.

### 주요 수정 파일
- `run_analyzer.bat`: 루트 디렉토리로 이동 및 프로세스 정리 로직 추가.
- `tools/analyzer_integrated/analyzer_server.py`: IP 수정, 타임스탬프 주입, 구형 로그 호환 로직.
- `tools/analyzer_integrated/analyzer_ui.html`: 아카이브 모드 UI 개선 및 데이터 부재 구간 시각화.

## [2026-02-05 00:20] Seeed XIAO ESP32C3 코드 수정 및 재업로드

### 진행 내용
- **코드 수정 반영**: 사용자가 수정한 `main.cpp`를 반영하여 빌드 및 업로드를 재실행함.
- **업로드 실행**: `pio run -t upload -e xiao_esp32c3_rev08 --upload-port COM6` 명령으로 업로드 완료.
- **결과**: 업로드 성공.

## [2026-02-05 00:20] Seeed XIAO ESP32C3 환경 설정 및 첫 업로드 성공

### 진행 내용
- **COM 포트 확인**: `pio device list`를 통해 `COM6`가 XIAO ESP32C3임을 확인함.
- **업로드 실행**: `pio run -t upload -e xiao_esp32c3_rev08 --upload-port COM6` 명령으로 Blink 예제 업로드를 수행함.
- **빌드 및 업로드 환경**: `platformio.ini`의 `espressif32@6.7.0` 환경을 사용하여 성공적으로 마무리함.

### 설정 정보 (platformio.ini)
```ini
[env:xiao_esp32c3_rev08]
platform = espressif32@6.7.0
board = seeed_xiao_esp32c3
framework = arduino
build_src_filter = -<*> +<main/mainRev08_seeed_esp32C3_01/>
lib_deps =
monitor_speed = 115200
lib_extra_dirs = 
build_flags =
    -DCORE_DEBUG_LEVEL=0
```

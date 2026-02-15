# Integrated Analyzer Guide

Boiler 4x Monitoring System은 대용량 온도 데이터를 효율적으로 수집하고 분석하기 위한 **Python 기반 통합 분석 도구(Integrated Analyzer)**를 제공합니다.

---

## 1. Getting Started

### 1.1 Execution
프로젝트 루트 디렉토리의 배치 파일을 실행하면 모든 준비가 완료됩니다.

```bash
run_analyzer.bat
```
이 스크립트는 다음 작업을 순차적으로 수행합니다:
1.  기존에 실행 중인 분석기 프로세스 종료 (포트 충돌 방지)
2.  Python 가상환경 활성화
3.  `analyzer_server.exe` (또는 .py) 실행
4.  기본 웹 브라우저 자동 실행 (`http://localhost:8080`)

### 1.2 Dashboard Features
*   **Live Monitoring**: 최근 6시간의 온도 데이터를 실시간으로 업데이트.
*   **History Browser**: 저장된 과거 데이터를 날짜별로 선택하여 조회.
*   **Interactive Chart**: 마우스 휠로 줌(Zoom), 드래그로 팬(Pan) 조작 지원.
*   **Custom Range**: 특정 시간대(예: 22:30 ~ 23:00)만 집중 분석 가능.
*   **Threshold Lines**: 상한(Upper)/하한(Lower) 관리선을 설정하여 이탈 여부 시각화.

---

## 2. Performance Optimization Report (v2.0)

### 2.1 The Problem
기존 버전은 2초마다 새로운 JSON 파일을 생성했습니다. 하루에 **43,200개**의 파일이 생성되었으며, 이를 로딩할 때 심각한 성능 저하가 발생했습니다.

*   **증상**: 대시보드 로딩 10초 이상 소요, 브라우저 멈춤.
*   **원인**: OS의 파일 오픈(Open) 오버헤드 + 대량의 JSON 파싱 부하.

### 2.2 The Solution: JSONL & Downsampling
v2.0에서는 저장 방식과 전송 방식을 혁신적으로 개선했습니다.

1.  **JSONL (JSON Lines) Storage**:
    *   하루치 데이터를 **단 하나의 파일**(`YYYY-MM-DD.jsonl`)에 저장합니다.
    *   파일 I/O 횟수를 1/40,000 수준으로 줄였습니다.
2.  **Server-side Downsampling**:
    *   서버가 데이터를 읽을 때, 브라우저 해상도(약 1920px)를 고려하여 **최대 1,000 포인트**로 데이터를 요약(Sampling)합니다.
    *   데이터의 경향성은 유지하면서 렌더링 부하를 90% 이상 감소시켰습니다.

### 2.3 Benchmark Result
| 구분 | 기존 (v1.0) | 개선 (v2.0) | 향상 |
| :--- | :--- | :--- | :--- |
| **파일 수 (1일 기준)** | 43,200개 | **1개** | **99.9% 감소** |
| **데이터 로딩 시간** | ~12초 | **< 0.2초** | **60배 단축** |
| **브라우저 메모리** | ~200MB | **~30MB** | **85% 절감** |

---

## 3. Data Structure

### 3.1 Log File Format
로그 파일은 `docs/logs` 디렉토리에 저장됩니다.

```jsonl
{"s1": 25.0, "s2": 24.5, "s3": 60.2, "s4": 40.1, "ts": 1707361200000}
{"s1": 25.1, "s2": 24.6, "s3": 60.3, "s4": 40.2, "ts": 1707361202000}
...
```
*   **s1~s4**: 각 센서의 온도값 (float)
*   **ts**: Unix Timestamp (ms), 한국 표준시(KST) 기준

### 3.2 Timezone Handling
데이터 수집 및 마이그레이션 시 **한국 표준시(UTC+9)**를 기준으로 타임스탬프를 보정하여 저장합니다. 이를 통해 그래프 상의 시간축이 실제 현지 시간과 정확히 일치합니다.

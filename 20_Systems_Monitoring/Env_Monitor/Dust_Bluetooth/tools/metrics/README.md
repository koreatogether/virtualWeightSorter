Integrated Metrics - README
# Metrics 도구 안내 (Windows / PowerShell 예제 포함)

이 디렉토리(`tools/metrics`)에는 프로젝트 품질 분석을 위한 여러 스크립트가 들어 있습니다.
목적에 따라 적절한 도구를 골라 사용하세요. 아래에는 각 스크립트의 권장 사용 시나리오와 간단한 실행 예제가 정리되어 있습니다.

## 어떤 도구를 언제 써야 하나요? (요약)

- 전체 통합 분석(권장, CI/릴리스용): `unified_metrics.py`
- 로컬 빠른 점검(간단 실행): `run_metrics_simple.py`
- Python 전용 심층 분석(coverage, radon, pylint 등): `python_coverage.py`
- Arduino 전용 분석: `arduino_metrics.py`
- 레거시 통합 관리자(참고용): `integrated_metrics.py` (현재는 대체됨)

---

## 1) 통합 리포트 (권장)

- 파일: `unified_metrics.py`
- 용도: 여러 수집기(collectors)를 조합해 점수화, 트렌드 비교, JSON/MD/HTML 익스포트까지 수행합니다. CI나 릴리스 리포트에 적합합니다.

PowerShell 예:

```powershell
& .\.venv\Scripts\Activate.ps1
python .\tools\metrics\unified_metrics.py --full --format json,md,html --output-dir tools/metrics/reports
```

옵션 요약:
- `--quick` (빠른 하위 집합)
- `--full` (전체 분석)
- `--domain-only` (도메인 관련 수집기만)
- `--format json,md,html` (출력 형식 지정)

산출물: `tools/metrics/reports/` 디렉터리에 `metrics_result_*.json`, 마크다운과 HTML(선택 시) 등이 생성됩니다.

---

## 2) 로컬 빠른 점검

- 파일: `run_metrics_simple.py`
- 용도: Windows 환경에서 바로 실행해 pytest 출력, radon 결과, 간단한 Arduino 파일 통계를 얻고 마크다운 리포트를 생성합니다. 개발 중 빠른 확인용.

PowerShell 예:

```powershell
& .\.venv\Scripts\Activate.ps1
python .\tools\metrics\run_metrics_simple.py
```

출력: `tools/metrics/reports/simple_metrics_*.md` 및 coverage HTML(있을 경우) 등

---

## 3) Python 전용 심층 분석

- 파일: `python_coverage.py`
- 용도: pytest(및 pytest-cov)를 통한 테스트/커버리지 수집, radon(복잡도), pylint(설정에 따라) 등을 실행해 `python_metrics_*.json` 및 `coverage.json`을 생성합니다.

PowerShell 예:

```powershell
& .\.venv\Scripts\Activate.ps1
python .\tools\metrics\python_coverage.py
```

유의사항: coverage JSON이 빈 경우(외부 툴 문제 등) stderr_tail이나 경고(`coverage_fallback`)가 리포트에 남습니다. 이 경우 `tools/metrics/reports/coverage.json`을 확인하세요.

---

## 4) Arduino 전용 분석

- 파일: `arduino_metrics.py`
- 용도: `src/arduino` 하위의 ino/cpp/h 파일을 읽어 라인 수, 주석 비율, 단순 복잡도/플래시·램 추정 등 요약을 제공합니다.

PowerShell 예:

```powershell
& .\.venv\Scripts\Activate.ps1
python .\tools\metrics\arduino_metrics.py
```

---

## 5) 레거시 통합 스크립트(참고용)

- 파일: `integrated_metrics.py`
- 설명: 과거 통합 관리자 스크립트로 현재는 `unified_metrics.py`를 사용하는 것이 권장됩니다. 내부 동작이나 대시보드 생성 방식 참고용으로 유지되어 있습니다.

---

## 사전 준비(권장)

1. 가상환경 활성화

```powershell
& .\.venv\Scripts\Activate.ps1
```

2. 의존성 설치

```powershell
& .\.venv\Scripts\Activate.ps1
pip install -r .\tools\metrics\requirements_metrics.txt
```

3. 선택적으로 `cloc` 설치 (Windows에서 Chocolatey 사용 권장)

```powershell
choco install cloc -y
```

## 출력 위치

- 기본 출력 디렉터리: `tools/metrics/reports/`
- 여기에는 JSON 리포트(`metrics_result_*.json`, `python_metrics_*.json` 등), `coverage.json`, `coverage_html/` 및 이미지/마크다운 리포트가 생성됩니다.

## 문제 발생 시 확인 포인트

- 커버리지 값이 0% 또는 빈 리포트일 때:
  - `tools/metrics/reports/coverage.json` 파일의 `files` 필드와 `stderr_tail`를 확인
  - pytest가 외부 의존성 또는 테스트 초기화 실패로 종료했는지 `pytest` stderr를 확인
- radon/pylint 등 분석 도구가 없으면 설치를 권장 (`pip install radon pylint`)
- Windows 콘솔 인코딩 문제로 출력이 깨지면 PowerShell에서 UTF-8 모드를 활성화하거나 스크립트 내의 인코딩 옵션을 확인

## CI 통합 권장

- GitHub Actions에서 `unified_metrics.py --full --format json,md,html`를 실행하고 `tools/metrics/reports/` 디렉터리를 artifact로 보관하면 자동 리포트 및 트렌드 추적이 가능합니다.

---

필요하시면 이 README에 더 상세한 예(예: `unified_metrics.py`의 출력 예시 JSON 요약, CI 워크플로 샘플)를 추가해 드릴게요.

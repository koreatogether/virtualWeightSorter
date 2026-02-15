# Tools 폴더 안내

이 리포지토리의 `tools/` 폴더에는 개발, 품질분석, 보안검사, 데모, 그리고 테스트 헬퍼 스크립트가 포함되어 있습니다.
아래는 각 하위 디렉토리와 주요 파일의 용도, 그리고 현 프로젝트 상황에서 유지(Keep) / 선택적 보존(Keep-Optional) / 제거(Remove) 권장 여부입니다.

---

## 요약 (한눈에)

- 유지 권장(프로덕션/CI에서 유용): `tools/metrics`, `tools/quality`, `tools/test_environment`, `tools/security`, `tools/run_all_checks.py`
- 선택적 보존(확장성/CI 향후 사용): `tools/demos`, `tools/reports`
- 제거 고려(불필요하거나 중복): 없음(현재 모두 향후 검증/디버깅에 유의미함)

---

## 하위 디렉토리별 상세

### `tools/demos/`
- 주요 파일:
  - `run_phase1_demo.py`
- 용도: 오프라인 시뮬레이터와 Dash 앱을 함께 실행하는 데모 스크립트로, 로컬 데모 또는 개발 시나리오에 유용합니다.
- 권장: Keep-Optional — 개발 데모용으로 유용하므로 보존 권장.

### `tools/metrics/`
- 주요 파일/폴더: `unified_metrics.py`, `run_metrics_simple.py`, `python_coverage.py`, `arduino_metrics.py`, `reports/`, `requirements_metrics.txt`
- 용도: 코드 품질/커버리지/메트릭 수집 및 리포트 생성. CI 리포트용으로 설계되어 있음.
- 권장: Keep — CI/릴리스 리포트에 유용하므로 유지 권장.

### `tools/quality/`
- 주요 파일: `quality_check.py`, `arduino_check.py`, `auto_fix.py`
- 용도: Black/Ruff/MyPy/pytest 등 통합 코드 품질 검사 및 자동 수리 도구.
- 권장: Keep — 개발 품질 검사에 필수적.

### `tools/reports/`
- 주요 파일: `integrated_report_*.json`, `coverage.json`, `htmlcov/`
- 용도: 메트릭/품질 검사의 결과를 보관하는 아카이브 디렉토리입니다.
- 권장: Keep-Optional — 기록용으로 유지하되, 오래된 리포트는 보관 정책에 따라 정리 가능.

### `tools/security/`
- 주요 파일: `trufflehog_check.py`, `bandit-report.json`, `security_scan_*.json`
- 용도: 보안/비밀정보 검사 스크립트 및 결과
- 권장: Keep — 보안 검사는 중요하므로 유지 권장.

### `tools/test_environment/`
- 주요 파일: `test_environment.py`, `check_arduino_libraries.py`, `setup_env.bat`, `TROUBLESHOOTING.md`
- 용도: 개발 환경(로컬) 점검 및 준비를 위한 헬퍼
- 권장: Keep — 신규 협업자 온보딩/환경 문제 해결에 유용

### `tools/tests/` (정리된 테스트 스크립트)
- 하위 분류: `integration/`, `ui/`, `serial/`, `misc/`
- 용도: 루트에 흩어져 있던 테스트 스크립트를 분류한 위치입니다. 각 파일은 해당 도메인별로 빠른 로컬 검증 또는 수동/자동 테스트에 사용됩니다.
- 권장: Keep — 테스트는 유지하되 pytest 통합/CI 스크립트에서 경로를 갱신 필요(아래 권장 참조).

---

## 권장 작업(우선순위)

1. CI/pytest 설정 업데이트
   - 현재 루트에 있던 테스트들을 `tools/tests/`로 이동했으므로 `pytest.ini` 또는 CI 워크플로(예: `.github/workflows/*`)에서 테스트 경로를 `tools/tests`로 포함하도록 업데이트하세요.
2. 보관 정책 수립
   - `tools/reports/`와 `tools/security/`의 오래된 JSON 리포트는 연도별 아카이브로 이동하거나 S3 같은 외부 스토리지로 이관 권장.
3. 문서화
   - `tools/README.md`(이 파일) 외에 `tools/metrics/README.md`(이미 존재) 및 `tools/quality/README.md` 를 보강하면 신규 기여자에게 도움이 됩니다.

---

필요하시면 다음 작업을 자동으로 해드립니다:
- `pytest.ini` 생성 또는 수정하여 `tools/tests` 포함
- `tools/README.md`에 CI 실행 예시 추가
- 오래된 리포트 자동 아카이브 스크립트 추가

간단 체크: 유지(Keep) / 선택적 보존(Keep-Optional) / 제거(Remove)


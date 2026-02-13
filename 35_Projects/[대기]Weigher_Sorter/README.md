# Arduino R4 Minima 중량 선별기 시뮬레이터

이 저장소는 Arduino R4 Minima 보드를 대상으로 한 조합형 중량 선별기(weigher-sorter) 시뮬레이터와 관련 유틸리티를 포함합니다.

## 개발 환경 활성화

아래는 가상 환경(또는 스크립트)을 활성화하는 방법입니다.

### 1) Windows (명령 프롬프트)
```bat
activate_env.bat
```

### 2) PowerShell
```powershell
.\activate_env.ps1
```

### 3) 테스트 실행(예)
```powershell
uv run python src/python/test_runner.py
```

## 자주 사용하는 명령

다음 명령들은 프로젝트의 주요 모듈을 실행하거나 테스트할 때 사용합니다.

```bash
# 테스트 러너 실행
uv run run.py test_runner

# 시뮬레이터와 개별 모듈 실행
uv run run.py simulator    # Arduino 시뮬레이터 실행
uv run run.py generators   # 난수/샘플 생성기 실행
uv run run.py algorithms   # 조합 알고리즘 실행
uv run run.py monitor      # 성능 모니터 실행
```

## 프로젝트 구조

```
06_weigher-sorter/
├─ src/python/
│  ├─ arduino_r4_simulator.py    # Arduino R4 Minima 시뮬레이터 엔트리
│  ├─ random_generators.py       # 무게/데이터 생성 유틸리티
│  ├─ combination_algorithms.py  # 조합 알고리즘 구현
│  ├─ performance_monitor.py     # 성능 측정 및 로깅
│  └─ test_runner.py             # 테스트/시나리오 실행기
├─ .venv/                         # (선택) 가상환경 디렉터리
├─ activate_env.bat               # Windows용 가상환경 활성화 스크립트
├─ activate_env.ps1               # PowerShell용 가상환경 활성화 스크립트
├─ run.py                         # 편의 실행 래퍼
└─ pyproject.toml                 # 의존성/빌드 설정
```

## 주요 기능 요약

- 다양한 난수 생성기 제공: Random, XorShift32, Hybrid 등
- 조합 탐색 알고리즘: 랜덤 샘플링, 그리디 + 로컬 서치, 동적 계획법 등
- Arduino R4 Minima 하드웨어 타겟 지원(시뮬레이션)
- 성능 모니터링 및 테스트 러너 포함

## 하드웨어/성능(참고)

- Arduino R4 Minima(예시): 32KB RAM, 256KB Flash 등 제한된 자원 환경을 가정
- 실제 하드웨어에서 동작시키려면 로드셀(HX711) 등 센서 인터페이스 구현 필요

## 빠른 시작

1. 가상환경 활성화 (Windows/Powershell 중 사용환경에 맞게)
2. 필요한 의존성 설치 (pyproject.toml 또는 프로젝트 설명에 따름)
3. 시뮬레이터 또는 테스트 러너 실행

---

필요하면 이 README에 더 많은 사용 예시(명령 예시, 시나리오, 기여 방법)를 추가해 드리겠습니다.
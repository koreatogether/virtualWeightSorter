# IoT 모니터링 시스템 프로젝트 세팅 템플릿

## 개요

이 템플릿은 DS18B20 온도 모니터링 시스템 프로젝트에서 검증된 세팅 과정을 기반으로 작성되었습니다. IoT 센서 모니터링 시스템이나 유사한 Python + Dash 기반 프로젝트를 시작할 때 이 템플릿을 참조하여 체계적인 개발 환경을 구축할 수 있습니다.

## 📋 프로젝트 세팅 체크리스트

### Phase 0: 프로젝트 초기화 (2-3시간)

#### 1. 프로젝트 기본 구조 생성
```bash
# 프로젝트 폴더 생성
mkdir [project_name]
cd [project_name]

# 기본 폴더 구조 생성
mkdir docs docs/architecture docs/development docs/template
mkdir src src/python src/arduino_mock
mkdir tools tools/metrics tools/quality tools/security tools/test_environment
mkdir tests
mkdir .kiro .kiro/specs .kiro/specs/[project_name]
```

#### 2. Git 저장소 초기화
```bash
# Git 초기화
git init

# GitHub 저장소 생성 후 연결
git remote add origin https://github.com/[username]/[repository_name].git
```

#### 3. 기본 설정 파일 생성
- [ ] `.gitignore` - Python, IDE, OS 관련 파일 제외
- [ ] `README.md` - 프로젝트 개요 및 사용법
- [ ] `requirements.txt` - 필수 패키지 목록 (초기 버전)

### Phase 1: 개발 환경 구축 (3-4시간)

#### 1. Python 환경 설정
```bash
# Python 3.10 기반 uv 환경 구축
uv init --python 3.10

# 필수 패키지 설치 (프로젝트에 맞게 조정)
uv add dash plotly pandas pyserial dash-bootstrap-components

# 개발 도구 설치
uv add --dev pytest pytest-cov ruff mypy radon safety
```

#### 2. pyproject.toml 설정
- [ ] 프로젝트 메타데이터 설정
- [ ] 개발 도구 설정 (ruff, mypy, pytest, coverage)
- [ ] 의존성 그룹 정의

#### 3. 개발 도구 구성
- [ ] `tools/run_all_checks.py` - 통합 품질 검사
- [ ] `tools/metrics/` - 코드 메트릭스 도구
- [ ] `tools/quality/` - 코드 품질 검사 도구
- [ ] `tools/security/` - 보안 검사 도구

### Phase 2: 환경 테스트 시스템 구축 (4-6시간)

#### 1. 종합 환경 테스트 도구
- [ ] `tools/test_environment/test_environment.py` - 27개 항목 종합 테스트
- [ ] `tools/test_environment/check_[hardware]_libraries.py` - 하드웨어 환경 확인
- [ ] `tools/test_environment/TROUBLESHOOTING.md` - 문제 해결 가이드

#### 2. 다중 터미널 지원 스크립트
- [ ] `tools/test_environment/test_env.bat` - Windows CMD/PowerShell
- [ ] `tools/test_environment/test_env.sh` - Git Bash/WSL/Linux
- [ ] `tools/test_environment/setup_env.bat` - Windows 환경 자동 설정

#### 3. 테스트 항목 정의
- Python 환경 (버전, 플랫폼, 가상환경)
- 필수 패키지 (import 테스트 및 버전 확인)
- 개발 도구 (실행 가능성 및 버전 확인)
- 프레임워크 기능 (앱 생성 및 레이아웃 테스트)
- 통신 기능 (시리얼, 네트워크 등)
- 데이터 처리 (JSON, DataFrame 등)
- 파일 시스템 (프로젝트 구조 및 I/O)
- 터미널 환경 (쉘, PATH, 인코딩)

### Phase 3: 아키텍처 설계 및 문서화 (3-5시간)

#### 1. 시스템 아키텍처 문서
- [ ] `docs/architecture/01_architecture.md`
  - 시스템 개요 및 구성 요소
  - 기술 스택 정의
  - 데이터 플로우 설계
  - JSON 통신 프로토콜 정의

#### 2. 개발 가이드 문서
- [ ] `docs/development/00_development_overview.md` - 전체 개발 가이드
- [ ] `docs/development/phase1_basic_system.md` - 기본 시스템 구축
- [ ] `docs/development/phase2_dashboard.md` - 대시보드 개발
- [ ] `docs/development/phase3_advanced_features.md` - 고급 기능

#### 3. 하드웨어 지원 문서 (해당시)
- [ ] `docs/[hardware]_setup.md` - 하드웨어 환경 설정 가이드

### Phase 4: 프로젝트 구조 최적화 (1-2시간)

#### 1. 파일 구조 정리
```
project_root/
├── docs/                    # 문서
│   ├── architecture/        # 시스템 아키텍처
│   ├── development/         # 개발 가이드
│   └── template/           # 템플릿 (이 파일 포함)
├── src/                    # 소스 코드
│   ├── python/             # Python 애플리케이션
│   └── [hardware]_mock/    # 하드웨어 시뮬레이터
├── tools/                  # 개발 도구
│   ├── test_environment/   # 환경 테스트
│   ├── metrics/           # 메트릭스
│   ├── quality/           # 품질 검사
│   └── security/          # 보안 검사
├── tests/                 # 테스트 코드
├── .kiro/                 # Kiro IDE 설정
├── pyproject.toml         # 프로젝트 설정
└── README.md              # 프로젝트 개요
```

#### 2. 도구 체계화
- 환경 테스트 관련 파일들을 전용 폴더로 정리
- 경로 참조 업데이트 및 문서화

## 🔧 핵심 구성 요소

### 1. Python 환경 (필수)
```toml
[project]
name = "[project-name]"
version = "0.1.0"
description = "[프로젝트 설명]"
requires-python = ">=3.10"
dependencies = [
    # 프로젝트에 맞게 조정
    "dash>=2.17.1",
    "plotly>=5.17.0", 
    "pandas>=2.1.4",
    # 추가 패키지...
]

[dependency-groups]
dev = [
    "pytest>=7.4.3",
    "pytest-cov>=4.1.0",
    "ruff>=0.1.6",
    "mypy>=1.7.1",
    "radon>=6.0.1",
    "safety>=2.3.5",
]
```

### 2. 환경 테스트 시스템 (핵심)
```python
# tools/test_environment/test_environment.py 기본 구조
class EnvironmentTester:
    def __init__(self):
        self.test_results = []
        self.failed_tests = []
    
    def test_python_environment(self):
        # Python 버전, 플랫폼, 가상환경 확인
        pass
    
    def test_required_packages(self):
        # 필수 패키지 import 테스트
        pass
    
    def test_development_tools(self):
        # 개발 도구 실행 테스트
        pass
    
    def test_framework_functionality(self):
        # 프레임워크 기본 기능 테스트
        pass
    
    def run_all_tests(self):
        # 모든 테스트 실행 및 결과 요약
        pass
```

### 3. 통합 품질 검사 (권장)
```python
# tools/run_all_checks.py 기본 구조
class IntegratedChecker:
    def run_python_checks(self):
        # Python 코드 품질 검사
        pass
    
    def run_security_checks(self):
        # 보안 검사
        pass
    
    def generate_summary_report(self):
        # 종합 리포트 생성
        pass
```

## 📊 성공 기준

### 환경 테스트 성공률
- **100%**: 완벽한 환경, 개발 시작 가능
- **90-99%**: 대부분 정상, 일부 도구 문제
- **80-89%**: 기본 기능 작동, 도구 재설치 필요
- **70% 미만**: 환경 재설정 필요

### 품질 검사 통과 기준
- 모든 import 성공
- 코드 린팅 통과 (ruff)
- 타입 검사 통과 (mypy)
- 보안 검사 HIGH 이슈 0개
- 기본 기능 테스트 통과

## 🚀 개발 워크플로우

### 일반적인 개발 사이클
```bash
# 1. 환경 테스트 (매번 개발 전)
uv run python tools/test_environment/test_environment.py

# 2. 코드 작성/수정

# 3. 품질 검사
uv run python tools/run_all_checks.py

# 4. 검사 통과 시 커밋
git add .
git commit -m "feat: 새로운 기능 추가"
git push
```

### 브랜치 전략
```
master              # 안정 버전
├── setup          # 환경 설정 (terminal 브랜치 역할)
├── phase1         # Phase 1 개발
├── phase2         # Phase 2 개발
└── phase3         # Phase 3 개발
```

## 📝 릴리즈 노트 템플릿

### 버전 관리 체계
- **v0.1.0**: 프로젝트 초기 설정
- **v0.1.1**: 환경 설정 및 테스트 시스템 완료
- **v0.1.2**: 하드웨어 지원 및 문서화 완료
- **v0.1.3**: 프로젝트 구조 정리 및 도구 체계화

### 릴리즈 노트 구조
```markdown
## v0.x.x - [기능 요약] (YYYY-MM-DD HH:mm:ss)

### 🎉 주요 성과

### 📋 완료된 작업
#### 1. [카테고리]
- ✅ [구체적 작업 내용]

### 🔧 기술 스택 확정/업데이트

### 📊 주요 기능 명세

### 🚀 다음 단계

### 📈 프로젝트 상태
- **현재 단계**: [현재 상태]
- **다음 마일스톤**: [다음 목표]
- **예상 완료**: [예상 시간]
```

## 🛠️ 커스터마이징 가이드

### 프로젝트 타입별 조정사항

#### IoT 센서 모니터링 시스템
- 하드웨어 시뮬레이터 추가
- 시리얼 통신 테스트 포함
- 센서별 환경 확인 스크립트

#### 웹 대시보드 시스템
- 웹 프레임워크 테스트 강화
- UI 컴포넌트 테스트 추가
- 브라우저 호환성 테스트

#### 데이터 분석 시스템
- 데이터 처리 라이브러리 테스트
- 대용량 데이터 처리 테스트
- 시각화 도구 테스트

### 패키지 선택 가이드
```python
# 웹 프레임워크
dash_packages = ["dash", "dash-bootstrap-components", "plotly"]
flask_packages = ["flask", "flask-sqlalchemy", "flask-migrate"]
fastapi_packages = ["fastapi", "uvicorn", "pydantic"]

# 데이터 처리
data_packages = ["pandas", "numpy", "scipy", "scikit-learn"]

# 통신
communication_packages = ["pyserial", "requests", "websockets", "mqtt"]

# 하드웨어 (Arduino)
arduino_packages = ["pyserial", "pyfirmata"]

# 하드웨어 (Raspberry Pi)
rpi_packages = ["RPi.GPIO", "gpiozero", "picamera"]
```

## 📚 참고 자료

### 검증된 설정 파일들
- `pyproject.toml` - 프로젝트 설정 및 도구 구성
- `.gitignore` - Python, IDE, OS 파일 제외 설정
- `requirements.txt` - 패키지 의존성 관리

### 필수 문서 템플릿
- `README.md` - 프로젝트 개요 및 사용법
- `docs/architecture/` - 시스템 아키텍처 문서
- `docs/development/` - 개발 가이드 문서
- `TROUBLESHOOTING.md` - 문제 해결 가이드

### 개발 도구 설정
- Ruff: 코드 린팅 및 포맷팅
- MyPy: 타입 검사
- Pytest: 단위 테스트
- Safety: 보안 취약점 검사
- Radon: 코드 복잡도 분석

## 🎯 예상 소요 시간

| 단계 | 최소 시간 | 최대 시간 | 설명 |
|------|----------|----------|------|
| Phase 0: 프로젝트 초기화 | 2시간 | 3시간 | 기본 구조 및 Git 설정 |
| Phase 1: 개발 환경 구축 | 3시간 | 4시간 | Python 환경 및 도구 설정 |
| Phase 2: 테스트 시스템 구축 | 4시간 | 6시간 | 환경 테스트 및 검증 시스템 |
| Phase 3: 아키텍처 설계 | 3시간 | 5시간 | 문서화 및 설계 |
| Phase 4: 구조 최적화 | 1시간 | 2시간 | 파일 정리 및 체계화 |
| **총 소요 시간** | **13시간** | **20시간** | **완전한 개발 환경 구축** |

이 템플릿을 사용하면 체계적이고 안정적인 프로젝트 개발 환경을 빠르게 구축할 수 있습니다.
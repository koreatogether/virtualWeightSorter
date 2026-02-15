# DS18B20 모니터링 시스템 문제 해결 가이드

## 🚨 환경 테스트 실패 시 해결 방법

### 1. uv 명령을 찾을 수 없는 경우

**증상**: `'uv'은(는) 내부 또는 외부 명령이 아닙니다`

**해결 방법**:
```bash
# PowerShell에서 uv 설치
irm https://astral.sh/uv/install.ps1 | iex

# 또는 pip로 설치
pip install uv

# PATH 환경변수 추가 (Windows)
$env:PATH = "$env:USERPROFILE\.local\bin;$env:PATH"
```

**자동 해결**:
```bash
# 환경 설정 스크립트 실행
setup_env.bat
```

### 2. dash_bootstrap_components 모듈 없음

**증상**: `No module named 'dash_bootstrap_components'`

**해결 방법**:
```bash
# uv로 패키지 추가
uv add dash-bootstrap-components

# 또는 패키지 동기화
uv sync
```

### 3. 개발 도구 실행 실패

**증상**: `[WinError 2] 지정된 파일을 찾을 수 없습니다`

**해결 방법**:
```bash
# 개발 도구 재설치
uv add --dev pytest pytest-cov ruff mypy radon safety

# 또는 전체 동기화
uv sync
```

### 4. Python 버전 불일치

**증상**: 시스템 Python(3.13)과 가상환경 Python(3.10) 버전 다름

**해결 방법**:
```bash
# Python 3.10으로 가상환경 재생성
uv init --python 3.10

# 패키지 재설치
uv sync
```

### 5. 가상환경 활성화 문제

**증상**: `가상환경이 활성화되지 않음`

**해결 방법**:
```bash
# uv 가상환경 사용 (권장)
uv run python test_environment.py

# 또는 수동 활성화 (Windows)
.venv\Scripts\activate

# 또는 수동 활성화 (Linux/Mac)
source .venv/bin/activate
```

## 🖥️ 터미널별 실행 가이드

### PowerShell (권장)
```powershell
# 환경 테스트
uv run python test_environment.py

# 개발 도구 실행
uv run python tools/run_all_checks.py
```

### CMD
```cmd
# 환경 설정 먼저 실행
setup_env.bat

# 환경 테스트
test_env.bat

# 또는 직접 실행
uv run python test_environment.py
```

### Git Bash
```bash
# 실행 권한 설정
chmod +x test_env.sh

# 환경 테스트
./test_env.sh

# 또는 직접 실행
uv run python test_environment.py
```

### WSL/Linux
```bash
# 환경 테스트
./test_env.sh

# Python 3.10 설치 (Ubuntu/Debian)
sudo apt update
sudo apt install python3.10 python3.10-venv

# uv 설치
curl -LsSf https://astral.sh/uv/install.sh | sh
```

## 🔧 완전 재설정 방법

모든 것이 실패할 경우 처음부터 다시 설정:

```bash
# 1. 기존 가상환경 삭제
rmdir /s .venv  # Windows
rm -rf .venv    # Linux/Mac

# 2. uv 재설치
pip uninstall uv
pip install uv

# 3. 프로젝트 재초기화
uv init --python 3.10

# 4. 패키지 설치
uv add dash==2.17.1 dash-bootstrap-components==1.5.0 plotly==5.17.0 pandas==2.1.4 pyserial==3.5

# 5. 개발 도구 설치
uv add --dev pytest==7.4.3 pytest-cov==4.1.0 ruff==0.1.6 mypy==1.7.1 radon==6.0.1 safety==2.3.5

# 6. 환경 테스트
uv run python test_environment.py
```

## 📊 테스트 결과 해석

### 성공률 기준
- **100%**: 완벽한 환경, Phase 1 개발 시작 가능
- **90-99%**: 대부분 정상, 일부 개발 도구 문제
- **80-89%**: 기본 기능 작동, 개발 도구 재설치 필요
- **70-79%**: 주요 패키지 누락, 패키지 재설치 필요
- **70% 미만**: 환경 재설정 필요

### 중요한 테스트 항목
1. **Python 버전**: 3.10 이상 필수
2. **필수 패키지**: dash, plotly, pandas, pyserial, dash_bootstrap_components
3. **가상환경**: uv 가상환경 또는 venv 활성화
4. **개발 도구**: pytest, ruff, mypy (개발 시 필요)

## 🆘 추가 도움

### 로그 확인
```bash
# 상세한 오류 로그 확인
uv run python test_environment.py > test_log.txt 2>&1
```

### 패키지 목록 확인
```bash
# 설치된 패키지 확인
uv pip list

# 의존성 트리 확인
uv pip show dash
```

### 환경 정보 수집
```bash
# 시스템 정보
python -c "import sys; print(sys.version)"
uv --version
pip --version

# 가상환경 정보
echo $VIRTUAL_ENV  # Linux/Mac
echo %VIRTUAL_ENV% # Windows
```

이 가이드를 따라하면 대부분의 환경 문제를 해결할 수 있습니다. 문제가 지속되면 `test_environment.py`의 상세한 오류 메시지를 확인하세요.
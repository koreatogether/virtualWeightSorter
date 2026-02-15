# 환경 테스트 도구

이 폴더는 DS18B20 모니터링 시스템의 개발 환경을 테스트하고 검증하는 도구들을 포함합니다.

## 📁 파일 구성

### 🧪 테스트 스크립트
- **`test_environment.py`** - 종합 환경 테스트 (27개 테스트 항목)
- **`check_arduino_libraries.py`** - Arduino 환경 확인 스크립트

### 🖥️ 터미널별 실행 스크립트
- **`test_env.bat`** - Windows CMD/PowerShell용 테스트 스크립트
- **`test_env.sh`** - Git Bash/WSL/Linux용 테스트 스크립트
- **`setup_env.bat`** - Windows 환경 자동 설정 스크립트

### 📚 문제 해결 가이드
- **`TROUBLESHOOTING.md`** - 상세한 문제 해결 가이드

## 🚀 사용 방법

### 빠른 테스트 (프로젝트 루트에서 실행)
```bash
# PowerShell (권장)
uv run python tools/test_environment/test_environment.py

# CMD
tools\test_environment\test_env.bat

# Git Bash
./tools/test_environment/test_env.sh
```

### Arduino 환경 확인
```bash
uv run python tools/test_environment/check_arduino_libraries.py
```

### 환경 설정 (Windows)
```bash
tools\test_environment\setup_env.bat
```

## 📊 테스트 항목

### Python 환경 테스트
- Python 버전 확인 (3.10+ 필요)
- 플랫폼 정보 확인
- 가상환경 활성화 상태 확인

### 필수 패키지 테스트
- Dash 웹 프레임워크
- Plotly 그래프 라이브러리
- Pandas 데이터 처리
- PySerial 시리얼 통신
- Dash Bootstrap Components

### 개발 도구 테스트
- pytest (단위 테스트)
- ruff (코드 린팅)
- mypy (타입 검사)
- radon (복잡도 분석)
- safety (보안 검사)

### 기능 테스트
- Dash 앱 생성 및 레이아웃
- 시리얼 통신 기능
- JSON 데이터 처리
- 파일 시스템 접근
- 터미널 환경 확인

## 🎯 성공 기준

- **100%**: 완벽한 환경, Phase 1 개발 시작 가능
- **90-99%**: 대부분 정상, 일부 개발 도구 문제
- **80-89%**: 기본 기능 작동, 개발 도구 재설치 필요
- **70-79%**: 주요 패키지 누락, 패키지 재설치 필요
- **70% 미만**: 환경 재설정 필요

## 🆘 문제 해결

문제가 발생하면 `TROUBLESHOOTING.md` 파일을 참조하세요.

### 자주 발생하는 문제
1. **uv 명령을 찾을 수 없음**: `setup_env.bat` 실행
2. **패키지 누락**: `uv sync` 실행
3. **가상환경 문제**: `uv run python` 사용

### 완전 재설정
```bash
# 가상환경 삭제 후 재생성
rmdir /s .venv
uv init --python 3.10
uv sync
```

## 📋 체크리스트

개발 시작 전 확인사항:
- [ ] Python 3.10+ 설치
- [ ] uv 패키지 매니저 설치
- [ ] 모든 필수 패키지 설치
- [ ] 개발 도구 정상 작동
- [ ] 환경 테스트 100% 통과

이 도구들을 사용하여 안정적인 개발 환경을 유지하세요!
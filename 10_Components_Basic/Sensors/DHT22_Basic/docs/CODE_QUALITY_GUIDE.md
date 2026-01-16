# 코드 품질 및 보안 검사 가이드

DHT22 프로젝트의 코드 품질 유지와 보안 강화를 위한 도구 사용법입니다.

## 🛠️ 설치된 도구들

### 코드 품질 도구
- **Black**: Python 코드 자동 포맷팅
- **Ruff**: 빠른 Python 린터 (Flake8, isort 등 통합)
- **MyPy**: 타입 힌트 정적 검사
- **pytest**: 단위 테스트 프레임워크

### 보안 검사 도구
- **Bandit**: Python 보안 취약점 스캔
- **Safety**: 의존성 취약점 검사
- **TruffleHog**: 민감한 정보 누출 검사 (자체 구현)

### 통합 도구
- **pre-commit**: Git 커밋 전 자동 검사

## 🚀 사용법

### 1. 개별 검사 실행

#### Python 코드 품질 검사만
```bash
run_quality_check.bat python
# 또는
python tools/quality/quality_check.py
```

#### Arduino 코드 검사만
```bash
run_quality_check.bat arduino
# 또는
python tools/quality/arduino_check.py
```

#### 보안 검사만
```bash
run_quality_check.bat security
# 또는
python tools/security/trufflehog_check.py
```

### 2. 전체 검사 실행
```bash
run_quality_check.bat all
# 또는
python tools/run_all_checks.py
```

### 3. Pre-commit 사용

#### 설치 (한 번만)
```bash
source .venv/Scripts/activate
uv run pre-commit install
```

#### 커밋 전 자동 검사
```bash
git add .
git commit -m "your message"
# → 자동으로 모든 검사가 실행됩니다
```

#### 수동으로 pre-commit 실행
```bash
uv run pre-commit run --all-files
```

## 📊 검사 항목 상세

### Python 코드 품질 검사
- ✅ Import 순서 및 정렬 (isort)
- ✅ 코드 스타일 (PEP8, Black)
- ✅ 문법 오류 및 안티패턴 (Ruff)
- ✅ 타입 힌트 검사 (MyPy)
- ✅ 단위 테스트 실행 (pytest)

### Arduino 코드 검사
- ✅ 기본 구조 검사 (setup, loop 함수)
- ✅ 문법 오류 검사 (괄호 매칭, 세미콜론)
- ✅ 코딩 스타일 (줄 길이, 공백)
- ✅ 라이브러리 include 검사
- ✅ 보안 검사 (하드코딩된 비밀번호)

### 보안 검사
- 🔒 API 키, 토큰, 패스워드 노출
- 🔒 개인정보 하드코딩
- 🔒 데이터베이스 연결 정보
- 🔒 환경 파일 (.env) 누출
- 🔒 의존성 취약점

## 📝 설정 파일들

### pyproject.toml
```toml
[tool.ruff]
line-length = 88
target-version = "py39"

[tool.black]
line-length = 88
target-version = ['py39']

[tool.mypy]
python_version = "3.9"
disallow_untyped_defs = false  # 대시보드 코드에 대해 완화
```

### .pre-commit-config.yaml
커밋 전 실행되는 훅들의 설정:
- 기본 체크 (YAML, JSON, TOML 문법)
- Black 포맷팅
- Ruff 린팅 및 포맷팅
- MyPy 타입 검사
- Bandit 보안 스캔
- Safety 의존성 검사
- 커스텀 검사 (TruffleHog, Arduino)

## 🔧 도구 호환성

모든 도구들이 서로 충돌하지 않도록 설정되었습니다:

1. **Black과 Ruff**: 같은 줄 길이 (88자) 사용
2. **Ruff의 포맷팅**: Black과 호환되도록 설정
3. **MyPy**: 대시보드 코드에 대해 완화된 설정
4. **Pre-commit**: 모든 도구를 순서대로 실행

## 📈 리포트

검사 결과는 다음 위치에 저장됩니다:
- `tools/quality/reports/` - Python 및 Arduino 품질 검사
- `tools/security/` - 보안 스캔 결과
- `tools/reports/` - 통합 리포트

## ⚡ 빠른 수정

### 자동 수정 가능한 문제들
```bash
# Ruff로 자동 수정
uv run ruff check src/python/ --fix

# Black으로 포맷팅
uv run black src/python/

# 모든 자동 수정 실행
uv run pre-commit run --all-files
```

### 수동 수정이 필요한 문제들
- MyPy 타입 오류
- 보안 취약점
- Arduino 구조적 문제
- 하드코딩된 민감 정보

## 🎯 권장 워크플로우

1. **개발 중**:
   ```bash
   run_quality_check.bat python  # 빠른 Python 검사
   ```

2. **커밋 전**:
   ```bash
   run_quality_check.bat all     # 전체 검사
   git add .
   git commit -m "message"       # pre-commit 자동 실행
   ```

3. **주기적으로**:
   ```bash
   uv run safety check           # 의존성 취약점 확인
   ```

## 🚨 문제 해결

### pre-commit 실패 시
1. 자동 수정 시도: `uv run ruff check --fix`
2. 수동 수정 후 다시 커밋
3. 특정 검사 건너뛰기: `git commit --no-verify`

### 성능 이슈
- MyPy가 느린 경우: `tools/quality/quality_check.py`에서 타임아웃 조정
- pre-commit 속도 개선: `.pre-commit-config.yaml`에서 불필요한 훅 제거

## 📞 지원

이슈가 있으면 다음을 확인하세요:
1. 가상환경이 활성화되어 있는지
2. `uv sync --extra dev`로 개발 의존성 설치 확인
3. `tools/` 디렉토리의 스크립트 실행 권한

---

**💡 팁**: 커밋 전에 항상 `run_quality_check.bat all`을 실행하면 코드 품질을 일정하게 유지할 수 있습니다!

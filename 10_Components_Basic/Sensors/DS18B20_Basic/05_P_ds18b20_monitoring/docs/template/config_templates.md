# 프로젝트 설정 파일 템플릿

DS18B20 온도 모니터링 시스템에서 검증된 설정 파일들의 템플릿입니다.

## 📄 pyproject.toml 템플릿

```toml
[project]
name = "[project-name]"
version = "0.1.0"
description = "[프로젝트 설명 - 예: IoT 센서 모니터링 시스템]"
readme = "README.md"
requires-python = ">=3.10"
authors = [
    {name = "[작성자명]", email = "[이메일]"}
]
keywords = ["[키워드1]", "[키워드2]", "[키워드3]", "monitoring", "iot"]
classifiers = [
    "Development Status :: 3 - Alpha",
    "Intended Audience :: Developers",
    "Topic :: Scientific/Engineering :: Interface Engine/Protocol Translator",
    "License :: OSI Approved :: MIT License",
    "Programming Language :: Python :: 3.10",
]
dependencies = [
    # 웹 프레임워크 (Dash 기반 프로젝트)
    "dash>=2.17.1",
    "dash-bootstrap-components>=1.5.0",
    
    # 데이터 시각화 및 처리
    "plotly>=5.17.0",
    "pandas>=2.1.4",
    
    # 통신 (시리얼, 네트워크 등 - 프로젝트에 맞게 선택)
    "pyserial>=3.5",
    # "requests>=2.31.0",
    # "websockets>=11.0",
    
    # 추가 패키지 (프로젝트 요구사항에 따라)
    # "numpy>=1.24.0",
    # "scipy>=1.10.0",
    # "scikit-learn>=1.3.0",
]

[dependency-groups]
dev = [
    # 테스트 도구
    "pytest>=7.4.3",
    "pytest-cov>=4.1.0",
    
    # 코드 품질 도구
    "ruff>=0.1.6",
    "mypy>=1.7.1",
    
    # 메트릭스 도구
    "radon>=6.0.1",
    
    # 보안 도구
    "safety>=2.3.5",
]

[tool.ruff]
target-version = "py310"
line-length = 88
select = [
    "E",  # pycodestyle errors
    "W",  # pycodestyle warnings
    "F",  # pyflakes
    "I",  # isort
    "B",  # flake8-bugbear
    "C4", # flake8-comprehensions
    "UP", # pyupgrade
]
ignore = [
    "E501",  # line too long, handled by black
    "B008",  # do not perform function calls in argument defaults
    "C901",  # too complex
]

[tool.ruff.per-file-ignores]
"__init__.py" = ["F401"]
"tests/**/*" = ["S101", "D"]

[tool.mypy]
python_version = "3.10"
warn_return_any = true
warn_unused_configs = true
disallow_untyped_defs = true
disallow_incomplete_defs = true
check_untyped_defs = true
disallow_untyped_decorators = true
no_implicit_optional = true
warn_redundant_casts = true
warn_unused_ignores = true
warn_no_return = true
warn_unreachable = true
strict_equality = true

[tool.pytest.ini_options]
minversion = "6.0"
addopts = "-ra -q --strict-markers --strict-config"
testpaths = ["tests"]
python_files = ["test_*.py", "*_test.py"]
python_classes = ["Test*"]
python_functions = ["test_*"]

[tool.coverage.run]
source = ["src"]
omit = [
    "*/tests/*",
    "*/test_*",
    "*/__pycache__/*",
    "*/venv/*",
    "*/.venv/*",
]

[tool.coverage.report]
exclude_lines = [
    "pragma: no cover",
    "def __repr__",
    "if self.debug:",
    "if settings.DEBUG",
    "raise AssertionError",
    "raise NotImplementedError",
    "if 0:",
    "if __name__ == .__main__.:",
    "class .*\\bProtocol\\):",
    "@(abc\\.)?abstractmethod",
]
```

## 📄 .gitignore 템플릿

```gitignore
# Python
__pycache__/
*.py[cod]
*$py.class
*.so
.Python
build/
develop-eggs/
dist/
downloads/
eggs/
.eggs/
lib/
lib64/
parts/
sdist/
var/
wheels/
share/python-wheels/
*.egg-info/
.installed.cfg
*.egg
MANIFEST

# Virtual Environment
.env
.venv
env/
venv/
ENV/
env.bak/
venv.bak/

# IDE
.vscode/
.idea/
*.swp
*.swo
*~

# OS
.DS_Store
.DS_Store?
._*
.Spotlight-V100
.Trashes
ehthumbs.db
Thumbs.db

# Project specific
*.log
*.tmp
temp/
tmp/

# Coverage reports
htmlcov/
.coverage
.coverage.*
coverage.xml
*.cover
.hypothesis/
.pytest_cache/

# MyPy
.mypy_cache/
.dmypy.json
dmypy.json

# Jupyter Notebook
.ipynb_checkpoints

# pyenv
.python-version

# Hardware specific (Arduino, Raspberry Pi 등)
*.hex
*.elf
*.map

# Reports (keep structure but ignore content)
tools/reports/*.json
tools/metrics/reports/*.html
tools/metrics/reports/*.json
tools/metrics/reports/*.md
tools/security/*.json
!tools/security/[security_script].py

# Communication logs
serial_logs/
*.serial

# Data files
data/
*.csv
*.xlsx

# Backup files
*.bak
*.backup
```

## 📄 requirements.txt 템플릿

```txt
# [프로젝트명] - 필수 패키지
# 설계 문서 기준으로 선별된 패키지들

# 웹 애플리케이션 프레임워크
dash==2.17.1
dash-bootstrap-components==1.5.0

# 데이터 시각화 및 그래프
plotly==5.17.0

# 데이터 처리
pandas==2.1.4

# 통신 (프로젝트에 맞게 선택)
pyserial==3.5
# requests==2.31.0
# websockets==11.0

# 개발 및 테스트 도구
pytest==7.4.3
pytest-cov==4.1.0

# 코드 품질 도구
ruff==0.1.6
mypy==1.7.1

# 메트릭스 도구
radon==6.0.1

# 보안 검사 도구
safety==2.3.5

# 표준 라이브러리 (별도 설치 불필요)
# - json (내장)
# - logging (내장)
# - datetime (내장)
# - random (내장)
```

## 📄 README.md 템플릿

```markdown
# [프로젝트명]

[프로젝트 간단 설명 - 예: Arduino Mock 시뮬레이터와 Dash 대시보드를 사용한 실시간 온도 모니터링 시스템]

## 🌡️ 시스템 개요

[센서/하드웨어]를 사용하여 실시간으로 [데이터]를 모니터링하고, 웹 기반 대시보드에서 데이터를 시각화하는 시스템입니다.

### 주요 특징
- **실시간 모니터링**: [데이터] 실시간 수집 및 표시
- **[하드웨어] Mock 시뮬레이터**: 실제 하드웨어 없이 테스트 가능
- **JSON 통신**: 구조화된 데이터 교환
- **웹 대시보드**: [프레임워크] 기반 반응형 인터페이스
- **설정 변경**: [설정 항목들] 동적 변경

## 🏗️ 시스템 구조

```
[하드웨어/센서] → [시뮬레이터] ↔ [통신방식] ↔ [웹앱]
                                              ↑
                                         [사용자 UI]
```

## 🚀 주요 기능

### 데이터 읽기
- ✅ [데이터1] 읽기
- ✅ [데이터2] 읽기
- ✅ [설정값] 읽기

### 설정 변경
- ✅ [설정1] 변경
- ✅ [설정2] 변경
- ✅ [설정3] 변경

### 사용자 인터페이스
- ✅ [기능1]
- ✅ [기능2]
- ✅ [기능3]

## 📁 프로젝트 구조

```
├── docs/                          # 문서
│   ├── architecture/              # 시스템 아키텍처
│   └── development/               # 개발 가이드
├── src/                           # 소스 코드
│   ├── [hardware]_mock/           # [하드웨어] 시뮬레이터
│   ├── python/                    # Python 애플리케이션
│   └── [app_type]/                # [앱타입] 애플리케이션
├── tools/                         # 개발 도구
│   ├── test_environment/          # 환경 테스트
│   ├── metrics/                   # 코드 메트릭스
│   ├── quality/                   # 품질 검사
│   └── security/                  # 보안 검사
├── tests/                         # 테스트 코드
└── .kiro/                         # Kiro IDE 설정
```

## 🛠️ 기술 스택

### [하드웨어] Mock Simulator
- **언어**: Python
- **라이브러리**: [관련 라이브러리들]
- **기능**: [하드웨어] 시뮬레이션

### [프레임워크] Application
- **언어**: Python
- **프레임워크**: [프레임워크명]
- **UI**: [UI 라이브러리]
- **통신**: [통신 방식]
- **데이터**: [데이터 처리 라이브러리]

## 🔧 설치 및 실행

### 요구사항
- Python 3.10+
- pip 또는 uv

### 설치
```bash
# 저장소 클론
git clone https://github.com/[username]/[repository].git
cd [repository]

# 의존성 설치
pip install -r requirements.txt
# 또는 uv 사용
uv sync
```

### 실행
```bash
# [하드웨어] Mock 시뮬레이터 실행
python src/[hardware]_mock/[simulator].py

# [앱] 실행 (새 터미널에서)
python src/[app_type]/app.py
```

## 🧪 테스트 및 품질 검사

### 환경 테스트
```bash
# 종합 환경 테스트
uv run python tools/test_environment/test_environment.py

# [하드웨어] 환경 확인
uv run python tools/test_environment/check_[hardware]_libraries.py
```

### 품질 검사
```bash
# 모든 품질 및 보안 검사
python tools/run_all_checks.py

# 개별 검사
python tools/quality/quality_check.py      # 코드 품질
python tools/security/trufflehog_check.py  # 보안 검사
```

## 📊 [통신 프로토콜] 통신 프로토콜

### [데이터] 데이터 ([소스] → [대상])
```json
{
  "[field1]": "[value1]",
  "[field2]": [value2],
  "[field3]": [value3],
  "timestamp": "2025-01-17T10:30:00Z"
}
```

## 🤝 기여 가이드

1. **개발 전 확인사항**
   - [도구 가이드 경로] 숙지
   - [개발 문서 경로] 참조

2. **개발 워크플로우**
   ```bash
   # 1. 환경 테스트
   uv run python tools/test_environment/test_environment.py
   # 2. 기능 개발
   # 3. 품질 검사
   python tools/run_all_checks.py
   # 4. 모든 검사 통과 시 커밋
   git add .
   git commit -m "feat: 새로운 기능 추가"
   git push
   ```

## 📈 로드맵

- [x] 프로젝트 초기 설정
- [x] 아키텍처 설계
- [x] 개발 도구 구성
- [ ] Phase 1: 기본 시스템 구축
- [ ] Phase 2: 대시보드 개발
- [ ] Phase 3: 고급 기능 구현

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다.

## 📞 문의

프로젝트 관련 문의사항이 있으시면 GitHub Issues를 통해 연락해 주세요.

---

**개발 상태**: 🚧 개발 중 (Phase 1 준비 단계)
```

## 📄 환경 테스트 스크립트 템플릿

### test_environment.py 기본 구조
```python
#!/usr/bin/env python3
"""
[프로젝트명] 환경 테스트 스크립트

이 스크립트는 다양한 터미널 환경에서 가상환경과 패키지들이
제대로 작동하는지 종합적으로 테스트합니다.
"""

import sys
import os
import platform
from datetime import datetime

class EnvironmentTester:
    def __init__(self):
        self.test_results = []
        self.failed_tests = []
        
    def log_test(self, test_name: str, success: bool, message: str = ""):
        """테스트 결과 로깅"""
        status = "✅ PASS" if success else "❌ FAIL"
        result = f"{status} {test_name}"
        if message:
            result += f" - {message}"
        
        print(result)
        self.test_results.append((test_name, success, message))
        
        if not success:
            self.failed_tests.append(test_name)
    
    def test_python_environment(self):
        """Python 환경 테스트"""
        print("\n" + "="*60)
        print("🐍 Python 환경 테스트")
        print("="*60)
        
        # Python 버전 확인
        version = sys.version_info
        version_str = f"{version.major}.{version.minor}.{version.micro}"
        
        if version.major == 3 and version.minor >= 10:
            self.log_test("Python 버전", True, f"Python {version_str}")
        else:
            self.log_test("Python 버전", False, f"Python {version_str} (3.10+ 필요)")
    
    def test_required_packages(self):
        """필수 패키지 import 테스트"""
        print("\n" + "="*60)
        print("📦 필수 패키지 테스트")
        print("="*60)
        
        # 프로젝트에 맞게 패키지 목록 수정
        required_packages = {
            '[package1]': '[설명1]',
            '[package2]': '[설명2]',
            # 추가 패키지들...
        }
        
        for package, description in required_packages.items():
            try:
                module = __import__(package)
                version = getattr(module, '__version__', 'Unknown')
                self.log_test(f"{package} import", True, f"{description} v{version}")
            except ImportError as e:
                self.log_test(f"{package} import", False, f"{description} - {str(e)}")
    
    def run_all_tests(self):
        """모든 테스트 실행"""
        print("🚀 [프로젝트명] 환경 테스트 시작")
        print(f"📅 테스트 시간: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        
        self.test_python_environment()
        self.test_required_packages()
        # 추가 테스트 메서드들...
        
        self.print_summary()
    
    def print_summary(self):
        """테스트 결과 요약 출력"""
        print("\n" + "="*60)
        print("📋 테스트 결과 요약")
        print("="*60)
        
        total_tests = len(self.test_results)
        passed_tests = sum(1 for _, success, _ in self.test_results if success)
        
        print(f"총 테스트: {total_tests}")
        print(f"✅ 성공: {passed_tests}")
        print(f"❌ 실패: {total_tests - passed_tests}")
        print(f"📊 성공률: {(passed_tests/total_tests*100):.1f}%")

if __name__ == "__main__":
    tester = EnvironmentTester()
    tester.run_all_tests()
```

이 템플릿들을 사용하면 DS18B20 프로젝트에서 검증된 방법론을 다른 프로젝트에도 적용할 수 있습니다.
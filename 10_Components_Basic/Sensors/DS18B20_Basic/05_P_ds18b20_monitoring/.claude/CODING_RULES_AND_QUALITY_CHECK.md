# 코딩 규칙 및 품질 검사 가이드

**작성일**: 2025-08-22  
**목적**: 코딩 실수 방지 및 일관된 코드 품질 유지

---

## 🚨 주요 실수 패턴 및 예방 규칙

### 1. 파일 편집 전 필수 작업

#### ❌ 실수 패턴
```
<tool_use_error>File has not been read yet. Read it first before writing to it.</tool_use_error>
```

#### ✅ 올바른 순서
```markdown
1. Read 도구로 파일 내용 확인
2. 수정할 부분 정확히 파악
3. Edit/MultiEdit 도구로 수정
```

#### 📋 체크리스트
- [ ] 파일을 Read 도구로 먼저 읽었는가?
- [ ] 수정할 라인 번호와 내용을 정확히 파악했는가?
- [ ] old_string이 파일 내용과 정확히 일치하는가?

---

### 2. 들여쓰기 및 구문 오류

#### ❌ 실수 패턴
```
error: Failed to parse tools\firmware_probe\probe_firmware.py:53:9: 
unindent does not match any outer indentation level
```

#### ✅ 예방 규칙
- **함수/클래스 내부 코드는 4칸 들여쓰기 일관성 유지**
- **조건문/반복문 블록은 상위 블록 + 4칸**
- **편집 시 기존 들여쓰기 레벨 정확히 파악 후 수정**

#### 📋 체크리스트
- [ ] 들여쓰기가 4칸 단위로 일관되는가?
- [ ] 블록 시작과 끝이 올바르게 매칭되는가?
- [ ] 편집 전후 들여쓰기 레벨이 일치하는가?

---

### 3. Import 문 규칙

#### ❌ 실수 패턴
```python
# 잘못된 예시
import sys
try:
    from debug_monitor import logger
except ImportError:
    pass
import dash  # E402: Module level import not at top of file
```

#### ✅ 올바른 패턴
```python
# 올바른 예시
import sys
from collections.abc import Callable
from typing import Any

import dash
import pandas as pd

# 조건부 import는 함수 내에서
def some_function():
    try:
        from debug_monitor import logger
    except ImportError:
        logger = None
```

#### 📋 체크리스트
- [ ] 표준 라이브러리 → 서드파티 → 로컬 순서인가?
- [ ] 조건부 import는 함수 내부에 배치했는가?
- [ ] 미사용 import는 제거했는가?
- [ ] `from typing import` → `from collections.abc import`로 최신화했는가?

---

### 4. 타입 힌트 현대화

#### ❌ 구식 문법
```python
isinstance(value, (int, float))           # UP038 오류
Union[int, str]                          # 구식
List[str]                                # 구식
Dict[str, Any]                           # 구식
```

#### ✅ 최신 문법 (Python 3.10+)
```python
isinstance(value, int | float)           # 최신
int | str                                # 최신
list[str]                                # 최신
dict[str, Any]                           # 최신
```

#### 📋 체크리스트
- [ ] `(Type1, Type2)` → `Type1 | Type2`로 변경했는가?
- [ ] `Union`, `List`, `Dict` → `|`, `list`, `dict`로 변경했는가?
- [ ] `Callable` 타입은 `collections.abc`에서 import했는가?

---

### 5. 예외 처리 패턴

#### ❌ 안티패턴
```python
try:
    risky_operation()
except Exception:
    pass  # SIM105 오류
```

#### ✅ 권장 패턴
```python
import contextlib

with contextlib.suppress(Exception):
    risky_operation()
```

#### 📋 체크리스트
- [ ] `try-except-pass` → `contextlib.suppress()`로 변경했는가?
- [ ] 구체적인 예외 타입을 지정했는가?
- [ ] 로깅이나 처리 로직이 있는가?

---

### 6. MyPy 타입 어노테이션 오류

#### ❌ 실수 패턴
```python
# Function is missing a type annotation [no-untyped-def]
def temperature_card():
    return html.Div()

# Function is missing a return type annotation [no-untyped-def]  
def get_layout():
    return html.Div()

# Function is missing a type annotation for one or more arguments [no-untyped-def]
def ensure_list(x):
    return []

# Function "builtins.any" is not valid as a type [valid-type]
def register_callbacks(app: any) -> None:
    pass

# Name "app" already defined (possibly by an import) [no-redef]
from src.python.app import app
from .app import app
from app import app
```

#### ✅ 올바른 패턴
```python
from typing import Any
from dash import html

# 반환 타입 명시
def temperature_card() -> html.Div:
    return html.Div()

def get_layout() -> html.Div:
    return html.Div()

# 인자 타입 명시
def ensure_list(x: Any) -> list:
    return []

# 올바른 타입 사용
def register_callbacks(app: Any) -> None:
    pass

# 중복 import 방지
try:
    from src.python.app import app
except Exception:
    try:
        from .app import app
    except Exception:
        from app import app  # type: ignore[no-redef]
```

#### 📋 체크리스트
- [ ] 모든 함수에 반환 타입 어노테이션을 추가했는가?
- [ ] 모든 함수 인자에 타입 힌트를 추가했는가?
- [ ] `any` 대신 `typing.Any`를 사용했는가?
- [ ] 중복 import에 `# type: ignore[no-redef]` 처리했는가?
- [ ] Dash 컴포넌트 함수는 `-> html.Div` 반환 타입을 명시했는가?

---

### 7. 코드 포맷팅 규칙

#### ❌ 실수 패턴
```python
def function():
    value = calculate()  
    return value       # W293: Blank line contains whitespace
```

#### ✅ 올바른 패턴
```python
def function():
    value = calculate()
    return value
```

#### 📋 체크리스트
- [ ] 공백만 있는 줄은 완전히 비웠는가?
- [ ] 줄 끝 공백(trailing whitespace)을 제거했는가?
- [ ] 함수/클래스 간 2줄 공백을 유지했는가?

---

## 🛠️ 필수 품질 검사 워크플로우

### Phase 1: 코딩 중 실시간 검사

```bash
# 파일별 즉시 검사
python -m ruff check src/python/specific_file.py
python -m ruff format src/python/specific_file.py
```

### Phase 2: 수정 완료 후 전체 검사

```bash
# 1. 린트 검사 및 자동 수정
python -m ruff check --fix src/ tools/
python -m ruff format src/ tools/

# 2. 타입 검사 (패키지 베이스 명시)
python -m mypy src/python --explicit-package-bases

# 3. 개별 테스트 실행 (전체 pytest 대신)
python -m pytest tests/test_protocol.py -v
```

### Phase 3: 최종 통합 검사

```bash
# 전체 품질 검사 실행
python tools/quality/quality_check.py

# 메트릭 수집
python tools/metrics/unified_metrics.py

# 보안 검사
python tools/security/trufflehog_check.py
```

---

## 📋 코딩 전후 체크리스트

### ✅ 코딩 시작 전
- [ ] 최신 코드를 pull 받았는가?
- [ ] 가상환경이 활성화되어 있는가?
- [ ] 수정할 파일의 현재 상태를 Read로 확인했는가?

### ✅ 코딩 중
- [ ] 들여쓰기를 4칸 단위로 일관되게 했는가?
- [ ] Import 순서를 표준 → 서드파티 → 로컬 순으로 했는가?
- [ ] 타입 힌트를 최신 문법으로 작성했는가?
- [ ] 미사용 import나 변수를 제거했는가?

### ✅ 코딩 완료 후
- [ ] `ruff check --fix` 실행했는가?
- [ ] `ruff format` 실행했는가?
- [ ] `mypy src/python --explicit-package-bases` 실행했는가?
- [ ] 관련 테스트를 개별 실행했는가?
- [ ] 전체 품질 검사를 실행했는가?

---

## 🚀 자동화 명령어 모음

### ⚡ 새로운 빠른 검사 도구 (권장)
```bash
# 전체 프로젝트 자동 수정 + 검사
python quick_quality_check.py --fix

# 특정 파일만 검사
python quick_quality_check.py src/python/dashboard/app.py

# 포괄적 검사 (자동수정 + 전체검사 + 상세리포트)
python quick_quality_check.py --full

# 전체 프로젝트 검사만
python quick_quality_check.py
```

### 🔧 기본 명령어
```bash
# 전체 자동 수정
python -m ruff check --fix --unsafe-fixes src/ tools/
python -m ruff format src/ tools/

# 타입 검사 (패키지 베이스 명시)  
python -m mypy src/python --explicit-package-bases

# 전체 린트 상태 확인
python -m ruff check src/ tools/
```

### 📊 품질 리포트 생성
```bash
# 통합 품질 검사
python tools/quality/quality_check.py

# 메트릭 리포트
python tools/metrics/unified_metrics.py

# 전체 검사 (메트릭 + 품질 + 보안)
python tools/run_all_checks.py
```

### 🧪 테스트 실행
```bash
# 개별 테스트 파일
python -m pytest tests/test_protocol.py -v

# 특정 카테고리 테스트
python -m pytest tests/integration/ -v
python -m pytest tests/unit/ -v

# 마커별 테스트
python -m pytest -m "not slow" -v
```

---

## 📈 품질 지표 목표

### 🎯 달성 목표
- **Ruff 린트**: 100% 통과 (All checks passed)
- **MyPy 타입**: 100% 통과 (Success: no issues found)
- **테스트 커버리지**: 60% 이상
- **보안 스캔**: 중/고위험 0개

### 📊 현재 상태 확인
```bash
# 현재 품질 상태 요약
python tools/quality/quality_check.py | grep "Overall results"

# 메트릭 점수 확인
python tools/metrics/unified_metrics.py | grep "total"
```

---

## 🔄 지속적 개선

### 📝 새로운 오류 발견 시
1. 이 문서의 "실수 패턴" 섹션에 추가
2. 예방 규칙과 체크리스트 업데이트
3. 자동화 가능한 검사는 도구에 반영

### 🛡️ 예방 중심 접근
- **사후 수정보다 사전 예방**
- **자동화 가능한 것은 도구로 해결**
- **반복되는 실수는 규칙으로 문서화**

---

**💡 핵심 원칙**: "코딩 → 즉시 검사 → 수정 → 재검사" 사이클을 습관화하여 고품질 코드 유지

**🎯 목표**: 실수 제로, 품질 100%, 자동화 극대화
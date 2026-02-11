# 보안 도구 추천 및 도입 가이드 (Flask/JS 소규모 프로젝트)

## 빠르게 시작하는 툴셋

### 1) 비밀정보 탐지
- `gitleaks`: 커밋/히스토리에서 키/토큰 탐지
- 용도: 깃허브 게시 전 유출 방지

### 2) Python 의존성 취약점 점검
- `pip-audit`: `requirements.txt` 기반 CVE 점검
- 용도: 배포 전 취약 라이브러리 제거

### 3) 정적 분석/코드 보안 점검
- `bandit`: Python 보안 안티패턴 점검
- `semgrep`: 규칙 기반 취약점 탐지 (Python/JS 모두)

### 4) 프런트엔드 취약점 점검
- `eslint` + 보안 플러그인
- 목적: 위험한 DOM 조작 패턴(`innerHTML`) 탐지 강화

### 5) 웹 보안 헤더/런타임 방어
- Flask에서 `Flask-Talisman` 적용
- 목적: CSP, HSTS, X-Frame-Options 등 기본 방어 헤더 적용

## 추천 실행 명령 (로컬)
```powershell
# Python
pip install pip-audit bandit
pip-audit -r 40_Software/requirements.txt
bandit -r 40_Software

# secrets
gitleaks detect --source .

# semgrep (설치 후)
semgrep --config p/security-audit 40_Software
```

## GitHub Actions 최소 파이프라인 제안
- 트리거: `pull_request`, `push(main)`
- 단계:
1. `pip-audit`
2. `bandit`
3. `gitleaks`
4. 테스트(추후 pytest 도입 시)

## 운영 전 보안 최소 기준 (권장)
- 인증/권한 도입 완료
- `debug=False` 확인
- 입력 검증/에러 처리 표준화
- DB를 SQLite/PostgreSQL로 전환
- 감사 로그 보관 기간/마스킹 정책 확정

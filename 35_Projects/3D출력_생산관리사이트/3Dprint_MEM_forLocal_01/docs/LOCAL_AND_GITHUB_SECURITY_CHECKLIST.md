# 로컬 개발/깃허브 게시 보안 체크리스트

## 1) 로컬 개발 시 주의사항
- 개발 서버는 기본 `127.0.0.1` 바인딩 유지. 외부망 공개 금지.
- `debug=True` 금지 (운영/공유 환경에서는 반드시 OFF).
- 실제 업무 데이터(`data/db.json`, `data/user_actions.log`)와 테스트 데이터를 분리.
- 로그/백업 파일에 제품명, 일정, 재고 등 민감 운영 데이터가 남는지 주기 점검.
- 브라우저 저장 데이터(자동완성/캐시)에 민감정보 남지 않도록 관리.
- 의존성 설치는 가상환경(`.venv`)에서만 수행하고 시스템 전역 설치 최소화.

## 2) GitHub 게시 전 필수 점검

### 절대 커밋 금지
- `.venv/`
- `40_Software/data/db.json`
- `40_Software/data/user_actions.log`
- `.env`, `.env.*`, 비밀키/토큰 파일
- 개인/고객 정보 포함된 샘플 데이터

### 반드시 추가할 `.gitignore` 예시
```gitignore
# Python
__pycache__/
*.pyc
.venv/

# Runtime data
40_Software/data/db.json
40_Software/data/user_actions.log

# Secrets
.env
.env.*

# OS/Editor
.DS_Store
Thumbs.db
.vscode/
.idea/
```

### 게시 전 실행 절차
1. 비밀 탐지 실행 (`gitleaks` 또는 `trufflehog`).
2. 의존성 취약점 점검 (`pip-audit`).
3. 기록 제거/치환 (실제 데이터 -> 더미 데이터).
4. 공개 README에 "개발용/운영용 설정 차이" 명시.

## 3) 공개 저장소 운영 수칙
- 브랜치 보호 규칙(최소 1명 리뷰) 적용.
- GitHub Advanced Security 또는 대체 스캐너 활성화.
- PR 템플릿에 보안 체크 항목 추가.
- 릴리즈 전 태그 기준 보안 점검 리포트 작성.

## 4) 현재 저장소 기준 우선 액션
1. 루트에 `.gitignore` 신설.
2. 이미 추적 중이면 `git rm --cached`로 데이터/venv 추적 해제.
3. `docs/`의 보안 문서를 PR 체크리스트와 연결.

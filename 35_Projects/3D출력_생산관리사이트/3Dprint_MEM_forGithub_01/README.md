# 3D 출력 생산 관리 사이트

3D 출력물의 생산 일정 관리 및 수량 파악을 위한 로컬 웹사이트 프로젝트입니다.

## 프로젝트 정보
- **상태:** Requirements Analysis (요구사항 분석 중)
- **시작일:** 2026-02-11
- **목표:** 효율적인 3D 프린팅 생산 공정 관리 시스템 구축

## 폴더 구조
- `00_Requirements`: 요구사항 정의 및 분석 문서
- `10_System_Design`: 시스템 아키텍처 및 DB 설계
- `20_Hardware`: (해당 시) 프린터 하드웨어 설정 관련
- `30_Firmware`: (해당 시) 프린터 펌웨어 관련
- `40_Software`: 웹사이트 소스 코드 (Python Flask)
- `50_Test_Verification`: 테스트 계획 및 결과
- `docs`: 일반 문서 및 설문지

## 진행 상황
- [2026-02-11] 프로젝트 폴더 구조 생성 및 요구사항 질문지 작성 완료
- [2026-02-11] 시스템 설계 (Data Schema, Architecture) 완료
- [2026-02-11] Flask 기반 기본 웹 서버 및 AI-Friendly HTML 구조 구축
- [2026-02-11] 로컬 가상환경(.venv) 설정 및 의존성 설치 완료

## 시작하기 (How to Run)
1. Python 설치 확인
2. `40_Software` 폴더로 이동
3. 가상환경 활성화 (Windows):
   ```powershell
   .\.venv\Scripts\activate
   ```
4. 서버 실행:
   ```powershell
   python app.py
   ```
5. 웹 브라우저에서 `http://127.0.0.1:5000` 접속
## 운영 방법 (보안 설정 포함)

### 1) 기본 실행 (로컬 전용)
기본값은 로컬 접근만 허용됩니다.

```powershell
cd 40_Software
python app.py
```

- 접속: `http://127.0.0.1:5000`
- 기본 보안값:
  - `ALLOW_REMOTE=false`
  - `REQUIRE_API_KEY=false`

### 2) 원격 접속 허용 + API Key 보호
인터넷/사내망 공개 시 최소 아래 설정을 권장합니다.

```powershell
cd 40_Software
$env:ALLOW_REMOTE="true"
$env:REQUIRE_API_KEY="true"
$env:API_KEY="CHANGE_THIS_TO_LONG_RANDOM_VALUE"
python app.py
```

주의:
- `API_KEY`는 저장소에 커밋하지 마세요.
- 운영에서는 `.env` 또는 서버 비밀변수로 주입하세요.

### 3) 보안 게이트 동작 요약
서버(`app.py`)에 아래가 적용되어 있습니다.
- 로컬 접근 제한(기본)
- 선택적 API Key 검증
- 상태변경 요청(POST/PATCH/DELETE) 헤더 검증: `X-Requested-With: XMLHttpRequest`
- 보안 헤더 설정: `CSP`, `X-Frame-Options`, `X-Content-Type-Options`, `Referrer-Policy`, `Cache-Control`
- 입력값 검증/문자열 정화

### 4) 운영 배포 권장사항
- Flask 개발서버 대신 WSGI 서버 사용(`gunicorn`/`waitress`)
- HTTPS는 리버스 프록시(Nginx/Caddy)에서 강제
- 방화벽으로 허용 IP/포트 최소화
- `40_Software/data/db.json`, `40_Software/data/user_actions.log`는 백업 정책 분리

### 5) 빠른 점검 체크리스트
- [ ] `debug=False` 확인
- [ ] `.gitignore`에 런타임/비밀 파일 제외 확인
- [ ] `API_KEY` 길고 랜덤한 값 적용
- [ ] HTTPS 및 접근제어 적용

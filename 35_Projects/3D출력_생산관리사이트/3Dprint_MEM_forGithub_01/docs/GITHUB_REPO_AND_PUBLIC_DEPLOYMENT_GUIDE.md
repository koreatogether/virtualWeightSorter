# GitHub 저장소 업로드 및 공개 사용 가이드

## 0) 먼저 알아둘 점
- GitHub 저장소에 코드를 올리는 것만으로는 웹앱이 자동으로 서비스되지 않습니다.
- 현재 프로젝트는 Flask 백엔드가 있으므로, 다른 사람들이 사용하려면 "실행 서버"가 필요합니다.
- 즉, 아래 2단계를 모두 해야 합니다.
1. GitHub 저장소에 코드 업로드
2. 배포 플랫폼(Render/Railway/Fly.io/VPS 등)에서 서버 실행

## 1) GitHub 개인 저장소 만들기
1. GitHub에서 `New repository` 생성
2. 저장소 이름 예: `3dprint-mes`
3. Private/Public 선택
4. 로컬에서 `3Dprint_MEM_forGithub_01` 폴더 기준으로 초기 업로드

```powershell
cd E:\project\35_Projects\3D출력_생산관리사이트\3Dprint_MEM_forGithub_01
git init
git add .
git commit -m "Initial public-ready commit"
git branch -M main
git remote add origin https://github.com/<YOUR_ID>/<REPO>.git
git push -u origin main
```

## 2) 다른 사람이 "실제로 사용"하게 하려면

### 방법 A: 백엔드 포함 배포 (권장)
- Render / Railway / Fly.io / VPS 중 하나에 `40_Software`를 배포
- 배포 서비스에 GitHub 저장소 연결
- Start Command 예시:
  - `python app.py` (간단 시작)
  - 운영 권장: gunicorn/waitress 사용

필수 환경변수(현재 코드 기준):
- `ALLOW_REMOTE=true` (원격 접근 허용)
- `REQUIRE_API_KEY=false` (공개 사용이면 false, 내부 사용이면 true)
- `API_KEY=<랜덤키>` (`REQUIRE_API_KEY=true`일 때만)

주의:
- 공개 서비스라면 인증/권한 체계(로그인) 없이는 데이터가 누구에게나 수정될 수 있습니다.
- 최소한 API Key 또는 계정 인증을 추가하세요.

### 방법 B: GitHub Pages (정적만 가능)
- GitHub Pages는 정적 페이지 전용
- Flask API(`app.py`)는 GitHub Pages에서 실행되지 않음
- 따라서 현재 구조를 그대로는 Pages 단독 공개 불가

## 3) 도메인/접속 설정
1. 배포 플랫폼 URL 확인 (`https://...`)
2. 필요 시 커스텀 도메인 연결
3. HTTPS 강제 적용
4. 접속 테스트:
   - 메인 페이지 로딩
   - 주문 생성/수정/삭제 API
   - 스케줄/재고/프린터 기능

## 4) 공개 전 체크리스트
- [ ] `.venv`, `db.json`, `user_actions.log`, `.env*` 커밋 제외
- [ ] `debug=False` 확인
- [ ] `ALLOW_REMOTE/REQUIRE_API_KEY/API_KEY` 운영값 설정
- [ ] 백업 정책 수립 (`40_Software/data`)
- [ ] 장애 대비: 배포 플랫폼 로그 확인 절차 준비

## 5) 운영 권장 추가 작업
- 인증/권한(로그인) 추가
- JSON 파일 저장소를 SQLite/PostgreSQL로 전환
- 정기 보안 점검(`pip-audit`, `bandit`, `gitleaks`) 자동화
- CI/CD(GitHub Actions)에서 테스트 + 보안 스캔 실행

## 6) 빠른 결론
- "GitHub 업로드"는 코드 공유 단계입니다.
- "다른 사람 사용"은 배포 서버 단계까지 완료해야 가능합니다.
- 현재 프로젝트는 `방법 A(백엔드 포함 배포)`로 진행해야 합니다.

# 웹사이트 보안/운영 점검 리포트 (2026-02-11)

대상: `40_Software` (Flask + Vanilla JS)

## 핵심 결론
- 현재 상태는 "내부/로컬 단독 사용" 전제에서는 빠른 개발에 적합합니다.
- 외부 공개 또는 사내 다중 사용자 환경으로 확장하려면 인증/권한, 입력검증, XSS/CSRF 방어, 배포 설정 분리가 필수입니다.

## 우선순위 보안 이슈

### P0 (즉시 조치)
1. 인증/권한 부재
- 모든 데이터/변경 API가 무인증으로 열려 있습니다.
- 근거: `40_Software/app.py:52`, `40_Software/app.py:56`, `40_Software/app.py:76`, `40_Software/app.py:103`, `40_Software/app.py:191` 등.
- 영향: 같은 네트워크/호스트에서 API 호출만으로 임의 수정/삭제 가능.

2. 디버그 모드로 서버 실행
- `debug=True`로 실행 중입니다.
- 근거: `40_Software/app.py:385`
- 영향: 운영 환경에서 매우 위험. 에러 정보 노출 및 디버그 기능 악용 가능성.

3. 저장형/반사형 XSS 가능성
- 사용자 입력값(product/material 등)을 `innerHTML`로 직접 렌더링합니다.
- 근거: `40_Software/static/js/main.js:264`, `40_Software/static/js/main.js:345`, `40_Software/static/js/main.js:670`, `40_Software/static/js/main.js:690`
- 영향: 악성 스크립트 삽입 시 세션/화면/데이터 조작 가능.

### P1 (이번 주 내 조치)
4. CSRF 방어 부재
- 상태 변경 요청(POST/PATCH/DELETE)에 CSRF 토큰 검증이 없습니다.
- 근거: `40_Software/static/js/main.js:453`, `40_Software/static/js/main.js:457`, `40_Software/static/js/main.js:461`
- 영향: 사용자가 로그인 체계를 갖추게 될 경우, 교차 사이트 요청 위조 리스크 증가.

5. 입력 스키마 검증 부족
- `request.json` 값을 바로 `float(...)` 변환/저장합니다.
- 근거: `40_Software/app.py:59`, `40_Software/app.py:79`, `40_Software/app.py:123`, `40_Software/app.py:194`
- 영향: 비정상 payload로 500 에러/데이터 품질 저하.

6. JSON 파일 단일 저장소 구조의 동시성 위험
- `db.json` 파일에 동시 read/write 시 경합 가능.
- 근거: `40_Software/app.py:21`, `40_Software/app.py:35`
- 영향: 다중 요청 시 데이터 손실/덮어쓰기 가능.

### P2 (중기 개선)
7. 감사 로그 민감정보 관리
- IP/행동/상세값이 평문 로그로 누적됩니다.
- 근거: `40_Software/app.py:13`, `40_Software/app.py:17`
- 영향: 로그 유출 시 운영 정보 노출.

8. 외부 CDN 의존 (무결성 검증 미적용)
- Flatpickr를 CDN에서 직접 로드하며 SRI/integrity가 없습니다.
- 근거: `40_Software/templates/index.html:9`, `40_Software/templates/index.html:273`
- 영향: 공급망 리스크.

## 기능/구조 관점의 추가 의견
- `initOrderFormEvents` 함수가 중복 정의되어 유지보수 리스크가 있습니다.
- 근거: `40_Software/static/js/main.js:200`, `40_Software/static/js/main.js:466`
- 권장: 중복 제거 후 단일 책임 함수로 정리.

## 바로 실행 가능한 개선 순서
1. `debug=False` + 환경변수 분기.
2. 인증/권한(최소 API 키 또는 세션 로그인) 도입.
3. 입력 검증(Pydantic/Marshmallow) 및 에러 핸들링 표준화.
4. `innerHTML` 제거, `textContent` 기반 렌더링으로 전환.
5. JSON DB -> SQLite/PostgreSQL 전환.
6. CSRF/CORS/보안 헤더(CSP 포함) 추가.

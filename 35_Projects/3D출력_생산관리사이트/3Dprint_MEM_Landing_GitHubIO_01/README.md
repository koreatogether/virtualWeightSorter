# 3Dprint_MEM Static Demo (GitHub Pages)

이 폴더는 `3D 출력 생산관리`의 **정적 데모 화면**입니다.

## 구성 파일
- `index.html`
- `styles.css`
- `static-data.js` (샘플 데이터)
- `app.js` (렌더링 로직)

## 특징
- 서버/API 없이 동작
- GitHub Pages에 바로 게시 가능
- 실제 CRUD 기능은 포함하지 않음

## GitHub Pages 게시 방법
1. 이 폴더를 별도 GitHub 저장소에 업로드
2. 저장소 `Settings > Pages`
3. `Branch: main`, `Folder: /root` 선택 후 저장
4. 발급 주소 접속

예시:
- `https://<github-id>.github.io/<repo-name>/`

## 주의
실제 운영 기능(주문 저장/수정/삭제, DB 반영)은 Flask 백엔드 배포 버전에서만 동작합니다.

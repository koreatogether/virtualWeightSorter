# Tasks

- [x] PlatformIO 프로젝트 초기화 및 `platformio.ini` 설정
- [x] 프로젝트 구조 재구성 (`src/` 폴더 이동 및 `.ino` -> `.cpp` 변환)
- [x] 빌드 에러 해결
    - [x] 라이브러리 충돌 (FreeRTOS) 해결
    - [x] 함수 선언 (Forward Declaration) 추가
- [x] 빌드 검증 (`pio run` 성공)
- [x] 최종 문서화
    - [x] `progressLog.md` 생성 및 기록
    - [x] `platformio_best_practices.md` 작성
    - [x] `walkthrough.md` 작성
    - [x] `README.md` 업데이트
- [x] 코드 비평 작업
    - [x] `docs/code_critique.md` 작성
- [x] 아키텍처 개선 (Refactoring)
    - [x] 1단계: 메모리 최적화 (String 제거, 정적 할당)
    - [x] 2단계: UI 로직 분리 (SerialUI 클래스 추출)
    - [x] 3단계: 완전 비차단형 시스템으로 전환
    - [x] 4단계: 추상화 정리 및 캡슐화 강화

- [ ] 기능 검증 및 테스트
- [ ] 최종 결과 보고 및 문서화 (walkthrough.md 업데이트)

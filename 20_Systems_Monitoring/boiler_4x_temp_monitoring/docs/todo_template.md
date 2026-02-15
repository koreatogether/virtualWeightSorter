# TODO 템플릿

파일명 규칙: `todoList_0N.md` (예: `todoList_03.md`)

---

## 요약 / Title
- [ ] 제목: 한 줄로 요약
- ID: TODO-0N
- 우선순위: Critical / High / Medium / Low
- 상태: not-started / in-progress / blocked / done
- 담당자: @username

## 설명 / Description
- 문제 또는 기능에 대한 상세 설명 (배경, 목적, 영향 범위).

## 요구 사항 / Requirements
- 기능적 요구사항(구체적, 측정 가능)
- 비기능적 요구사항(성능, 메모리, 전력 소비 등)

## 구현 노트 / Implementation Notes
- 관련 파일/모듈: `NextionManager.cpp`, `SensorManager.cpp` 등
- 설계 결정(간단 요약)
- 외부 의존성 / 하드웨어 필요 여부

## 수락 기준 / Acceptance Criteria
- (1) 자동화/수동 테스트로 검증 가능한 항목들을 나열
- (2) 실패 시 재현 방법 및 로그 위치

## 테스트 케이스 / Tests
- 테스트 시나리오 1: 입력 → 기대 출력
- 테스트 시나리오 2: 경계값/예외 처리
- 장기 테스트(예: 48시간 HIL) 요구 여부

## 체크리스트
- [ ] 설계 문서 업데이트
- [ ] 코드 구현(PR 생성)
- [ ] 단위/통합 테스트 추가
- [ ] 빌드/CI 통과
- [ ] 문서(README/progressLog) 업데이트

## 일정/추정
- 시작일: YYYY-MM-DD
- 마감일(목표): YYYY-MM-DD
- 추정 작업 시간: X시간

## 산출물/첨부
- 관련 로그 경로: `logs/...`
- 스크린샷/캡처: `assets/...`
- 측정값(전원/전류 등): 첨부 또는 여기에 기록

## 관련 항목/참조
- 관련 TODO: TODO-XX, TODO-YY
- 관련 이슈: #123
- 관련 문서: `raptor_checklist.md`, `progressLog.md`

---

### 예시 (간단)
Title: Nextion 핸드셰이크 실패시 자동 복구 구현
ID: TODO-03
우선순위: High
상태: not-started
담당자: @lee

Description:
- Nextion이 핸드셰이크에 실패하면 재시도/재설정(soft reset)하고, 실패 시 LCD에 오류 메시지를 표시한다.

Requirements:
- 핸드셰이크 타임아웃(예: 2s) 이후 3회 재시도
- 재시도 실패 시 Nextion에 `t0.txt="HMI Error"` 전송

Acceptance:
- 핸드셰이크 실패를 시뮬레이션하여 3회 이후 오류 메시지가 표시됨.
- 시리얼 로그에 실패/재시도 기록이 남음.

Tests:
- 시뮬레이션 스크립트로 5회 통신 차단 후 정상 복구 확인


---

템플릿을 사용해 새 TODO 파일을 생성하면 제가 자동으로 요약/우선순위 기반 작업 목록을 생성해 드립니다.
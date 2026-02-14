# Implementation Plan - Build & Upload Fix

## 1. 분석 (Analysis)
- **현상**: `pio run` 실행 시 `src/App.cpp`에서 다수의 `qualified-id` 에러 및 `expected '}' at end of input` 에러 발생.
- **원인**:
    - `parseSensorSelection` 함수의 로직이 미완성 상태로 방치되어 중괄호 닫기가 누락됨.
    - 리팩토링 과정에서 `AppState` 명칭이 변경되었으나 소스코드 일부(`MENU_ACTIVE`)에 반영되지 않음.
    - 메뉴 재출력 로직(`MENU_REPRINT_INTERVAL`)이 타임아웃용 타이머(`menuDisplayMillis`)를 갱신하여 시스템 복귀가 불가능한 로직 결함.

## 2. 해결 방안 (Solution)
- `App.cpp`의 라인 820 부근에 누락된 `}` 추가.
- `MENU_ACTIVE` 사용처를 찾아 `MAIN_MENU`로 일괄 변경.
- `App` 클래스에 `lastMenuPrintMillis`를 도입하여 메뉴 재출력과 UI 타임아웃 변수를 분리 관리.

## 3. 검증 단계 (Verification)
- `pio run`: 컴파일 성공 여부 확인.
- `pio run --target upload`: 타겟 보드(COM6) 연결 및 플래싱 성공 확인.
- **타임아웃 실측**: 15초 간격 메뉴 재출력 유지 및 60초 시점 `NORMAL_OPERATION` 복귀 완료.

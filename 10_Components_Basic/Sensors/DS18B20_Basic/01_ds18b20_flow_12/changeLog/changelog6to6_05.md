# Changelog 6 to 6_05

## 1. 주요 버그 수정 및 안정성 강화

### 1.1. 메뉴 입력 불능(먹통) 현상 해결
- **문제:** `menu` 진입 후 `1`번(개별 ID 변경)을 선택하면 시리얼 입력이 먹통이 되어 시스템 진행이 불가능해지는 치명적인 버그가 있었습니다.
- **원인:** `processEditIndividualId()` 함수 내에서 입력 버퍼 관리가 미흡하고, `processMenuInput()` 함수 시작 시 버퍼가 완전히 비워지지 않아 발생했습니다.
- **조치:**
  - `processMenuInput()` 함수 시작 부분에 `comm.clearInputBuffer()`를 추가하여 메뉴 진입 시 항상 깨끗한 입력 버퍼 상태를 보장했습니다.
  - `processEditIndividualId()` 함수를 완전히 구현하고, 해당 상태 진입 및 종료 시, 그리고 타임아웃/취소 시 `comm.clearInputBuffer()`를 적절히 호출하도록 수정했습니다.

### 1.2. 선택적 ID 변경 입력 파싱 오류 개선
- **문제:** `12 3 4` (공백 포함 두 자리 숫자), `1,10,5` (쉼표 포함 두 자리 숫자), `1231` (붙여쓰기 숫자) 등 다양한 형식의 센서 번호 입력이 올바르게 파싱되지 않는 문제가 있었습니다.
- **원인:** `parseSensorSelection()` 함수가 여러 자리 숫자와 다양한 구분자를 정확히 처리하지 못했습니다.
- **조치:** `parseSensorSelection()` 함수의 로직을 더욱 견고하게 리팩토링하여, 공백, 쉼표 등 다양한 구분자를 사용한 여러 자리 숫자 입력과 붙여쓰기 숫자를 모두 정확하게 파싱하도록 개선했습니다.

### 1.3. 메뉴 취소 후 센서 테이블 업데이트 지연 개선 (UX)
- **문제:** 메뉴에서 `NORMAL_OPERATION` 상태로 돌아갈 때 센서 테이블이 즉시 업데이트되지 않고 최대 15초까지 지연되는 문제가 있었습니다.
- **원인:** `show18b20Table_loop()` 함수가 `SENSOR_READ_INTERVAL` 타이머에만 의존하여 발생했습니다.
- **조치:** `NORMAL_OPERATION` 상태로 전환되는 모든 함수(`processMenuInput()`, `processEditIndividualId()`, `processEditSelectiveId()`, `processAutoAssignIds()`, `resetAllSensorIds()`)에서 `previousMillis = 0;`을 설정하여, 해당 상태로 돌아가는 즉시 `show18b20Table_loop()`가 실행되어 센서 테이블이 업데이트되도록 했습니다.

## 2. 기능 안정성 및 사용자 경험 향상
- 이번 업데이트를 통해 시리얼 입력 처리의 안정성을 크게 향상시키고, 사용자 인터페이스의 반응성을 개선하여 전반적인 사용자 경험을 향상시켰습니다.

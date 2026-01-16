# Changelog 6 to 6_02

## 주요 변경 내용

### 1. ID 변경 메뉴 응답 속도 개선
- **문제점:** `menu` 명령어 입력 시, ID 변경 메뉴가 즉시 표시되지 않고 최대 15초까지 지연되는 문제가 있었습니다.
- **원인:** 메뉴 표시 로직(`displayIdChangeMenu`)이 15초 주기의 타이머(`MENU_REPRINT_INTERVAL`)에만 의존하고 있었기 때문입니다.
- **해결책:**
  - `forceMenuPrint` 라는 `bool` 타입의 플래그 변수를 추가했습니다.
  - 사용자가 `menu`를 입력하면(`handleUserCommands`), 이 플래그를 `true`로 설정합니다.
  - `displayIdChangeMenu` 함수는 이제 타이머 조건 외에도 `forceMenuPrint` 플래그가 `true`일 때 즉시 메뉴를 출력합니다.
  - 메뉴가 출력된 후 플래그는 다시 `false`로 초기화되어, 이후의 재출력은 타이머에 의해 정상적으로 제어됩니다.
- **결과:** 사용자가 메뉴를 호출하면 지연 없이 즉시 피드백을 받을 수 있도록 사용자 경험(UX)이 향상되었습니다.

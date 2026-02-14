# Project-specific Copilot instructions

이 프로젝트는 `.github/COPILOT_INSTRUCTIONS.md`와 저장소 전반의 코딩 규칙을 따릅니다.

추가 지침:
- 펌웨어는 비차단(Non-blocking) 구조 권장, `delay()`는 초기화/테스트에만 사용
- `OneWire`, `DallasTemperature` 사용 시 풀업과 오류 처리(디바이스 없음, 통신 오류)에 주의
- 문서와 주석은 한국어로 작성

작업을 시작하기 전에 `00_Requirements/README.md`와 `30_Firmware/README.md`를 확인하세요.
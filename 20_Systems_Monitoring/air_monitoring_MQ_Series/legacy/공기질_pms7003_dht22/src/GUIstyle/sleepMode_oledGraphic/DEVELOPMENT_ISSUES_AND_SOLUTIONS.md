# 개발 이슈 및 해결 과정 (Development Issues & Solutions)

이 문서는 PMS7003 OLED GUI 프로젝트 개발 과정에서 발생한 주요 문제점들과 그 해결 방법을 기록한 문서입니다.

---

## 1. 컴파일 에러 (Compilation Error)

### 문제 상황
*   코드를 수정한 후 컴파일 시 `'encoder' was not declared in this scope` 및 `'handleEncoderInterrupt' was not declared` 에러가 발생함.
*   확인 결과, `RotaryEncoder` 클래스 정의와 전역 객체 선언 부분이 실수로 삭제되거나 주석 처리되어 있었음.

### 해결 방법
*   **코드 복구**: `RotaryEncoder` 클래스 정의, `encoder` 전역 객체, 그리고 인터럽트 처리를 위한 `handleEncoderInterrupt` 함수를 다시 작성하여 복구함.
*   이후 파일 내용이 일부 잘리는 현상이 있어, 전체 코드를 다시 덮어쓰는 방식으로 완전하게 복구함.

---

## 2. 절전 모드 타이머 로직 (Sleep Timer Logic)

### 문제 상황
*   **초기 문제**: 1분 절전 설정 시, 30초 측정 후 30초만 쉬고 다시 켜지는 현상 발생. (측정 시간 포함해서 계산함)
*   **수정 요청**: 절전 시간 변경 시, 현재 진행 중인 카운트다운이 갑자기 늘어나는 현상 발생.

### 해결 방법
*   **기준 시간 변경**: `lastSensorWakeTime` 대신 `lastSensorSleepTime`을 도입하여, 센서가 **꺼진 시점**부터 시간을 재도록 수정.
*   **예약 적용 시스템**: `nextSleepInterval` 변수를 도입. 설정 메뉴에서 시간을 변경하면 즉시 적용하지 않고 `nextSleepInterval`에 저장해 둠.
*   **적용 시점**: 센서가 **다음 번 절전 모드에 진입할 때** `sleepInterval = nextSleepInterval`을 실행하여, 현재 주기가 아닌 다음 주기부터 변경된 시간이 적용되도록 함.

---

## 3. 화면 보호기 버그 (Screen Saver Bug)

### 문제 상황
*   설정 메뉴에서 시간을 고르고 버튼을 누르는 순간, 화면이 바로 꺼져버리는(화면 보호기 진입) 현상 발생.

### 해결 방법
*   **원인 분석**: `sys.lastInteractionTime`을 업데이트할 때 `millis()`를 사용했는데, 루프 시작 시점의 `currentMillis`보다 미세하게 늦은 시간이 기록될 수 있음. 다음 루프에서 `currentMillis - sys.lastInteractionTime`을 계산할 때 `unsigned long` 언더플로우(Underflow)가 발생하여 아주 큰 숫자가 됨.
*   **조치**: 모든 시간 업데이트에 `millis()` 대신 루프 시작 시 고정된 `currentMillis`를 사용하여 시간 차 계산의 오류를 방지함.

---

## 4. 화면 깨어남 문제 (Screen Wakeup Issue)

### 문제 상황
*   화면 보호기 상태(화면 꺼짐)에서 로터리 인코더 버튼을 눌러도 화면이 켜지지 않음.

### 해결 방법
*   **1차 시도 (실패)**: 디바운스 시간을 줄이고 디버그 메시지를 추가했으나 여전히 반응이 없거나 느림.
*   **2차 시도 (성공)**: `encoder.isButtonPressed()` 함수(소프트웨어 디바운스)에만 의존하지 않고, `digitalRead(Config::PIN_ENC_SW) == LOW`를 통해 **하드웨어 핀 상태를 직접 확인**하는 로직을 추가함.
*   버튼이 눌리는 즉시(LOW 신호 감지 시) 무조건 화면을 켜도록 하여 반응성을 확보함.

---

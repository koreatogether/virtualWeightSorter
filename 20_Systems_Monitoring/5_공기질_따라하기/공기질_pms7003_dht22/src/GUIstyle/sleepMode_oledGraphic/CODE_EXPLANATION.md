# 코드 흐름 설명서 (Code Explanation)

이 문서는 `u8g2libStyle.ino` 파일의 구조와 주요 로직의 흐름을 설명합니다.

---

## 1. 전체 구조 (Structure)

코드는 크게 6가지 영역으로 나뉘어 있습니다.

1.  **설정 및 상수 (Configuration)**: 핀 번호, 통신 속도, 타임아웃 시간 등을 정의합니다.
2.  **전역 객체 (Global Objects)**: OLED(`u8g2`), 시스템 상태(`sys`), 인코더(`encoder`) 객체를 생성합니다.
3.  **상태 관리 (State Management)**:
    *   `AppState`: 현재 화면(모니터, 그래프, 통계, 설정, 절전)을 정의하는 열거형.
    *   `SystemState`: 센서 데이터, 현재 상태, 타이머 등을 저장하는 구조체.
4.  **드라이버 (Drivers)**:
    *   `RotaryEncoder`: 인터럽트 기반으로 회전과 버튼 입력을 처리하는 클래스.
    *   `PmsDriver`: PMS7003 센서와 시리얼 통신을 하며 데이터를 읽거나 명령(Sleep/Wake)을 보냅니다.
5.  **UI 렌더링 (Rendering)**: 각 화면(모니터, 그래프 등)을 OLED에 그리는 함수들입니다.
6.  **메인 로직 (Main Logic)**: `setup()`과 `loop()` 함수.

---

## 2. 주요 로직 흐름 (Flow Chart)

### Setup (초기화)
1.  **통신 시작**: 시리얼(디버그용)과 Serial1(센서용)을 시작합니다.
2.  **OLED 시작**: 화면을 초기화합니다.
3.  **인코더 시작**: 핀을 설정하고, 회전 감지를 위해 **인터럽트(Interrupt)**를 연결합니다.
4.  **센서 깨우기**: 처음에 센서를 Active 모드로 설정하여 측정을 시작합니다.

### Loop (무한 반복)

`loop()` 함수는 매 순간 다음 4가지 작업을 순서대로 수행합니다.

#### 1. 입력 처리 (Input Handling)
*   **인코더 회전**: 회전이 감지되면 화면을 전환하거나(모니터<->그래프 등), 설정 값을 변경합니다.
*   **버튼 클릭**: 설정 메뉴에서 값을 선택하거나, 꺼진 화면을 깨웁니다.
*   **화면 깨우기**: 만약 화면이 꺼져있다면(`SLEEP` 상태), 버튼이나 회전 입력이 들어올 때 즉시 화면을 켜고(`MONITOR` 상태로 변경) 루프를 처음부터 다시 시작합니다.

#### 2. 센서 제어 (Sensor Logic)
*   **측정 중 (Active)**:
    *   센서에서 데이터를 계속 읽어옵니다 (`PmsDriver::update`).
    *   30초가 지나면 센서를 재우고(`PmsDriver::sleep`), 현재 시간을 `lastSensorSleepTime`에 기록합니다.
    *   이때, 예약된 다음 절전 시간(`nextSleepInterval`)을 실제 적용 시간(`sleepInterval`)으로 업데이트합니다.
*   **휴식 중 (Sleep)**:
    *   설정된 시간(`sleepInterval`)이 지날 때까지 기다립니다.
    *   시간이 다 되면 센서를 깨웁니다(`PmsDriver::wake`).

#### 3. 화면 보호기 (Screen Saver)
*   마지막 조작 시간(`lastInteractionTime`)으로부터 60초가 지났는지 확인합니다.
*   지났다면 화면 상태를 `SLEEP`으로 바꾸고 OLED 전원을 끕니다.

#### 4. 화면 그리기 (Rendering)
*   현재 상태(`sys.currentState`)에 맞는 그리기 함수를 호출합니다.
    *   `renderMonitor()`: 숫자 위주 표시
    *   `renderGraph()`: 막대 그래프 표시
    *   `renderStats()`: 이벤트 카운트 표시
    *   `renderSettings()`: 절전 시간 설정 메뉴
*   `u8g2` 라이브러리의 페이지 버퍼 방식을 사용하여 화면을 갱신합니다.

---

## 3. 핵심 포인트 (Key Points)

*   **인터럽트 (Interrupt)**: 로터리 인코더는 `loop()`와 상관없이 돌아가는 즉시 감지되어야 하므로 인터럽트를 사용합니다.
*   **비동기 타이머 (Millis)**: `delay()`를 쓰지 않고 `millis()`를 사용하여, 센서가 쉬는 동안에도 화면은 계속 갱신되고 버튼 입력도 받습니다.
*   **상태 머신 (State Machine)**: `sys.currentState` 변수 하나로 현재 프로그램이 무엇을 해야 하는지(어떤 화면을 보여줄지)를 관리합니다.

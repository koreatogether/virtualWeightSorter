# 화면 깨어남 실패 현상 분석 보고서 (Debug Analysis Report)

## 1. 문제 상황 (Issue Description)
*   **현상**: 화면 보호기(Screen Saver) 진입 후 로터리 인코더 버튼을 눌러 깨우는 동작이 2~3회까지는 정상 작동하나, 그 이후(3~4회차)부터는 버튼을 눌러도 화면이 켜지지 않음.
*   **특이사항**: 초기 몇 번은 정상 동작하므로, 하드웨어 결선 문제는 아닐 가능성이 높음. 소프트웨어적인 상태 꼬임(State Lock)이나 메모리/타이밍 문제로 추정됨.

## 2. 코드 분석 및 예상 원인 (Code Analysis & Hypotheses)

현재 코드(`u8g2libStyle_debug_analysis.ino`)를 기반으로 의심되는 원인들을 분석했습니다.

### 가설 1: `u8g2.setPowerSave()` 함수의 반복 호출 문제
*   **분석**: `u8g2` 라이브러리의 `setPowerSave(1)`(Sleep)과 `setPowerSave(0)`(Wake)을 반복적으로 호출할 때, 특정 OLED 컨트롤러(SSD1306)에서 상태가 제대로 복구되지 않는 경우가 간혹 보고됩니다.
*   **의심**: `setPowerSave(0)` 호출 후 화면이 켜지는 데 필요한 내부 초기화가 완전히 이루어지지 않은 상태에서 다시 그리기를 시도하거나, 너무 빈번하게 끄고 켜기를 반복하면 디스플레이 컨트롤러가 먹통이 될 수 있습니다.
*   **검증 방법**: `setPowerSave(0)` 호출 직후 `u8g2.initDisplay()`를 강제로 호출하여 디스플레이를 재초기화해 봅니다.

### 가설 2: 인터럽트와 메인 루프의 충돌 (Race Condition)
*   **분석**: `RotaryEncoder`는 인터럽트(`ISR`)를 통해 `_position` 등을 업데이트합니다. 메인 루프(`loop`)에서도 이 값을 읽고 씁니다.
*   **의심**: 화면이 꺼져있는 동안(`SLEEP` 상태)에도 인터럽트는 계속 발생합니다. 만약 `SLEEP` 상태에서 버튼이나 회전 입력이 아주 빠르게 반복될 때, `sys.currentState`를 `MONITOR`로 바꾸는 로직과 다시 `SLEEP`으로 보내려는 로직(`SCREEN_SAVER_TIMEOUT` 체크) 사이에서 경합(Race Condition)이 발생할 수 있습니다.
    *   특히 `sys.lastInteractionTime` 갱신 시점과 `currentMillis` 비교 시점이 미묘하게 엇갈리면, 깨어나자마자 다시 잠드는 현상이 반복될 수 있습니다.
*   **검증 방법**: 깨어난 직후(`Waking up!`)에는 일정 시간(예: 1~2초) 동안은 절대로 다시 화면 보호기로 들어가지 않도록 **강제 유예 시간(Grace Period)**을 둡니다.

### 가설 3: `return` 문으로 인한 로직 건너뛰기
*   **분석**: 현재 `loop()` 내에서 화면을 깨울 때 `return;`을 사용하여 루프를 즉시 종료하고 다음 턴으로 넘깁니다.
    ```cpp
    if ((diff != 0 || clicked || rawBtn) && sys.currentState == AppState::SLEEP) {
        // ...
        return; // <--- 여기
    }
    ```
*   **의심**: `return`을 하면 그 아래에 있는 `if (sys.currentState != AppState::SLEEP)` 블록(화면 그리기)이 해당 루프에서는 실행되지 않습니다. 다음 루프에서 그려져야 하는데, 만약 다음 루프 진입 전에 어떤 이유로 다시 `SLEEP` 조건이 만족되거나, `u8g2` 버퍼가 꼬이면 화면이 안 켜진 것처럼 보일 수 있습니다.
*   **검증 방법**: `return`을 제거하고, 상태 변경 후 바로 화면을 한 번 그리도록 로직을 수정합니다.

### 가설 4: 시리얼 통신 버퍼 오버플로우
*   **분석**: `Serial.println`을 디버깅용으로 사용 중입니다.
*   **의심**: 만약 버튼이 계속 눌려있거나 노이즈로 인해 `rawBtn`이 계속 `true`가 되면, `Serial.println("Waking up...")`이 고속으로 반복 출력되어 시리얼 버퍼가 가득 차고, 이로 인해 아두이노가 일시적으로 멈추거나(Blocking) 느려질 수 있습니다.
*   **검증 방법**: `Waking up` 메시지는 상태가 `SLEEP`에서 `MONITOR`로 바뀔 때 **딱 한 번만** 출력되도록 플래그를 사용합니다.

---

## 3. 제안하는 수정 방안 (Proposed Fixes)

다음 순서대로 코드를 수정하여 테스트해 볼 것을 권장합니다.

1.  **화면 깨움 로직 개선**: `return` 제거 및 상태 변경 시 즉시 화면 갱신.
2.  **재잠듦 방지**: 깨어난 후 최소 2초간은 화면 보호기 진입 금지.
3.  **OLED 재초기화**: `setPowerSave(0)` 대신 `begin()` 또는 `initDisplay()` 사용 고려.

### 수정 코드 예시 (제안)

```cpp
// loop() 내부

// ... 입력 처리 ...

// 화면 깨우기 로직 수정
if (sys.currentState == AppState::SLEEP) {
    if (diff != 0 || clicked || rawBtn) {
        Serial.println("Wake Up Triggered!");
        
        sys.currentState = AppState::MONITOR;
        sys.lastInteractionTime = currentMillis; // 시간 갱신
        
        u8g2.setPowerSave(0); // 화면 켜기
        // u8g2.initDisplay(); // 필요 시 추가 (강력한 초기화)
        
        // 깨어난 직후 화면을 바로 그리기 위해 루프를 종료하지 않고 진행
    }
}

// 화면 보호기 로직 수정 (유예 시간 추가)
// 깨어난 지 2초(2000ms)가 지났을 때만 화면 보호기 체크
if (sys.currentState != AppState::SLEEP && 
    (currentMillis - sys.lastInteractionTime > Config::SCREEN_SAVER_TIMEOUT) &&
    (currentMillis - sys.lastInteractionTime > 2000)) { 
    
    Serial.println("Entering Sleep...");
    sys.currentState = AppState::SLEEP;
    u8g2.setPowerSave(1);
}
```

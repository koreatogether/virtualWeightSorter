# 시나리오 10: 전체 센서 임계값 일괄 설정

## 목적
모든 센서에 동일한 상한/하한 온도 임계값을 일괄 적용하는 기능을 테스트합니다.

## 전제 조건
- DS18B20 센서 최소 3개 이상 연결됨
- 각 센서에 고유 ID가 할당됨 (1, 2, 3...)
- 시리얼 통신이 정상적으로 초기화됨

## 시나리오 단계

### 1. 메뉴 진입
1. 시리얼 모니터에서 "menu" 또는 "m" 입력
2. ID 변경 메뉴에서 "6" 입력하여 "Threshold Settings" 선택
3. 임계값 설정 메뉴가 표시됨

### 2. 전체 일괄 설정 선택
1. "2" 입력하여 "Set Global Threshold" 선택
2. 전체 센서에 대한 안내 메시지가 표시됨:
   ```
   --- Global Threshold Setting ---
   Will apply thresholds to all 3 sensors.
   Enter high temperature threshold (-55 to 125°C) or press Enter to skip:
   ```

### 3. 상한 온도 일괄 설정
1. "80" 입력하여 모든 센서의 상한 온도를 80°C로 설정
2. 확인 메시지가 표시됨:
   ```
   High threshold set to: 80°C
   ```

### 4. 하한 온도 일괄 설정
1. 하한 온도 입력 프롬프트가 표시됨:
   ```
   Enter low temperature threshold (-55 to 125°C) or press Enter to skip:
   ```
2. "5" 입력하여 모든 센서의 하한 온도를 5°C로 설정
3. 확인 메시지가 표시됨:
   ```
   Low threshold set to: 5°C
   ```

### 5. 일괄 적용 실행
1. 모든 센서에 임계값이 적용됨:
   ```
   Applying thresholds to all sensors...
   ✅ Thresholds successfully applied to all sensors!
   Returning to threshold menu.
   ```

### 6. 설정 확인
1. 임계값 설정 메뉴에서 "3" 입력하여 "View Current Thresholds" 선택
2. 모든 센서에 동일한 임계값이 설정된 것을 확인:
   ```
   --- Current Threshold Settings ---
   | Sensor | ID  | High Temp | Low Temp |
   | ------ | --- | --------- | -------- |
   | 1      | 1   | 80°C      | 5°C      |
   | 2      | 2   | 80°C      | 5°C      |
   | 3      | 3   | 80°C      | 5°C      |
   ```

## 부분적 설정 테스트

### 7. 상한 온도만 설정
1. "2" 입력하여 전체 일괄 설정 선택
2. 상한 온도에 "85" 입력
3. 하한 온도에서 Enter 키만 입력 (스킵)
4. 결과 확인:
   ```
   High threshold set to: 85°C
   Skipped low threshold.
   Applying thresholds to all sensors...
   ✅ Thresholds successfully applied to all sensors!
   ```

### 8. 하한 온도만 설정
1. "2" 입력하여 전체 일괄 설정 선택
2. 상한 온도에서 Enter 키만 입력 (스킵)
3. 하한 온도에 "0" 입력
4. 결과 확인:
   ```
   Skipped high threshold.
   Low threshold set to: 0°C
   Applying thresholds to all sensors...
   ✅ Thresholds successfully applied to all sensors!
   ```

### 9. 설정값 없이 진행
1. "2" 입력하여 전체 일괄 설정 선택
2. 상한 온도에서 Enter 키만 입력 (스킵)
3. 하한 온도에서 Enter 키만 입력 (스킵)
4. 결과 확인:
   ```
   Skipped high threshold.
   Skipped low threshold.
   No thresholds were set.
   Returning to threshold menu.
   ```

## 예외 상황 테스트

### 10. 잘못된 온도 범위 입력
1. 전체 일괄 설정에서 상한 온도에 "-100" 입력 (범위 미만)
2. 오류 메시지 표시:
   ```
   Invalid temperature. Enter value between -55 and 125°C:
   ```
3. "75" 입력하여 정상 범위 값 설정

### 11. 하한 > 상한 조건 테스트
1. 상한 온도를 "40" 설정
2. 하한 온도에 "50" 입력 (상한보다 높음)
3. 오류 메시지 표시:
   ```
   Low threshold must be less than high threshold. Try again:
   ```
4. "20" 입력하여 정상 값 설정

### 12. 센서 없는 상황
1. 센서가 연결되지 않은 상태에서 전체 일괄 설정 선택
2. 안내 메시지 표시:
   ```
   --- Global Threshold Setting ---
   No sensors found. Returning to threshold menu.
   ```

### 13. 부분적 실패 상황 시뮬레이션
(일부 센서에서 설정 실패하는 경우 - Mock 객체로 테스트)
```
Applying thresholds to all sensors...
⚠️ Thresholds applied to 2 out of 3 sensors.
```

### 14. 취소 기능 테스트
1. 상한 온도 입력 단계에서 "cancel" 입력
2. 취소 메시지 표시:
   ```
   Cancelled. Returning to threshold menu.
   ```

### 15. 타임아웃 테스트
1. 온도 입력 중 60초간 입력 없이 대기
2. 타임아웃 메시지 표시:
   ```
   ⏰ Threshold setting timeout. Returning to normal operation.
   ```

## 성공 기준
- 모든 센서에 동일한 임계값을 정상적으로 일괄 설정할 수 있음
- 상한/하한 온도를 선택적으로 설정할 수 있음
- 설정값이 없을 때 적절한 안내 메시지 표시
- 잘못된 입력에 대한 적절한 오류 처리
- 부분적 실패 상황에 대한 정확한 리포트
- 취소 및 타임아웃 처리가 정상 작동

## 관련 파일
- `App.h`: ThresholdSubState 열거형, 관련 메서드 선언
- `App.cpp`: processGlobalThreshold() 메서드
- `DS18B20_Sensor.h/cpp`: setThresholdHighByIndex(), setThresholdLowByIndex() 메서드
- `test_scenario10.cpp`: 해당 시나리오의 유닛 테스트

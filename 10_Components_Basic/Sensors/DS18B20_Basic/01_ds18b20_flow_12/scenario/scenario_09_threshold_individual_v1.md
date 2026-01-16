# 시나리오 09: 개별 센서 임계값 설정 (소프트웨어 기반)

## 목적
개별 센서에 대한 상한/하한 온도 임계값을 소프트웨어적으로(th/tl 배열 기반) 설정하고 확인하는 기능을 테스트합니다.

## 전제 조건
- DS18B20 센서 최소 2개 이상 연결됨
- 각 센서에 고유 ID가 할당됨 (1, 2, 3, 4...)
- 시리얼 통신이 정상적으로 초기화됨
- 소프트웨어 임계값 시스템 (DataProcessor 기반) 적용됨

## 시나리오 단계

### 1. 초기 상태 확인
1. 시스템 시작 시 센서 테이블 확인:
   ```
   |  No |   Device ID   |        ADDRESS      |  Temp(℃) |  기준상한 | 초과온도 | 기준하한 | 초과온도 | 상태         |
   |   1 |             1 | 2858:8284:0000:000E |    28.50 |   125.00 |       + |   -55.00 |       - | 이상없음 |
   |   2 |             2 | 285C:8285:0000:005D |    29.00 |   125.00 |       + |   -55.00 |       - | 이상없음 |
   ```
2. 기본 임계값이 상한 125°C, 하한 -55°C로 설정되어 있음을 확인

### 2. 메뉴 진입
1. 시리얼 모니터에서 "menu" 또는 "m" 입력
2. ID 변경 메뉴가 표시됨
3. "6" 입력하여 "Threshold Settings" 선택
4. 임계값 설정 메뉴가 표시됨:
   ```
   -------------------------------
   Threshold Settings Menu:
   1. Set Individual Threshold
   2. Set Global Threshold  
   3. View Current Thresholds
   4. Clear Thresholds
   5. Back to Main Menu
   Enter the number of the desired option and press Enter.
   Menu will timeout in 60 seconds if no input is received.
   -------------------------------
   ```

### 3. 개별 임계값 설정 선택
1. "1" 입력하여 "Set Individual Threshold" 선택
2. 현재 센서 목록과 임계값이 표시됨:
   ```
   --- Individual Sensor Threshold Setting ---
   Current sensors:
   Sensor 1 (ID: 1) - High: 125°C, Low: -55°C
   Sensor 2 (ID: 2) - High: 125°C, Low: -55°C
   Sensor 3 (ID: 3) - High: 125°C, Low: -55°C
   Sensor 4 (ID: 4) - High: 125°C, Low: -55°C
   Enter sensor number (1-4) or 'c' to cancel:
   ```

### 4. 센서 선택 및 상한 온도 설정
1. "1" 입력하여 첫 번째 센서 선택
2. 상한 온도 입력 프롬프트가 표시됨:
   ```
   Selected Sensor 1
   Enter high temperature threshold (-55 to 125°C) or press Enter to skip:
   ```
3. "35" 입력하여 상한 온도를 35°C로 설정 (정수만 입력)
4. 확인 메시지가 표시됨:
   ```
   High threshold set to: 35°C
   ```

### 5. 하한 온도 설정
1. 하한 온도 입력 프롬프트가 표시됨:
   ```
   Enter low temperature threshold (-55 to 125°C) or press Enter to skip:
   ```
2. "20" 입력하여 하한 온도를 20°C로 설정 (정수만 입력)
3. 확인 메시지가 표시됨:
   ```
   Low threshold set to: 20°C
   ✅ Thresholds successfully applied!
   ```

### 6. 즉시 테이블 업데이트 확인
1. 설정 완료 후 센서 테이블이 즉시 업데이트됨:
   ```
   Sensor table updated after threshold setting:
   |  No |   Device ID   |        ADDRESS      |  Temp(℃) |  기준상한 | 초과온도 | 기준하한 | 초과온도 | 상태         |
   |   1 |             1 | 2858:8284:0000:000E |    28.50 |    35.00 |       + |    20.00 |       - | 이상없음 |
   |   2 |             2 | 285C:8285:0000:005D |    29.00 |   125.00 |       + |   -55.00 |       - | 이상없음 |
   ```
2. 센서 1의 임계값이 변경되고, 다른 센서는 기본값 유지됨을 확인
3. "Returning to threshold menu." 메시지 후 메뉴로 복귀

### 7. ID 지속성 확인 (중요!)
1. 다음 주기적 보고에서 센서 ID가 유지되는지 확인:
   ```
   ==================== NEW REPORT ====================
   |  No |   Device ID   |        ADDRESS      |  Temp(℃) |  기준상한 | 초과온도 | 기준하한 | 초과온도 | 상태         |
   |   1 |             1 | 2858:8284:0000:000E |    28.50 |    35.00 |       + |    20.00 |       - | 이상없음 |
   |   2 |             2 | 285C:8285:0000:005D |    29.00 |   125.00 |       + |   -55.00 |       - | 이상없음 |
   ```
2. **중요**: 센서 1이 "NONE"으로 사라지지 않고 ID가 정상 유지되어야 함

### 8. 설정 확인
1. 임계값 설정 메뉴에서 "3" 입력하여 "View Current Thresholds" 선택
2. 설정된 임계값이 표시됨:
   ```
   --- Current Threshold Settings ---
   | Sensor | ID  | High Temp | Low Temp |
   | ------ | --- | --------- | -------- |
   | 1      | 1   | 35°C      | 20°C     |
   | 2      | 2   | 125°C     | -55°C    |
   | 3      | 3   | 125°C     | -55°C    |
   | 4      | 4   | 125°C     | -55°C    |
   ```

## 예외 상황 테스트

### 9. 잘못된 온도 범위 입력
1. 개별 임계값 설정에서 센서 선택 후
2. 상한 온도에 "150" 입력 (범위 초과)
3. 오류 메시지 표시:
   ```
   Invalid temperature. Enter value between -55 and 125°C:
   ```
4. "50" 입력하여 정상 범위 값 설정

### 10. 하한 > 상한 조건 테스트
1. 상한 온도를 "50" 설정
2. 하한 온도에 "60" 입력 (상한보다 높음)
3. 오류 메시지 표시:
   ```
   Low threshold must be less than high threshold. Try again:
   ```
4. "30" 입력하여 정상 값 설정

### 11. 소수점 입력 거부 테스트
1. 상한 온도에 "35.5" 입력
2. 정수로 변환되어 "35"로 처리됨
3. 소수점은 무시되고 정수 부분만 사용됨

### 12. 취소 기능 테스트
1. 센서 선택 단계에서 "c" 입력
2. 취소 메시지 표시:
   ```
   Cancelled. Returning to threshold menu.
   ```

### 13. 타임아웃 테스트
1. 임계값 설정 중 60초간 입력 없이 대기
2. 타임아웃 메시지 표시:
   ```
   ⏰ Threshold setting timeout. Returning to normal operation.
   ```

## 성공 기준
- 개별 센서에 상한/하한 온도를 정수로 정상 설정할 수 있음
- 설정된 임계값이 DataProcessor의 th/tl 배열에 저장됨 (함수명: getTh/getTl)
- 임계값 설정 후 센서 ID가 사라지지 않고 유지됨 ⭐ **핵심**
- 설정된 임계값이 센서 테이블에 즉시 반영됨 (테이블 내 기준상한/기준하한이 th/tl 값임을 명시)
- 잘못된 입력에 대한 적절한 오류 처리
- 취소 및 타임아웃 처리가 정상 작동
- 메뉴 간 전환이 원활함

## 관련 파일
- `App.h`: ThresholdSubState 열거형, 관련 메서드 선언
- `App.cpp`: processIndividualThreshold() 메서드
- `DataProcessor.h/cpp`: 소프트웨어 임계값 관리 (setThreshold, getThresholdHigh, getThresholdLow)
- `test_scenario9.cpp`: 해당 시나리오의 유닛 테스트

## 주요 변경 사항 (v6.10)
- 하드웨어 기반 임계값에서 소프트웨어 기반으로 전환
- 센서 ID와 임계값의 메모리 충돌 문제 해결
- 정수만 입력 받도록 변경 (소수점 입력 불가)
- `printCurrentSensorReport()`에서 `begin()` 호출 제거로 ID 지속성 보장

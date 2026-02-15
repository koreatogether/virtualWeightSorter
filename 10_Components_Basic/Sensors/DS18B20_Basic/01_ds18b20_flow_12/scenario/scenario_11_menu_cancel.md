# 시나리오 11: 메뉴 취소 기능 검증

## 📋 시나리오 정보
- **시나리오 ID**: 11
- **제목**: 메뉴 취소 기능 정상 동작 검증
- **우선순위**: 🚨 높음 (사용자 보고 이슈)
- **테스트 타입**: 메뉴 시스템 기능 테스트
- **관련 이슈**: m → 5번을 눌러도 취소가 안되는 현상

## 🎯 테스트 목표
메뉴에서 5번 취소 선택 시 정상적으로 일반 모드로 복귀하는지 검증

## 📝 상세 시나리오

### 사전 조건
- 시스템이 정상 동작 모드 (NORMAL_OPERATION)
- 센서가 1개 이상 감지된 상태
- 시리얼 통신 정상 연결

### 테스트 케이스 11.1: 기본 메뉴 취소
**단계**:
1. 정상 동작 모드에서 'm' 입력
2. 메뉴가 표시되는지 확인
3. '5' 입력 (Cancel 선택)
4. 취소 메시지 확인
5. 일반 모드로 복귀 확인

**예상 결과**:
```
===== ID Change Menu =====
1. Change Individual Sensor ID
2. Change Multiple Sensor IDs  
3. Auto-assign IDs
4. Reset All IDs
5. Cancel
6. Threshold Settings
Enter your choice: 5
Cancelled. Returning to normal operation.
[센서 테이블 다시 표시]
```

### 테스트 케이스 11.2: 임계값 메뉴 취소
**단계**:
1. 정상 동작 모드에서 'm' 입력
2. '6' 입력 (Threshold Settings)
3. 임계값 메뉴 표시 확인
4. '5' 입력 (Cancel/Return)
5. 메인 메뉴로 복귀 확인

**예상 결과**:
```
===== Threshold Menu =====
1. Set Individual Sensor Threshold
2. Set Global Threshold
3. View Current Thresholds
4. Clear All Thresholds
5. Return to Main Menu
Enter your choice: 5
Returning to main menu.
[메인 메뉴 다시 표시]
```

### 테스트 케이스 11.3: 연속 취소 동작
**단계**:
1. 'm' → '6' → '5' → '5' 연속 입력
2. 각 단계에서 정상 복귀 확인
3. 최종적으로 일반 모드 도달 확인

### 테스트 케이스 11.4: 취소 후 입력 버퍼 확인
**단계**:
1. 메뉴에서 취소 실행
2. 즉시 다른 명령어 입력 테스트
3. 입력 버퍼가 정상 초기화되었는지 확인

## 🔧 구현해야 할 단위 테스트

### 테스트 함수 목록
1. `test_menu_cancel_from_main_menu()` - 메인 메뉴 취소
2. `test_menu_cancel_from_threshold_menu()` - 임계값 메뉴 취소  
3. `test_input_buffer_clear_after_cancel()` - 취소 후 입력 버퍼 초기화
4. `test_state_transition_on_cancel()` - 취소 시 상태 전환
5. `test_consecutive_menu_operations()` - 연속 메뉴 동작

### Mock 객체 설정
```cpp
// MockCommunicator에서 모의할 입력 시퀀스
mockComm->setInputSequence({"m", "5"});
mockComm->expectOutput("Cancelled. Returning to normal operation.");
```

## 🐛 디버깅 포인트

### 확인할 코드 위치
1. **App.cpp:252-257**: 메인 메뉴 case 5 처리
```cpp
case 5:
    _comm->println("Cancelled. Returning to normal operation.");
    currentAppState = NORMAL_OPERATION;
    _comm->clearInputBuffer();
    printCurrentSensorReport();
    break;
```

2. **App.cpp:798-802**: 임계값 메뉴 case 5 처리
```cpp
case 5:
    _comm->println("Returning to main menu.");
    currentAppState = MENU_ACTIVE;
    currentThresholdSubState = THRESHOLD_NONE;
    break;
```

### 잠재적 문제점
1. **입력 버퍼 처리**: `clearInputBuffer()` 호출 타이밍
2. **상태 전환**: `currentAppState` 변경 후 추가 처리 필요성
3. **메뉴 표시**: `forceMenuPrint` 플래그 관리
4. **타이밍 이슈**: `menuDisplayMillis` 설정

## 📊 검증 기준

### 성공 조건
- [ ] 5번 입력 시 즉시 취소 메시지 출력
- [ ] 상태가 올바른 모드로 전환 (NORMAL_OPERATION 또는 MENU_ACTIVE)
- [ ] 입력 버퍼가 정상 초기화
- [ ] 다음 입력이 정상 처리됨
- [ ] 메뉴 표시 플래그가 올바르게 설정

### 실패 조건 (현재 이슈)
- [ ] 5번 입력 후에도 메뉴 상태 유지
- [ ] 취소 메시지 출력되지 않음
- [ ] 입력 버퍼 문제로 다음 입력 무시
- [ ] 상태 전환 실패

## 🚀 테스트 실행 방법

### 수동 테스트
```bash
# 1. 펌웨어 업로드
arduino-cli upload --fqbn arduino:renesas_uno:unor4wifi --port COM4 --input-dir ./build

# 2. 시리얼 모니터 연결 (9600 baud)
# 3. 시나리오 11 수동 실행
```

### 자동 테스트
```bash
# 유닛 테스트 실행
cd test
./run_test.bat scenario11
```

## 📁 관련 파일
- `test/test_scenario11.cpp` - 구현할 테스트 파일
- `App.cpp:252-257` - 메인 메뉴 취소 로직
- `App.cpp:798-802` - 임계값 메뉴 취소 로직
- `Communicator.h` - clearInputBuffer() 구현

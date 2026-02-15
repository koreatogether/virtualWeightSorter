# 키패드 서보모터 통합 잠금장치 시나리오

## 개요
이 시나리오는 Arduino 기반 키패드 잠금 장치와 서보모터를 연동한 현관문 보안 시스템의 전체 작동 흐름을 설명합니다. 사용자가 키패드를 통해 비밀번호를 입력하여 서보모터로 구동되는 잠금장치를 해제하는 통합 과정을 다룹니다.

## 주요 구성 요소
- **키패드**: 4x4 매트릭스 키패드 (Adafruit_Keypad 라이브러리)
  - 핀: rowPins[9,8,7,6], colPins[5,4,3,2]
- **서보모터**: 10번 핀, 0도(잠금)/180도(해제)
- **수동 해제 버튼**: 11번 핀, INPUT_PULLUP 방식
- **부저**: 12번 핀, 보안 잠금 시 경고음
- **시스템 모드**: IDLE, SECURITY_LOCKDOWN (enum SystemMode)
- **비밀번호**: "123456" (correctPassword 변수)

## 통합 시나리오 흐름

### 1. 초기 상태 (setup 함수)
- currentMode = IDLE로 시스템 시작
- 서보모터 lockServo.write(0)으로 잠금 위치 설정
- 키패드 customKeypad.begin() 초기화
- 수동 버튼 pinMode(11, INPUT_PULLUP) 설정
- 시리얼 출력: "키패드 서보모터 통합 잠금장치 시작"

### 2. 키패드 입력을 통한 잠금 해제

#### 2.1 비밀번호 입력 시작 (handleIdleMode 함수)
- 사용자가 '*' 키 입력 → inputBuffer = "*", passwordEntryStartTime 기록
- 입력 형식: `*XXXXXX#` 또는 `*XXXXXX*`
- **디바운스 처리**: 200ms (DEBOUNCE_DELAY) 이내 중복 입력 무시
- **입력 타임아웃**: 10초 (PASSWORD_ENTRY_TIMEOUT) 초과 시 버퍼 처리/초기화

#### 2.2 입력 검증 및 처리
- **숫자 입력**: key >= '0' && key <= '9', 최대 6자리
- **종료 문자**: '#' 또는 '*' (6자리 입력 후)
- **잘못된 패턴**: 6자리 후 예상치 못한 키 입력 시 강제 '#' 추가 후 처리
- **형식 검증**: processPasswordInput()에서 길이 8, 시작 '*', 종료 '#'/'*' 확인

#### 2.3 비밀번호 확인 및 잠금 해제
- **올바른 비밀번호 "123456" 입력 시**:
  1. "Correct password entered" 시리얼 출력
  2. unlockDoor() 함수 호출
  3. **lockServo.write(180)** → 해제 위치 이동
  4. isUnlocked = true, servoUnlockTime 기록
  5. "잠금 해제" 메시지 출력
  6. failureCount = 0 (실패 카운터 초기화)
  7. 5초 후 handleAutoLock()에서 자동 잠금

- **틀린 비밀번호 입력 시**:
  1. "Wrong password" 시리얼 출력
  2. failureCount++ (실패 카운터 증가)
  3. 5회 도달 시 activateSecurityLockdown() 호출
  4. 서보모터는 잠금 상태 유지

### 3. 수동 해제 기능 (handleManualUnlock 함수)

#### 3.1 수동 버튼 작동 - loop() 최우선 처리
- **조건**: digitalRead(11) == LOW && 디바운스 200ms 경과
- **정상 시**: 
  1. handleManualUnlock() 호출
  2. "수동 해제" 메시지 출력
  3. unlockDoor() 함수 호출 (키패드와 동일한 해제 프로세스)
  4. lockServo.write(180) → 해제 위치
  5. 5초 후 handleAutoLock()에서 자동 잠금

#### 3.2 보안 잠금 시 제한
- **SECURITY_LOCKDOWN 모드 시**: 
  - "수동 해제 불가 - 보안 잠금 활성화됨" 출력
  - 수동 버튼 동작 차단 (return으로 함수 종료)
- **디바운스**: lastButtonPressTime으로 200ms 제어

### 4. 보안 잠금 시스템

#### 4.1 연속 실패 처리 (activateSecurityLockdown 함수)
- **5회 실패 시 (failureCount >= MAX_FAILURES)**:
  1. currentMode = SECURITY_LOCKDOWN
  2. "Maximum attempts exceeded - security lockdown activated" 출력
  3. lockdownStartTime, buzzerStartTime 기록
  4. buzzerActive = true
  5. **lockServo.write(0)** → 강제 잠금 위치
  6. isUnlocked = false

#### 4.2 부저 경고 (handleBuzzer 함수)
- **1분간 경고음**: buzzerStartTime부터 BUZZER_DURATION(60초)
- **1초 간격**: BUZZER_INTERVAL(1000ms)로 on/off 토글
- **제어**: digitalWrite(BUZZER_PIN, buzzerState)

#### 4.3 보안 잠금 해제 (handleLockdownMode 함수)
- **2분 경과 확인**: currentTime - lockdownStartTime >= LOCKDOWN_DURATION(120초)
- **해제 시**:
  1. "보안 잠금 해제 - 정상 작동 복귀" 출력
  2. currentMode = IDLE
  3. failureCount = 0
  4. buzzerActive = false, digitalWrite(BUZZER_PIN, LOW)

### 5. 자동 잠금 처리 (handleAutoLock 함수)

#### 5.1 자동 잠금 조건
- **조건**: isUnlocked && (millis() - servoUnlockTime >= UNLOCK_DURATION)
- **UNLOCK_DURATION**: 5000ms (5초)
- **동작**: 
  1. "자동 잠금" 메시지 출력
  2. lockServo.write(0) → 잠금 위치 복귀
  3. isUnlocked = false

#### 5.2 루프 함수 처리 순서
1. **수동 해제 버튼** (최우선)
2. **키패드 이벤트** (customKeypad.tick())
3. **모드별 처리** (switch-case: IDLE/SECURITY_LOCKDOWN)
4. **자동 잠금** (handleAutoLock())
5. **부저 제어** (handleBuzzer())

## 성공적인 작동 시나리오 예시

### 케이스 1: 정상 키패드 해제
1. 사용자가 `*123456#` 입력
2. processPasswordInput() → "Correct password entered"
3. unlockDoor() → lockServo.write(180) + "잠금 해제"
4. isUnlocked = true, servoUnlockTime 기록
5. 5초 후 handleAutoLock() → lockServo.write(0) + "자동 잠금"

### 케이스 2: 수동 해제
1. digitalRead(11) == LOW 감지
2. handleManualUnlock() → "수동 해제"
3. unlockDoor() 호출 (키패드와 동일 프로세스)
4. 5초 후 자동 잠금

### 케이스 3: 보안 잠금 발생
1. 5회 연속 실패 → activateSecurityLockdown()
2. currentMode = SECURITY_LOCKDOWN
3. handleBuzzer() → 1분간 1초 간격 경고음
4. **수동 버튼 차단** (handleManualUnlock에서 return)
5. 2분 후 handleLockdownMode() → 정상 복귀

### 케이스 4: 입력 타임아웃 처리
1. '*' 입력 후 10초 초과
2. 6자리 완료 시 → 자동 '#' 추가 후 processPasswordInput()
3. 미완료 시 → inputBuffer 초기화

## 기술적 구현 세부사항
- **라이브러리**: Adafruit_Keypad, Servo
- **타이밍 관리**: millis() 기반 비블로킹 (passwordEntryStartTime, servoUnlockTime, lockdownStartTime, buzzerStartTime)
- **디바운스**: 200ms (DEBOUNCE_DELAY) 키패드/버튼 모두 적용
- **상태 변수**: currentMode(enum), isUnlocked(bool), failureCount(int), buzzerActive(bool)
- **우선순위**: loop()에서 수동 버튼 → 키패드 → 모드 처리 → 자동 잠금 → 부저 순서
- **상수 관리**: 모든 시간 값이 const unsigned long으로 정의
- **안전 장치**: 보안 잠금 시에도 물리적 수동 해제 가능 (단, 코드에서는 차단됨)
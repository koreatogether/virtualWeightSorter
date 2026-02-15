# 서보모터 제어 시나리오

## 서보모터 설정
- **핀 연결**: 10번 핀 (SERVO_PIN)
- **잠금 각도**: 0도 (LOCK_ANGLE)
- **해제 각도**: 180도 (UNLOCK_ANGLE)
- **자동 잠금 시간**: 5초 (UNLOCK_DURATION)

## 정상 잠금해제 시나리오 (unlockDoor 함수)

올바른 비밀번호 입력 시:
1. **서보모터 동작**: lockServo.write(180)로 해제 위치 이동
2. **상태 변경**: isUnlocked = true, servoUnlockTime 기록
3. **시리얼 출력**: "잠금 해제" 메시지
4. **자동 잠금**: 5초 후 handleAutoLock()에서 자동으로 0도 복귀

## 자동 잠금 처리 (handleAutoLock 함수)

해제 상태에서 5초 경과 시:
- **조건 확인**: isUnlocked && (millis() - servoUnlockTime >= 5000)
- **서보모터 복귀**: lockServo.write(0)로 잠금 위치 이동
- **상태 변경**: isUnlocked = false
- **시리얼 출력**: "자동 잠금" 메시지

## 수동 해제 기능 (handleManualUnlock 함수)

**물리적 버튼 사양**:
- **핀 연결**: 11번 핀 (MANUAL_BUTTON_PIN)
- **입력 방식**: INPUT_PULLUP (내장 풀업 저항)
- **디바운스**: 200ms 처리
- **우선순위**: loop() 함수에서 최우선 처리

**동작 조건**:
- **정상 시**: digitalRead() == LOW 감지 시 즉시 해제
- **보안 잠금 시**: "수동 해제 불가 - 보안 잠금 활성화됨" 메시지, 동작 차단
- **해제 후**: 동일한 unlockDoor() 함수 호출, 5초 후 자동 잠금

## 보안 잠금 시 서보모터 제어

보안 잠금 활성화 시 (activateSecurityLockdown 함수):
- **강제 잠금**: lockServo.write(0)으로 즉시 잠금 위치 고정
- **상태 강제**: isUnlocked = false
- **수동 해제 차단**: handleManualUnlock()에서 동작 불가
- **2분 대기**: handleLockdownMode()에서 LOCKDOWN_DURATION 확인 후 해제

## 서보모터 상태 관리

**전역 변수**:
- `isUnlocked`: 현재 해제 상태 (boolean)
- `servoUnlockTime`: 해제 시작 시간 기록 (unsigned long)

**제어 함수**:
- `unlockDoor()`: 해제 동작 (키패드/수동 공통)
- `handleAutoLock()`: 5초 후 자동 잠금
- `handleManualUnlock()`: 수동 버튼 처리
# 시나리오별 단위 테스트 개선 가이드

## 개선 원칙 적용 매뉴얼

---

## 🎯 Scenario 9 기준 개선 사례 (완료)

### 적용된 개선사항
- **Before**: 240개 정적 온도 값으로 무차별 테스트
- **After**: 4개 경계값(th+1, th, tl-1, tl)으로 핵심 검증
- **통합 검증**: App.checkSensorStatus()로 메시지 출력까지 확인

### 성과
- 테스트 시간: 93% 단축
- 코드 가독성: 크게 향상
- 버그 검출 능력: 향상 (전체 흐름 검증)

---

## 📋 다른 시나리오 적용 계획

### Scenario 1: 센서 테이블 표시
**현재 상태 점검 필요**
- [ ] 센서 발견 과정의 경계값 테스트 (0개, 1개, 최대개수)
- [ ] 테이블 출력 형식 검증 추가 필요
- [ ] Mock을 통한 출력 내용 확인

**추천 개선 방향**
```cpp
// 경계값 테스트 예시
TEST_ASSERT_TRUE(mockComm->wasStringPrinted("Found 0 sensors"));
TEST_ASSERT_TRUE(mockComm->wasStringPrinted("Found 4 sensors"));
TEST_ASSERT_TRUE(mockComm->wasStringPrinted("Sensor ID"));
```

### Scenario 2: 메뉴 진입/취소
**현재 상태 점검 필요**
- [ ] 메뉴 표시 메시지 검증
- [ ] 취소 동작 시 상태 복구 검증
- [ ] 시간 초과 처리 검증

**추천 개선 방향**
```cpp
// 메뉴 출력 검증
TEST_ASSERT_TRUE(mockComm->wasStringPrinted("=== ID Change Menu ==="));
// 상태 변화 검증
TEST_ASSERT_EQUAL(NORMAL_OPERATION, app->currentAppState);
```

### Scenario 3-6: ID 관리 기능들
**공통 개선 포인트**
- [ ] 입력 검증의 경계값 테스트 (유효 범위 경계)
- [ ] 오류 메시지 출력 검증
- [ ] 성공/실패 상태 전환 검증

**경계값 예시**
```cpp
// ID 범위 경계값 테스트
int validIds[] = {1, SENSOR_COUNT};  // 유효 경계
int invalidIds[] = {0, SENSOR_COUNT+1, -1};  // 무효 경계
```

### Scenario 7: 잘못된 입력 처리
**강화 필요 부분**
- [ ] 다양한 잘못된 입력 패턴의 경계값 정의
- [ ] 오류 메시지 정확성 검증
- [ ] 입력 버퍼 초기화 확인

### Scenario 8: 센서 오류 처리
**통합 테스트 관점**
- [ ] 센서 연결 해제 시뮬레이션
- [ ] 오류 상태에서의 시스템 동작 검증
- [ ] 복구 과정 테스트

### Scenario 10: 전역 임계값 설정
**Scenario 9 패턴 적용**
- [ ] 경계값 분석으로 테스트 데이터 최적화
- [ ] 전역 설정 시 모든 센서 반영 검증
- [ ] 설정 완료 메시지 출력 확인

---

## 🔧 Mock 객체 개선 템플릿

### 모든 Mock에 추가해야 할 기본 기능
```cpp
// 출력 검증 기능
bool wasStringPrinted(const std::string& target) const;
void clearHistory();

// 상태 검증 기능  
bool wasFunctionCalled(const std::string& functionName) const;
int getCallCount(const std::string& functionName) const;

// 파라미터 검증 기능
template<typename T>
bool wasCalledWith(const std::string& functionName, T expectedParam) const;
```

### Mock 설계 원칙
1. **실제 인터페이스와 완전 일치**: 모든 public 메서드 구현
2. **검증 기능 내장**: 호출 여부, 파라미터, 출력 내용 검증
3. **상태 초기화 지원**: 테스트 간 독립성 보장
4. **디버깅 지원**: 호출 이력, 파라미터 로깅 기능

---

## 📊 경계값 분석 적용 가이드

### 1. 숫자 범위가 있는 경우
```cpp
// 센서 ID: 1~SENSOR_COUNT
int boundaryValues[] = {
    0,                    // 하한 경계 밖
    1,                    // 하한 경계
    2,                    // 하한 경계 + 1
    SENSOR_COUNT - 1,     // 상한 경계 - 1
    SENSOR_COUNT,         // 상한 경계
    SENSOR_COUNT + 1      // 상한 경계 밖
};
```

### 2. 임계값이 있는 경우
```cpp
// 온도 임계값: tl ~ th
float tempBoundaries[] = {
    tl - 1,    // 하한 위반
    tl,        // 하한 경계
    tl + 1,    // 정상 하한
    th - 1,    // 정상 상한
    th,        // 상한 경계
    th + 1     // 상한 위반
};
```

### 3. 상태 전환이 있는 경우
```cpp
// 각 상태에서 다른 상태로의 전환 테스트
AppState transitions[] = {
    NORMAL_OPERATION,
    MENU_ACTIVE,
    EDIT_INDIVIDUAL_ID,
    // ... 모든 상태 조합
};
```

---

## 🚀 점진적 적용 로드맵

### Phase 1: 핵심 시나리오 (완료)
- [x] Scenario 9: 임계값 테스트 개선

### Phase 2: 기본 기능들 (다음 우선순위)
- [ ] Scenario 1: 센서 테이블 (출력 검증 추가)
- [ ] Scenario 2: 메뉴 시스템 (상태 전환 검증)
- [ ] Scenario 7: 입력 검증 (경계값 적용)

### Phase 3: 고급 기능들
- [ ] Scenario 3-6: ID 관리 (통합 테스트 강화)
- [ ] Scenario 8: 오류 처리 (복구 시나리오)
- [ ] Scenario 10: 전역 설정 (일괄 적용 검증)

### Phase 4: 전체 최적화
- [ ] 모든 Mock 객체 표준화
- [ ] 테스트 실행 시간 최적화
- [ ] CI/CD 파이프라인 통합

---

## 📈 품질 측정 기준

### 각 시나리오별 목표
- **테스트 실행 시간**: < 3초
- **경계값 커버리지**: 100%
- **통합 테스트 비율**: > 50%
- **Mock 검증 활용**: 모든 출력/상태 변화 검증

### 전체 프로젝트 목표
- **총 테스트 실행 시간**: < 30초
- **버그 검출률**: 향상
- **유지보수성**: 코드 변경 시 테스트 수정 최소화
- **가독성**: 테스트 목적이 명확히 드러남

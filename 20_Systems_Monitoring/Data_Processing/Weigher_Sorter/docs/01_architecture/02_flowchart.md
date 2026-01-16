## 포도 선별 알고리즘 테스트 순서

### **Phase 1: 난수 생성기 특성 분석 (편중 제한 없음)**

#### Step 1.1: 기본 난수 생성기 구현
```cpp
// 3가지 난수 생성 방식 준비
1. Arduino Random() - 기본 내장 함수
2. XorShift32 - 메모리 효율적 PRNG  
3. Hybrid (XorShift + 아날로그 노이즈) - 더 높은 엔트로피
```

#### Step 1.2: 편중 패턴 분석 (10,000세트)
```
각 생성기별로 12개씩 10,000세트 생성
├── 구간별 분포도 측정 (500-540, 540-580, 580-620, 620-660, 660-700)
├── 연속 유사값 발생 빈도 (±20g 이내)
├── 극단 편중 케이스 카운트 (한 구간 6개 이상)
└── 표준편차 및 엔트로피 계산
```

#### Step 1.3: 인간 패턴 시뮬레이터 추가
```cpp
4. 숙련자 시뮬레이터 - 균등 분포 지향
5. 초보자 시뮬레이터 - 특정 구간 선호 + 피로도 반영
6. 랜덤 작업자 - 매 세트마다 다른 성향
```

### **Phase 2: 조합 알고리즘 성능 테스트**

#### Step 2.1: 알고리즘 구현
```cpp
1. 무작위 대입 (Random Sampling)
   - 1000회 랜덤 시도 중 최선 선택
   
2. 탐욕 알고리즘 + 스왑 최적화
   - 큰 무게부터 선택 → 목표 초과시 스왑
   
3. 동적 프로그래밍 (가능한 경우만)
   - ESP32: 전체 탐색
   - Arduino R4: 근사 DP (메모리 제약)
```

#### Step 2.2: 100,000회 본격 테스트
```
테스트 매트릭스 (6 생성기 × 3 알고리즘 = 18 조합)

각 조합별 측정 항목:
├── 목표 달성률 (1,930g ~ 2,070g)
├── 평균 오차 (|2000 - 실제|)
├── 표준편차
├── 연산 시간 (μs)
├── 메모리 사용량
└── 최악/최선 케이스 기록
```

### **Phase 3: 결과 분석 및 시각화**

#### Step 3.1: 데이터 집계
```cpp
struct TestResult {
    float successRate;      // 목표 범위 달성률
    float avgError;         // 평균 오차
    float stdDev;          // 표준편차  
    unsigned long avgTime;  // 평균 연산시간
    int worstError;        // 최악 오차
    int distribution[5];    // 구간별 분포
};
```

#### Step 3.2: 비교 분석 보고서
```
=== 1차 분석: 난수 생성기별 특성 ===
- 편중도 순위 (가장 균등 → 가장 편중)
- 실제 현장 유사도 평가
- 연산 효율성

=== 2차 분석: 조합 알고리즘 성능 ===
- 정확도 vs 속도 트레이드오프
- 메모리 제약 환경 최적안
- 극단 케이스 처리 능력

=== 3차 분석: 최적 조합 도출 ===
[난수 생성기] × [조합 알고리즘] 매트릭스
- Arduino R4: [추천 조합]
- ESP32: [추천 조합]
```

### **Phase 4: 개선 및 최적화**

#### Step 4.1: 1차 개선 (문제점 해결)
```
발견된 문제점:
□ 특정 무게대 과소 선택
□ 연산 시간 초과 케이스
□ 메모리 오버플로우
→ 각 문제별 패치 적용
```

#### Step 4.2: 2차 개선 (성능 튜닝)
```
□ 캐싱 전략 도입
□ 조기 종료 조건 최적화
□ 비트 연산 활용
→ 10,000회 추가 테스트
```

#### Step 4.3: 3차 개선 (현장 피드백 반영)
```
□ 실제 포도 무게 분포 데이터 반영
□ 작업 시간대별 패턴 변화 적용
□ 계절별 보정 계수 추가
```

### **Phase 5: 최종 검증**

#### Step 5.1: 통합 테스트
```
최종 선정 조합으로 1,000,000회 테스트
├── 안정성 검증 (메모리 누수, 오버플로우)
├── 일관성 검증 (분산 분석)
└── 극한 상황 테스트 (모두 500g, 모두 700g 등)
```

#### Step 5.2: 실전 시뮬레이션
```cpp
// 하루 작업량 시뮬레이션 (8시간, 300박스)
for (int hour = 0; hour < 8; hour++) {
    // 시간별 피로도 반영
    // 점심시간 후 패턴 변화
    // 마감 시간 압박 반영
}
```

### **최종 산출물**

1. **성능 비교 매트릭스 (시각화)**
2. **최적 알고리즘 조합 추천서**
3. **Arduino/ESP32별 최적화 코드**
4. **현장 적용 가이드라인**

이 순서로 진행하면 체계적이고 과학적인 테스트가 가능할 것 같습니다!


--------------------------


```mermaid
flowchart TD
    Start([시작: 포도 선별 알고리즘 테스트]) --> Phase1{Phase 1: 난수 생성기 특성 분석}
    
    %% Phase 1 분기
    Phase1 --> Step11[Step 1.1: 기본 난수 생성기 구현]
    Step11 --> RNG1[Arduino Random]
    Step11 --> RNG2[XorShift32]
    Step11 --> RNG3[Hybrid XorShift + 노이즈]
    
    RNG1 --> Step12[Step 1.2: 편중 패턴 분석]
    RNG2 --> Step12
    RNG3 --> Step12
    
    Step12 --> Analysis1[구간별 분포도 측정<br/>500-540, 540-580, 580-620<br/>620-660, 660-700]
    Step12 --> Analysis2[연속 유사값 발생 빈도<br/>±20g 이내]
    Step12 --> Analysis3[극단 편중 케이스<br/>한 구간 6개 이상]
    Step12 --> Analysis4[표준편차 및 엔트로피 계산]
    
    Analysis1 --> Step13[Step 1.3: 인간 패턴 시뮬레이터]
    Analysis2 --> Step13
    Analysis3 --> Step13
    Analysis4 --> Step13
    
    Step13 --> Human1[숙련자 시뮬레이터<br/>균등 분포 지향]
    Step13 --> Human2[초보자 시뮬레이터<br/>특정 구간 선호 + 피로도]
    Step13 --> Human3[랜덤 작업자<br/>매 세트마다 다른 성향]
    
    Human1 --> Phase2{Phase 2: 조합 알고리즘 성능 테스트}
    Human2 --> Phase2
    Human3 --> Phase2
    
    %% Phase 2 분기
    Phase2 --> Step21[Step 2.1: 알고리즘 구현]
    Step21 --> Algo1[무작위 대입<br/>1000회 랜덤 시도]
    Step21 --> Algo2[탐욕 알고리즘 + 스왑<br/>큰 무게부터 선택]
    Step21 --> Algo3[동적 프로그래밍<br/>ESP32: 전체탐색<br/>Arduino: 근사DP]
    
    Algo1 --> Step22[Step 2.2: 100,000회 테스트]
    Algo2 --> Step22
    Algo3 --> Step22
    
    Step22 --> Matrix[테스트 매트릭스<br/>6 생성기 × 3 알고리즘<br/>= 18 조합]
    Matrix --> Metrics[측정 항목:<br/>• 목표 달성률<br/>• 평균 오차<br/>• 표준편차<br/>• 연산 시간<br/>• 메모리 사용량<br/>• 최악/최선 케이스]
    
    Metrics --> Phase3{Phase 3: 결과 분석 및 시각화}
    
    %% Phase 3 분기
    Phase3 --> Step31[Step 3.1: 데이터 집계]
    Step31 --> DataStruct[TestResult 구조체<br/>successRate, avgError<br/>stdDev, avgTime<br/>worstError, distribution]
    
    DataStruct --> Step32[Step 3.2: 비교 분석 보고서]
    Step32 --> Report1[1차 분석:<br/>난수 생성기별 특성<br/>편중도, 현장유사도, 연산효율]
    Step32 --> Report2[2차 분석:<br/>조합 알고리즘 성능<br/>정확도 vs 속도, 메모리제약]
    Step32 --> Report3[3차 분석:<br/>최적 조합 도출<br/>Arduino R4 vs ESP32]
    
    Report1 --> Phase4{Phase 4: 개선 및 최적화}
    Report2 --> Phase4
    Report3 --> Phase4
    
    %% Phase 4 분기
    Phase4 --> Step41[Step 4.1: 1차 개선<br/>문제점 해결]
    Step41 --> Fix1[특정 무게대 과소 선택 해결]
    Step41 --> Fix2[연산 시간 초과 해결]
    Step41 --> Fix3[메모리 오버플로우 해결]
    
    Fix1 --> Step42[Step 4.2: 2차 개선<br/>성능 튜닝]
    Fix2 --> Step42
    Fix3 --> Step42
    
    Step42 --> Tune1[캐싱 전략 도입]
    Step42 --> Tune2[조기 종료 조건 최적화]
    Step42 --> Tune3[비트 연산 활용]
    
    Tune1 --> AdditionalTest[10,000회 추가 테스트]
    Tune2 --> AdditionalTest
    Tune3 --> AdditionalTest
    
    AdditionalTest --> Step43[Step 4.3: 3차 개선<br/>현장 피드백 반영]
    Step43 --> Field1[실제 포도 무게 분포<br/>데이터 반영]
    Step43 --> Field2[작업 시간대별<br/>패턴 변화 적용]
    Step43 --> Field3[계절별 보정 계수 추가]
    
    Field1 --> Phase5{Phase 5: 최종 검증}
    Field2 --> Phase5
    Field3 --> Phase5
    
    %% Phase 5 분기
    Phase5 --> Step51[Step 5.1: 통합 테스트<br/>1,000,000회]
    Step51 --> Verify1[안정성 검증<br/>메모리 누수, 오버플로우]
    Step51 --> Verify2[일관성 검증<br/>분산 분석]
    Step51 --> Verify3[극한 상황 테스트<br/>모두 500g, 모두 700g]
    
    Verify1 --> Step52[Step 5.2: 실전 시뮬레이션]
    Verify2 --> Step52
    Verify3 --> Step52
    
    Step52 --> Simulate[하루 작업량 시뮬레이션<br/>8시간, 300박스<br/>• 시간별 피로도<br/>• 점심시간 후 변화<br/>• 마감 압박 반영]
    
    Simulate --> Final[최종 산출물]
    
    %% 최종 산출물 분기
    Final --> Output1[성능 비교 매트릭스<br/>시각화]
    Final --> Output2[최적 알고리즘 조합<br/>추천서]
    Final --> Output3[Arduino/ESP32별<br/>최적화 코드]
    Final --> Output4[현장 적용<br/>가이드라인]
    
    Output1 --> End([테스트 완료])
    Output2 --> End
    Output3 --> End
    Output4 --> End
    
    %% 스타일 정의
    classDef phaseStyle fill:#e1f5fe,stroke:#01579b,stroke-width:3px
    classDef stepStyle fill:#f3e5f5,stroke:#4a148c,stroke-width:2px
    classDef algorithmStyle fill:#fff3e0,stroke:#e65100,stroke-width:2px
    classDef resultStyle fill:#e8f5e8,stroke:#1b5e20,stroke-width:2px
    classDef outputStyle fill:#fce4ec,stroke:#880e4f,stroke-width:2px
    
    class Phase1,Phase2,Phase3,Phase4,Phase5 phaseStyle
    class Step11,Step12,Step13,Step21,Step22,Step31,Step32,Step41,Step42,Step43,Step51,Step52 stepStyle
    class RNG1,RNG2,RNG3,Algo1,Algo2,Algo3,Human1,Human2,Human3 algorithmStyle
    class Matrix,Metrics,DataStruct,Report1,Report2,Report3 resultStyle
    class Output1,Output2,Output3,Output4 outputStyle

    ```
    
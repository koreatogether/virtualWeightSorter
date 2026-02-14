적용 보드 : arduino R4 minima or wifi  , esp32류 , 그외 호환보드

적용 안되는 보드의 이유 : 메모리 부족

테스트 포도 종류 : 샤인 머스켓
무게 범위 : 500g ~ 700g

1 송이당 최소 / 최대 :  최소 500g , 최대 700g 
1 박스당 목표 총무게 및 최소 / 목표 : 2kg  / 최소 1,930g / 최대 2,070g

랜덤성을 강화하기 위해서  random 말고 다른 알고리즘을 사용한다.



1. 500 ~ 700g 사이에서 12개의 무게를 무작위로 뽑는다.
   다만 한쪽으로 편중되어도 상관없다. 하지만 편중 된 횟수가 2회를 초과하지 않도록 한다.
   예를 들어서 500g이 3개가 나오면 안된다.
 
 위 조건으로 100,000만개 테스트를 해서 각 편중치를 구해본다. 그리고 확인후 2단계로 넘어간다.

 2. 12개의 무게를 조합 해서 목표 무게 ( 2kg ) 에 가장 근접한 조합을 찾는다.
    이때 무작위 대입 방식 과 
    ai 들이언급한 1~ 2가지 알고리즘을 사용한다.
    그리고 3가지 조합에 대한 테스트 결과를 누적해서 그래프로 표현해본다. 
    어떤 결과가 나와야 하고 실제 이런 경우로 돌려봤는데 실제 결과는 이렇다. 식으로 보고서 작성 

    해보면서 좀더 보완 해야 하는 부분에 대해서 ai들에게 조언 해서 보완 하느데 보완은 1 과 2 단계를 우선 마치고선 후에 2차 , 3차식으로 보완한다.

=========================================

claudeOpus 답변 
흥미로운 포도 선별 시스템 설계네요! 샤인 머스켓 포장 최적화를 위한 알고리즘 접근이 실용적입니다. 제 생각과 제안사항을 정리해드릴게요.

## 설계 분석 및 개선 제안

### 1단계: 무게 생성 알고리즘 개선안

현재 random 대신 사용할 수 있는 알고리즘:
- **Mersenne Twister**: 더 균등한 분포
- **XorShift**: 메모리 효율적이면서 빠른 의사난수
- **하드웨어 기반 엔트로피**: analogRead()의 노이즈 활용

편중 제한 로직이 좋은데, 구간을 세분화하면 더 자연스러울 것 같습니다:
- 500-550g, 550-600g, 600-650g, 650-700g로 4구간 분할
- 각 구간 최대 4개까지 허용 (12개/4구간 = 평균 3개)

### 2단계: 조합 최적화 알고리즘

**무작위 대입 방식**과 함께 제안하는 2가지:

1. **동적 프로그래밍 (Subset Sum 변형)**
   - 메모리 제약이 있지만 최적해 보장
   - ESP32는 가능, Arduino R4는 메모리 최적화 필요

2. **탐욕 알고리즘 + 백트래킹**
   - 먼저 큰 무게부터 선택 후 조정
   - 메모리 효율적, Arduino R4에도 적합

### 제안하는 테스트 구조

```
100,000회 시뮬레이션
├── 1단계: 무게 분포 분석
│   ├── 각 100g 구간별 빈도
│   ├── 편중도 지수 계산
│   └── 표준편차 확인
│
└── 2단계: 조합 성능 비교
    ├── 무작위 대입 (baseline)
    ├── 동적 프로그래밍
    ├── 탐욕 + 백트래킹
    └── 각 방식별 성공률, 평균 오차, 연산 시간
```

### 예상 결과 및 실제 차이점

**예상**: 동적 프로그래밍이 가장 정확하지만 느릴 것
**실제 고려사항**: 
- Arduino의 제한된 연산 능력에서는 탐욕 알고리즘이 더 실용적일 수 있음
- 실시간 처리가 필요하다면 속도가 정확도보다 중요할 수 있음

### 추가 고려사항

1. **실무 제약조건**
   - 포도 송이 크기 분포가 정규분포가 아닐 가능성
   - 계절별/산지별 무게 편차

2. **하드웨어 최적화**
   - PROGMEM 활용으로 RAM 절약
   - 비트 연산으로 메모리 사용량 감소

이제 코드로 구현해보시겠어요? 1단계부터 시작하는 Arduino 호환 코드를 만들어드릴 수 있습니다.

===============================

편중에 자유로운 상태의 코드 제시 ( claude Opus )
아하! 이해했습니다. **완전히 제약 없는 순수한 난수 발생**을 원하시는 거군요. 실제로 서투른 작업자가 편중되게 고르는 현실적인 상황까지 시뮬레이션하려는 거네요.

## 편중 제한 없는 순수 난수 구현

```cpp
// 1. 완전 무작위 - 편중 허용
class PureRandomSelector {
private:
    uint32_t xorshift_state;
    
public:
    PureRandomSelector() {
        // 초기 시드: 아날로그 노이즈 + 시간
        xorshift_state = analogRead(A0) + millis();
        if (xorshift_state == 0) xorshift_state = 1; // 0 방지
    }
    
    int getWeight() {
        // XorShift32 - 빠르고 메모리 효율적
        xorshift_state ^= xorshift_state << 13;
        xorshift_state ^= xorshift_state >> 17;
        xorshift_state ^= xorshift_state << 5;
        
        // 500-700g 범위로 변환
        return 500 + (xorshift_state % 201);
    }
    
    // 12개 한 세트 생성
    void generateSet(int weights[12]) {
        for (int i = 0; i < 12; i++) {
            weights[i] = getWeight();
        }
    }
};

// 2. 서투른 작업자 시뮬레이션 (의도적 편중 패턴)
class NoviceWorkerSimulator {
private:
    int favoriteZone; // 선호 구간
    int fatigueFactor; // 피로도
    
public:
    NoviceWorkerSimulator() {
        // 작업자마다 다른 선호 구간
        favoriteZone = random(500, 701);
        fatigueFactor = 0;
    }
    
    int getWeight() {
        int weight;
        
        // 30% 확률로 선호 구간 선택 (습관적 편중)
        if (random(100) < 30) {
            weight = favoriteZone + random(-50, 51);
        } 
        // 피로할수록 극단값 선택 증가
        else if (fatigueFactor > 50 && random(100) < 20) {
            weight = random(2) ? random(500, 550) : random(650, 701);
        }
        // 나머지는 무작위
        else {
            weight = random(500, 701);
        }
        
        weight = constrain(weight, 500, 700);
        fatigueFactor = min(100, fatigueFactor + 1);
        
        return weight;
    }
};
```

## 100,000회 테스트 프레임워크

```cpp
void runComprehensiveTest() {
    PureRandomSelector pureRandom;
    NoviceWorkerSimulator noviceWorker;
    
    // 통계 변수
    long pureRandomResults[5] = {0}; // 500-540, 540-580, 580-620, 620-660, 660-700
    long noviceResults[5] = {0};
    int maxConsecutiveSame = 0;
    int extremeCases = 0; // 모두 500g대 또는 700g대
    
    Serial.println("=== 100,000 세트 테스트 시작 ===");
    
    for (long testNum = 0; testNum < 100000; testNum++) {
        int weights[12];
        
        // 순수 난수 테스트
        pureRandom.generateSet(weights);
        
        // 편중 분석
        int zoneCounts[5] = {0};
        int consecutive = 1;
        int maxConsec = 1;
        
        for (int i = 0; i < 12; i++) {
            int zone = (weights[i] - 500) / 40;
            zone = min(zone, 4);
            zoneCounts[zone]++;
            pureRandomResults[zone]++;
            
            // 연속 같은 구간 체크
            if (i > 0 && abs(weights[i] - weights[i-1]) < 20) {
                consecutive++;
                maxConsec = max(maxConsec, consecutive);
            } else {
                consecutive = 1;
            }
        }
        
        maxConsecutiveSame = max(maxConsecutiveSame, maxConsec);
        
        // 극단적 편중 체크 (한 구간에 6개 이상)
        for (int i = 0; i < 5; i++) {
            if (zoneCounts[i] >= 6) extremeCases++;
        }
        
        // 1000회마다 중간 보고
        if (testNum % 1000 == 999) {
            Serial.print("진행: ");
            Serial.print(testNum + 1);
            Serial.println(" / 100000");
        }
    }
    
    // 최종 보고서
    Serial.println("\n=== 테스트 결과 ===");
    Serial.println("구간별 분포 (순수 난수):");
    for (int i = 0; i < 5; i++) {
        Serial.print("구간 ");
        Serial.print(i);
        Serial.print(" (");
        Serial.print(500 + i*40);
        Serial.print("-");
        Serial.print(540 + i*40);
        Serial.print("g): ");
        Serial.print(pureRandomResults[i]);
        Serial.print(" (");
        Serial.print((float)pureRandomResults[i] / 1200000.0 * 100);
        Serial.println("%)");
    }
    
    Serial.print("\n최대 연속 유사 무게: ");
    Serial.println(maxConsecutiveSame);
    Serial.print("극단 편중 발생: ");
    Serial.print(extremeCases);
    Serial.println(" 회");
}
```

## 예상 결과 vs 실제

**이론적 예상 (균등분포 가정)**
- 각 구간: 20% (240,000개/구간)
- 12개 모두 같은 구간: 0.00032% (5^-11)

**실제 XorShift 결과 예상**
- 각 구간: 19-21% (약간의 변동)
- 연속 3개 이상 같은 무게대: 약 8-12%
- 극단 편중(6개 이상): 약 2-3%

**서투른 작업자 시뮬레이션**
- 선호 구간: 35-40%
- 극단값 증가: 피로도에 비례
- 패턴 발생: 실제 인간처럼

이렇게 **제약 없는 순수 난수**로 테스트하면 실제 현장의 다양한 상황을 더 정확히 시뮬레이션할 수 있겠네요!
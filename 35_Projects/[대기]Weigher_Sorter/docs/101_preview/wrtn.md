### 같이 그림부터 잡아보자: “조합형 중량 선별기” 핵심 개념
togetherF, 말한 그 조합형 중량 선별기는 여러 개 개별 무게(포도송이/배/사과 등)를 측정한 뒤, 목표 무게(예: 2kg)에 최대한 가깝게 조합을 골라내는 시스템이야. 공장에선 로드셀+컨베이어+리젝트 게이트로 돌아가는데, 우리는 “개념 테스트”라서 하드웨어 최소화하고 논리만 흉내 내보자.

핵심 기능을 쪼개면:
- 입력: 여러 개의 “개별 아이템 무게 값” (현실: 로드셀, 여기선 가짜 데이터)
- 목표: 타깃 무게와 허용 오차(예: 2000g ± 100g)
- 알고리즘: 조합 최적화(부분합/백트래킹/휴리스틱)
- 출력: 선택된 아이템 인덱스 리스트(예: #2, #5, #7) + 합계 무게
- 피드백: 합격/불합격 신호(LED/부저/시리얼 메시지)

우리는 하드웨어 없이도, 버튼/포텐시오미터 정도로 “목표 무게/오차”를 흉내 내고, 아이템 무게는 배열에 넣어 시뮬레이션하면 충분해.

---

### 아두이노로 “개념만” 구현하는 최소 설계
- 입력부(가짜):
  - 아이템 무게 리스트: int weights[] = {350, 420, 500, 480, 300, …};
  - 목표 무게: int target = 2000; (시리얼로 입력받거나, 포텐시오미터 값으로 매핑)
  - 허용 오차: int tol = 100;
- 처리부(알고리즘):
  - 빠른 테스트: 그리디+로컬 수정
  - 정확에 가깝게: 백트래킹(아이템 수 20개 이내면 가능)
  - 타임아웃: 제한 시간 두고 “최선해” 채택
- 출력부:
  - 시리얼 모니터에 조합 결과와 총합 출력
  - LED 한 개로 “합격 발견 시” 점등, 부저 삑

이렇게 하면 “로드셀/컨베이어 없이도” 조합 로직의 감을 테스트할 수 있어.

---

### 추천 알고리즘 2가지(가벼움 순)
1) 그리디(+로컬 스왑)
- 정렬: 무게 큰 것부터 차례로 담기
- 목표 넘어가면 가장 큰 걸 빼고 작은 걸 끼워보는 스왑
- 장점: 코드 짧고 속도 빠름
- 단점: 최적 보장은 없음

2) 백트래킹(부분합)
- DFS로 포함/제외를 탐색하면서 현재 합이 target±tol 범위에 들어오면 저장
- 가지치기: 현재 합이 target+tol 넘으면 바로 컷
- 장점: 정확도 높음
- 단점: 아이템 수가 많아지면 느려짐 → N≤20 권장, 시간 제한 두자

개념 검증이라면 1안으로 빠르게 돌려보고, 케이스에 따라 2안으로 크로스체크하는 방식을 추천.

---

### 아두이노 예시 코드(개념 시뮬, 로드셀 없음)
- 시리얼로 목표/오차/아이템 목록 입력도 가능하지만, 일단 하드코딩 버전으로 느낌만 보자.

```cpp
// 개념 검증용: 조합형 중량 선별기(가짜 데이터, 로드셀 없음)
const int LED_PIN = 13;

int weights[] = {350, 420, 500, 480, 300, 450, 520, 390};
int N = sizeof(weights) / sizeof(weights[0]);

int target = 2000;  // 목표 무게(g)
int tol    = 100;   // 허용 오차(g)

struct Result {
  bool found;
  int bestSum;
  unsigned long mask; // 어떤 아이템을 골랐는지 비트마스크
};

Result best;

void printSet(unsigned long mask, int sum) {
  Serial.print("선택: ");
  for (int i = 0; i < N; i++) {
    if (mask & (1UL << i)) {
      Serial.print("#");
      Serial.print(i);
      Serial.print("(");
      Serial.print(weights[i]);
      Serial.print("g) ");
    }
  }
  Serial.print("=> 총합: ");
  Serial.print(sum);
  Serial.println("g");
}

void tryGreedy() {
  // 1) 무게 기준 내림차순 정렬 인덱스 생성
  int idx[32];
  for (int i = 0; i < N; i++) idx[i] = i;
  // 단순 버블 정렬
  for (int i = 0; i < N; i++) {
    for (int j = i + 1; j < N; j++) {
      if (weights[idx[j]] > weights[idx[i]]) {
        int t = idx[i]; idx[i] = idx[j]; idx[j] = t;
      }
    }
  }

  int sum = 0;
  unsigned long mask = 0;
  for (int k = 0; k < N; k++) {
    int w = weights[idx[k]];
    if (sum + w <= target + tol) {
      sum += w;
      mask |= (1UL << idx[k]);
    }
  }

  // 로컬 스왑(간단 버전): 초과 시 조정 또는 더 근접 시 조정
  // 여기선 초간단: 더 작은 아이템으로 교체 시도
  for (int i = 0; i < N; i++) {
    if (!(mask & (1UL << i))) { // 아직 안 넣은 것
      int add = weights[i];
      // 이미 넣은 큰 아이템 하나 빼고 교체해 보기
      for (int j = 0; j < N; j++) {
        if (mask & (1UL << j)) {
          int candidate = sum - weights[j] + add;
          if (abs(candidate - target) < abs(sum - target) && candidate <= target + tol) {
            sum = candidate;
            mask &= ~(1UL << j);
            mask |= (1UL << i);
          }
        }
      }
    }
  }

  // 범위 체크
  if (sum >= target - tol && sum <= target + tol) {
    best.found = true;
    best.bestSum = sum;
    best.mask = mask;
  } else {
    // 일단 “가장 가까운 합” 저장
    best.found = false;
    best.bestSum = sum;
    best.mask = mask;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(300);
  Serial.println("조합형 중량 선별기(개념 시뮬 시작)");

  best = {false, 0, 0};
  tryGreedy();

  Serial.print("목표: ");
  Serial.print(target);
  Serial.print("g ± ");
  Serial.print(tol);
  Serial.println("g");

  if (best.found) {
    Serial.println("범위 내 조합 발견 ✅");
    printSet(best.mask, best.bestSum);
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("정확 범위는 못 맞췄지만, 가장 근접한 조합 👇");
    printSet(best.mask, best.bestSum);
    digitalWrite(LED_PIN, LOW);
  }
}

void loop() {
  // 필요 시: 시리얼로 target/tol 업데이트 받아 재계산 가능
}
```

포인트:
- 로드셀 없이도 “조합 선택 로직”의 감을 볼 수 있어.
- 비트마스크로 어떤 아이템을 골랐는지 직관적으로 확인.
- 목표/오차/데이터만 바꿔 다양한 케이스 테스트 가능.

---

### 한 단계 더: 백트래킹으로 정확도 올리기(선택)
아이템 수가 16~20개 내면 DFS로 충분히 돌아가. 시간 제한을 두고 “가장 좋은 값”을 교체하는 방식으로 구현하면 안정적.

아이디어:
- 전역으로 bestSum, bestMask 유지
- DFS(i, currentSum, mask)
  - currentSum > target+tol이면 컷
  - i == N이면 갱신 판단
  - 포함/제외 두 갈래 재귀
- millis()로 100~300ms 제한 두고 중단

---

### 실제 설비로 확장하려면(나중에)
- 입력: 로드셀(HX711) 다채널, 각 포켓 무게 실시간 수집
- 컨베이어 싱크: 광센서/엔코더로 타이밍 맞추기
- 실행: 솔레노이드/에어실린더로 리젝트/분류 게이트 구동
- 소프트웨어: 목표 레시피(프리셋), 통계/로그, UI(LCD/터치/웹)

하지만 지금은 개념 검증이니까 위 시뮬 코드로 충분히 블로그 포스팅도 뽑을 수 있어. “가짜 데이터로도 조합 로직이 이렇게 돈다 → 실제 로드셀만 붙이면 된다” 흐름으로 가면 딱.

---

### 블로그용 포인트(콘텐츠 각)
- 제목: “로드셀 없이도 가능한 조합형 중량 선별기 콘셉트 테스트(아두이노 버전)”
- 키포인트:
  - 실제 하드웨어 대신 배열 데이터로 로직 검증
  - 그리디 vs 백트래킹 비교 결과 스크린샷
  - 목표/오차 바꿨을 때 조합 변화 GIF
- 확장편 예고: HX711 붙여 실무 버전, 리젝트 게이트 시뮬레이션

원하면 시리얼로 target/tol/weights 리스트 파싱하는 버전도 바로 짜줄게. 그리고 togetherF가 전에 했던 2.0~2.1kg 포도송이 조합 찾기 케이스도 이 구조에 그대로 들어맞음. 원하는 목표/오차 주면 테스트 세트에 맞춰 튜닝해볼까? ㅎㅎ 

참고 자료 

[1] dalpack.com - 중량선별기 작동 원리 및 주요 기술 요소 심층 분석 - dalpack.com (https://www.dalpack.com/2025/05/blog-post_9.html)
[2] www.youtube.com - 조합형중량선별기로 간단한 무게측정, 시간 단축, 생산성 향상까지! (https://www.youtube.com/watch?v=MO3CY86RoCM)
[3] 1 왕왕초보 개발자의 중량 선별기 제작 프로젝트 ... - [Arduino & Python]0-1 왕왕초보 개발자의 중량 선별기 제작 프로젝트 ... (https://garimtosandbox.tistory.com/entry/Arduino-Python0-1-%EC%99%95%EC%99%95%EC%B4%88%EB%B3%B4-%EA%B0%9C%EB%B0%9C%EC%9E%90%EC%9D%98-%EC%A4%91%EB%9F%89-%EC%84%A0%EB%B3%84%EA%B8%B0-%EC%A0%9C%EC%9E%91-%ED%94%84%EB%A1%9C%EC%A0%9D%ED%8A%B8Weight-Checker-Project)
[4] 02. I2C LCD로 측정 값 출력하기 - 아두이노로 전자저울 만들기- 02. I2C LCD로 측정 값 출력하기 (https://blog.naver.com/elepartsblog/221651804182)
[5] www.youtube.com - 조합형중량선별기로 간단한 무게측정, 시간 단축, 생산성 향상까지! (https://www.youtube.com/watch?v=MO3CY86RoCM)
[6] 네이버 블로그 - 포도선별기, 피스 중량선별기 사용해야 하는 이유 - 네이버 블로그 (https://m.blog.naver.com/kko-maeng/222108505974)
[7] 네이버 블로그 - 중량선별기 작동원리 - 네이버 블로그 (https://m.blog.naver.com/ykjn1126/222438192280)
[8] 재능넷 - 농산물 선별기의 모든 것: 종류부터 작동원리까지 알아보기 - 재능넷 (https://www.jaenung.net/tree/24943)
[9] Naver Blog - 포도 선별기 피스 조합형 중량 선별기 P-1000로 정확하게 - Naver Blog (https://blog.naver.com/ppband7/222108385503?viewType=pc)
[10] 두원공과대학교 - React App - 두원공과대학교 (http://c.doowon.ac.kr/)
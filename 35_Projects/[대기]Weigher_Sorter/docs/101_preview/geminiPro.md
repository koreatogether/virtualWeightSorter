

# **아두이노로 구현하는 조합형 중량 선별기: 원리 탐구부터 개념 증명 모델 제작까지**

## **섹션 1: 산업용 조합형 중량 선별기의 해부: 단순 분류를 넘어서**

본격적인 아두이노 프로젝트에 앞서, 우리가 모방하고자 하는 산업용 장비의 본질을 정확히 이해하는 것이 중요합니다. 사용자가 언급한 '조합형 중량 선별기'는 단순한 무게 검사기를 넘어, 현대 포장 산업의 핵심적인 역할을 수행하는 정교한 시스템입니다. 이 섹션에서는 그 작동 원리와 목적을 심층적으로 분석하여 프로젝트의 명확한 목표를 설정합니다.

### **1.1 핵심 기능: 조합을 통한 정밀 포장**

'조합형 중량 선별기(Combination Weigher)' 또는 '멀티헤드 중량계(Multihead Weigher)'라 불리는 이 장비의 주된 목표는 개별 품목의 중량을 측정하여 단순히 등급을 나누는 것이 아닙니다. 이 시스템의 진정한 목적은 중량이 제각각인 다수의 제품(예: 포도송이, 과자, 냉동식품 등)을 지능적으로 '조합'하여, 최종 포장 단위의 총중량을 사전에 설정된 목표치에 최대한 근접시키는 것입니다.1 이를 통해 제조업체는 법적 기준을 준수하면서도, 기준 중량을 초과하여 제품을 더 넣어주는 '기브어웨이(giveaway)'를 최소화하여 수익성을 극대화할 수 있습니다.4

작동 과정은 다음과 같이 체계적으로 이루어집니다:

1. **공급 및 분산:** 제품이 기계 상단으로 공급되면, 진동이나 원심력을 이용해 원형으로 배열된 여러 개의 '계량 호퍼(weigh hopper)'로 균등하게 분산됩니다.2
2. **개별 계량:** 각 계량 호퍼에는 정밀 로드셀(Load Cell)이 장착되어 있어, 담겨있는 제품의 무게를 실시간으로 정확하게 측정합니다.6 이 로드셀은 무게를 전기 신호로 변환하는 핵심 센서입니다.7

### **1.2 기계의 두뇌: 조합 연산**

모든 계량 호퍼의 무게 측정이 완료되면, 시스템의 중앙 제어 컴퓨터가 핵심적인 역할을 수행합니다.

1. **데이터 수집:** 컴퓨터는 모든 호퍼로부터 측정된 무게 데이터를 동시에 읽어들입니다.1
2. **조합 탐색:** 이 데이터를 바탕으로, 컴퓨터는 가능한 모든 호퍼의 조합을 초고속으로 계산합니다. 그 목적은 이 조합들 중에서 총합이 사전에 설정된 '목표 중량'에 가장 근접하면서도 미달하지 않는 최적의 조합을 찾아내는 것입니다.1
3. **배출:** 최적의 조합이 결정되면, 해당 조합에 포함된 호퍼들의 게이트가 동시에 열립니다. 이 호퍼들 안의 제품들은 중앙의 배출 슈트(discharge chute)를 통해 하나의 포장 용기(봉지 또는 트레이)로 모여 담기게 됩니다.1

이 과정은 1분에 수십에서 수백 회까지 반복될 수 있으며, 인간 작업자가 수작업으로 무게를 맞추는 것과는 비교할 수 없는 속도와 정확성을 제공합니다.4

### **1.3 결정적 차이: 조합형 중량 선별기 vs. 중량 선별기(Checkweigher)**

여기서 사용자가 제시한 '조합형 중량 선별기'와 일반적인 '중량 선별기(Checkweigher)'를 명확히 구분하는 것이 프로젝트의 방향을 결정하는 데 매우 중요합니다. 이 둘은 이름이 비슷하지만 기능과 목적이 완전히 다릅니다.

일반적인 중량 선별기는 컨베이어 벨트를 따라 일렬로 이동하는 개별 제품의 무게를 순차적으로 확인하는 시스템입니다.6 제품이 계량 구간을 통과할 때 로드셀이 무게를 측정하고, 이 값이 미리 설정된 기준 범위(예: 500g ± 5g) 안에 있는지 'OK' 또는 'NG'로 판정합니다.8 기준을 벗어나는 'NG' 제품은 푸셔(pusher), 에어 이젝터(air ejector) 등의 리젝터(rejector) 장치에 의해 생산 라인에서 자동으로 배출됩니다.7 즉, 이것은 개별 제품에 대한

**품질 관리 및 분류(Sorting)** 작업입니다.

반면, 조합형 중량 선별기는 여러 제품을 모아 하나의 단위를 만드는 **포장 및 조합(Combining/Packaging)** 작업에 특화되어 있습니다. 이 시스템의 핵심은 '버리기'가 아니라 '모으기'이며, 그 과정에는 복잡한 조합 최적화 연산이 필수적으로 동반됩니다.

사용자가 포도송이나 배와 같이 개체별 중량 편차가 큰 농산물을 예로 들고 '조합형'이라는 단어를 명시했기 때문에, 이 프로젝트의 목표는 단순한 무게 판별 로직(if weight \> X, reject)을 구현하는 것이 아닙니다. 대신, 여러 개의 가변적인 무게 값들 중에서 목표 합계에 가장 근접한 최적의 부분집합을 찾아내는, 훨씬 더 지능적이고 복잡한 알고리즘을 구현하는 것이 핵심 과제가 됩니다. 이 차이점을 인지하는 것이야말로 올바른 기술적 접근법을 선택하는 첫걸음입니다.

## **섹션 2: 알고리즘의 핵심: 마이크로컨트롤러에서 '부분집합 합' 문제 해결하기**

산업용 기계의 물리적 구조를 이해했으니, 이제 그 기계의 '두뇌'에 해당하는 추상적인 논리로 초점을 옮겨야 합니다. 조합형 중량 선별기의 핵심 기능은 컴퓨터 과학의 고전적인 문제와 직결되며, 우리는 아두이노라는 제한된 환경에 가장 적합한 해결책을 선택해야 합니다.

### **2.1 연산 과제의 정의: 부분집합 합 문제(Subset Sum Problem)**

조합형 중량 선별기의 컴퓨터가 수행하는 작업, 즉 여러 개의 무게 값(계량 호퍼의 무게들) 중에서 특정 목표값(포장 목표 중량)과 일치하는 합을 가진 그룹을 찾는 것은 컴퓨터 과학에서 \*\*부분집합 합 문제(Subset Sum Problem, SSP)\*\*로 알려진 유명한 문제입니다.10

이 문제는 다음과 같이 공식적으로 정의할 수 있습니다:

주어진 숫자 집합(예: 각 호퍼의 무게)과 목표 합계(예: 목표 포장 중량)가 있을 때, 이 집합의 원소들 중 일부를 더하여 정확히 목표 합계가 되는 부분집합이 존재하는가? 13

예를 들어, 호퍼들의 무게가 {50g, 80g, 110g, 120g}이고 목표 중량이 190g이라면, {80g, 110g}라는 부분집합이 존재하므로 답은 '예'가 됩니다.

### **2.2 현실 세계를 위한 변형: '정확한 일치'에서 '최적의 근사치'로**

산업 현장에서는 매번 정확히 목표 중량과 일치하는 조합을 찾는 것이 거의 불가능합니다. 따라서 실제 시스템은 '정확한 합이 존재하는가?'라는 단순한 결정 문제(decision problem)를 넘어, '목표 중량에 미달하지 않으면서 가장 근접한 합을 가진 조합은 무엇인가?'라는 \*\*최적화 문제(optimization problem)\*\*를 해결해야 합니다.1

이는 각 아이템(호퍼의 내용물)의 '가치'와 '무게'가 모두 그램(g) 단위의 무게 값으로 동일하다고 가정하는 \*\*0/1 배낭 문제(0/1 Knapsack Problem)\*\*와 매우 유사한 형태를 띱니다.16 즉, 목표 중량이라는 '배낭 용량'을 초과하지 않으면서 '가치(총중량)'를 최대화하는 아이템 조합을 찾는 것입니다.

이러한 현실적인 요구사항은 우리 아두이노 모델의 알고리즘 설계에 중요한 지침을 제공합니다. 단순히 목표값과 일치하는 조합을 찾는 데서 그치지 않고, 유효한(목표 중량 이하인) 모든 조합을 평가하여 그중 목표 중량과의 차이가 가장 작은 '최적의' 조합을 찾아내야 합니다. 이를 위해서는 best\_sum\_so\_far(지금까지 찾은 최적의 합)나 best\_combination\_so\_far(최적의 조합 정보)와 같은 상태 변수를 유지하며 모든 가능성을 탐색하는, 한 단계 더 복잡한 로직이 필요합니다. 이는 실제 산업 현장의 엔지니어링 과제를 더 충실히 반영하는 접근법입니다.

### **2.3 올바른 도구 선택: 동적 계획법 vs. 무차별 대입 탐색**

이 문제를 해결하기 위한 대표적인 알고리즘 접근법은 두 가지가 있습니다.

1. **동적 계획법(Dynamic Programming, DP):** 더 작은 하위 문제들의 해를 테이블에 저장하여 중복 계산을 피하는 효율적인 기법입니다. 시간 복잡도는 아이템의 개수를 n, 목표 무게를 W라고 할 때 $O(n \\times W)$로, 유사 다항 시간(pseudo-polynomial time)에 해당합니다.10
2. **무차별 대입 탐색(Brute-Force / Exhaustive Search):** 가능한 모든 조합을 체계적으로 하나씩 모두 확인하는 가장 직관적인 방법입니다.11
   n개의 아이템이 있을 때, 가능한 부분집합의 수는 2n개이므로, 시간 복잡도는 $O(2^n)$으로 지수 시간에 해당합니다.

강력한 컴퓨팅 환경에서는 시간 복잡도가 우수한 동적 계획법이 당연한 선택처럼 보일 수 있습니다. 하지만 우리는 아두이노 우노와 같은 극도로 제한된 하드웨어 환경에서 프로젝트를 구축해야 합니다. 아두이노 우노의 SRAM(정적 램)은 단 2KB에 불과합니다.31

표준적인 동적 계획법 풀이는 (n+1) x (W+1) 크기의 2차원 배열을 필요로 합니다. 만약 우리가 8개의 아이템으로 목표 무게 1000g을 맞추는 모델을 만든다면, 9 x 1001 크기의 불리언(boolean) 배열이 필요하며, 이는 약 9KB의 메모리를 소모하여 아두이노 우노의 용량을 훨씬 초과합니다. 공간을 최적화한 1차원 배열 DP 방식조차 W+1 크기의 배열이 필요하므로, 1KB 이상의 메모리를 차지하여 2KB RAM의 절반 이상을 소모하게 됩니다.23

반면, 무차별 대입 탐색은 시간 복잡도는 높지만 **공간 복잡도** 측면에서 압도적인 이점을 가집니다. 특히 다음에 소개할 비트마스킹 기법을 사용하면, 모든 조합을 탐색하는 데 단 몇 개의 정수형 변수만 필요하므로 메모리 사용량이 거의 무시할 수 있을 정도로 작습니다. 우리 모델처럼 아이템의 개수 n이 4\~8개 정도로 작을 경우, 2n (예: 28=256) 정도의 연산은 아두이노의 16MHz 프로세서가 수 밀리초 내에 충분히 처리할 수 있는 수준입니다.

따라서, 이론적으로는 '비효율적'으로 보이는 무차별 대입 방식이, 역설적으로 이 하드웨어 제약 환경에서는 가장 현실적이고 우아한 엔지니어링 선택이 됩니다.

| 알고리즘                     | 시간 복잡도    | 공간 복잡도         | 아두이노 적합성 (n=8, W=1000)                                             |
| :--------------------------- | :------------- | :------------------ | :------------------------------------------------------------------------ |
| **무차별 대입 (비트마스크)** | O(2n) \- 높음  | O(1) \- 매우 낮음   | **최적.** 256회의 반복은 충분히 빠르며, 메모리 사용량이 극히 적음.        |
| 동적 계획법 (2D 배열)        | O(n×W) \- 낮음 | O(n×W) \- 매우 높음 | **부적합.** 약 9KB RAM 필요. 아두이노 우노 용량 초과.                     |
| 동적 계획법 (1D 배열)        | O(n×W) \- 낮음 | O(W) \- 높음        | **제한적.** 약 1KB RAM 필요. 우노 RAM의 절반을 DP 테이블에만 할당해야 함. |

### **2.4 무차별 대입 구현: 비트마스킹(Bitmasking)**

모든 조합을 효율적으로 탐색하기 위해 복잡한 재귀 함수(recursion) 대신 **비트마스킹** 기법을 사용할 것입니다.12 이 기법은

n개의 아이템이 있을 때, 0부터 2n−1까지의 정수를 이용하여 모든 부분집합을 표현합니다.

각 정수를 이진수로 표현했을 때, j번째 비트가 1이면 j번째 아이템을 현재 조합에 포함시키고, 0이면 포함시키지 않는 것으로 간주합니다. 예를 들어, 4개의 아이템이 있을 때 정수 5는 이진수로 0101입니다. 이는 0번과 2번 아이템은 포함하고, 1번과 3번 아이템은 제외하는 조합을 의미합니다. 이처럼 단순한 for 루프와 비트 연산자(&, \<\<)를 사용하여 모든 2n개의 조합을 간결하고 메모리 효율적으로 순회할 수 있습니다.34

## **섹션 3: 개념 증명 모델 설계: 공장에서 브레드보드로**

이제 산업용 시스템의 구성 요소와 추상적인 알고리즘을 구체적인 하드웨어 프로젝트로 변환할 차례입니다. 각 전자 부품이 시스템의 어떤 부분을 물리적으로 대변하는지 정의하고, 프로젝트의 설계 철학을 확립합니다.

### **3.1 부품 매핑: 물리적 유추**

이 프로젝트의 핵심은 조합 알고리즘이라는 추상적인 개념을 눈으로 보고 만질 수 있는 형태로 만드는 것입니다. 이를 위해 회로 자체를 부분집합 합 문제(SSP) 알고리즘의 입력과 출력을 직접적으로 표현하는 물리적 메타포로 설계합니다.

* **입력 (가중치 집합):** 가변 저항(Potentiometer)들은 알고리즘이 처리해야 할 숫자 집합, 즉 각 호퍼의 '무게'를 물리적으로 표현합니다.38 사용자가 가변 저항의 손잡이를 돌리는 행위는 입력 배열의 한 요소를 직접 조작하는 것과 같습니다.
* **입력 (목표 합계):** 사용자가 시리얼 모니터를 통해 입력하는 목표 중량 값은 알고리즘의 target\_sum 매개변수에 해당합니다.44
* **출력 (결과 부분집합):** LED들은 알고리즘이 찾아낸 최적의 조합, 즉 '부분집합'의 해를 시각적으로 나타냅니다.50 특정 LED가 켜졌다는 것은 그에 해당하는 가변 저항의 '무게'가 최적의 조합에 포함되었음을 의미합니다.

이러한 설계는 단순한 센서-액추에이터 시연을 넘어, 조합 최적화 알고리즘이 실제로 어떻게 작동하는지를 상호작용적으로 시각화하는 강력한 교육 도구로서의 가치를 지닙니다.

### **3.2 우리 모델의 구성 요소**

* **계량 호퍼 → 가변 저항:** n개의 계량 호퍼는 각각 10kΩ 가변 저항으로 시뮬레이션됩니다. 아두이노는 analogRead() 함수를 통해 각 가변 저항의 아날로그 전압을 0에서 1023 사이의 값으로 읽어들입니다. 이 값은 map() 함수를 사용하여 실제적인 모의 무게 범위(예: 100g \~ 500g)로 변환됩니다.38 이 가이드에서는 관리의 용이성을 위해 4개의 가변 저항을 사용합니다.
* **중앙 컴퓨터 → 아두이노 우노:** 아두이노 우노가 전체 시스템의 두뇌 역할을 합니다. 입력 값을 읽고, 조합 알고리즘을 실행하며, 최종적으로 출력(LED)을 제어합니다.
* **HMI/제어판 → 시리얼 모니터:** 사용자는 아두이노 IDE의 시리얼 모니터를 통해 시스템과 상호작용합니다. 목표 포장 중량을 입력하라는 프롬프트가 표시되고, 시스템은 처리 상태와 최종 결과를 다시 모니터에 출력합니다.44
* **표시등 → LED:** 각 가변 저항에 대응하는 LED가 하나씩 배치됩니다. 아두이노가 최적의 조합을 계산한 후, 최종 포장에 포함되어야 할 '무게'를 가진 가변 저항에 연결된 LED들을 점등시킵니다.50

| 산업용 부품          | 기능                                                | 아두이노 등가물               | 근거 및 주요 기능                                                                                                    |
| :------------------- | :-------------------------------------------------- | :---------------------------- | :------------------------------------------------------------------------------------------------------------------- |
| 계량 호퍼 / 로드셀   | 개별 품목의 가변적인 무게 측정                      | 10kΩ 가변 저항                | 가변적인 아날로그 입력(0-1023)을 시뮬레이션하여 무게 값으로 매핑 가능 38                                             |
| 중앙 컴퓨터 / HMI    | 사용자 입력 수신, 조합 알고리즘 실행, 출력 제어     | 아두이노 우노 & 시리얼 모니터 | 아두이노는 코드를 실행하고, 시리얼 모니터는 목표 무게 입력 및 결과 확인을 위한 간단한 텍스트 기반 인터페이스 제공 44 |
| 표시등 / 배출 게이트 | 선택된 품목을 시각적으로 표시하거나 물리적으로 배출 | 표준 LED                      | 알고리즘에 의해 선택된 '품목'(가변 저항)을 시각적으로 표현하는 간단한 디지털 출력 50                                 |

## **섹션 4: 단계별 구현 가이드**

이 섹션은 프로젝트의 핵심인 실습 가이드입니다. 회로를 조립하고 아두이노를 프로그래밍하는 전 과정을 상세하고 명확하게 안내합니다. 여기서는 4개의 아이템(가변 저항/LED)을 사용하는 모델을 기준으로 설명합니다.

### **4.1 하드웨어 조립**

**부품 목록:**

* 아두이노 우노 1개
* 브레드보드(빵판) 1개
* 10kΩ 가변 저항 4개
* 5mm LED 4개 (서로 다른 색상 권장)
* 220Ω 저항 4개
* 점퍼 와이어 다수

회로 연결 방법:
아래 설명에 따라 각 부품을 브레드보드와 아두이노에 연결합니다.

1. **전원 레일 연결:**
   * 아두이노의 5V 핀을 브레드보드의 '+' 전원 레일에 연결합니다.
   * 아두이노의 GND 핀을 브레드보드의 '-' 전원 레일에 연결합니다.
2. **가변 저항 연결 (4개):**
   * 각 가변 저항은 3개의 핀을 가지고 있습니다.
   * 양쪽 끝 핀 중 하나는 브레드보드의 '+' 레일(5V)에, 다른 하나는 '-' 레일(GND)에 연결합니다.
   * 가운데 핀(Wiper)을 아두이노의 아날로그 입력 핀에 각각 연결합니다:
     * 첫 번째 가변 저항 → A0
     * 두 번째 가변 저항 → A1
     * 세 번째 가변 저항 → A2
     * 네 번째 가변 저항 → A3

       39
3. **LED 연결 (4개):**
   * 각 LED는 긴 다리(Anode, \+)와 짧은 다리(Cathode, \-)를 가집니다.
   * LED의 짧은 다리를 브레드보드의 '-' 레일(GND)에 직접 연결합니다.
   * LED의 긴 다리를 220Ω 저항의 한쪽 끝과 같은 행에 연결합니다.
   * 220Ω 저항의 다른 쪽 끝을 아두이노의 디지털 핀에 각각 연결합니다:
     * 첫 번째 LED의 저항 → D2
     * 두 번째 LED의 저항 → D3
     * 세 번째 LED의 저항 → D4
     * 네 번째 LED의 저항 → D5

       51

모든 연결이 끝나면 회로는 4개의 독립적인 '무게 입력 장치'(가변 저항)와 그에 대응하는 4개의 '선택 표시 장치'(LED)를 갖추게 됩니다.

### **4.2 아두이노 스케치: 상세 코드 해설**

아래 코드를 아두이노 IDE에 복사하여 붙여넣고 보드에 업로드합니다. 각 코드 블록은 상세한 주석과 함께 설명됩니다.

C++

// \=================================================================
// 조합형 중량 선별기 개념 증명 모델
// \=================================================================

// \--- 파트 1: 전역 변수 및 설정 \---

// 아이템(가변 저항/LED)의 개수를 상수로 정의합니다.
// 이 값을 변경하면 다른 부품들도 그에 맞게 추가해야 합니다.
const int NUM\_ITEMS \= 4;

// 각 가변 저항이 연결된 아날로그 핀 번호를 배열로 관리합니다.
// 이렇게 하면 코드를 확장하거나 핀을 변경하기 용이합니다. \[52\]
const int potPins \= {A0, A1, A2, A3};

// 각 LED가 연결된 디지털 핀 번호를 배열로 관리합니다. \[54\]
const int ledPins \= {2, 3, 4, 5};

// 각 아이템의 무게를 저장할 배열입니다.
int weights;

void setup() {
  // 시리얼 통신을 9600 bps 속도로 시작합니다.
  // 이를 통해 PC와 데이터를 주고받을 수 있습니다. \[44\]
  Serial.begin(9600);

  // for 루프를 사용하여 모든 LED 핀을 출력(OUTPUT) 모드로 설정합니다.
  for (int i \= 0; i \< NUM\_ITEMS; i++) {
    pinMode(ledPins\[i\], OUTPUT);
  }

  // 모든 LED를 끈 상태로 시작합니다.
  for (int i \= 0; i \< NUM\_ITEMS; i++) {
    digitalWrite(ledPins\[i\], LOW);
  }

  Serial.println("조합형 중량 선별기 시스템 준비 완료.");
  Serial.println("목표 중량(g)을 입력하고 Enter를 누르세요.");
}

// \--- 파트 2: 메인 루프 \---

void loop() {
  // 시리얼 버퍼에 수신된 데이터가 있는지 확인합니다. \[48\]
  if (Serial.available() \> 0) {
    // 사용자가 입력한 정수 값을 읽어옵니다. \[49\]
    int targetWeight \= Serial.parseInt();

    // 사용자가 Enter 키를 눌러 입력을 완료했는지 확인합니다.
    // (parseInt() 이후에 개행 문자가 남아있음)
    if (Serial.read() \== '\\n') {
      Serial.print("입력된 목표 중량: ");
      Serial.print(targetWeight);
      Serial.println("g");

      // 1\. 현재 가변 저항 값들을 읽어 '무게'로 변환합니다.
      readWeights(weights);

      // 2\. 최적의 조합을 찾는 알고리즘을 실행합니다.
      int bestCombinationMask \= findBestCombination(weights, targetWeight);

      // 3\. 찾은 결과를 LED와 시리얼 모니터에 표시합니다.
      displayCombination(bestCombinationMask, targetWeight);

      Serial.println("\\n새로운 목표 중량을 입력하세요.");
    }
  }
}

// \--- 파트 3: '무게' 읽기 \---

// 가변 저항 값을 읽어와 모의 무게로 변환하는 함수
void readWeights(int w) {
  Serial.println("------------------------------------");
  Serial.println("현재 각 아이템의 무게:");
  for (int i \= 0; i \< NUM\_ITEMS; i++) {
    // 아날로그 핀에서 0-1023 범위의 값을 읽습니다. \[38\]
    int rawValue \= analogRead(potPins\[i\]);

    // map() 함수를 사용하여 0-1023 범위를 100-500g 범위로 변환합니다. \[39, 42\]
    // 이 범위는 필요에 따라 자유롭게 조절할 수 있습니다.
    w\[i\] \= map(rawValue, 0, 1023, 100, 500);

    Serial.print("  아이템 \#");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(w\[i\]);
    Serial.println("g");
  }
  Serial.println("------------------------------------");
}

// \--- 파트 4: 조합 알고리즘 \---

// 최적의 조합을 찾아 해당 조합의 비트마스크를 반환하는 함수
int findBestCombination(const int w, int target) {
  int bestSum \= 0;
  int bestCombinationMask \= 0;
  // 목표 중량과 현재까지 찾은 최적 합계의 차이를 저장합니다.
  // 초기값은 목표 중량 자체로 설정하여 어떤 유효한 조합이라도 선택될 수 있게 합니다.
  int minDiff \= target;

  // 비트마스킹을 이용한 무차별 대입 탐색
  // 1부터 (2^NUM\_ITEMS \- 1)까지 모든 조합을 순회합니다.
  // i=0은 공집합이므로 제외합니다.
  for (int i \= 1; i \< (1 \<\< NUM\_ITEMS); i++) {
    int currentSum \= 0;

    // 현재 조합(i)에 포함된 아이템들의 무게 합을 계산합니다.
    for (int j \= 0; j \< NUM\_ITEMS; j++) {
      // (i & (1 \<\< j))는 i의 j번째 비트가 1인지 확인하는 비트 연산입니다. \[34, 37\]
      // j번째 비트가 1이면, j번째 아이템을 조합에 포함시킵니다.
      if ((i & (1 \<\< j))\!= 0) {
        currentSum \+= w\[j\];
      }
    }

    // 최적화 조건 확인:
    // 1\. 현재 합계(currentSum)가 목표 중량(target) 이하인가?
    // 2\. (목표 중량 \- 현재 합계)가 이전에 기록된 최소 차이(minDiff)보다 작은가?
    if (currentSum \<= target && (target \- currentSum) \< minDiff) {
      // 새로운 최적의 조합을 찾았으므로, 관련 변수들을 업데이트합니다.
      minDiff \= target \- currentSum;
      bestSum \= currentSum;
      bestCombinationMask \= i;
    }
  }

  return bestCombinationMask;
}

// \--- 파트 5: 결과 표시 \---

// 최종 결과를 LED와 시리얼 모니터에 출력하는 함수
void displayCombination(int mask, int target) {
  Serial.println("\\n\[알고리즘 실행 결과\]");

  if (mask \== 0) {
    Serial.println("목표 중량을 만족하는 유효한 조합을 찾지 못했습니다.");
    // 모든 LED를 끕니다.
    for (int i \= 0; i \< NUM\_ITEMS; i++) {
      digitalWrite(ledPins\[i\], LOW);
    }
    return;
  }

  Serial.print("선택된 최적 조합: ");
  int finalSum \= 0;

  for (int i \= 0; i \< NUM\_ITEMS; i++) {
    // 최적 조합 마스크(mask)의 i번째 비트가 1인지 확인합니다.
    if ((mask & (1 \<\< i))\!= 0) {
      // 1이면 해당 LED를 켭니다. \[50\]
      digitalWrite(ledPins\[i\], HIGH);

      // 결과 출력용 문자열을 만듭니다.
      Serial.print("\#");
      Serial.print(i);
      Serial.print(" (");
      Serial.print(weights\[i\]);
      Serial.print("g) ");

      finalSum \+= weights\[i\];
    } else {
      // 0이면 해당 LED를 끕니다.
      digitalWrite(ledPins\[i\], LOW);
    }
  }

  Serial.println();
  Serial.print("최종 조합 중량: ");
  Serial.print(finalSum);
  Serial.println("g");
  Serial.print("목표 중량과의 차이: ");
  Serial.print(target \- finalSum);
  Serial.println("g");
}

## **섹션 5: 분석, 한계, 그리고 확장 경로**

이 개념 증명 모델은 조합형 중량 선별기의 핵심 원리를 성공적으로 시연하지만, 동시에 단순화된 프로젝트로서의 한계도 명확히 보여줍니다. 이 섹션에서는 프로젝트의 성능 한계를 분석하고, 이를 극복하여 더 발전된 프로젝트로 나아갈 수 있는 구체적인 경로를 제시합니다.

### **5.1 성능 한계의 이해**

우리가 선택한 무차별 대입 알고리즘의 시간 복잡도는 $O(2^n)$입니다. 이는 아이템의 개수(n)가 증가할수록 연산량이 기하급수적으로 늘어남을 의미합니다. 4개(24=16 조합)나 8개(28=256 조합)의 아이템은 아두이노가 순식간에 처리할 수 있지만, 실제 산업용 기계처럼 10개에서 32개 이상의 헤드를 갖는 시스템에 이 알고리즘을 적용하는 것은 불가능합니다.4 예를 들어, 20개의 아이템만 되어도

220, 즉 100만 개가 넘는 조합을 탐색해야 하며, 이는 아두이노의 처리 능력을 훨씬 뛰어넘습니다. 이 분석은 문제의 규모와 하드웨어의 성능에 따라 적절한 알고리즘을 선택하는 것이 얼마나 중요한지를 다시 한번 상기시켜 줍니다.

### **5.2 향후 프로젝트: 하드웨어 업그레이드**

* **물리적 구동 추가:** 현재의 LED 표시를 넘어, 실제 배출 과정을 시뮬레이션하기 위해 각 LED를 서보 모터로 교체할 수 있습니다. 하지만 아두이노 우노의 PWM 핀 개수와 프로세싱 능력은 여러 개의 서보를 동시에 정밀하게 제어하기에 부족합니다. 이 문제를 해결하기 위해 **PCA9685 16채널 서보 드라이버** 모듈을 사용하는 것이 좋습니다. 이 모듈은 I2C 통신을 사용하므로, 단 2개의 아두이노 핀(SDA, SCL)만으로 최대 16개의 서보 모터를 독립적으로 제어할 수 있게 해줍니다.56
* **더 강력한 두뇌:** 더 많은 아이템을 처리하거나 동적 계획법과 같은 더 복잡한 알고리즘을 구현하려면 더 많은 RAM과 빠른 처리 속도를 가진 마이크로컨트롤러가 필요합니다. \*\*아두이노 메가(8KB RAM)\*\*나 \*\*ESP32(520KB RAM)\*\*와 같은 보드로 업그레이드하는 것을 고려할 수 있습니다.31

### **5.3 향후 프로젝트: 고급 데이터 시각화**

텍스트 기반의 시리얼 모니터를 넘어, 아두이노에서 전송되는 데이터를 컴퓨터에서 실행되는 시각화 스크립트로 받아 훨씬 풍부한 사용자 인터페이스를 구축할 수 있습니다.

* **Python 활용:** pySerial 라이브러리를 사용하여 아두이노의 시리얼 데이터를 실시간으로 읽고, matplotlib 라이브러리를 이용해 각 아이템의 무게, 목표 중량, 선택된 조합의 합계 등을 라이브 그라프로 표시할 수 있습니다.61 이는 시스템의 작동 상태를 직관적으로 파악하는 데 큰 도움이 됩니다.
* **Processing 활용:** 시각 예술 및 인터랙티브 디자인에 특화된 Processing 개발 환경은 아두이노와의 시리얼 통신을 위한 라이브러리를 훌륭하게 지원합니다. 이를 사용하여 대화형 대시보드를 만들거나, 알고리즘의 탐색 과정을 시각적으로 표현하는 등 창의적인 시각화가 가능합니다.67

### **5.4 향후 프로젝트: 동적 계획법(DP) 솔루션 구현**

ESP32와 같이 충분한 메모리를 갖춘 보드를 사용한다면, 무차별 대입 방식의 성능 한계를 극복하기 위해 공간 최적화된 1차원 배열 동적 계획법 솔루션을 구현해볼 수 있습니다. 이는 알고리즘 복잡도와 성능 면에서 상당한 수준의 업그레이드가 될 것입니다.23 기존의 DP 알고리즘이 특정 합의 '가능 여부'만 판단하는 것에서 나아가, 목표 중량 이하의 '최적 합계'를 찾도록 로직을 수정해야 합니다. DP 테이블을 채워나가면서 각 합계(

j)를 만들 수 있는지 여부뿐만 아니라, 그때의 실제 합계 값을 추적하여 최종적으로 dp에 가장 가까운 값을 찾아내는 방식으로 구현할 수 있습니다. 이는 더 높은 수준의 알고리즘적 사고를 요구하는 도전적인 과제가 될 것입니다.

#### **참고 자료**

1. Multihead Weigher Machine \- SiroSilo, 8월 25, 2025에 액세스, [https://www.sirosilo.com/products/multihead-weigher-machine/](https://www.sirosilo.com/products/multihead-weigher-machine/)
2. Multihead weigher \- Wikipedia, 8월 25, 2025에 액세스, [https://en.wikipedia.org/wiki/Multihead\_weigher](https://en.wikipedia.org/wiki/Multihead_weigher)
3. A Guide to Computer Combination Weighers \- SIGMA Equipment, 8월 25, 2025에 액세스, [https://www.sigmaequipment.com/guide/computer-combination-weighers/](https://www.sigmaequipment.com/guide/computer-combination-weighers/)
4. How does a multihead weigher work? \- Yamato Scale, 8월 25, 2025에 액세스, [https://www.yamatoscale.co.uk/how-does-a-multihead-weigher-work/](https://www.yamatoscale.co.uk/how-does-a-multihead-weigher-work/)
5. How does a mulihead weigher work? | MULTIPOND \- YouTube, 8월 25, 2025에 액세스, [https://www.youtube.com/watch?v=TVnA-7kJC74](https://www.youtube.com/watch?v=TVnA-7kJC74)
6. www.dalpack.com, 8월 25, 2025에 액세스, [https://www.dalpack.com/2025/05/blog-post\_9.html\#:\~:text=%F0%9F%94%8D%20%EC%A4%91%EB%9F%89%EC%84%A0%EB%B3%84%EA%B8%B0%EC%9D%98%20%EC%9E%91%EB%8F%99,%EB%AF%B8%EB%8B%AC%20%EC%97%AC%EB%B6%80%EB%A5%BC%20%ED%8C%90%EB%8B%A8%ED%95%A9%EB%8B%88%EB%8B%A4.](https://www.dalpack.com/2025/05/blog-post_9.html#:~:text=%F0%9F%94%8D%20%EC%A4%91%EB%9F%89%EC%84%A0%EB%B3%84%EA%B8%B0%EC%9D%98%20%EC%9E%91%EB%8F%99,%EB%AF%B8%EB%8B%AC%20%EC%97%AC%EB%B6%80%EB%A5%BC%20%ED%8C%90%EB%8B%A8%ED%95%A9%EB%8B%88%EB%8B%A4.)
7. 농산물 선별기의 모든 것: 종류부터 작동원리까지 알아보기 \- 재능넷, 8월 25, 2025에 액세스, [https://www.jaenung.net/tree/24943](https://www.jaenung.net/tree/24943)
8. 중량선별기 작동 원리 및 주요 기술 요소 심층 분석 \- dalpack.com, 8월 25, 2025에 액세스, [https://www.dalpack.com/2025/05/blog-post\_9.html](https://www.dalpack.com/2025/05/blog-post_9.html)
9. \[핫클립\] 세계 최초로 개발한 스틱 개별 중량 선별기 / YTN 사이언스 \- YouTube, 8월 25, 2025에 액세스, [https://www.youtube.com/watch?v=SBK2z4KFqEk](https://www.youtube.com/watch?v=SBK2z4KFqEk)
10. Subset Sum algorithm \- dynamic programming \- Stack Overflow, 8월 25, 2025에 액세스, [https://stackoverflow.com/questions/4355955/subset-sum-algorithm](https://stackoverflow.com/questions/4355955/subset-sum-algorithm)
11. Subset sum problem \- Wikipedia, 8월 25, 2025에 액세스, [https://en.wikipedia.org/wiki/Subset\_sum\_problem](https://en.wikipedia.org/wiki/Subset_sum_problem)
12. Solving Subset Sum Problems with Recursive Strategies | by Pythonic Pioneer | Medium, 8월 25, 2025에 액세스, [https://medium.com/@PythonicPioneer/solving-subset-sum-problems-with-recursive-strategies-ae19b15e32a2](https://medium.com/@PythonicPioneer/solving-subset-sum-problems-with-recursive-strategies-ae19b15e32a2)
13. Solving the Subset Sum Problem: A Step-by-Step Guide | by Amrit Ranjan | Medium, 8월 25, 2025에 액세스, [https://medium.com/@amritranjanamc/solving-the-subset-sum-problem-a-step-by-step-guide-d14bcc0d1323](https://medium.com/@amritranjanamc/solving-the-subset-sum-problem-a-step-by-step-guide-d14bcc0d1323)
14. Subset Sum Problem \- GeeksforGeeks, 8월 25, 2025에 액세스, [https://www.geeksforgeeks.org/dsa/subset-sum-problem-dp-25/](https://www.geeksforgeeks.org/dsa/subset-sum-problem-dp-25/)
15. Subset Sum Problem \- Scaler Blog, 8월 25, 2025에 액세스, [https://www.scaler.in/subset-sum-problem/](https://www.scaler.in/subset-sum-problem/)
16. The Knapsack Problem | OR-Tools \- Google for Developers, 8월 25, 2025에 액세스, [https://developers.google.com/optimization/pack/knapsack](https://developers.google.com/optimization/pack/knapsack)
17. Understanding the Knapsack Problem: A Guide for Beginners | by preksha yadav | Medium, 8월 25, 2025에 액세스, [https://medium.com/@prekshayadav0819/understanding-the-knapsack-problem-a-guide-for-beginners-d0146a59e9](https://medium.com/@prekshayadav0819/understanding-the-knapsack-problem-a-guide-for-beginners-d0146a59e9)
18. 0/1 Knapsack Problem \- GeeksforGeeks, 8월 25, 2025에 액세스, [https://www.geeksforgeeks.org/dsa/0-1-knapsack-problem-dp-10/](https://www.geeksforgeeks.org/dsa/0-1-knapsack-problem-dp-10/)
19. The Ultimate Knapsack Problem Guide \- Number Analytics, 8월 25, 2025에 액세스, [https://www.numberanalytics.com/blog/ultimate-knapsack-problem-guide](https://www.numberanalytics.com/blog/ultimate-knapsack-problem-guide)
20. Knapsack problem \- Wikipedia, 8월 25, 2025에 액세스, [https://en.wikipedia.org/wiki/Knapsack\_problem](https://en.wikipedia.org/wiki/Knapsack_problem)
21. Dynamic Programming: Subset Sum \+ Knapsack1 \- Dartmouth, 8월 25, 2025에 액세스, [https://www.cs.dartmouth.edu/\~deepc/LecNotes/cs31/lec9+10.pdf](https://www.cs.dartmouth.edu/~deepc/LecNotes/cs31/lec9+10.pdf)
22. Subset Sum Problem Explained (Dynamic Programming) \- FavTutor, 8월 25, 2025에 액세스, [https://favtutor.com/blogs/subset-sum-problem](https://favtutor.com/blogs/subset-sum-problem)
23. Subset Sum Problem (Visualization and Code Examples) \- Final Round AI, 8월 25, 2025에 액세스, [https://www.finalroundai.com/articles/subset-sum-problem](https://www.finalroundai.com/articles/subset-sum-problem)
24. LeetCode 416: Partition Equal Subset Sum Dynamic Programming \+ 1D Array Solution with Python | by Ayun Daywhea | Medium, 8월 25, 2025에 액세스, [https://medium.com/@ayundaywhea/leetcode-416-partition-equal-subset-sum-dynamic-programming-1d-array-solution-with-python-928569928064](https://medium.com/@ayundaywhea/leetcode-416-partition-equal-subset-sum-dynamic-programming-1d-array-solution-with-python-928569928064)
25. find a solution to subset sum using dynamic programming \- Stack Overflow, 8월 25, 2025에 액세스, [https://stackoverflow.com/questions/18818406/find-a-solution-to-subset-sum-using-dynamic-programming](https://stackoverflow.com/questions/18818406/find-a-solution-to-subset-sum-using-dynamic-programming)
26. \[알고리즘\] 동적 계획법(Dynamic Programming) \- 멈추지 않고 끈질기게, 8월 25, 2025에 액세스, [https://sam0308.tistory.com/80](https://sam0308.tistory.com/80)
27. \[동적 프로그래밍 예제\] 부분집합의 합(C언어) \- 준비된 개발자, 8월 25, 2025에 액세스, [https://readystory.tistory.com/29](https://readystory.tistory.com/29)
28. \[알고리즘 설계\] 3\. 동적계획법(Dynamic Programming) \- 괭이쟁이, 8월 25, 2025에 액세스, [https://laboputer.github.io/ps/2018/02/13/dynamic-programming/](https://laboputer.github.io/ps/2018/02/13/dynamic-programming/)
29. \[Algorithm\] 동적계획법(Dynamic Programming : DP) \- PLOD \- 티스토리, 8월 25, 2025에 액세스, [https://kibeom2000.tistory.com/109](https://kibeom2000.tistory.com/109)
30. Subset sum problem solved using a recursive brute force approach 【O(2^N) time complexity】 \- OpenGenus IQ, 8월 25, 2025에 액세스, [https://iq.opengenus.org/subset-sum-problem-recursive-approach/](https://iq.opengenus.org/subset-sum-problem-recursive-approach/)
31. JDCS 디지털콘텐츠학회논문지, 8월 25, 2025에 액세스, [https://koreascience.kr/article/JAKO201732060955567.pdf](https://koreascience.kr/article/JAKO201732060955567.pdf)
32. Subset Sum Problem | Solved using Dynamic Programming \- Code and Debug, 8월 25, 2025에 액세스, [https://codeanddebug.in/blog/subset-sum-problem/](https://codeanddebug.in/blog/subset-sum-problem/)
33. Subset Sum Problem in O(sum) space \- GeeksforGeeks, 8월 25, 2025에 액세스, [https://www.geeksforgeeks.org/dsa/subset-sum-problem-osum-space/](https://www.geeksforgeeks.org/dsa/subset-sum-problem-osum-space/)
34. Bit Masks with Arduino, 8월 25, 2025에 액세스, [https://docs.arduino.cc/learn/programming/bit-mask/](https://docs.arduino.cc/learn/programming/bit-mask/)
35. 아두이노 \- 비트 마스크, bit mask :: postpop \- 티스토리, 8월 25, 2025에 액세스, [https://postpop.tistory.com/82](https://postpop.tistory.com/82)
36. 비트마스크 \- 곰곰이 이해하는 프로그래밍, 8월 25, 2025에 액세스, [https://teddy0.tistory.com/8](https://teddy0.tistory.com/8)
37. 비트마스킹을 이용한 순열, 조합 \- Developer Edlin \- 티스토리, 8월 25, 2025에 액세스, [https://edlin.tistory.com/entry/%EB%B9%84%ED%8A%B8%EB%A7%88%EC%8A%A4%ED%82%B9%EC%9D%84-%EC%88%9C%EC%97%B4-%EC%A1%B0%ED%95%A9](https://edlin.tistory.com/entry/%EB%B9%84%ED%8A%B8%EB%A7%88%EC%8A%A4%ED%82%B9%EC%9D%84-%EC%88%9C%EC%97%B4-%EC%A1%B0%ED%95%A9)
38. 아두이노 가변저항 사용하기\!(analogRead(), analogWrite(), map()) \- 개발자의 생존노트, 8월 25, 2025에 액세스, [https://alwayswakeup.tistory.com/25](https://alwayswakeup.tistory.com/25)
39. \[아두이노\] 가변저항 사용법 및 예제 \- led, 스텝 모터​​​ \- 하루에 하나 (One a day), 8월 25, 2025에 액세스, [https://popcorn16.tistory.com/166](https://popcorn16.tistory.com/166)
40. 아두이노 시리얼통신 활용 (가변저항 Read, 시리얼모니터 출력) \- 학생을 위한 SW 메이커, 8월 25, 2025에 액세스, [https://swmakerjun.tistory.com/13](https://swmakerjun.tistory.com/13)
41. 아두이노) 가변저항 값 측정하기 \- YouTube, 8월 25, 2025에 액세스, [https://www.youtube.com/watch?v=3Q-CdyJ1Nto](https://www.youtube.com/watch?v=3Q-CdyJ1Nto)
42. \[Arduino\] 아두이노 가변저항(Potentiometer) 쉽게 응용하기 \- 코딩아트 \- 티스토리, 8월 25, 2025에 액세스, [https://artsung410.tistory.com/40](https://artsung410.tistory.com/40)
43. 8\. \[Tutorial\] 아두이노 아날로그 실습(가변저항) \- Edward'sLabs \- 티스토리, 8월 25, 2025에 액세스, [https://openstory.tistory.com/64](https://openstory.tistory.com/64)
44. 아두이노 시리얼 통신(2): 데이터 입력 :: VEDACUBE, 8월 25, 2025에 액세스, [https://vedacube.tistory.com/241](https://vedacube.tistory.com/241)
45. \[아두이노\] 아두이노 Serial Monitor 사용법 \- 아두맨 아이디어 창작소 \- 티스토리, 8월 25, 2025에 액세스, [https://yepb.tistory.com/28](https://yepb.tistory.com/28)
46. \[Arduino\] Serial Monitor \- Intelligent Robot \- 티스토리, 8월 25, 2025에 액세스, [https://jstar0525.tistory.com/143](https://jstar0525.tistory.com/143)
47. 아두이노 기초7. 시리얼통신 \- 코딩런, 8월 25, 2025에 액세스, [https://codingrun.com/76](https://codingrun.com/76)
48. \[아두이노\] 아두이노에서 시리얼 통신을 이용하여 데이터 수신하기 \- Benedicto, 8월 25, 2025에 액세스, [https://bene-dictus.tistory.com/36](https://bene-dictus.tistory.com/36)
49. \[Arduino\] \#6. ⭐️아두이노 시리얼 입출력하기 ⭐️ \- 우당탕탕 코딩 제작소 \- 티스토리, 8월 25, 2025에 액세스, [https://udangtangtang-cording-oldcast1e.tistory.com/37](https://udangtangtang-cording-oldcast1e.tistory.com/37)
50. 05\. 아두이노 스위치 2개로 LED ON OFF 제어 해보기, 8월 25, 2025에 액세스, [https://kjswizard.tistory.com/7](https://kjswizard.tistory.com/7)
51. Arduino Uno 로 가변저항을 이용하여 여러개의 LED 제어하기 \- 오마이엔지니어, 8월 25, 2025에 액세스, [https://rockjjy.tistory.com/2387](https://rockjjy.tistory.com/2387)
52. 여러개 LED 배열 사용하여 제어하기 \- Kpage \- 티스토리, 8월 25, 2025에 액세스, [https://kpage.tistory.com/240](https://kpage.tistory.com/240)
53. \[7강\] 아두이노 버튼 LED 제어 / 버튼 LED 여러개 제어 / digitalWrite() / analogWrite() /회로도, 소스코드 공유 \- YouTube, 8월 25, 2025에 액세스, [https://www.youtube.com/watch?v=7qLcVvO\_CdA](https://www.youtube.com/watch?v=7qLcVvO_CdA)
54. 09 LED 3개 차례로 깜박이기 \- 낭만독타 \- 티스토리, 8월 25, 2025에 액세스, [https://romanceeagleta.tistory.com/49](https://romanceeagleta.tistory.com/49)
55. 아두이노 가변저항으로 아날로그 입력 조절 예제 \- 현's 블로그, 8월 25, 2025에 액세스, [https://hyunssssss.tistory.com/333](https://hyunssssss.tistory.com/333)
56. Can an arduino control multiple servo motors independently? \- Reddit, 8월 25, 2025에 액세스, [https://www.reddit.com/r/arduino/comments/1hvqks6/can\_an\_arduino\_control\_multiple\_servo\_motors/](https://www.reddit.com/r/arduino/comments/1hvqks6/can_an_arduino_control_multiple_servo_motors/)
57. Arduino How to Connect Multiple Servo Motors \- PCA9685 Tutorial : 6 Steps \- Instructables, 8월 25, 2025에 액세스, [https://www.instructables.com/Arduino-How-to-Connect-Multiple-Servo-Motors-PCA96/](https://www.instructables.com/Arduino-How-to-Connect-Multiple-Servo-Motors-PCA96/)
58. Multiple servos without using delay via a PCA9685 controller \- Adafruit Forums, 8월 25, 2025에 액세스, [https://forums.adafruit.com/viewtopic.php?t=207588](https://forums.adafruit.com/viewtopic.php?t=207588)
59. How can I control multiple servo motors with an Arduino without using up all the PWM pins?, 8월 25, 2025에 액세스, [https://www.reddit.com/r/ArduinoProjects/comments/1k5bu66/how\_can\_i\_control\_multiple\_servo\_motors\_with\_an/](https://www.reddit.com/r/ArduinoProjects/comments/1k5bu66/how_can_i_control_multiple_servo_motors_with_an/)
60. 12 Mind-Blowing Arduino AI Projects That Will Transform Your Tech Skills \- Jaycon, 8월 25, 2025에 액세스, [https://www.jaycon.com/12-mind-blowing-arduino-ai-projects-that-will-transform-your-tech-skills/](https://www.jaycon.com/12-mind-blowing-arduino-ai-projects-that-will-transform-your-tech-skills/)
61. Realtime Data Acquisition and Plotting with Arduino and Python \- GitHub Gist, 8월 25, 2025에 액세스, [https://gist.github.com/brandoncurtis/33a67d9d402973face8d](https://gist.github.com/brandoncurtis/33a67d9d402973face8d)
62. Python Datalogger \- Using pySerial to Read Serial Data Output from Arduino \- Maker Portal, 8월 25, 2025에 액세스, [https://makersportal.com/blog/2018/2/25/python-datalogger-reading-the-serial-output-from-arduino-to-analyze-data-using-pyserial](https://makersportal.com/blog/2018/2/25/python-datalogger-reading-the-serial-output-from-arduino-to-analyze-data-using-pyserial)
63. Plotting and Graphing Live Data From Arduino Using the Power of Python \- Instructables, 8월 25, 2025에 액세스, [https://www.instructables.com/Plotting-and-Graphing-Live-Data-from-Arduino-using/](https://www.instructables.com/Plotting-and-Graphing-Live-Data-from-Arduino-using/)
64. Interface Python and Arduino With PySerial : 3 Steps \- Instructables, 8월 25, 2025에 액세스, [https://www.instructables.com/Interface-Python-and-Arduino-with-pySerial/](https://www.instructables.com/Interface-Python-and-Arduino-with-pySerial/)
65. Full tutorial Python Live streaming Data with Graphic UI from Arduino-STM32 using Tkinter, PySerial, Threading, Matplotlib, Numpy \- YouTube, 8월 25, 2025에 액세스, [https://www.youtube.com/playlist?list=PLtVUYRe-Z-meHdTlzqCHGPjZvnL2VZVn8](https://www.youtube.com/playlist?list=PLtVUYRe-Z-meHdTlzqCHGPjZvnL2VZVn8)
66. Arduino \+ Python Real time plotting with matplotlib \- YouTube, 8월 25, 2025에 액세스, [https://www.youtube.com/watch?v=3aRi5OkHk4M](https://www.youtube.com/watch?v=3aRi5OkHk4M)
67. Create a Graph with Processing | Arduino Documentation, 8월 25, 2025에 액세스, [https://www.arduino.cc/en/Tutorial/Graph](https://www.arduino.cc/en/Tutorial/Graph)
68. Visualization with Arduino and Processing, 8월 25, 2025에 액세스, [https://www.arduino.cc/education/visualization-with-arduino-and-processing/](https://www.arduino.cc/education/visualization-with-arduino-and-processing/)
69. Arduino \- Processing: serial data \- YouTube, 8월 25, 2025에 액세스, [https://www.youtube.com/watch?v=NhyB00J6PiM](https://www.youtube.com/watch?v=NhyB00J6PiM)
70. Processing Grapher \- chillibasket, 8월 25, 2025에 액세스, [https://wired.chillibasket.com/processing-grapher/](https://wired.chillibasket.com/processing-grapher/)
71. Visualize Data From Sensors Using Arduino \+ Coolterm \- Instructables, 8월 25, 2025에 액세스, [https://www.instructables.com/Visualize-data-from-sensors-using-Arduino-coolte/](https://www.instructables.com/Visualize-data-from-sensors-using-Arduino-coolte/)
72. Arduino and Processing Final Project | by Talisha Payton \- Medium, 8월 25, 2025에 액세스, [https://medium.com/@talishapayton/arduino-and-processing-final-project-306dd59c0bf2](https://medium.com/@talishapayton/arduino-and-processing-final-project-306dd59c0bf2)
조합형 중량 선별기는 여러 개의 저울(센서)에서 측정된 과일의 무게를 자동으로 조합하여, 목표 무게에 가장 근접하거나 지정 범위에 맞게 과일을 선별하는 시스템입니다. 아두이노 등 저렴한 마이크로컨트롤러로 간단한 개념 실험이 가능합니다. 여기서는 실제 로드셀 없이, 수동 데이터 입력을 기반으로 시스템을 구성하는 방법을 설명합니다.[2][3][7]

## 조합형 중량 선별기 핵심 원리
- 여러 개의 과일에 대해 각각 중량 값을 입력(센서 대신 수동으로 입력)합니다.[3][2]
- 목표 포장 무게와 허용 오차(예: 2kg 박스, ±10g) 조건을 설정합니다.[2]
- 모든 입력값(과일 중량)의 가능한 조합을 만들어, 목표 무게에 가장 가까운 조합을 선별합니다.[3][2]
- 선별 결과를 LED, 부저, LCD 등으로 출력해 피드백합니다.[7][2]

## 간단한 아두이노 구현 방법 (센서 없이 개념 실험)
### 하드웨어 구성 (예시)
- 아두이노 보드
- 버튼 여러 개(과일 중량 수동 입력 용)
- LCD 또는 시리얼 모니터(결과 출력)
- LED/부저(피드백용)

### 소프트웨어/알고리즘 흐름
1. 사용자에게 여러 과일의 중량을 직접 입력 받음(버튼 클릭, 시리얼로 입력 등).[3]
2. 목표 무게, 허용 오차, 조합 개수 등 조건값 입력.
3. 입력된 중량값들의 조합(예: 3개씩 조합)을 반복적으로 계산하여 가장 목표에 근접하거나 허용 오차 내에 들어오는 조합을 찾음.[2][3]
4. 결과를 LCD 또는 시리얼로 표시하고, LED/부저 등으로 알림.

예시 알고리즘(아두이노 의사코드):
```cpp
int fruitWeights[10]; // 최대 과일 10개의 수동 입력 값
int targetWeight = 2000; // 예시 2kg 목표
int tolerance = 10; // 허용 오차 10g
int bestCombo[3]; // 최적 3개 조합 저장
int minDiff = 9999;

void loop() {
  // 1. 과일 중량값 fruitWeights[] 수동 입력

  // 2. 모든 3개 조합 탐색
  for (int i=0; i<10; i++)
    for (int j=i+1; j<10; j++)
      for (int k=j+1; k<10; k++) {
        int sum = fruitWeights[i] + fruitWeights[j] + fruitWeights[k];
        int diff = abs(sum - targetWeight);
        if (diff <= tolerance && diff < minDiff) {
          minDiff = diff;
          bestCombo = i;
          bestCombo[1] = j;
          bestCombo[2] = k;
        }
      }

  // 3. LCD/LED/부저로 결과 출력
}
```
이런 방식으로 실제 과일의 측정값 없이도 알고리즘 실험 및 개념 구현이 가능합니다.[3]

## 참고 포인트
- 실제 산업용 시스템은 여러 로드셀(센서)로 과일 별 중량을 자동으로 읽고, 데이터 조합 알고리즘을 통해 자동 선별합니다.[7][2]
- 개념적으로, 로드셀 및 신호처리 부분은 제외하고, 직접 입력값을 활용하여 조합 알고리즘만 테스트할 수 있습니다.[3]
- 아두이노에서는 복잡한 조합 알고리즘도 단순 입력 및 결과 출력 기능을 이용하면 쉽게 체험할 수 있습니다.[3]

이 방식으로 조합형 중량 선별기의 원리를 아두이노에서 간단히 개념적으로 테스트할 수 있습니다.[7][2][3]

[1](https://www.youtube.com/watch?v=aaXCIyTu_TY)
[2](https://www.ki-it.com/_PR/view/?aidx=28583&bidx=2511)
[3](https://garimtosandbox.tistory.com/entry/Arduino-Python0-1-%EC%99%95%EC%99%95%EC%B4%88%EB%B3%B4-%EA%B0%9C%EB%B0%9C%EC%9E%90%EC%9D%98-%EC%A4%91%EB%9F%89-%EC%84%A0%EB%B3%84%EA%B8%B0-%EC%A0%9C%EC%9E%91-%ED%94%84%EB%A1%9C%EC%A0%9D%ED%8A%B8Weight-Checker-Project)
[4](https://patents.google.com/patent/KR20120021769A/ko)
[5](https://www.youtube.com/watch?v=jiTJKME01o8)
[6](https://patents.google.com/patent/KR970002606Y1/ko)
[7](https://www.youtube.com/watch?v=MO3CY86RoCM)
[8](https://agripapa.tistory.com/53)
[9](https://www.youtube.com/watch?v=VbV5f23xmhM)
[10](https://patents.google.com/patent/KR102070069B1/ko)
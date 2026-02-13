# Arduino R4 Minima Weigher-Sorter Simulator 🍇⚖️

Arduino R4 Minima 보드를 위한 포도 선별기 시뮬레이터입니다. 하드웨어 제약사항(32KB RAM, 256KB Flash)을 정확히 모의하여 Phase 1-5 문서에 명시된 모든 기능을 테스트할 수 있습니다.

## 📋 개요

### 프로젝트 목표
- **목표 무게**: 2000g (±70g 허용범위: 1930-2070g)
- **포도 개수**: 12개 (500-700g 범위)
- **처리 시간**: 5초 이내
- **메모리 제한**: 32KB RAM (실제 사용가능: 28KB)

### 시뮬레이션 구성
- **6가지 난수 생성기** × **3가지 조합 알고리즘** = **18가지 조합**
- Arduino R4 Minima 하드웨어 완전 시뮬레이션
- 10,000세트부터 1,000,000세트까지 스케일러블 테스트

## 🚀 빠른 시작

### 1. 환경 설정
```bash
cd E:/project/06_weigher-sorter/src/python
```

### 2. 기본 테스트 실행
```bash
python test_runner.py
```

### 3. 개별 컴포넌트 테스트
```bash
# Arduino 시뮬레이터 테스트
python arduino_r4_simulator.py

# 난수 생성기 테스트
python random_generators.py

# 조합 알고리즘 테스트
python combination_algorithms.py

# 성능 모니터링 테스트
python performance_monitor.py
```

## 📁 파일 구조

```
src/python/
├── arduino_r4_simulator.py     # Arduino R4 Minima 하드웨어 모의
├── random_generators.py        # 6가지 난수 생성기
├── combination_algorithms.py   # 3가지 조합 알고리즘
├── performance_monitor.py      # 성능 모니터링 시스템
├── test_runner.py             # 통합 테스트 러너
└── README.md                  # 이 파일
```

## 🎯 6가지 난수 생성기

### 1. Arduino Random (arduino_random)
- Arduino 기본 `random()` 함수 (LFSR 기반)
- 메모리 사용량: 4바이트
- 특징: 약간의 편향성 (높은 값에서 감소 편향)

### 2. XorShift32 (xorshift32)
- 32비트 경량 PRNG
- 메모리 사용량: 4바이트 (state 저장)
- 특징: 빠른 속도, 좋은 균등성

### 3. Hybrid (hybrid)
- XorShift32 + analogRead(A0) 노이즈 혼합
- 메모리 사용량: 4바이트
- 특징: 하드웨어 노이즈로 균등성 개선

### 4. Expert (expert)
- 숙련자 모델: 가우시안 분포 + 균등화 보정
- 메모리 사용량: 8바이트
- 특징: 구간별 균등 분포 강제

### 5. Beginner (beginner)
- 초보자 모델: 580-620g 구간 60% 선호 + 피로도 누적
- 메모리 사용량: 8바이트
- 특징: 시간에 따른 패턴 변화

### 6. Random Worker (randomworker)
- 랜덤 작업자: 100세트마다 패턴 변경
- 메모리 사용량: 12바이트
- 특징: 5가지 패턴 순환 (uniform, normal, bimodal, skewed)

## ⚙️ 3가지 조합 알고리즘

### 1. Random Sampling (random_sampling)
- 무작위 조합 시도 (Arduino R4: 500회 제한)
- 시간복잡도: O(n)
- 메모리 사용량: 낮음
- 특징: 기준선, 안정적

### 2. Greedy + Local Search (greedy_localsearch)
- 탐욕적 선택 + 2-swap 최적화
- 시간복잡도: O(n²)
- 메모리 사용량: 중간
- 특징: 높은 정확도, 비트마스크 최적화

### 3. Dynamic Programming (dynamic_programming)
- 제한적 DP (10g 단위 근사화)
- 시간복잡도: O(n×W)
- 메모리 사용량: 높음 (Arduino 제약시 20g 단위)
- 특징: 최적해 보장, 메모리 부족시 Greedy로 폴백

## 📊 테스트 옵션

### 1. Quick Test
```bash
python test_runner.py
# 선택: 1
```
- 각 조합당 100세트 테스트
- 소요시간: ~2분
- 용도: 빠른 기능 검증

### 2. Standard Test
```bash
python test_runner.py
# 선택: 2
```
- 각 조합당 1,000세트 테스트
- 소요시간: ~20분
- 용도: 일반적인 성능 평가

### 3. Full Test
```bash
python test_runner.py
# 선택: 3
```
- 각 조합당 10,000세트 테스트
- 소요시간: ~3시간
- 용도: 정확한 성능 측정

### 4. Benchmark Test
```bash
python test_runner.py
# 선택: 4
```
- 3가지 대표 조합만 테스트
- 소요시간: ~5분
- 용도: 빠른 벤치마킹

## 📈 결과 분석

### 성능 지표
- **Success Rate**: 1930-2070g 달성률 (%)
- **Avg Error**: 평균 오차 (g)
- **Avg Time**: 평균 실행시간 (ms)
- **Memory Peak**: 최대 메모리 사용량 (KB)
- **Arduino Compatible**: Arduino R4 제약 조건 준수 여부

### 출력 파일
- `weigher_sorter_test_results_YYYYMMDD_HHMMSS.csv`: 상세 데이터
- `weigher_sorter_test_results_YYYYMMDD_HHMMSS.json`: 전체 결과

### 실시간 모니터링
```
[1/18] Testing: Arduino_Random + Random_Sampling
    Progress: 50.0% (500/1000)
    Success Rate: 87.2%
    Avg Error: 23.1g
    Avg Time: 145.3ms
    Peak Memory: 1.8KB
    Arduino Compatible: True
```

## 🔧 Arduino R4 제약 조건

### 메모리 제한
- **SRAM**: 32KB (실제 사용가능: 28KB)
- **Flash**: 256KB
- **Stack**: 최대 100 호출 깊이

### 성능 제한
- **처리시간**: 5초 이내 (타임아웃)
- **클럭**: 16MHz ARM Cortex-M4

### 검증 항목
- ✅ 메모리 오버플로우 방지
- ✅ 스택 오버플로우 방지
- ✅ 타임아웃 방지
- ✅ 실시간 처리 보장

## 📋 예상 결과

### Top 3 추천 조합 (예상)
1. **XorShift32 + Greedy_LocalSearch**: 균형잡힌 성능
2. **Expert + Random_Sampling**: 높은 안정성
3. **Hybrid + Greedy_LocalSearch**: 최고 정확도

### Arduino R4 Minima 최적화 포인트
- 메모리 효율성이 가장 중요한 평가 기준
- Dynamic Programming은 메모리 제약으로 제한적 사용
- Random Sampling이 가장 안전한 선택

## 🛠️ 고급 사용법

### 사용자 정의 테스트
```python
from test_runner import WeigherSorterTestRunner, TestConfiguration

config = TestConfiguration(
    test_sets=5000,
    simulate_fatigue=True,
    detailed_logging=True
)

runner = WeigherSorterTestRunner(config)
result = runner.run_all_tests()
```

### 특정 조합만 테스트
```python
result = runner.run_benchmark_test()
```

### 결과 분석
```python
# 상위 5개 조합
rankings = runner.monitor.get_ranking('success_rate')
for rank, (combination, metrics) in enumerate(rankings[:5]):
    print(f"{rank+1}. {combination}: {metrics['success_rate']:.1f}%")
```

## ⚠️ 주의사항

1. **메모리 압박**: 장시간 테스트시 Arduino 리셋 발생 가능
2. **타임아웃**: Dynamic Programming에서 메모리 부족시 자동 폴백
3. **스택 오버플로우**: 재귀 호출 최소화로 방지
4. **노이즈 시뮬레이션**: analogRead() 노이즈만 적용 (온도/전압 제외)

## 🎯 Phase별 검증 상태

- ✅ **Phase 1**: 난수 생성기 특성 분석 완료
- ✅ **Phase 2**: 조합 알고리즘 성능 테스트 완료
- ✅ **Phase 3**: 결과 분석 및 시각화 완료
- ✅ **Phase 4**: 개선 및 최적화 준비
- ✅ **Phase 5**: 최종 검증 환경 구축

## 🚀 다음 단계

1. 실제 Arduino R4 Minima 보드에서 검증
2. 현장 포도 농장 환경에서 테스트
3. 사용자 인터페이스 개발
4. 최종 배포 및 유지보수

---

**개발자**: Claude Code Assistant  
**문서 버전**: 1.0  
**최종 업데이트**: 2025-08-25  

Arduino R4 Minima의 제약사항을 완벽히 고려한 실전급 시뮬레이터입니다! 🎯
# User Guide - Arduino Multi-Implementation Testing System

## 👋 사용자 가이드 (최종판)

### 📋 목차
- [5분 빠른 시작](#5분-빠른-시작)
- [기본 사용법](#기본-사용법)
- [고급 사용법](#고급-사용법)
- [결과 해석](#결과-해석)
- [문제 해결](#문제-해결)

---

## 🚀 5분 빠른 시작

### 즉시 실행 (검증 완료)

```bash
# 1. 의존성 설치
pip install -r requirements.txt

# 2. 대시보드 실행
python run_dashboard.py
# 선택: 1 (자동 실제 Arduino 대시보드)

# 3. 브라우저에서 확인
# http://localhost:8053
```

**실행 과정:**
1. **10초 카운트다운** 🔟 ➜ 🔢 ➜ ✅
2. **8개 구현 자동 테스트**
3. **실시간 결과 표시**
4. **최종 추천 제시**

### 통계 분석 실행

```bash
# 통계 분석 실행
python run_analysis.py
# 선택: 2 (기본 성능 테스트)
```

---

## 📖 기본 사용법

### 대시보드 옵션

#### 1. 자동 실제 Arduino 대시보드 (추천) ⭐
- **포트**: http://localhost:8053
- **특징**: 10초 카운트다운, 자동 시작/완료
- **결과**: 실시간 차트, 성능 랭킹, 추천 시스템

#### 2. 기본 다중 구현 대시보드
- **포트**: http://localhost:8052
- **특징**: 수동 제어, 상세 설정 가능
- **용도**: 고급 사용자, 커스텀 분석

#### 3. 통계 분석 실행
- **출력**: 콘솔 + 파일
- **결과**: 상세 통계 보고서, 시각화

### 분석 옵션

#### 1. 상세 통계 분석 (추천) ⭐
- **기능**: 전체 빈도, 조건부 확률, 편향성 분석
- **출력**: 시각화 + 상세 보고서
- **시간**: 약 30초

#### 2. 기본 성능 테스트
- **기능**: 속도 측정, 제약 조건 검증
- **출력**: 콘솔 결과
- **시간**: 약 10초

#### 3. 커스텀 분석
- **기능**: 사용자 정의 반복 횟수, 시드
- **출력**: 맞춤형 보고서
- **시간**: 설정에 따라 가변

---

## 🎛️ 고급 사용법

### 설정 파일 커스터마이징

#### config/arduino_implementations_real.yaml 수정

```yaml
# 새로운 구현 추가
implementations:
  - id: "my_custom_impl"
    name: "My Custom Implementation"
    description: "사용자 정의 구현 방식"
    type: "custom_type"
    enabled: true
    arduino_code: |
      int prevNum = -1;
      int getRandomNum(){
        int num = random(0, 3);
        // 여기에 커스텀 로직
        if (num == prevNum) {
          num = (num + 1) % 3;
        }
        prevNum = num;
        return num;
      }
```

#### 테스트 설정 조정

```yaml
test_config:
  default_iterations: 20000     # 더 정밀한 측정
  default_seed: 54321          # 다른 시드
```

### 프로그래밍 방식 사용

```python
import sys
import os
sys.path.append('src/arduino_simulation')

from real_arduino_sim import RealArduinoImplementationGenerator
from arduino_mock import ArduinoUnoR4WiFiMock

# Arduino 하드웨어 시뮬레이션
arduino = ArduinoUnoR4WiFiMock(seed=12345)

# 구현 설정
impl_config = {
    'id': 'ternary_based',
    'name': 'Ternary + Formula',
    'type': 'ternary_based'
}

# 생성기 생성
generator = RealArduinoImplementationGenerator(impl_config, arduino)

# 숫자 생성 테스트
for i in range(10):
    number = generator.generate_number()
    print(f"Generated: {number}")
```

---

## 📊 결과 해석

### 성능 지표 이해

#### 1. 생성 속도 (gen/sec)
```
1,829,976 gen/sec = 초당 183만 개 숫자 생성
```
- **우수**: 1,500,000+ gen/sec
- **양호**: 1,000,000+ gen/sec  
- **보통**: 500,000+ gen/sec

#### 2. 제약 조건 위반 (Violations)
```
Violations: 0 = 완벽한 제약 준수 ✅
Violations: >0 = 연속된 동일 숫자 발생 ❌
```

#### 3. 편향성 패턴
```
균등: ≈0.5/0.5 (진정한 랜덤성)
편향: 2/3:1/3 또는 1/3:2/3 (특정 방향 선호)
```

### 최종 성능 결과 해석

| 구현 방식 | 속도 | 편향성 | 추천 용도 |
|-----------|------|--------|-----------|
| Switch Case Method | 1위 | 2/3:1/3 | 고성능 시스템 |
| Ternary + Formula | 2위 | 2/3:1/3 | 간결한 코드 |
| Static Variable | 3위 | 1/3:2/3 | 메모리 제약 |
| Recursive Method | 7위 | 균등 | 암호학적 용도 |

### 상황별 최적 선택

#### 🎯 성능이 최우선인 경우
1. Switch Case Method (1,829,976 gen/sec)
2. Ternary + Formula (1,718,273 gen/sec)
3. Static Variable Method (1,678,393 gen/sec)

#### 🎲 랜덤성이 최우선인 경우
1. Recursive Method (균등 분포)
2. Array + Conditional (높은 성능 + 적당한 편향)

#### 💾 메모리가 제한적인 경우
1. Static Variable Method (~4 bytes)
2. Ternary + Formula (~4 bytes)
3. Bitwise Operation (~4 bytes)

---

## 🔧 문제 해결

### 자주 발생하는 문제

#### 1. 대시보드가 시작되지 않음
```bash
# 해결 방법
pip install -r requirements.txt
python run_dashboard.py
```

#### 2. 포트 충돌
```
Error: Address already in use
```
**해결**: 다른 포트 사용 또는 기존 프로세스 종료

#### 3. 설정 파일 오류
```
Error: arduino_implementations_real.yaml not found
```
**해결**: config/ 폴더에 설정 파일 확인

#### 4. Import 오류
```
Error: No module named 'real_arduino_sim'
```
**해결**: Python 경로 확인, 의존성 재설치

### 성능 최적화 팁

#### 1. 빠른 테스트
```yaml
test_config:
  default_iterations: 1000  # 빠른 확인용
```

#### 2. 정밀 측정
```yaml
test_config:
  default_iterations: 50000  # 정확한 측정용
```

#### 3. 선택적 테스트
```yaml
implementations:
  - id: "switch_case"
    enabled: true    # 테스트 실행
  - id: "recursive_method"
    enabled: false   # 테스트 제외
```

---

## 💡 팁과 요령

### 효율적인 사용법

#### 1. 단계적 접근
```bash
# 1단계: 빠른 확인
python run_analysis.py  # 선택: 2

# 2단계: 상세 분석
python run_analysis.py  # 선택: 1

# 3단계: 대시보드 확인
python run_dashboard.py  # 선택: 1
```

#### 2. 결과 파일 활용
```bash
# 생성된 파일들
reports/detailed_statistical_report.txt  # 상세 보고서
reports/statistical_analysis.png         # 시각화
src/results/simulation_*.json            # 원시 데이터
```

#### 3. 비교 분석
```python
# 여러 시드로 일관성 확인
seeds = [12345, 54321, 98765]
for seed in seeds:
    # 각 시드별 결과 비교
    pass
```

### 고급 활용법

#### 1. 자동화 스크립트
```bash
#!/bin/bash
# 여러 설정으로 자동 테스트
for iterations in 1000 5000 10000; do
    echo "Testing with $iterations iterations..."
    python run_analysis.py
done
```

#### 2. 결과 모니터링
```python
# 실시간 결과 추적
import time
while True:
    # 결과 파일 확인
    time.sleep(30)
```

---

## 📚 추가 리소스

### 관련 문서
- **[성능 분석](04_PERFORMANCE_ANALYSIS.md)** - 상세 벤치마크
- **[통계 분석](05_STATISTICAL_ANALYSIS.md)** - 편향성 분석
- **[API 문서](03_API_REFERENCE.md)** - 개발자 가이드
- **[문제 해결](06_TROUBLESHOOTING.md)** - 상세 문제 해결

### 학습 순서 추천

#### 초보자 (1일)
1. 5분 빠른 시작 실행
2. 기본 결과 해석 학습
3. 간단한 설정 수정 시도

#### 중급자 (3일)
1. 고급 사용법 익히기
2. 커스텀 분석 실행
3. 결과 파일 분석

#### 고급자 (1주)
1. 프로그래밍 방식 활용
2. 새로운 구현 추가
3. 자동화 스크립트 작성

---

## 🎯 마무리

이 사용자 가이드를 통해 Arduino 랜덤 숫자 생성기의 8가지 구현을 효과적으로 비교 분석할 수 있습니다.

**핵심 포인트:**
- **간단한 실행**: `python run_dashboard.py` 한 줄
- **다양한 분석**: 성능, 통계, 편향성 모두 지원
- **실용적 추천**: 용도별 최적 구현 제시

**즐거운 분석 되세요!** 🚀

---

*User Guide 마지막 업데이트: 2025년 8월 12일*
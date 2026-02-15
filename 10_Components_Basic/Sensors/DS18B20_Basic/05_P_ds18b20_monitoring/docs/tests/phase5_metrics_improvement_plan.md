# Phase 5 - 매트릭 도구 개선 및 점수 향상 계획서

**작성일**: 2025년 8월 18일 20시 50분  
**브랜치**: `phase3_a_quality`  
**목적**: 매트릭 도구 통합/개선 및 점수 70점 달성

## 🎯 Phase 5 목표

### 🏆 최종 목표
- **매트릭 점수**: 현재 상태 → 70점 달성
- **도구 통합**: 중복 도구 정리 및 통합
- **도구 최적화**: 프로젝트에 맞는 매트릭 수집
- **자동화 강화**: 원클릭 매트릭 분석 시스템

## 📊 현재 매트릭 도구 분석

### 기존 도구 현황
1. **`tools/metrics/integrated_metrics.py`**: 통합 매트릭 관리자
2. **`tools/metrics/run_metrics_simple.py`**: 간단한 매트릭 실행기
3. **`tools/metrics/python_coverage.py`**: Python 커버리지 도구
4. **`tools/metrics/arduino_metrics.py`**: Arduino 코드 분석
5. **`tools/run_all_checks.py`**: 전체 품질 검사 (일부 매트릭 포함)

### 중복 및 개선 필요 사항
- ❌ **중복된 커버리지 수집**: 여러 도구에서 동일 작업
- ❌ **일관성 없는 리포트 형식**: JSON, MD, HTML 혼재
- ❌ **Windows 호환성 문제**: 인코딩 및 폰트 이슈
- ❌ **매트릭 기준 불명확**: 70점 달성 기준 모호
- ❌ **의존성 문제**: matplotlib, 폰트 설정 오류

## 🛠️ Phase 5 세부 계획

### Phase 5A: 도구 분석 및 정리 (Day 1-2)

#### Step 1: 현재 매트릭 점수 측정
- [ ] 모든 매트릭 도구 실행 및 현재 상태 파악
- [ ] 점수 산정 방식 분석 및 문서화
- [ ] 70점 달성을 위한 필요 개선사항 식별

#### Step 2: 중복 도구 식별 및 분류
- [ ] 기능별 도구 매핑 (커버리지, 복잡도, 품질 등)
- [ ] 중복 기능 식별 및 통합 대상 선정
- [ ] 유지/개선/통합/삭제 도구 분류

#### Step 3: 프로젝트 특화 요구사항 정의
- [ ] DS18B20 프로젝트에 특화된 매트릭 정의
- [ ] Arduino + Python 혼합 프로젝트 매트릭 전략
- [ ] 품질 점수와 매트릭 점수 연동 방안

### Phase 5B: 도구 통합 및 개선 (Day 3-4)

#### Step 1: 통합 매트릭 엔진 개발
```python
# 새로운 통합 도구: tools/metrics/unified_metrics.py
class UnifiedMetricsEngine:
    - Python 코드 품질 (커버리지, 복잡도, 타입 안전성)
    - Arduino 코드 품질 (라인 수, 주석 비율, 구조)
    - 프로젝트 매트릭 (파일 구조, 문서화, 테스트)
    - 통합 점수 계산 (0-100점 스케일)
```

#### Step 2: Windows 호환성 강화
- [ ] 인코딩 문제 완전 해결
- [ ] matplotlib 폰트 설정 최적화
- [ ] subprocess 실행 안정성 개선
- [ ] 에러 핸들링 강화

#### Step 3: 리포트 통일
- [ ] 일관된 JSON 스키마 정의
- [ ] 통합 HTML 대시보드 생성
- [ ] 트렌드 분석 및 시각화 개선

### Phase 5C: 매트릭 최적화 (Day 5-6)

#### Step 1: 코드 품질 매트릭 향상
**Python 코드 개선**:
- [ ] 테스트 커버리지 현재 0% → 50% 목표
- [ ] 순환 복잡도 개선 (복잡한 함수 리팩토링)
- [ ] 타입 힌트 완성도 측정 및 개선
- [ ] 문서화 완성도 측정 (docstring 커버리지)

**Arduino 코드 개선**:
- [ ] 주석 비율 현재 상태 → 20% 목표
- [ ] 함수 크기 및 복잡도 분석
- [ ] 코드 중복 감지 및 제거

#### Step 2: 프로젝트 구조 매트릭 향상
- [ ] 모듈화 점수 측정 및 개선
- [ ] 의존성 그래프 분석
- [ ] 파일 조직화 점수 개선
- [ ] README 및 문서 품질 점수

### Phase 5D: 자동화 및 통합 (Day 7)

#### Step 1: CI/CD 통합 준비
- [ ] 매트릭 수집 자동화 스크립트
- [ ] 점수 임계값 설정 및 알림
- [ ] 트렌드 모니터링 시스템

#### Step 2: 최종 검증 및 튜닝
- [ ] 70점 달성 확인 및 미세 조정
- [ ] 성능 최적화 및 안정성 검증
- [ ] 사용자 가이드 작성

## 📋 매트릭 점수 구성 (목표: 70점)

### 점수 배분 계획
```
총 100점 = 70점 목표

1. 코드 품질 (40점)
   - Python 커버리지 (15점): 50% = 7.5점
   - 복잡도 점수 (15점): 양호 = 12점  
   - 타입 안전성 (10점): 100% = 10점

2. 프로젝트 구조 (25점)
   - 모듈화 (10점): 양호 = 8점
   - 문서화 (10점): 충분 = 8점
   - 의존성 관리 (5점): 우수 = 5점

3. 테스트 품질 (20점)
   - 단위 테스트 (15점): 50% = 7.5점
   - 통합 테스트 (5점): 기본 = 2.5점

4. 도구 및 자동화 (15점)
   - 품질 도구 (10점): 완료 = 10점
   - CI/CD 준비 (5점): 기본 = 3점

예상 총점: 7.5 + 12 + 10 + 8 + 8 + 5 + 7.5 + 2.5 + 10 + 3 = 73.5점 ✅
```

## 🔄 실행 체크리스트

### Phase 5A: 도구 분석 및 정리 ✅ 완료 (2025-08-18)
- [x] 현재 매트릭 점수 측정 *(unified_metrics quick/full 실행으로 기본 점수/구성 산출 및 스코어 산식 초안 확정)*
- [x] 중복 도구 식별 및 분류 *(`phase5_tool_analysis.md` 기반 – 통합 대상 및 유지 대상 확정)*  
- [x] 프로젝트 특화 요구사항 정의 *(도메인/실시간/EEPROM/프로토콜/구성 drift/latency/UI/observability 지표 목록 및 카테고리 → 점수 매핑 수립)*
- [x] 스키마 v1 정의 및 경량 검증기 적용 (`metrics_schema_v1.json`, `json_exporter` 구조 검증)
- [x] 커버리지 안정성 가드 (빈 리포트 재시도 + fallback 사용 시 warning)
- [x] 트렌드 계산 로직 및 단위 테스트 추가 (점수/커버리지/이슈 개선 분류)
- [x] Python Collector 커버리지 fallback 단위 테스트

> Phase 5A 산출물: unified_metrics 스켈레톤, 점수 breakdown 초안, schema v1, trend & fallback tests, coverage retry/fallback 경고 체계

### Phase 5B: 도구 통합 및 개선 (진행 중)
- [x] 통합 매트릭 엔진 개발 *(스켈레톤 + scoring 초안 + trend + warnings/top_offenders + fail-under 준비)*
   - 남은 항목: Arduino collector 통합, latency profiler 확장, scorer 분리 리팩터링, HTML/MD 시각화 고도화
- [ ] Windows 호환성 강화 *(일부 UTF-8 처리 적용, 폰트/그래프/경로 검증 및 PowerShell CI 테스트 대기)*
- [ ] 리포트 형식 통일 *(JSON 스키마 완료, Markdown/HTML 대시보드 통합 및 이전 개별 리포트 폐기 진행 예정)*
 - [x] CI fail-under/스키마 자동 검증 워크플로 추가 *(기존 `.github/workflows/metrics_quality.yml`은 단계적 개선으로 현재는 보존하지 않음 — 통합된 `unified_metrics` 기반의 새로운 파이프라인 사용 권장)*
- [ ] 레거시 스크립트 단계적 제거 (`python_coverage.py`, `run_metrics_simple.py`, `integrated_metrics.py`) 및 deprecation 배너 표시

### Phase 5C: 매트릭 최적화  (예정)
- [ ] Python 코드 품질 향상 *(커버리지 + 문서화 + 복잡도 refactor)*
- [ ] Arduino 코드 품질 향상 *(주석 비율/라인 통계 Collector 도입 후 목표치 측정)*
- [ ] 프로젝트 구조 개선 *(doc coverage / avg defs / 모듈화 지표 튜닝)*

### Phase 5D: 자동화 및 통합 (예정)
- [ ] CI/CD 통합 준비 *(GitHub Actions + 아티팩트 업로드 + 배지)*
- [ ] 최종 검증 및 70점 달성 확인 *(fail-under 상향 조정, 회귀 모니터)*

## 🎯 성공 기준

### 정량적 목표
- **매트릭 총점**: 70점 이상 달성
- **Python 커버리지**: 50% 이상
- **Arduino 주석 비율**: 20% 이상  
- **도구 통합**: 5개 → 3개 이하로 정리
- **리포트 통일**: 단일 대시보드 제공

### 정성적 목표
- **사용성 개선**: 원클릭 매트릭 수집
- **가독성 향상**: 직관적인 점수 시스템
- **유지보수성**: 확장 가능한 구조
- **안정성**: Windows 환경 완전 지원

## 🚀 다음 단계 (업데이트 2025-08-18)

### 즉시 수행 예정 (Phase 5B 잔여)
1. **Validator 강화**: `scores.total` / `scores.breakdown` 타입 & 필수 필드 추가 검사
2. **추가 Trend 회귀 테스트**: flat / regressing / no-previous 케이스
3. **Deprecated 배너**: 레거시 스크립트 실행 시 안내 출력
4. **HTML/Markdown 통합 리포트**: 기존 분리 출력 대체
5. **Windows 실행 검증**: PowerShell 환경에서 workflow와 동일 플로우 재현 시험

### 단기 우선순위 (5B→5C 전환 전)
1. Coverage 0→안정적 수치 상승 (회귀 방지 경로 확립)
2. Arduino collector 1차 통합 (라인/주석/파일 수)
3. Domain & Runtime collector 확장 (latency p95 샘플링 mock → 실제 측정)
4. Fail-under 단계적 상향 (40 → 50 → 60 → 70)
5. README / 한글 문서 사용 예 & 경고/트렌드 설명 추가

### 중기 (Phase 5C)
1. 복잡도 높은 함수 리팩터링 목표 리스트 도출
2. docstring 커버리지 측정 파이프라인 (현재 필드 doc_coverage 활용) 고도화
3. Arduino 주석 비율 20% 달성 + 측정 자동화
4. Drift / persistence 실패 시 감점 로직 fail-hard 조건 자동화

### 장기 (Phase 5D 이후)
1. GitHub PR 코멘트 자동 요약 (총점/증감/경고)
2. 배지(shields.io)용 경량 JSON endpoint 제공
3. 점수 히스토리 Sparkline 포함 HTML 대시보드
4. 메트릭 히스토리 기반 회귀 감지 (3회 평균 대비 하락 알림)

---

**📋 작성자**: AI Assistant  
**🎯 검토자**: 사용자  
**📅 다음 검토일**: 2025-08-19  
**🔄 업데이트 주기**: 매일 진행상황 업데이트

---

## 📋 Phase 5 시작 준비 완료!

Phase 3A의 100% 품질 달성을 바탕으로, 이제 매트릭 도구 개선을 통한 
종합적인 프로젝트 품질 향상을 목표로 합니다. 

**준비됐으면 Phase 5A부터 시작하겠습니다!** 🚀
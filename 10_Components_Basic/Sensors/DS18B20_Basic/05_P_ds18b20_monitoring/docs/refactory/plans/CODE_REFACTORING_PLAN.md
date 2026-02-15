# 코드 리팩토링 계획서

**작성일**: 2025-08-22  
**목적**: src 폴더 내 대용량 파일들의 코드 줄 수 감소 및 모듈화  
**목표**: 가독성 향상, 유지보수성 증대, 단일 책임 원칙 준수

---

## 📊 현재 상태 분석

### 상위 10개 대용량 파일 (총 줄 수)

| 순위 | 파일명 | 줄 수 | 주요 기능 | 우선순위 |
|------|--------|-------|-----------|----------|
| 1 | `src/python/dashboard/app.py` | **2,498** | Dash 앱 메인 | 🔥 최우선 |
| 2 | `src/python/dashboard/data_manager.py` | **877** | 데이터 관리 | 🔥 최우선 |
| 3 | `src/python/simulator/protocol.py` | **823** | 프로토콜 처리 | ⚡ 높음 |
| 4 | `src/python/dashboard/port_ui_components.py` | **745** | UI 컴포넌트 | ⚡ 높음 |
| 5 | `src/python/dashboard/port_callbacks.py` | **539** | 콜백 관리 | 📊 중간 |
| 6 | `src/python/simulator/ds18b20_simulator.py` | **533** | 센서 시뮬레이터 | 📊 중간 |
| 7 | `src/python/simulator/offline_simulator.py` | **519** | 오프라인 시뮬레이터 | 📊 중간 |
| 8 | `src/python/dashboard/port_manager.py` | **504** | 포트 관리 | 🔧 낮음 |
| 9 | `src/python/dashboard/serial_handler_fixed.py` | **368** | 시리얼 통신 | 🔧 낮음 |
| 10 | `src/python/simulator/simulator_manager.py` | **295** | 시뮬레이터 관리 | 🔧 낮음 |

**총 줄 수**: 7,700줄  
**목표 감소**: 30-40% (2,300-3,100줄 감소)

---

## 🎯 대계획 (Big Picture Strategy)

### 1. 아키텍처 패턴 도입
- **MVC 패턴** 완전 분리
- **Factory 패턴** 적용 (UI 컴포넌트, 시뮬레이터)
- **Observer 패턴** 적용 (콜백 시스템)
- **Strategy 패턴** 적용 (시뮬레이션 전략)

### 2. 모듈 분리 전략
- **수직 분리**: 기능별 모듈 분리
- **수평 분리**: 계층별 분리 (View, Controller, Model)
- **공통 모듈**: 유틸리티, 상수, 설정 분리

### 3. 코드 품질 개선
- **DRY 원칙**: 중복 코드 제거
- **SOLID 원칙**: 단일 책임, 개방-폐쇄 원칙 적용
- **Clean Code**: 함수 크기 20줄 이하, 클래스 200줄 이하

---

## 🔥 Phase 1: 최우선 리팩토링 (app.py - 2,498줄 → 800줄)

### 📋 현재 문제점
- **단일 파일에 모든 기능 집중** (UI + 로직 + 콜백)
- **거대한 함수들** (일부 함수 100줄 이상)
- **중복된 UI 생성 코드**
- **복잡한 콜백 체인**

### 🎯 목표 구조
```
src/python/dashboard/
├── app.py (800줄) - 메인 앱 진입점
├── layouts/
│   ├── __init__.py
│   ├── main_layout.py (200줄) - 메인 레이아웃
│   ├── sensor_layout.py (150줄) - 센서 관련 UI
│   ├── chart_layout.py (150줄) - 차트 UI
│   └── control_layout.py (100줄) - 제어 UI
├── callbacks/
│   ├── __init__.py
│   ├── sensor_callbacks.py (200줄) - 센서 콜백
│   ├── chart_callbacks.py (150줄) - 차트 콜백
│   ├── ui_callbacks.py (150줄) - UI 상태 콜백
│   └── data_callbacks.py (100줄) - 데이터 처리 콜백
├── components/
│   ├── __init__.py
│   ├── charts.py (200줄) - 차트 컴포넌트
│   ├── controls.py (150줄) - 제어 컴포넌트
│   └── indicators.py (100줄) - 지시계 컴포넌트
└── utils/
    ├── __init__.py
    ├── constants.py (50줄) - 상수 정의
    └── helpers.py (100줄) - 헬퍼 함수
```

### 📝 소계획 1.1: 레이아웃 분리 (1주차)
```python
# 현재 (app.py 내부)
def create_main_layout():  # 200줄
def create_sensor_panel():  # 150줄
def create_chart_section():  # 150줄

# 목표 (분리 후)
# layouts/main_layout.py
from .sensor_layout import create_sensor_panel
from .chart_layout import create_chart_section
def create_main_layout():  # 50줄
```

### 📝 소계획 1.2: 콜백 분리 (2주차)
```python
# 현재 (app.py 내부)
@app.callback(...)  # 50개+ 콜백이 한 파일에

# 목표 (분리 후)
# callbacks/sensor_callbacks.py
def register_sensor_callbacks(app): # 센서 관련 콜백만
# callbacks/chart_callbacks.py  
def register_chart_callbacks(app): # 차트 관련 콜백만
```

### 📝 소계획 1.3: 컴포넌트 팩토리 (3주차)
```python
# 현재 (중복 코드 다수)
dbc.Card([...], style={...})  # 20회 반복

# 목표 (팩토리 패턴)
# components/charts.py
class ChartFactory:
    @staticmethod
    def create_temperature_chart(): ...
    @staticmethod  
    def create_realtime_chart(): ...
```

---

## 🔥 Phase 2: 최우선 리팩토링 (data_manager.py - 877줄 → 400줄)

### 📋 현재 문제점
- **거대한 단일 클래스** (모든 데이터 관리 기능)
- **메서드 크기 과대** (일부 메서드 50줄 이상)
- **다중 책임** (저장, 분석, 내보내기, 알림)

### 🎯 목표 구조
```
src/python/dashboard/data/
├── __init__.py
├── manager.py (200줄) - 메인 매니저 (코디네이터)
├── storage.py (150줄) - 데이터 저장소
├── analyzer.py (150줄) - 데이터 분석
├── exporter.py (100줄) - 데이터 내보내기
├── alerts.py (100줄) - 알림 시스템
└── models.py (80줄) - 데이터 모델
```

### 📝 소계획 2.1: 데이터 저장소 분리
```python
# 현재 (data_manager.py 내부)
class DataManager:
    def add_data(self): ...  # 30줄
    def get_data(self): ...  # 25줄
    def filter_data(self): ...  # 40줄

# 목표 (분리 후)
# data/storage.py
class DataStorage:
    def add(self): ...  # 10줄
    def get(self): ...  # 8줄
    def filter(self): ...  # 15줄
```

### 📝 소계획 2.2: 분석 엔진 분리
```python
# 현재 (복잡한 통계 계산이 혼재)
def calculate_statistics(self): ...  # 60줄

# 목표 (전용 분석 클래스)
# data/analyzer.py
class DataAnalyzer:
    def calculate_basic_stats(self): ...  # 15줄
    def calculate_trends(self): ...  # 20줄
    def detect_anomalies(self): ...  # 25줄
```

---

## ⚡ Phase 3: 높은 우선순위 (protocol.py - 823줄 → 400줄)

### 📋 현재 문제점
- **모든 프로토콜 기능이 한 파일**
- **파싱, 검증, 생성이 혼재**
- **메시지 타입별 처리 로직 중복**

### 🎯 목표 구조
```
src/python/simulator/protocol/
├── __init__.py
├── base.py (100줄) - 기본 프로토콜 클래스
├── parser.py (120줄) - 메시지 파싱
├── validator.py (100줄) - 데이터 검증
├── builder.py (80줄) - 메시지 생성
├── handlers/
│   ├── sensor_handler.py (80줄)
│   ├── command_handler.py (80줄)
│   └── response_handler.py (60줄)
└── types.py (100줄) - 타입 정의
```

### 📝 소계획 3.1: 메시지 타입별 핸들러 분리
```python
# 현재 (거대한 switch 문)
def handle_message(self, msg_type, data):  # 100줄
    if msg_type == "sensor_data": ...  # 25줄
    elif msg_type == "command": ...  # 30줄
    elif msg_type == "response": ...  # 45줄

# 목표 (핸들러 패턴)
# handlers/sensor_handler.py
class SensorHandler:
    def handle(self, data): ...  # 15줄
```

---

## ⚡ Phase 4: 높은 우선순위 (port_ui_components.py - 745줄 → 350줄)

### 📋 현재 문제점
- **모든 UI 컴포넌트가 한 파일**
- **컴포넌트 생성 코드 중복**
- **스타일링 코드 산재**

### 🎯 목표 구조
```
src/python/dashboard/components/
├── __init__.py
├── base.py (80줄) - 기본 컴포넌트 클래스
├── port/
│   ├── port_selector.py (100줄)
│   ├── port_status.py (80줄)
│   └── port_controls.py (90줄)
├── styles/
│   ├── __init__.py
│   ├── colors.py (30줄)
│   ├── layouts.py (40줄)
│   └── components.py (50줄)
└── factories/
    ├── __init__.py
    ├── card_factory.py (60줄)
    └── button_factory.py (40줄)
```

---

## 📊 Phase 5-7: 중간 우선순위 (각각 300-400줄 목표)

### Phase 5: port_callbacks.py (539줄 → 300줄)
- 콜백 타입별 분리
- 이벤트 핸들러 패턴 적용

### Phase 6: ds18b20_simulator.py (533줄 → 350줄) 
- 시뮬레이션 로직과 데이터 생성 분리
- 설정 관리 분리

### Phase 7: offline_simulator.py (519줄 → 300줄)
- 온라인/오프라인 시뮬레이터 공통 기능 추출
- 추상 기본 클래스 도입

---

## 🛠️ 구현 로드맵

### 1단계: 준비 단계 (1주)
- [ ] 현재 코드 의존성 분석
- [ ] 단위 테스트 커버리지 확인
- [ ] 백업 브랜치 생성
- [ ] 리팩토링 전 기준 메트릭 수집

### 2단계: Phase 1 실행 (3주)
- [ ] **Week 1**: app.py 레이아웃 분리
- [ ] **Week 2**: app.py 콜백 분리  
- [ ] **Week 3**: app.py 컴포넌트 팩토리 적용

### 3단계: Phase 2 실행 (2주)
- [ ] **Week 4**: data_manager.py 저장소 분리
- [ ] **Week 5**: data_manager.py 분석 엔진 분리

### 4단계: Phase 3-4 실행 (3주)
- [ ] **Week 6**: protocol.py 핸들러 분리
- [ ] **Week 7**: protocol.py 파서/빌더 분리
- [ ] **Week 8**: port_ui_components.py 컴포넌트 분리

### 5단계: Phase 5-7 실행 (3주)
- [ ] **Week 9**: 콜백 시스템 리팩토링
- [ ] **Week 10**: 시뮬레이터 분리
- [ ] **Week 11**: 최종 정리 및 테스트

### 6단계: 검증 및 최적화 (1주)
- [ ] **Week 12**: 전체 시스템 테스트, 성능 검증, 문서화

---

## 📏 성공 지표

### 정량적 목표
- **총 코드 줄 수**: 7,700줄 → 5,400줄 (30% 감소)
- **최대 파일 크기**: 2,498줄 → 800줄 (68% 감소)
- **평균 함수 크기**: 25줄 → 15줄 (40% 감소)
- **클래스당 메서드 수**: 15개 → 8개 (47% 감소)

### 정성적 목표
- **가독성**: 파일별 단일 책임 명확화
- **유지보수성**: 기능 추가 시 수정 범위 최소화
- **테스트 용이성**: 단위 테스트 커버리지 80% 이상
- **성능**: 기존 기능 성능 저하 없음

### 코드 품질 지표
- **Cyclomatic Complexity**: 평균 5 이하
- **Code Duplication**: 5% 이하
- **Technical Debt Ratio**: 10% 이하

---

## 🔧 리팩토링 도구 및 규칙

### 사용 도구
```bash
# 리팩토링 전후 품질 검사
python tools/afterCoding/quick_quality_check.py --fix

# 메트릭 측정
python tools/metrics/unified_metrics.py

# 의존성 분석 (추가 필요)
python -m pydeps src/python/dashboard/app.py --show-deps
```

### 리팩토링 규칙
1. **테스트 우선**: 기능 변경 전 테스트 작성
2. **점진적 변경**: 한 번에 한 가지 책임만 분리
3. **하위 호환성**: 기존 API 유지
4. **문서화**: 변경사항 즉시 문서화

---

## 🎯 기대 효과

### 개발 효율성
- **신규 기능 추가 시간 50% 단축**
- **버그 수정 시간 40% 단축**
- **코드 리뷰 시간 60% 단축**

### 코드 품질
- **단위 테스트 커버리지 현재 대비 2배 향상**
- **코드 중복도 70% 감소**
- **신규 개발자 온보딩 시간 50% 단축**

### 장기적 이익
- **기술 부채 50% 감소**
- **유지보수 비용 30% 절감**
- **확장성 및 재사용성 대폭 개선**

---

**💡 핵심 원칙**: "Big Bang 리팩토링 금지, 점진적 개선으로 위험 최소화"  
**🎯 최종 목표**: "읽기 쉽고, 수정하기 쉽고, 테스트하기 쉬운 코드"
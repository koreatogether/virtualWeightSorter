# DS18B20 온도 센서 모니터링 시스템

Arduino Mock 시뮬레이터와 Dash 대시보드를 사용한 실시간 온도 모니터링 시스템

## 🌡️ 시스템 개요

DS18B20 디지털 온도 센서를 사용하여 실시간으로 온도를 모니터링하고, 웹 기반 대시보드에서 데이터를 시각화하는 시스템입니다.

### 주요 특징
- **실시간 모니터링**: 온도 데이터 실시간 수집 및 표시 (소수 1자리)
- **Arduino Mock 시뮬레이터**: 실제 하드웨어 없이 테스트 가능
- **JSON 통신**: 구조화된 데이터 교환
- **웹 대시보드**: Dash 기반 반응형 인터페이스
- **설정 변경**: ID, 임계값(TH/TL), 측정주기 동적 변경

## 🏗️ 시스템 구조

```
[DS18B20 센서] → [Arduino Mock] ↔ [Serial/JSON] ↔ [Dash App]
                                                    ↑
                                               [사용자 UI]
```

## 🚀 주요 기능

### 데이터 읽기
- ✅ 온도값 읽기 (소수 1자리 정밀도)
- ✅ 센서 ADDR 읽기
- ✅ 임계값 읽기 (TH/TL)
- ✅ 측정주기 읽기

### 설정 변경
- ✅ 센서 ID 변경 (1~8 범위값)
- ✅ 임계값 변경 (TH/TL)
- ✅ 측정주기 변경

### 사용자 인터페이스
- ✅ 3가지 제어 버튼
- ✅ 실시간 온도 그래프
- ✅ 다이얼 방식 표시 주기 변경
- ✅ 센서별 로그 창

## 📁 프로젝트 구조

```
├── docs/                          # 문서
│   ├── architecture/              # 시스템 아키텍처
│   └── development/               # 개발 가이드
├── src/                           # 소스 코드
│   ├── arduino_mock/              # Arduino 시뮬레이터
│   ├── python/                    # Python 애플리케이션
│   └── dash_app/                  # Dash 대시보드
├── tools/                         # 개발 도구
│   ├── metrics/                   # 코드 메트릭스
│   ├── quality/                   # 품질 검사
│   └── security/                  # 보안 검사
├── tests/                         # 테스트 코드
└── .kiro/                         # Kiro IDE 설정
    └── specs/                     # 개발 스펙
```

## 🛠️ 기술 스택

### Arduino Mock Simulator
- **언어**: Python
- **라이브러리**: pySerial, json, random
- **기능**: DS18B20 센서 시뮬레이션

### Dash Application
- **언어**: Python
- **프레임워크**: Dash, Plotly
- **UI**: Dash Bootstrap Components
- **통신**: pySerial
- **데이터**: pandas, json

## 📋 개발 단계

### Phase 1: 기본 시스템 구축 (12-19시간)
- Arduino Mock 시뮬레이터 개발
- JSON 통신 프로토콜 구현
- Dash 앱 기본 구조

### Phase 2: 대시보드 개발 (22-31시간)
- 실시간 온도 그래프
- 제어 버튼 인터페이스
- 기본 로그 시스템

### Phase 3: 고급 기능 구현 (36-50시간)
- 고급 로그 시스템
- 다이얼 방식 표시 주기 변경
- 데이터 내보내기 기능

## 🔧 설치 및 실행

### 요구사항
- Python 3.8+
- pip 또는 uv

### 설치
```bash
# 저장소 클론
git clone https://github.com/koreatogether/private_ds18b20_monitoring.git
cd private_ds18b20_monitoring

# 의존성 설치
pip install -r requirements.txt
# 또는 uv 사용
uv sync
```

### 실행
```bash
# Arduino Mock 시뮬레이터 실행
python src/arduino_mock/ds18b20_simulator.py

# Dash 앱 실행 (새 터미널에서)
python src/dash_app/app.py
```

## 🧪 테스트 및 품질 검사

### 통합 검사 실행
```bash
# 모든 품질 및 보안 검사
python tools/run_all_checks.py

# 메트릭스 분석
python tools/metrics/run_metrics_simple.py

# 개별 검사
python tools/quality/quality_check.py      # 코드 품질
python tools/security/trufflehog_check.py  # 보안 검사
```

### 테스트 실행
```bash
# 단위 테스트
pytest tests/

# 커버리지 포함
pytest tests/ --cov=src/python --cov-report=html
```

## 📊 JSON 통신 프로토콜

### 센서 데이터 (Arduino → Dash)
```json
{
  "sensor_addr": "28FF123456789ABC",
  "temperature": 25.5,
  "th_value": 30.0,
  "tl_value": 20.0,
  "measurement_interval": 1000,
  "timestamp": "2025-01-17T10:30:00Z"
}
```

### 설정 변경 명령 (Dash → Arduino)
```json
{
  "command": "set_config",
  "sensor_id": "28FF123456789ABC",
  "config_type": "addr|th|tl|interval",
  "new_value": "새로운 값"
}
```

## 🤝 기여 가이드

1. **개발 전 확인사항**
   - [도구 가이드](.kiro/specs/dht22-monitoring-system/tool_guide.md) 숙지
   - [개발 문서](docs/development/) 참조

2. **개발 워크플로우**
   ```bash
   # 1. 기능 개발
   # 2. 품질 검사
   python tools/run_all_checks.py
   # 3. 모든 검사 통과 시 커밋
   git add .
   git commit -m "feat: 새로운 기능 추가"
   git push
   ```

3. **커밋 메시지 규칙**
   - `feat:` 새로운 기능
   - `fix:` 버그 수정
   - `docs:` 문서 수정
   - `test:` 테스트 추가/수정
   - `refactor:` 코드 리팩토링

## 📈 로드맵

- [x] 프로젝트 초기 설정
- [x] 아키텍처 설계
- [x] 개발 도구 구성
- [x] Phase 1: 기본 시스템 구축
- [x] Phase 2: 대시보드 개발  
- [x] **Phase 3A: 품질 개선 (100% 완료)**
  - [x] 코드 린팅 및 포맷팅 완료
  - [x] 타입 체크 완료
  - [x] 보안 취약점 해결
  - [x] 의존성 업데이트
- [ ] Phase 4: 테스트 환경 구축
- [ ] 다중 센서 지원
- [ ] 데이터베이스 연동

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다.

## 📞 문의

프로젝트 관련 문의사항이 있으시면 GitHub Issues를 통해 연락해 주세요.

---

## 🏆 품질 상태

![Quality](https://img.shields.io/badge/Quality-100%25-brightgreen)
![Ruff](https://img.shields.io/badge/Ruff-Passing-green)
![MyPy](https://img.shields.io/badge/MyPy-Passing-green)
![Security](https://img.shields.io/badge/Security-Clean-green)

**현재 품질 점수**: 4/4 통과 (100%) 🎯

---

**개발 상태**: ✅ Phase 3A 완료 (품질 개선 100% 달성)
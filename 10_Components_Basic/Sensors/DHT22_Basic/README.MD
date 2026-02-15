# DHT22 환경 모니터링 시스템

Arduino DHT22 센서와 Python Dash를 이용한 실시간 환경 모니터링 시스템입니다.

## 🏗️ 프로젝트 구조

```
04_P_dht22_monitoring/
├── src/
│   ├── arduino/
│   │   └── dht22_sensor/
│   │       └── dht22_sensor.ino      # Arduino 센서 코드
│   └── python/
│       ├── dashboard/
│       │   └── app.py               # Dash 웹 대시보드
│       └── utils/
│           ├── serial_reader.py     # 시리얼 통신 및 시뮬레이터
│           └── data_processor.py    # 데이터 처리 유틸리티
├── pyproject.toml                  # Python 의존성 관리
├── run_dashboard.bat               # 대시보드 실행 스크립트
└── README.md                       # 이 파일
```

## 🚀 빠른 시작

### 1. 가상환경 설정
```bash
# UV를 이용한 가상환경 생성 및 의존성 설치
uv venv
uv sync
```

### 2. 대시보드 실행
```bash
# Windows
run_dashboard.bat

# 또는 직접 실행
source .venv/Scripts/activate
python src/python/dashboard/app.py
```

### 3. 대시보드 접속
브라우저에서 `http://localhost:8050` 접속

## 🔧 하드웨어 설정

### Arduino 연결
```
DHT22 센서 -> Arduino
VCC      -> 5V
GND      -> GND
DATA     -> Pin 2
```

### 필요한 Arduino 라이브러리
- DHT sensor library (Adafruit)
- ArduinoJson
- Adafruit Unified Sensor

## 📊 기능

### 실시간 모니터링
- 온도 (°C)
- 습도 (%)
- 이슬점 계산
- 불쾌지수 및 체감 등급

### 데이터 시각화
- 실시간 온도/습도 차트
- 통계 테이블 (최소/최대/평균/현재값)
- 체감 등급 표시

### 시뮬레이터 모드
Arduino가 연결되지 않은 경우 자동으로 시뮬레이터 모드로 전환

## ⚙️ 설정

### 시리얼 포트 변경
`src/python/dashboard/app.py` 파일에서:
```python
sensor = DHT22SerialReader(port='COM3')  # 포트 변경
```

### 시뮬레이터/하드웨어 모드 변경
`src/python/dashboard/app.py` 파일에서:
```python
USE_SIMULATOR = False  # False: 아두이노 사용, True: 시뮬레이터 사용
```

## 🛠️ 개발

### 코드 품질 도구
```bash
# 코드 포맷팅
uv run black src/python/

# 린팅
uv run ruff check src/python/

# 테스트
uv run pytest
```

### 의존성 추가
```bash
# 새 패키지 추가
uv add 패키지명

# 개발 의존성 추가
uv add --dev 패키지명
```

## 📝 라이센스

이 프로젝트는 개인 학습 및 연구 목적으로 제작되었습니다.

## 🤝 기여

이슈나 개선사항이 있으시면 언제든 제보해주세요!

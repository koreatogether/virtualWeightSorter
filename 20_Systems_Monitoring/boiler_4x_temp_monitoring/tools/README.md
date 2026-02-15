# Tools / Scripts

이 폴더에는 데이터 수집 및 시스템 관리에 필요한 스크립트들이 포함되어 있습니다.

## 1. data_collector.py (핵심 수집기)
아두이노의 실시간 데이터를 가져와 PC 하드드라이브에 저장하는 역할을 합니다.

### 주요 기능
- **자동 트리거링**: 아두이노가 켜지면 자동으로 수집을 시작하고, 꺼지면 대기 모드로 전환됩니다.
- **7일 순환 저장**: `docs/logs/` 위치에 최장 7일간의 데이터를 보관하며, 7일이 넘으면 가장 오래된 데이터를 스스로 삭제합니다.
- **JSON 포맷**: 분석하기 쉬운 JSON 형식으로 초 단위 기록을 저장합니다.
- **인증 연동**: 아두이노에 설정된 대시보드 비밀번호를 사용하여 안전하게 데이터를 가져옵니다.

### 실행 방법
```powershell
# 필수 라이브러리 설치 (requests)
pip install requests

# 스크립트 실행
python tools/data_collector/data_collector.py
```

## 2. log_player.py (로그 시각화 플레이어)
`data_collector.py`로 수집된 JSON 로그 파일들을 읽어 시간 흐름에 따라 재생하고 분석하는 GUI 도구입니다.

### 주요 기능 (Rev 02 기준)
- **프레임 단위 이동 (Jog Control)**: 영상 편집기처럼 1프레임, 10프레임 단위로 앞뒤 이동이 가능합니다.
- **구간 통계 (Range Stats)**: 현재 보고 있는 화면 구간의 최대/평균 온도차를 자동 계산합니다.
- **드래그 패닝 & 줌**: 마우스 드래그로 그래프를 좌우로 이동하거나 원하는 구간을 확대합니다.
- **임계치 라인 (Threshold)**: 계획 온도를 입력하여 관리 기준선과 실제 값을 비교할 수 있습니다.
- **인터랙티브 데이터 커서**: 마우스 이동에 따라 모든 센서(S1~S4)의 온도값과 편차(ΔT)를 실시간 툴팁으로 확인합니다.

### 실행 방법
```powershell
# 필수 라이브러리 설치 (matplotlib)
pip install matplotlib

# 플레이어 실행 (최신 Rev02)
python tools/log_player/mainrev02/log_player.py
```
## 3. Data Collector R4 WiFi (추천 - R4 WiFi 전용)
UNO R4 WiFi 보드의 mDNS(`boiler.local`) 기능을 활용하여 더욱 안정적으로 데이터를 수집하는 현대적인 수집기입니다.

### 주요 기능
- **mDNS 및 고정 IP 지원**: `boiler.local` 도메인을 사용하거나 IP가 변경되어도 자동으로 검색하여 연결합니다.
- **자동 인증**: 설정 파일(`config_r4.json`)에 저장된 비밀번호로 자동 인증을 수행합니다.
- **컬러 콘솔 로그**: 연결 상태와 저장 현황을 색상으로 구분하여 가독성을 높였습니다.
- **설정 분리**: 모든 접속 정보와 수집 옵션을 별도 JSON 파일에서 관리합니다.

### 실행 방법
1. `tools/data_collector/config_r4.json`에서 장치 정보(IP/Password)를 수정합니다.
2. 다음 명령어로 실행합니다:
```powershell
python tools/data_collector/data_collector_r4wifi.py
```

## 4. Web Log Player (추천 - R4 WiFi 전용)
설치가 필요 없는 웹 브라우저 기반의 로그 분석 도구입니다.

### 주요 기능
- **브라우저 실행**: HTML 파일을 크롬/엣지로 열기만 하면 즉시 사용 가능합니다.
- **폴더 통째로 로드**: 로그가 담긴 폴더를 선택하면 수천 개의 파일을 한 번에 분석합니다.
- **Chart.js 기반 시각화**: 부드러운 줌, 이동, 실시간 데이터 툴팁을 제공합니다.
- **통계 및 가이드**: 현재 화면 구간의 Max/Avg 온도차를 계산하고, 임계치 라인을 표시합니다.

### 실행 방법
1. `tools/log_player/web_r4wifi/log_player.html` 파일을 브라우저로 드래그하여 엽니다.
2. **[로그 폴더 선택]** 버튼을 클릭하고 로그 데이터가 저장된 폴더(`docs/logs/YYYY-MM-DD`)를 선택합니다.

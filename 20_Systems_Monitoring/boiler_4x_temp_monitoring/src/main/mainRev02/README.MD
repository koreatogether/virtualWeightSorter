mainRev01 의 main.cpp 에 이어서
blynk 앱을 통한 연결을 시도 하는 main.cpp 파일입니다.

이 단계부터는 모듈별로 파일을 분리하여 관리합니다.

### 파일 구조
- **Config.h**: 핀 번호 및 시스템 상수 설정
- **blynkpassWord.h**: Blynk ID, Auth 토큰 및 WiFi 비밀번호 등 보안 정보 관리
- **SensorManager.h/cpp**: DS18B20 센서 초기화 및 데이터 수집 로직
- **DisplayManager.h/cpp**: LCD 디스플레이 제어 및 시각화 로직
- **BlynkManager.h/cpp**: Blynk 서버 통신 전용 모듈
- **main.cpp**: 전체 시스템 오케스트레이션 및 루프 관리

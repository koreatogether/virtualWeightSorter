# Main System Revision 04

이 리비전은 **물리 버튼을 통한 화면 전환 기능**이 추가되고, 전체 코드가 클래스 단위로 모듈화된 배포용 버전입니다.

## 주요 기능
- **물리 버튼(D3)**: 버튼을 눌러 4개의 LCD 화면(온도차1, 온도차2, 전체 온도, 시스템 정보)을 순환합니다.
- **모듈화 구조**: `SensorManager`, `DisplayManager`, `WiFiManager` 클래스로 가독성과 유지보수성 향상.
- **웹 대시보드**: 로컬 네트워크 어디서든 접속하여 실시간 온도 확인 가능.

## 개발 목표 (Work-in-Progress)
- [ ] **mDNS 도입**: IP 주소 대신 `boiler.local`로 접속 가능하게 개선.
- [ ] **UI 개선**: 웹 대시보드 인터페이스 고도화. 


## 파일 구조
- `main.cpp`: 전체 루틴 및 웹 서버 요청 처리.
- `Dashboard.h`: 웹 대시보드 HTML/JS 정의.
- `WiFiManager.h/cpp`: 네트워크 연결 관리.
- `SensorManager.h/cpp`: DS18B20 4개 센서의 주소 스캔 및 온도 측정.
- `DisplayManager.h/cpp`: 20x4 LCD 인터페이스 및 상태 표시.
- `wifiCredentials.h`: 환경 맞춤형 WiFi 및 비밀번호 설정.

## 상태 표시 (LCD)
- 화면 우측 하단에 `[LOG]` 표시가 보이면 PC에서 데이터가 성공적으로 기록되고 있음을 의미합니다.

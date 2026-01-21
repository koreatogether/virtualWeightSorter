# Main System Revision 04 (WIP)

이 리비전은 차세대 기능 확장을 위한 작업 공간입니다. `mainRev03`의 안정적인 기능을 바탕으로 아래 로드맵 기능들을 구현할 예정입니다.

## 개발 목표 (Work-in-Progress)
- [ ] **mDNS 도입**: IP 주소 대신 `boiler.local`로 접속 가능하게 개선.
- [ ] **알림 시스템**: 임계 온도 초과 시 시각적/네트워크 알림 기능.
- [ ] **UI 개선**: LCD 및 웹 대시보드 인터페이스 고도화.
- [ ] **유량 센서 연동 준비**: 향후 열량 계산을 위한 기초 데이터 수집 설계.

## 파일 구조
- `main.cpp`: 전체 루틴 및 웹 서버 요청 처리.
- `Dashboard.h`: 웹 대시보드 HTML/JS 정의.
- `WiFiManager.h/cpp`: 네트워크 연결 관리.
- `SensorManager.h/cpp`: DS18B20 4개 센서의 주소 스캔 및 온도 측정.
- `DisplayManager.h/cpp`: 20x4 LCD 인터페이스 및 상태 표시.
- `wifiCredentials.h`: 환경 맞춤형 WiFi 및 비밀번호 설정.

## 상태 표시 (LCD)
- 화면 우측 하단에 `[LOG]` 표시가 보이면 PC에서 데이터가 성공적으로 기록되고 있음을 의미합니다.

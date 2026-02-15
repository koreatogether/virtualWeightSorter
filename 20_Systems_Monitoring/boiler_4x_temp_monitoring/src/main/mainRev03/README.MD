# Main System Revision 03

이 리비전은 보일러 4구 온도 모니터링 시스템의 최종 안정화 버전입니다.

## 주요 특징
1. **로컬 웹 대시보드**: 별도의 클라우드 없이 아두이노 IP로 접속하여 실시간 온도 확인 가능.
2. **PC 로깅 자동화**: PC 수집기(`data_collector.py`)와 동기화되어 `[LOG]` 상태 표시 및 7일 순환 저장 지원.
3. **비차단 방식(Non-blocking)**: 센서 측정, 화면 전환, 웹 서버 처리가 타이머를 통해 효율적으로 병렬 동작.
4. **보안**: 간이 비밀번호 및 쿠키 기반의 대시보드 접근 제어.

## 파일 구조
- `main.cpp`: 전체 루틴 및 웹 서버 요청 처리.
- `Dashboard.h`: 웹 대시보드 HTML/JS 정의.
- `WiFiManager.h/cpp`: 네트워크 연결 관리.
- `SensorManager.h/cpp`: DS18B20 4개 센서의 주소 스캔 및 온도 측정.
- `DisplayManager.h/cpp`: 20x4 LCD 인터페이스 및 상태 표시.
- `wifiCredentials.h`: 환경 맞춤형 WiFi 및 비밀번호 설정.

## 상태 표시 (LCD)
- 화면 우측 하단에 `[LOG]` 표시가 보이면 PC에서 데이터가 성공적으로 기록되고 있음을 의미합니다.

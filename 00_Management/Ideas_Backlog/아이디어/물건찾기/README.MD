# 물건찾기 (Object Finding) 프로젝트 아이디어

이 문서는 아두이노 및 ESP32 기술 스택을 활용한 '물건찾기' 시스템에 대한 브레인스토밍 결과와 고려 사항을 정리한 것입니다.

## 1. 주요 구현 방향

### A. BLE RSSI 기반 근접 추적 (Low Cost)
ESP32의 Bluetooth Low Energy(BLE) 기능을 사용하여 태그(iBeacon 등)의 신호 세기(RSSI)를 측정합니다.
- **장점**: 별도의 추가 모듈 없이 ESP32만으로 구현 가능.
- **특징**: "멀어짐/가까워짐"을 Nextion 디스플레이나 웹 대시보드에 시각화.

### B. UWB 기반 정밀 위치 추적 (High Accuracy)
DW1000/DW3000과 같은 Ultra-Wideband 모듈을 사용합니다.
- **장점**: 센티미터 단위의 정확한 위치 측정 가능.
- **고려사항**: 모듈 단가가 높고 구현 복잡도가 상승함.

### C. 액티브 알림 태그 (Active Trigger)
웹 플레이어나 전용 리모컨에서 신호를 보내면 태그에서 부저가 울리거나 LED가 깜빡입니다.
- **동작**: Deep Sleep 모드에서 주기적으로 깨어나 WiFi/BLE 명령을 확인.
- **용도**: 가방, 지갑 등 집 안에서 잃어버리기 쉬운 물건에 부착.

### D. AI 기반 시각 탐색 (Robotic Integration)
카메라가 달린 로봇 팔(ESP32-S3 Eye 등)을 활용하여 특정 사물을 인식하고 위치를 파악합니다.
- **기술**: TinyML 또는 PC 연동 이미지 처리.

## 2. 기술적 고려 사항

- **전력 관리**: 코인 셀 배터리(CR2032) 하나로 수개월 이상 동작하기 위해 **Deep Sleep** 최적화 필수.
- **메쉬 네트워크**: 집 전체를 커버하기 위해 여러 대의 ESP32를 **ESP-NOW**나 **ESP-MESH**로 연결 (Anchors).
- **소프트웨어 구조**: 
    - 기존의 **Layered Architecture** (Application, Domain, Infrastructure) 유지.
    - **Web Log Player** 구조를 재활용하여 신호 강도 변화 기록 및 시각화.

## 3. 아이디어 백로그 및 향후 과제
- [ ] 태그 식별을 위한 `registry.md` 작성
- [ ] RSSI 거리 변환 알고리즘 테스트
- [ ] 저전력 프로토타입 회로 설계
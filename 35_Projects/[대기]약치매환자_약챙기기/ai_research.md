# AI Research - Open Medication Dispenser (비상업/오픈 개발 기준)

작성일: 2026-02-16  
검증 기준일: 2026-02-16 (링크 실접속 + GitHub API/포럼 JSON 확인)

## 1) 조사 목표 재정의
- 상업 완제품 판매가 아니라, `오픈소스 기반 약 디스펜서`를 직접 개발할 때 참고할 만한 사례를 찾음.
- 디스펜서 하드웨어 중심(배출/잠금/알림) + 필요한 최소 소프트웨어(알림/로그)까지 포함.
- 해외 커뮤니티에서 실제 반응이 있는지(댓글/조회/스타 등) 확인.

## 2) 선별 기준
- `재현 가능성`: 회로/부품/펌웨어 구조가 공개되어 있는가
- `안전성`: 오배출 방지(잠금, 회전 인덱싱, 스케줄 제어) 설계가 있는가
- `커뮤니티 반응`: GitHub stars/forks, 포럼 게시글 반응
- `유지보수성`: 최근까지 업데이트 흔적이 있는가

## 3) GitHub 중심 오픈 프로젝트 (디스펜서 우선)

| 프로젝트 | 성격 | 커뮤니티 지표(확인값) | 기술 스택 | 판단 |
|---|---|---|---|---|
| julianvbw/ble-esp32-pilldispenser | ESP32 BLE 기반 디스펜서 + Flutter 앱 + 3D 설계 | `1 stars`, `0 forks` | ESP32(Arduino/C++), BLE, Flutter, Fusion360 | 하드웨어-앱 연동 구조 참고용으로 유용 |
| vimal2372/Smart-Pill-Box-For-Medicine-Reminder | Arduino 기반 스마트 약통 리마인더 | `2 stars`, `0 forks` | Arduino, RTC 기반 리마인더 | 단순 구조 MVP/학습용으로 적합 |
| Kevin-MrYe/Automatic-Prescription-Medicine-Monitoring-System | 약통 + Wi-Fi 웹 모니터링 | `1 stars`, `0 forks` | Arduino + 웹 연동 | 모니터링 구조 참고에 유리 |
| iamnijat/healsense | 복약 리마인더 앱(하드웨어 보조) | `100 stars`, `30 forks` | Flutter(Dart), 로컬 데이터/리마인더 아키텍처 | 하드웨어 단독보다 앱 연동 UX 강화에 매우 유용 |

참고 링크
- https://github.com/julianvbw/ble-esp32-pilldispenser
- https://github.com/vimal2372/Smart-Pill-Box-For-Medicine-Reminder
- https://github.com/Kevin-MrYe/Automatic-Prescription-Medicine-Monitoring-System
- https://github.com/iamnijat/healsense

(별/포크 수치: GitHub API 조회 기준, 2026-02-16)

## 4) 해외 커뮤니티 반응 (의견 교환량)

| 커뮤니티 | 주제 | 반응 지표(확인값) | 시사점 |
|---|---|---|---|
| Home Assistant Community | Shaztech Pill Dispenser 연동 논의 | `1 post`, `48 views` | 연동 사례는 아직 초기 단계이나 실제 사용자 요구가 존재 |
| Arduino Forum | 모터 위치 복구/인터럽트 등 디스펜서 구현 이슈 | `20 posts` | 실사용 단계에서 제어 안정성(복구/정밀도) 이슈가 핵심 |
| Hackaday | DIY Pill Dispenser 검색 허브 | 검색 결과 페이지 정상 접근 | 메이커 생태계에서 관련 제작 주제가 지속적으로 등장 |

참고 링크
- https://community.home-assistant.io/t/integration-for-shaztech-pill-dispenser/981622
- https://forum.arduino.cc/t/help-with-saving-motor-position-and-recovery-after-interruptions/1245809
- https://hackaday.com/?s=pill+dispenser

## 5) "이건 괜찮다" 추천 후보

### A안 (가장 추천): ble-esp32-pilldispenser 기반 포크 + 경량 커스터마이즈
- 이유: 디스펜서 하드웨어, BLE, 모바일 앱, 3D 파트가 한 저장소 문맥으로 연결됨.
- 적용: 한국 사용자용 `요일/시간 한글 UI`, `보호자 알림`, `미복용 재알림` 추가.
- 리스크: 원 프로젝트 커뮤니티 규모가 작아 유지보수는 직접 책임져야 함.

### B안 (빠른 MVP): Smart-Pill-Box-For-Medicine-Reminder 기반 1차 프로토타입
- 이유: 구조가 단순하여 2~4주 내 동작 시연에 유리.
- 적용: 1일 3회 알림 + 버튼 확인(복용 확인) + 로그 저장 우선.
- 리스크: 잠금/오배출 방지 구조는 별도 보강 필요.

### C안 (보조 소프트웨어): healsense 조합
- 이유: 하드웨어만으로는 복약 순응도 개선이 제한됨.
- 적용: 디스펜서 이벤트를 앱 알림/로그와 연결해 보호자 확인 흐름 구현.
- 장점: 오픈소스 앱 아키텍처를 재사용해 앱 개발 시간을 단축 가능.

## 6) 오픈 개발 시 권장 아키텍처
- 디바이스: ESP32 + RTC + 스텝모터(회전식 슬롯) + 센서(배출 확인)
- 펌웨어: 스케줄 엔진(로컬 우선), 오프라인 동작 보장, 수동 배출 제한 로직
- 연동: BLE(초기) → MQTT/Wi-Fi API(확장)
- 앱/웹: 복용 스케줄 설정, 미복용 알림, 이벤트 로그(보호자 공유)
- 안전: "한 번에 1회차만" 접근 가능하도록 물리 잠금/인덱싱 설계

## 7) 주의사항 (한국 적용)
- 의료기기 해당 여부는 기능 범위(치료/진단 주장 여부)에 따라 달라질 수 있어 문구/기능 정의를 보수적으로 가져가야 함.
- 초기 단계에서는 `복약 보조기기` 포지셔닝으로 시작하고, 임상적 효능 주장 문구는 피하는 것이 안전.

## 8) 다음 실행 제안
1. `ble-esp32-pilldispenser`와 `Smart-Pill-Box-For-Medicine-Reminder`를 포크해 공통 인터페이스(스케줄/배출/로그) 추출
2. 2주 MVP 목표: "1일 3회 자동/반자동 배출 + 미복용 2회 재알림 + 보호자 알림"
3. 실제 사용자 인터뷰 10건 수집 후 알림 강도/잠금 구조 우선순위 확정

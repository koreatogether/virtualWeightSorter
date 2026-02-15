# AI Research - Open Medication Dispenser (비상업/오픈 개발 기준)

작성일: 2026-02-15

## 1) 조사 목표 재정의
- 상업 완제품 판매가 아니라, `오픈소스 기반 약 디스펜서`를 직접 개발할 때 참고할 만한 사례를 찾음.
- 디스펜서 하드웨어 중심(배출/잠금/알림) + 필요한 최소 소프트웨어(알림/로그)까지 포함.
- 해외 커뮤니티에서 실제 반응이 있는지(댓글/조회/스타 등) 확인.

## 2) 선별 기준
- `재현 가능성`: 회로/부품/펌웨어 구조가 공개되어 있는가
- `안전성`: 오배출 방지(잠금, 회전 인덱싱, 스케줄 제어) 설계가 있는가
- `커뮤니티 반응`: GitHub stars/forks, 포럼 조회/댓글 등
- `유지보수성`: 최근까지 업데이트 흔적이 있는가

## 3) GitHub 중심 오픈 프로젝트 (디스펜서 우선)

| 프로젝트 | 성격 | 커뮤니티 지표 | 기술 스택 | 판단 |
|---|---|---|---|---|
| TheAssemblersLtd/SmartPillbox | ESP32 기반 스마트 약통/디스펜서 계열 | GitHub API 기준 `241 stars`, `39 forks` | ESP32, 임베디드 펌웨어(C/C++), 하드웨어 조립 | 오픈 개발 시작점으로 가장 강함(레퍼런스/반응 모두 우수) |
| callmemaxi/ESP32-Pill-Dispenser | 개인 개발형 ESP32 디스펜서 | `48 stars`, `12 forks` | ESP32 + 간결한 디스펜서 로직 | 소형 MVP 만들기 좋음(복잡도 낮음) |
| rimarjoni/AutomaticPillDispenser | 아두이노/자동 약 배출 프로토타입 | `2 stars` (초기 프로젝트) | Arduino 중심 프로토타이핑 | 구조 참고용. 바로 제품화보단 학습/개조용 |
| travisvn/medtimer | 오픈소스 복약 알림 앱(디스펜서 보조) | `43 stars`, `24 forks` | Flutter 앱 + 알림/스케줄 | 하드웨어 단독보다 앱 연동 시 사용자 체감 크게 개선 |

참고 링크
- https://github.com/TheAssemblersLtd/SmartPillbox
- https://github.com/callmemaxi/ESP32-Pill-Dispenser
- https://github.com/rimarjoni/AutomaticPillDispenser
- https://github.com/travisvn/medtimer

(별/포크 수치는 GitHub API 조회 기준, 2026-02-15)

## 4) 해외 커뮤니티 반응 (의견 교환량)

| 커뮤니티 | 주제 | 반응 지표(확인값) | 시사점 |
|---|---|---|---|
| Home Assistant Community | 스마트 약 디스펜서/리마인더 구현 토론 | `11 replies`, `4328 views` | 실사용자는 단순 알림보다 자동화 연동(센서/알림/로그)을 원함 |
| Arduino Forum | Automatic Pill Box 회로/코드 질문 | `22,332 views` | 오래된 주제지만 지속 수요가 큼. 하드웨어 문제해결 니즈 높음 |
| Hackaday(기사/프로젝트 허브) | DIY Smart Pill Dispenser | 프로젝트/기사가 반복적으로 게시 | 메이커 생태계에서 디스펜서는 꾸준한 재제작 주제 |

참고 링크
- https://community.home-assistant.io/t/smart-pill-dispenser-reminder/749243
- https://forum.arduino.cc/t/automatic-pill-box/386872
- https://hackaday.com/?s=pill+dispenser

## 5) "이건 괜찮다" 추천 후보

### A안 (가장 추천): SmartPillbox 기반 포크 + 경량 커스터마이즈
- 이유: 스타/포크가 가장 높고, 단순 데모보다 시스템 형태에 가까움.
- 적용: 한국 사용자용으로 `요일/시간 한글 UI`, `보호자 알림`, `미복용 재알림` 추가.
- 리스크: 하드웨어 BOM/3D 파트 수가 늘면 제작 난이도 상승.

### B안 (빠른 MVP): ESP32-Pill-Dispenser 기반 1차 프로토타입
- 이유: 구조가 단순해 2~4주 내 동작 시연이 가능.
- 적용: 1일 3회 배출 + 부저/LED + 버튼 확인(복용 확인) 우선.
- 리스크: 잠금/오배출 방지 구조를 별도로 보강해야 함.

### C안 (보조 소프트웨어): medtimer 조합
- 이유: 하드웨어만으로는 복약 순응도 개선이 제한됨.
- 적용: 디스펜서 이벤트를 앱 알림/로그와 연결해 보호자 확인 기능 구현.

## 6) 오픈 개발 시 권장 아키텍처
- 디바이스: ESP32 + RTC + 스텝모터(회전식 슬롯) + 로드셀/리드스위치(배출 확인)
- 펌웨어: 스케줄 엔진(로컬 우선), 오프라인 동작 보장, 수동 배출 제한 로직
- 연동: MQTT 또는 BLE(초기), 이후 Wi-Fi API
- 앱/웹: 복용 스케줄 설정, 미복용 알림, 이벤트 로그(보호자 공유)
- 안전: "한 번에 1회차만" 접근 가능하도록 물리 잠금/인덱싱 설계

## 7) 주의사항 (한국 적용)
- 의료기기 해당 여부는 기능 범위(치료/진단 주장 여부)에 따라 달라질 수 있어 문구/기능 정의를 보수적으로 가져가야 함.
- 초기 단계에서는 `복약 보조기기` 포지셔닝으로 시작하고, 임상적 효능 주장 문구는 피하는 것이 안전.

## 8) 다음 실행 제안
1. SmartPillbox와 ESP32-Pill-Dispenser를 포크해 공통 인터페이스(스케줄/배출/로그) 추출
2. 2주 MVP 목표: "1일 3회 자동 배출 + 미복용 2회 재알림 + 보호자 알림"
3. 당건카페/치노사모 인터뷰 질문지로 실제 사용 시나리오 10건 수집

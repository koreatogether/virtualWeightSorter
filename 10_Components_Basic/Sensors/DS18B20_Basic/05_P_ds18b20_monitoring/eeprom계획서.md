# EEPROM 및 센서 ID/주소 매핑 개선 계획서

작성일: 2025-08-24
담당: Firmware(Arduino) · Python(UI/Backend) · QA
버전: v0.1 (초안)

## 목적
- DS18B20 센서의 논리적 센서 ID(01~08)와 물리적 주소(ROM code, 8바이트) 간 매핑의 일관성 보장
- Arduino ↔ Python 간 프로토콜에서 “ID”와 “주소”를 명확히 구분하고, 잘못된 정보 노출/사용을 제거
- EEPROM 저장 구조를 버전 관리하여 다중 센서/임계값(TH/TL) 지원 시 안정적으로 동작

## 배경 및 현재 문제
최근 로그 분석 결과:
- Python에서 센서 ID 02에 대해 TH/TL 설정 요청
- Arduino는 주소 `28FF641F43B82384`(…2384)를 대상으로 처리 시도
- 실제로는 `285882840000000E`(…000E)가 ID 02에 매핑되어야 함
- system_status 응답에서 `"sensor_addr":"02"`처럼 물리 주소 대신 ID가 들어오는 문제 확인
- 결과적으로 UI가 잘못된 매핑 정보를 받아 임계값 설정 대상이 어긋남

추정 원인:
1) EEPROM에 잘못된 ID↔주소 매핑이 저장됨(또는 이관 과정 중 오류)
2) Arduino 통신 코드에서 상태 전송 시 물리 주소 대신 ID를 내보냄
3) Python에서 선택된 센서/대상 주소 처리 로직 불일치 또는 파싱 오류

참고 명령(시리얼):
- 상태 확인: `{"type":"command","command":"get_status"}`
- 센서 목록: `{"type":"command","command":"list_sensors"}`
- 간단 ID 설정(비JSON): `SET_SENSOR_ID:<ADDR>:<ID>` 예) `SET_SENSOR_ID:285C82850000005D:02`

## 용어 정의(Contract)
- 물리 주소(Address, ROM Code): 8바이트(예: 28XXXXXXXXXXXXXX), 문자열은 대문자 16진수 16자리
- 센서 ID: 사용자/시스템이 부여한 논리 번호 01~08
- 매핑: ID → Address (1:1), Address → ID (역방향은 RAM에서 계산 가능)
- 임계값(TH/TL): DS18B20 알람 임계값. 주소(또는 ID) 단위로 저장/적용

에러 모드:
- ID에 매핑된 주소 없음(미페어링)
- 주소 포맷 오류/콜론 포함/소문자 혼용
- EEPROM 손상/버전 불일치

## 목표 상태(수행 후 Success Criteria)
- get_status에 실제 물리 주소가 정확히 포함됨(잘못된 ID 문자열 아님)
- list_sensors가 각 항목에 id, address, online/present, th/tl를 명확히 제공
- set_threshold(TH/TL) 명령이 주소 기준으로 정확히 동작
- EEPROM 초기화/이관 시 매핑이 유효하고 재부팅 후에도 유지
- Python UI에서 선택한 센서가 Arduino에서 동일한 대상 주소로 인식

## 저장소 현황 요약(관측)
- Arduino: `src/arduino/main/communication.cpp` 에서 상태 전송 로직 존재, 최근 간단명령 `SET_SENSOR_ID` 추가
- Python: TH/TL 모달/콜백, 포트/연결/진단 로직, 실시간 로그 카드 등 구현. 주소 문자열 처리에 콜론 제거 로직 추가됨
- 테스트: 시리얼/시뮬레이터 관련 테스트 스위트 다수 존재(`tests/`, `test_serial_*`, `test_json_communication.py` 등)

## 아키텍처/데이터 모델 정리
### EEPROM 스키마 제안(v2)
- Header: MAGIC 2B ("DS"), VERSION 1B (=2), CRC 2B(optional, 이후 단계)
- Slots(최대 8): [
  {
    id: u8(1~8),
    address: u8[8],
    th: i8,
    tl: i8,
    enabled: u8(0/1)
  } x N(<=8)
]
- 기타: 선택된 센서 ID, 샘플링 주기 등 메타(옵션)

마이그레이션 정책:
- v1 → v2 시 부팅 시점에서 변환 시도
- v1에 주소 미보관 시, 현재 원와이어 스캔 결과와 기존 ID 순서를 기반으로 페어링 도우미 실행(완전 자동이 불가하면 “미페어링” 상태로 두고 Python에서 설정 유도)

### 런타임(RAM) 인덱스
- id_to_addr[1..8] 캐시, addr_to_id(map) 캐시
- 부팅 시 EEPROM 읽어 재구성 → 누락 시 `None`

## 프로토콜 명세(정리/수정 제안)
- get_status 응답 예:
```
{
  "type": "status",
  "uptime_ms": 123456,
  "selected_sensor_id": 2,
  "selected_sensor_addr": "285882840000000E",
  "sensors_online": 5,
  "last_command": "set_threshold",
  "last_error": null
}
```
- list_sensors 응답 예:
```
{
  "type": "sensors",
  "items": [
    {"id":1, "address":"285C82850000005D", "present":true,  "th":30, "tl":10},
    {"id":2, "address":"285882840000000E", "present":true,  "th":32, "tl":12},
    {"id":3, "address":"28E6AA830000005A", "present":true,  "th":null, "tl":null},
    {"id":4, "address":"28E79B850000002D", "present":true,  "th":null, "tl":null},
    {"id":5, "address":"28FF641F43B82384", "present":true,  "th":null, "tl":null}
  ]
}
```
- set_threshold 요청:
```
{
  "type":"command",
  "command":"set_threshold",
  "address":"285882840000000E",
  "th": 32,
  "tl": 12
}
```
- set_sensor_id 요청(간단 명령 유지): `SET_SENSOR_ID:<ADDR>:<ID>`
  - JSON 등가안(추후): `{ "type":"command", "command":"set_sensor_id", "address":"...", "id":2 }`

## 변경 계획(Arduino)
1) sendSystemStatus() 수정
   - selected_sensor_addr 필드에 실제 주소 문자열 출력
   - 기존 sensor_addr에 ID를 넣던 로직 제거/정정
   - 주소 포맷터 함수 통일: 8바이트 → 대문자 HEX 16자리, 구분자 없음
2) list_sensors 구축 점검
   - 각 item에 id/address/present/th/tl 제공 보장
   - present는 OneWire 스캔 결과 기반
3) EEPROM 모듈 리팩터
   - 버전 필드 추가, v2 구조 적용
   - 부팅 시 마이그레이션: v1 감지 → v2 저장, 캐시 재구성
   - 쓰기 디바운스(이미 적용): 변경 후 5s 내 병합 저장
4) 명령 핸들러 보강
   - set_threshold: 주소 기준으로 탐색 후 적용, 저장
   - set_sensor_id: 입력 검증(주소 포맷/ID 범위), 매핑 갱신 및 저장
5) 진단 로그
   - 부팅 시 매핑 테이블 덤프(ID→ADDR)
   - set_* 처리 후 적용 대상과 결과 로그

## 변경 계획(Python)
1) 주소/ID 구분 강화
   - UI에서 선택은 ID를 기준으로 하되, 전송 시 항상 address 사용
   - get_status, list_sensors 파싱 시 필드명 엄격 검사
2) 유효성 확인
   - set_threshold 전: 선택 ID→address 확인 실패 시 사용자 경고 및 재동기화(list_sensors 재요청)
   - 주소 문자열 정규화(대문자 16자리, 콜론/공백 제거) 유틸 고정화
3) 상태 표시
   - 상태 패널에 selected_sensor_id와 selected_sensor_addr 모두 표시
   - 매핑 불일치 감지 시 배너/토스트 경고
4) 테스트 보강
   - 시뮬레이터를 이용한 end-to-end: set_sensor_id → list_sensors → set_threshold → get_status 검증

## 교체/재바인딩 시나리오: ID 7 센서 고장 후 새 센서(다른 주소)로 교체
상황: 1~8 센서에 ID가 이미 부여됨. ID 7 센서가 고장. 새 센서를 연결했더니 “ID 7로 사용하고자 함” (동일 ID, 물리 주소는 변경).

핵심 원칙
- ID는 논리 슬롯(1~8)이며, 해당 슬롯의 주소 필드만 새 주소로 갱신한다.
- 슬롯 단위로 TH/TL를 보관하므로, 주소 갱신 시 기존 ID 7의 TH/TL는 그대로 유지된다(마이그레이션 불필요).
- 충돌 방지: 동일 주소가 다른 슬롯에 이미 매핑되어 있으면 거절 또는 이전 슬롯 해제 후 진행.

동작 순서(권장, 수동 승인 포함)
1) 탐지
  - 부팅/주기 스캔에서 ID 7의 기존 주소가 present=false로 감지
  - 스캔 목록에 새로운 ‘미매핑 주소(unknown)’가 present=true로 등장
  - Python UI에 “ID 7 교체 후보 감지” 배너(unknown 주소 후보들 표시)
2) 사용자 승인(안전)
  - UI에서 “ID 7 교체” 선택 → 새 주소를 지정(드롭다운/자동 제안)
  - 확인 모달: "ID 7의 주소를 OLD→NEW로 변경하고 기존 센서 매핑을 해제합니다. TH/TL는 유지됩니다. 계속하시겠습니까?"
3) 재바인딩 실행
  - Python → Arduino: `SET_SENSOR_ID:<NEW_ADDR>:07` 또는 JSON `{command:"set_sensor_id", address:NEW, id:7}`
  - Arduino:
    - 입력 검증(ID 범위, 주소 포맷)
    - 충돌 검사: NEW_ADDR가 다른 슬롯에 매핑되어 있으면 오류 반환
    - 슬롯 7의 address를 NEW_ADDR로 갱신, enabled=1 유지, TH/TL 보존
    - 디바운스 후 EEPROM 저장, 로그에 OLD_ADDR→NEW_ADDR 변경 기록
4) 검증
  - Python: `list_sensors` 재요청 → id=7의 address가 NEW_ADDR인지 확인
  - `get_status`에서 `selected_sensor_id`가 7일 경우 `selected_sensor_addr`도 NEW_ADDR인지 확인
  - 필요 시 `set_threshold`로 TH/TL 재설정 정상 여부 확인

자동 바인딩(선택, 정책 기반)
- 조건: (a) ID 7의 기존 주소 present=false (b) unknown 주소가 정확히 1개만 present=true (c) 사용자 옵션 `auto_rebind_on_single_unknown=true`
- 동작: 위 수동 승인 2) 단계를 생략하고 자동으로 재바인딩 수행(로그에 “Auto-rebind executed” 기록)
- 안전: 자동 수행 시에도 Python UI에 토스트로 변경 사실 통보 및 되돌리기 버튼 제공(rollback via 이전 주소 복구는 불가하므로 해제 처리)

에지 케이스 및 충돌 처리
- 이전 센서가 나중에 다시 연결됨(OLD_ADDR present=true 복귀):
  - 현재 ID 7은 NEW_ADDR에 매핑되어 있으므로 OLD_ADDR는 “미매핑 주소”로 표시
  - UI에서 “OLD 센서를 새 ID에 등록” 또는 “제외” 선택 제공
- NEW_ADDR가 이미 다른 ID에 매핑되어 있는 경우:
  - set_sensor_id 단계에서 거절하고 충돌 ID 표시
  - 사용자는 기존 ID를 해제하거나 다른 주소를 선택
- 센서 수 다수 증가/감소 시:
  - unknown 주소 목록을 UI에 표시하고, 일괄 매핑 도우미 제공(다중 선택 → 일괄 set_sensor_id)

로그/감사
- Arduino: `REMAP id=7 old=OLD_ADDR new=NEW_ADDR` 라인 기록
- Python: 교체 작업자/시간, 이전/이후 매핑 상태, TH/TL 유지 여부 기록

테스트(교체 시나리오)
- Happy path: OLD_ADDR 미연결, NEW_ADDR 단일 발견 → 수동 승인 후 재바인딩 → list_sensors/ get_status 일치
- 충돌: NEW_ADDR가 다른 ID에 매핑 → 오류 반환 검증
- 회귀: 재부팅 후 id=7→NEW_ADDR 유지 확인
- 복귀 센서: OLD_ADDR 재등장 시 unknown으로 노출 확인

## 검증 및 테스트
- 단위
  - Arduino: 주소 포맷터, EEPROM v2 read/write, id_to_addr 캐시
  - Python: 주소 정규화, 파싱, 명령 빌더
- 통합
  - 부팅 직후 list_sensors의 주소/ID 일관성
  - set_sensor_id로 재매핑 후 재부팅해도 유지되는지 확인
  - set_threshold 대상이 정확히 적용되는지(온도 알람 레지스터 읽기 포함 가능 시)
- 수동 체크리스트(시리얼)
  1) `get_status` 수신에 selected_sensor_addr가 실제 주소인지 확인(16자리 HEX)
  2) `list_sensors`의 id=2가 `285882840000000E`인지 확인
  3) `SET_SENSOR_ID:28FF641F43B82384:05` 실행 후 list_sensors 재확인
  4) `set_threshold`(JSON)로 ID 2의 주소에 TH/TL 설정 → 응답 OK 및 EEPROM 저장 확인

## 롤백/백업 전략
- EEPROM 백업 명령 추가(선택): dump/load (Base64/HEX)
- 마이그레이션 실패 시 v1 레이아웃 유지 부팅 옵션(디버그 빌드 플래그)
- 기존 바이너리로 펌웨어 롤백 가능하도록 tag 보관

## 일정(제안)
- Day 0: 본 계획서 확정, 브랜치 생성(rePhase1/eeprom-v2)
- Day 1: Arduino 수정(상태/목록/포맷터) + 단위테스트
- Day 2: EEPROM v2 + 마이그레이션 + 디바운스 저장 재점검
- Day 3: Python 파싱/유효성/경고 UI + 통합테스트
- Day 4: QA 시나리오 실행, 회귀 테스트, 문서화 업데이트

## 수용 기준(Acceptance Criteria)
- 상태/목록 응답 필드 정확성(수동/자동 테스트 모두 PASS)
- set_threshold가 의도한 물리 주소에만 적용됨
- 재부팅 후 매핑/임계값 유지
- 시뮬레이터/실기 모두 동일하게 동작

## 작업 체크리스트
- [ ] Arduino: 주소 포맷터 통일(hex16, no colon)
- [ ] Arduino: sendSystemStatus()의 sensor_addr → selected_sensor_addr(실주소)
- [ ] Arduino: list_sensors 항목에 id/address/present/th/tl 보장
- [ ] Arduino: set_sensor_id 유효성 강화 + 저장
- [ ] Arduino: set_threshold 주소기반 적용 + 저장
- [ ] Arduino: EEPROM v2 도입 + v1→v2 마이그레이션
- [ ] Python: 주소 정규화 유틸 고정화 및 일괄 사용
- [ ] Python: 전송 시 address 사용 보장(set_threshold 등)
- [ ] Python: 불일치 감지 경고/재동기화 처리
- [ ] 통합 테스트(E2E) 추가 및 CI 연결
 - [ ] 교체 시나리오(ID 유지, 주소 변경) UI 플로우 및 펌웨어 충돌 처리 구현

## 부록: 예상 코드 터치 포인트
- Arduino: `src/arduino/main/communication.cpp`, `eeprom_*.[ch]pp`, `sensors.[ch]pp`
- Python: 통신 레이어(JSON 빌더/파서), 콜백 등록부, UI 상태 패널, 테스트(`tests/`)

---
질문/의견은 이 문서 PR 스레드에 코멘트로 남겨주세요. 다음 커밋부터 위 체크리스트를 기준으로 진행합니다.

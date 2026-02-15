# 커뮤니티 조사: 포도 중량 선별(2kg 박스, 최대 3송이 제약) 유사 메이커 활동

작성일: 2026-02-15
대상 폴더: `[대기]Weigher_Sorter/docs`

## 1) 우리가 찾는 문제 정의
- 목표 예시: `2kg(허용오차 포함)` 한 박스에 들어갈 포도 송이 조합을 자동 선택
- 제약 예시: `최대 3송이` 제한, 각 송이 무게 편차 존재
- 필요한 기술: 다중 로드셀 측정 + 조합 탐색 + 배출(솔레노이드/에어실린더)

## 2) 결론 요약
- **해외 메이커 커뮤니티에는 매우 유사한 문제(다중 버킷/다중 로드셀/목표중량 근사 조합)**가 이미 반복적으로 등장함.
- **국내는 공개 접근 가능한 카페/포럼 데이터가 제한적**이지만, 코코아팹/팹랩 네트워크에서 `로드셀-HX711` 기반 제작/질문 활동은 확인됨.
- 즉, 이 프로젝트는 "새로운 문제"라기보다 "검증된 메이커 주제를 한국 현장형으로 고도화"하는 방향이 타당함.

## 3) 해외 커뮤니티 근거 (유사도 높은 순)

### A. Arduino Forum - Multiple head weighing scale (2015)
- 링크: https://forum.arduino.cc/t/multiple-head-weighing-scale/345228
- 핵심: "12개 로드셀 버킷 중 목표중량(2.5kg)에 가장 가까운 조합 선택" 질문
- 유사점: 현재 프로젝트의 `조합형 선별기` 문제와 거의 동일한 구조
- 시사점: 커뮤니티에서 이미 "다중 버킷 + 조합 탐색"을 현실 과제로 다룸

### B. Arduino Forum - Scale with weigh checker (2023)
- 링크: https://forum.arduino.cc/t/scale-with-weigh-checker/1077428
- 핵심: 로드셀 측정값 범위에 따라 릴레이/솔레노이드로 분기 배출
- 유사점: 단순 계측이 아니라 `자동 분류 액추에이션`까지 포함
- 시사점: 당신의 프로젝트에서 "배출 제어부" 구현 참고 사례로 적합

### C. Arduino Forum - Precision check weigher (2012)
- 링크: https://forum.arduino.cc/t/precision-check-weigher/116624
- 핵심: 목표 범위 내/외를 판정해 제품을 다른 경로로 분기하는 check-weigher 요구
- 유사점: 산업 체크웨이어 축소판 구조와 동일

### D. Hackster 프로젝트(실제 제작 공개)
- Weigh Objects with an Arduino Scale: https://www.hackster.io/team-arduinotronics/arduino-scale-b821ae
- 표시 지표: 공개 페이지 기준 조회수 `98,342`
- 시사점: 로드셀+HX711 기반 제작 수요가 장기간 꾸준함

## 4) 오픈소스/기술 기반 근거 (GitHub)

### A. HX711 표준 라이브러리 생태계
- bogde/HX711: https://github.com/bogde/HX711
- RobTillaart/HX711: https://github.com/RobTillaart/HX711
- olkal/HX711_ADC: https://github.com/olkal/HX711_ADC
- 의미: 로드셀 계측 파트는 이미 성숙한 오픈소스 기반이 있어, 프로젝트 리스크는 "센서 읽기"보다 "조합/기구/제어 통합" 쪽이 큼.

### B. SparkFun OpenScale
- 링크: https://github.com/sparkfun/OpenScale
- 의미: 하드웨어/펌웨어가 공개된 계측 보드 레퍼런스. 장기 안정 계측(정적/컨베이어형) 사례로 참고 가치 있음.

## 5) 국내 커뮤니티 근거

### A. 코코아팹(Kocoafab)
- 프로젝트 허브: https://kocoafab.cc/make
- 확인 포인트: 페이지에 `총 275 개의 프로젝트` 표시
- HX711 질문글: https://kocoafab.cc/fboard/view/2137
- 의미: 로드셀/HX711의 실제 제작-질문 흐름이 존재

### B. Fab Lab Seoul / TIDE
- Fab Lab Seoul: https://fablabs.io/labs/fablabseoul
- TIDE FabLab 안내: https://tideinstitute.org/fablab/
- 의미: 국내 오프라인 메이커 네트워크(장비/교육/협업) 접근 채널로 적합

## 6) 해석: "2kg + 최대 3송이" 문제에 바로 연결하면

### 왜 가능성이 높은가
- 다중 헤드 계량기 아이디어 자체는 메이커 커뮤니티에서 오랫동안 반복 검증됨
- 오픈소스(HX711 계열)로 계측 레이어 구현 난이도가 낮아짐
- 병목은 알고리즘보다 `기구 안정성(진동/잔류물/배출 신뢰도)`과 `보정 자동화`

### 구현 권장 프레임
1. `계측층`: 다중 로드셀 안정화(필터, tare, drift 보정)
2. `선택층`: 2kg 목표 + 최대 3송이 제약의 조합 탐색
3. `실행층`: 선택된 버킷만 개방(솔레노이드/실린더)
4. `검증층`: 최종 박스 중량 재측정 후 pass/fail 로그

## 7) 커뮤니티 활용 실행안 (2주)
1. Arduino Forum 유사 스레드 패턴 기반으로 질문 템플릿 작성(센서 수, 오차, 속도, 제약 명시)
2. 코코아팹에 "HX711 다중채널 + 조합 선별" 진행 로그 게시
3. FabLab Seoul/TIDE 장비 접근을 통해 배출 메커니즘(3송이 제한 게이트) 빠른 프로토타입 제작

## 8) 조사 한계
- 네이버 카페(cafe.naver.com)는 robots 제한으로 게시글 규모/반응 지표를 자동 검증하기 어려움.
- 따라서 국내는 공개 접근 가능한 커뮤니티/메이커스페이스 중심으로 근거를 구성함.

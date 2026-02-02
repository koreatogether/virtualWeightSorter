# 진행 로그 (Progress Log)

## 목적
- 01 → 02 전환 과정에서 발생한 오류와 수정 사항을 기록
- 향후 작업 진행 상황을 수시로 기록

---

## 2026-01-30

### 01 → 02 전환 중 발생한 주요 오류

1. **중복 정의 오류 (redefinition)**
   - 증상: `prevUpdateMillis`, `setup()`, `loop()` 등이 중복 정의되어 컴파일 실패
   - 원인: 서로 다른 코드 블록이 한 파일에 병합되며 중복 선언 발생
   - 조치: 중복 코드 블록을 제거하고 단일 구조로 정리

2. **그래프 미표시 문제**
   - 증상: `01`에서는 웨이브폼 및 숫자 표시 정상, `02`에서는 표시 안 됨
   - 원인: `02`에서 `SoftwareSerial + nexInit()` 기반으로 변경되며 R4 WiFi(Serial1) 환경과 충돌
   - 조치: `01`과 동일한 **Serial1 직접 명령 방식**으로 복원

3. **화면 보호기 기능 적용 위치 문제**
   - 요구사항: 화면 보호기는 지속 동작해야 하므로 `loop()`에서 관리 필요
   - 조치: 보호기 설정을 `loop()`에서 1회 적용되도록 함수화하여 반영

4. **delay 제거 요구**
   - 요구사항: `delay()` 없이 동작하도록 리팩토링
   - 조치: `millis()` 기반 비차단 타이머 로직으로 업데이트 루프 구성

5. **그래프 표시 방식 개선 (직선/간격 요구 반영)**
   - 요구사항: 온도 샘플 간 직선으로 표시, 샘플 간 가로 간격 유지
   - 조치: 점-점 보간(선형) 로직을 적용하고 `LINE_STEPS`로 가로 폭 제어
   - 변경: 보간 단계 수를 조정하여 샘플 1개당 20칸 폭으로 표시

---

## 2026-01-31

### main001_nextion_04.ino 듀얼 스테이트 버튼 및 웨이브폼 동작 변경

1. **멀티 채널 선택 지원**
   - 증상: 기존 로직은 단일 채널만 선택 가능
   - 원인: `currentChannelMode` 기반 단일 선택 구조
   - 조치: `activeChannels[]` 기반 멀티 선택 구조로 변경, bt0~bt3 독립 토글, bt4는 전체 ON/OFF 토글
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_04.ino](src/main/mainrev07_tftlcd/main001_nextion_04.ino)

2. **채널 OFF 시 화면에서 제거**
   - 요구사항: 비활성화 시 해당 채널 그래프가 보이지 않도록 처리
   - 조치: 채널 OFF 시 `cle <waveformId>,<channel>` 호출로 해당 채널만 지움
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_04.ino](src/main/mainrev07_tftlcd/main001_nextion_04.ino)

3. **늦게 활성화된 채널 동기화**
   - 증상: 늦게 켠 채널이 과거 시점부터 그려져 시점이 뒤쳐짐
   - 조치: 채널 ON 시 `cle` 후 `lastPlotValues[ch] = -1`로 초기화하여 현재 시점부터 그리기
   - 결과: 늦게 활성화된 채널도 동일 시점 기준으로 그려짐
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_04.ino](src/main/mainrev07_tftlcd/main001_nextion_04.ino)

### main001_nextion_05.ino 버퍼 기반 동기화 추가

1. **버퍼 기반 리플레이 구조 추가**
   - 요구사항: 늦게 활성화된 채널도 다른 채널과 동일 시점부터 그리기
   - 조치: 2초 간격 샘플을 링 버퍼에 저장하고, 채널 활성화 시 버퍼를 리플레이하여 동일 시점 정렬
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_05.ino](src/main/mainrev07_tftlcd/main001_nextion_05.ino)

2. **버퍼 크기 기본값 설정**
   - 기준: Nano RAM 여유를 고려하여 120 샘플(약 4분) 기본값 적용
   - 변경: `BUFFER_LENGTH = 120` (채널 4개 기준 약 480바이트)
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_05.ino](src/main/mainrev07_tftlcd/main001_nextion_05.ino)

---

## 2026-02-01

### main001_nextion_05.ino Nano 하드웨어 시리얼 적용 및 클리어 안정화

1. **Nano 하드웨어 시리얼 통신 정리**
   - 증상: Nano에서 `Serial`과 HMI 통신이 섞이거나 설정 충돌 가능
   - 원인: HMI 초기화 속도 변경과 보드별 포트 사용 흐름이 불일치
   - 조치: `HMISerial` 기준으로 초기화 순서를 정리하고 `nexInit()` 포함, `DEBUG_ENABLE`로 디버깅을 제한
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_05.ino](src/main/mainrev07_tftlcd/main001_nextion_05.ino)

2. **모든 채널 OFF 시 재그림 후 삭제되는 현상 개선**
   - 증상: 버퍼가 많이 쌓인 상태에서 전체 OFF 시 그래프가 다시 그려졌다가 지워짐
   - 원인: 대량의 `add` 명령이 큐에 남아 `cle`보다 먼저 처리됨
   - 조치: 전체 OFF 시 `cle <waveformId>,255` 한 번으로 정리하고, 짧은 드로우 일시 정지 적용
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_05.ino](src/main/mainrev07_tftlcd/main001_nextion_05.ino)

3. **채널 ON 리플레이량 제한**
   - 증상: 장시간 버퍼 후 채널 ON 시 리플레이가 길어져 응답 지연
   - 원인: 버퍼 전체를 리플레이하여 전송량이 과도함
   - 조치: `REPLAY_MAX_SAMPLES`로 리플레이 샘플 수 상한 적용
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_05.ino](src/main/mainrev07_tftlcd/main001_nextion_05.ino)

4. **06 버전 큐 기반 로직 도입 및 보드 변경 결정**
   - 증상: Nano에서 전역 변수 메모리 사용량이 한계를 초과
   - 원인: High/Low 큐 및 문자열 버퍼로 SRAM 사용량 급증
   - 조치: 06 버전은 큐 기반 구조로 유지하고, 보드는 Mega 2560으로 변경하기로 결정
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_06.ino](src/main/mainrev07_tftlcd/main001_nextion_06.ino)

### main001_nextion_06_nano.ino 상용 표준 최적화 및 안정화 (rev1~rev5)

1. **상용 Nextion 표준 분석 및 최적화 (rev1)**
   - 요구사항: 상용 수준의 통신 효율성 확보
   - 조치: `snprintf` 기반 명령 사전 조합, `LINE_STEPS` 축소(25→8) 적용
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino)

2. **디스플레이 안정성 및 통신 신뢰성 개선 (rev2)**
   - 증상: `addt` 명령 사용 시 특정 환경에서 수직선 노이즈 및 데이터 누락 발생 (이미지 분석 결과)
   - 원인: `addt` Ready 응답 대기 실패 및 고속 전송 시 Nextion 처리 한계
   - 조치: 안정적인 `add` 명령으로 복귀하되, 전송 순서를 **채널별 → 스텝별(Lockstep)**로 변경하여 X축 동기화 기반 마련. `SEND_DELAY_MS`(1ms) 도입으로 버퍼 오버플로우 방지.
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino)

3. **X축 글로벌 동기화 및 패딩 로직 도입 (rev3)**
   - 증상: 채널을 늦게 켤 경우 X축 시작 위치가 다른 채널과 불일치
   - 원인: 리플레이 시 현재 시스템의 절대 X 위치를 고려하지 않음
   - 조치: `globalPointCount` 변수를 도입하여 시스템 시작 후 전체 진행도를 추적. 새 채널 활성화 시 현재 위치까지 **패딩(더미 데이터)**을 먼저 채워넣어 모든 채널이 동일한 오른쪽 끝에서 시작하도록 수정.
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino)

4. **전체 채널 토글 버그 수정 및 Lockstep 리플레이 (rev4)**
   - 증상: "모든채널" 버튼 토글 시 1채널 외 나머지 채널이 누락되거나 동기화 실패
   - 원인: 첫 채널 리플레이 중 `globalPointCount`가 증가하여 이후 채널들이 과도한 패딩을 추가함
   - 조치: `replayAllChannelsLockstep()` 함수를 신설하여 모든 활성 채널의 버퍼를 한 스텝씩 동시에 그려나가도록 수정 (완벽한 X축 동기화 실현).
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino)

5. **개별 채널 버퍼 기억 및 리플레이 (rev5)**
   - 요구사항: 모든채널 활성화뿐만 아니라 개별 채널 활성화 시에도 과거 데이터를 기억해서 그려주길 원함
   - 조치: `syncChannelToCurrentPosition()` 함수를 개선하여 다른 활성 채널이 없는 상태에서 개별 채널을 켜더라도 버퍼의 유효 데이터(최대 40샘플)를 리플레이하도록 수정.
   - 파일: [src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino)

## 진행 결과 요약

- **안정성**: `addt` 대신 타이밍 제어된 `add` 조합을 사용하여 전송 누락 및 노이즈 해결
- **동기화**: 글로벌 카운터 기반 패딩 및 Lockstep 리플레이로 채널 간 X축 완전 일치
- **UX**: 개별/전체 채널 토글 시 과거 데이터(최대 4분) 즉시 복구로 끊김 없는 모니터링 실현

## 2026-02-02 (작업 완료)

### main001_nextion_06_nano.ino 개별 채널 혼합 동기화 로직 완성 (rev6)

1. **개별 채널 혼합 토글 시 리플레이 누락 해결 (rev6)**
   - **증상**: 1채널 활성화 후 3채널을 추가로 켰을 때, 3채널이 패딩 없이 시작점부터 그려지는 현상.
   - **원인**: `syncChannelToCurrentPosition()`에서 첫 활성 채널의 시작 시점을 과거 리플레이 분량을 고려하지 않고 현재 시점으로 설정하여 계산 오류 발생.
   - **조치**: 첫 채널 활성화 시에도 리플레이 분량만큼 시작 시점(`channelSyncPoint`)을 과거로 소급 적용.
   - **결과**: 어떠한 채널 조합(예: 1+3, 2+4 등)에서도 모든 그래프가 동일한 X축 위에서 완벽 동기화됨.
   - **파일**: [src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_06_nano.ino)

---

### 💡 구현 방식 vs 상용/공식 가이드 비교 분석

| 구분 | Nextion 공식 가이드 | 현재 구현 방식 (Antigravity 최적화) |
| :--- | :--- | :--- |
| **동기화 전략** | 같은 시점의 데이터를 순차적으로 전송 | **글로벌 포인트 카운터** 기반의 절대 X축 추적 |
| **채널 가변성** | 가이드 없음 (단순 add 권장) | **동적 패딩(Dummy Data)** 추가를 통한 시점 일치 |
| **과거 데이터** | 유실 시 복구 방법 없음 | **링 버퍼 리플레이** 기능을 통한 데이터 유지 |
| **전송 안정성** | 구체적 가이드 부재 | **스텝별 순차 전송(Lockstep)** + 인터벌 딜레이 적용 |

> [!NOTE]
> **상용 관점 평가**: 현재 방식은 Nextion의 하드웨어적 제약을 아두이노 소프트웨어 로직으로 극복한 사례입니다. 중간에 채널을 켜도 과거 데이터가 즉시 채워지며 정렬되는 기능은 고성능 산업용 모니터링 시스템의 핵심 로직과 동일합니다.

## 다음 진행 계획

- [x] 화면 보호기 동작 테스트
- [x] 채널 간 X축 완전 동기화 검증
- [x] 개별 채널 버퍼 역사 재생 검증
- [ ] 설정 버튼 만들어서 화면 밝기 ( 1 ~ 4단계 ) 및 화면 보호기 10초 , 20초 , 30초 구현하기 및 되돌아가기 버튼 을 추가하기 
- [ ] 2P ~ 4P 버튼에 대해서 동작 후 각 페이지로 이동할 수있게 페이지마다 버튼들 추가하기 


[ ] 실제 온도 센서(DS18B20) 하드웨어 연결 및 실데이터 매핑 테스트
- [ ] 센서 오류(Disconnected) 발생 시 그래프 및 숫자 표시 예외 처리

---

## 로그 작성 규칙
- 날짜별로 추가
- 증상/원인/조치 순서로 기록
- 변경된 파일과 적용된 이유 명시

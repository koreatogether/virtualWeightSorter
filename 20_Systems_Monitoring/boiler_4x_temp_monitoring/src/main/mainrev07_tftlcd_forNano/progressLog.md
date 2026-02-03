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

---

## 2026-02-02

### 1. b4 (설정) 버튼 추가 및 메타데이터 업데이트
- **내용**: Nextion HMI에 새롭게 추가된 `b4` 버튼(설정)을 인식하도록 시스템 확장
- **변경 사항**: 
    - `metaData_button.json`: `b4` 버튼 정보(ID: 25, txt: "설정", h: 70 등) 추가 반영
- **파일**: [docs/nextionHMI/copyImageComponent/260202/button/metaData_button.json](docs/nextionHMI/copyImageComponent/260202/button/metaData_button.json)

### 2. 페이지 네비게이션 로직 개선 및 글로벌화 (rev07)
- **내용**: 어느 페이지에서든 `b0` ~ `b4` 버튼을 통해 1P~4P 및 설정 페이지로 자유롭게 이동할 수 있도록 터치 이벤트 핸들러 리팩토링
- **조치**: 
    - `PAGE_BUTTON_IDS`를 5개로 확장 (ID 25 추가)
    - `handleTouchEvent`에서 페이지 이동 로직을 최우선으로 배치하고 전역(Global)으로 동작하게 수정
    - **채널 제어 로직 보호**: `bt0` ~ `bt3` 채널 토글 및 `bt4` 모든채널 버튼 로직은 `page 0`에서만 작동하도록 제한하여 타 페이지와의 ID 충돌 방지
- **파일**: [src/main/mainrev07_tftlcd/main001_nextion_07_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_07_nano.ino)

### 4. 아두이노 코드 최적화 및 페이지 이동 로직 제거
- **내용**: 모든 페이지 네비게이션을 Nextion MCU에서 직접 처리하도록 변경함에 따라 아두이노 소스코드 효율화
- **조치**: 
    - `PAGE_BUTTON_IDS` 상수 및 관련 처리 로직 완전 삭제
    - `handleTouchEvent`에서 페이지 이동 감지 루프 제거
- **파일**: [src/main/mainrev07_tftlcd/main001_nextion_07_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_07_nano.ino)

### 5. 설정 페이지(page 4) 밝기 조절 라디오 버튼 로직 구현
- **내용**: 밝기 조절 버튼 선택 시 기존 선택된 버튼의 강조색이 꺼지지 않는 문제 해결
- **해결 방식 (Nextion MCU 직접 처리)**: 
    - 각 밝기 버튼의 `Touch Release Event`에 타 버튼의 상태를 끄는 로직(`btX.val=0`)과 자신의 상태를 켜는 로직(`btY.val=1`)을 직접 작성
    - 예: `bt10` 클릭 시 -> `bt11.val=0`, `bt12.val=0`, `bt10.val=1`, `dims=30` 실행
- **결과**: 아두이노 개입 없이 Nextion 내부에서 즉각적인 시각적 피드백과 시스템 설정 변경 완료

### 6. 컴포넌트 이름 충돌 방지 가이드 (t1 관련)
- **현상**: 아두이노가 `page 0`의 시간 표시를 위해 보내는 `t1.txt` 명령이 `page 4`의 동일 이름 컴포넌트에 영향을 주는 문제
- **해결**: Nextion Editor에서 `page 4` 이후의 컴포넌트 이름을 `t1`이 아닌 고유 이름(예: `t_title`)으로 변경하여 아두이노의 전역 명령 간섭 차단

### 7. 아두이노 코드 내 화면 보호기 관리 로직 제거
- **내용**: Nextion MCU의 내장 기능(`thsp`, `thup`)을 활용하기로 함에 따라 아두이노 측의 중복 관리 코드를 제거
- **조치**: 
    - `manageDisplaySystem()` 함수 선언, 호출부 및 정의 완전 삭제
    - 초기 설정(`thsp=60`, `thup=1`) 역시 Nextion Editor 내부 설정으로 이관 권장
- **파일**: [src/main/mainrev07_tftlcd/main001_nextion_07_nano.ino](src/main/mainrev07_tftlcd/main001_nextion_07_nano.ino)

### 9. thsp(절전 시간) 초기화 버그 해결 및 설정 유지 성공
- **증상**: `page 4`에서 절전 시간을 설정해도 메인 페이지(`page 0`)를 갔다 오면 다시 `60`초로 초기화됨.
- **원인**: `page 0`의 초기화 이벤트(`Pre/Postinitialize`)에 `thsp=60` 코드가 포함되어 있어, 메인 페이지로 이동할 때마다 설정값이 덮어씌워짐.
- **해결**: 
    - `page 0`의 이벤트를 삭제하고, 시스템이 최초 기동될 때 단 한 번만 실행되는 **`Program.s`**로 초기 설정(`thsp=60`, `thup=1`)을 이관함.
    - `page 4`의 `Postinitialize` 복구 로직과 결합하여 페이지 전환 시에도 완벽한 설정 유지 확인.

### 10. 웨이브폼 상한/하한 가이드라인(점선) 상세 구현 (2px 두께)
- **목적**: 실시간 온도 변화를 상한/하한 기준선과 시각적으로 즉시 비교 가능하도록 구현
- **구현 원리**: Nextion MCU의 하드웨어 가속(`line` 명령어)과 `Timer`를 활용하여 아두이노 통신 부하 제로화
- **상세 코드 (page 0, tm0 Timer Event)**:
    - **주기**: 500ms (`tim=500`)
    - **입력 범위**: `page 4`의 `n5`(상한), `n6`(하한) 컴포넌트 (0~100°C 범위)
    - **매핑 로직**: `pixelY = 바닥좌표(313) - (입력값 * 높이(141) / 100)`
    ```nextion
    // 1. 상한선 값 가져오기 및 범위 제한 (0-100)
    sys1=page4.n5.val
    if(sys1>100) { sys1=100 }
    
    // 픽셀 좌표 변환 (y:313~172)
    sys1=sys1*141
    sys1=sys1/100
    sys1=313-sys1
    
    // 2. 하한선 값 가져오기 및 범위 제한 (0-100)
    sys2=page4.n6.val
    if(sys2>100) { sys2=100 }
    
    // 픽셀 좌표 변환
    sys2=sys2*141
    sys2=sys2/100
    sys2=313-sys2
    
    // 3. 점선 그리기 루프 (x:8~429, 2픽셀 두께)
    for(sys0=8;sys0<429;sys0+=20)
    {
      // 상한 점선 (빨간색 63488) - 2px 두께
      line sys0,sys1,sys0+10,sys1,63488
      line sys0,sys1+1,sys0+10,sys1+1,63488
      
      // 하한 점선 (파란색 31) - 2px 두께
      line sys0,sys2,sys0+10,sys2,31
      line sys0,sys2+1,sys0+10,sys2+1,31
    }
    ```
- **특이사항**: 픽셀 단위 조절이 아닌 사용자 친화적인 **0~100도** 스케일링 적용 및 시인성 확보를 위한 **2개 라인 오버레이** 처리

## 진행 결과 요약

- **사용자 인터페이스 완성**: 모든 페이지 이동, 화면 밝기 조절, 절전 시간 설정이 아두이노 개입 없이 Nextion MCU 내부에서 완벽하게 작동 및 상태 유지.
- **최종 구조**: 아두이노는 시간 관리 및 실시간 온도 그래프 데이터 전송에 집중하는 효율적 분업 구조 확립.

## 다음 진행 계획

- [x] 설정 페이지 밝기/절전 시간 라디오 버튼 로직 완성
- [x] 페이지 전환 시 설정 상태 유지 및 복구 로직 구현
- [x] thsp 초기화 버그 수정 및 전역 설정(Program.s) 최적화
- [x] 실제 온도 센서(DS18B20) 하드웨어 연결 및 실데이터 매핑 테스트

---

### main001_nextion_08_nano_last.ino DS18B20 센서 하드웨어 통합

1.  **DS18B20 센서 및 라이브러리 연동**
    - **목표**: 가상 시뮬레이션 데이터를 실제 온도 센서 값으로 대체
    - **통합**: `DallasTemperature` 및 `OneWire` 라이브러리 적용, D2 핀을 데이터 버스로 설정
    - **최적화**: 비차단(Non-blocking) 리딩 방식을 도입하여 `sensors.getTempC()` 호출 시 루프가 멈추지 않도록 구현 (화면 멈춤 방지)

2.  **S2 미표시 및 S3=S4 값 중복 문제 해결**
    - **증상**: 프로브 상에서는 4개 센서가 다 뜨지만, 08버전 코드에서는 2번 센서 값이 0으로 나오고 3번과 4번 값이 동일하게 표시됨
    - **원인**: 
        - 기존 시뮬레이션용 제약 코드(상한선 클리핑 로직)가 잔존하여 실제 값을 덮어쌈
        - `updateWaveformData` 내의 임시 배열 없이 직접 변수 할당 과정에서 인덱스 혼선 발생
    - **조치**:
        - 시뮬레이션용 비교/제약 로직(`if t1 > t0`)을 완전히 제거하여 센서 독립성 보장
        - `readTemps[4]` 로컬 배열을 도입하여 읽기-전송 과정을 일원화
        - `initSensors()`에서 버스 스캔 안정화 시간(150ms) 추가 및 주소 누락 시 `memset`으로 명시적 초기화

3.  **센서 순서 고정 (프로브 테스트 순서 일치)**
    - **요구사항**: 보드 재부팅 시마다 순서가 바뀌지 않고, 프로브 테스트 코드와 동일한 순서(#0~#3)로 고정
    - **구현**: 인위적인 정렬 로직(내림차순 등)을 제거하고 라이브러리 기본 동작(주소 오름차순)을 그대로 사용하여 프로브 결과와 **1:1 매칭** (S1=#0 .. S4=#3)

4.  **메모리(RAM) 최적화**
    - **현황**: 라이브러리 추가로 RAM 사용량이 80% 육박
    - **조치**: `BUFFER_LENGTH`를 120에서 100으로 조정하여 RAM 점유율을 **77%** 수준으로 방어, 스택 오버플로우 방지

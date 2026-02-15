 graph TD
      %% 사용자 인터페이스 시작점
      A[사용자가 대시보드에서<br/>TH/TL 버튼 클릭] --> B[Python: control_buttons.py<br/>버튼 렌더링]

      %% 대시보드 콜백 처리
      B --> C[Python: threshold_callbacks.py<br/>toggle_threshold_modal]
      C --> D{현재 센서 ID 유효성 체크<br/>01-08 범위?}

      D -->|No| D1[에러 메시지 표시<br/>센서 ID 없거나 범위 초과]
      D -->|Yes| E[Python: _get_sensor_thresholds_from_arduino_by_id<br/>현재 TH/TL 값 실시간 조회]

      %% Arduino 통신 - 조회
      E --> F[JSON 명령 생성<br/>get_sensor_config + sensor_id]
      F --> G[시리얼 통신으로<br/>Arduino에 명령 전송]

      %% Arduino 처리 - 조회
      G --> H[Arduino: communication.cpp<br/>processSerialCommand]
      H --> I[Arduino: handleGetSensorConfigCommand<br/>센서 ID로 설정 조회]
      I --> J[Arduino: config_manager.cpp<br/>getSensorConfig by ID]

      J --> K{EEPROM에서<br/>센서 설정 발견?}
      K -->|No| K1[기본값 반환<br/>TH=30.0, TL=15.0]
      K -->|Yes| K2[저장된 TH/TL 값 반환]

      K1 --> L[JSON 응답 전송<br/>Python으로]
      K2 --> L

      %% 모달 표시
      L --> M[Python: threshold_modal.py<br/>create_threshold_sensor_info]
      M --> N[모달 표시<br/>- 센서 ID<br/>- 센서 주소<br/>- 현재 TH/TL 값]

      %% 사용자 입력
      N --> O[사용자가 새로운<br/>TH/TL 값 입력]
      O --> P[Python: _handle_threshold_validation<br/>입력값 유효성 검사]

      P --> Q{유효한 값?<br/>-55~125°C, TH>TL}
      Q -->|No| Q1[에러 메시지 표시<br/>확인 버튼 비활성화]
      Q -->|Yes| R[확인 버튼 활성화<br/>사용자가 확인 클릭]

      %% Arduino 통신 - 설정
      R --> S[Python: _execute_threshold_change<br/>ID 기반 명령 생성]
      S --> T[JSON 명령 생성<br/>set_threshold + sensor_id + th_value/tl_value]
      T --> U[시리얼 통신으로<br/>Arduino에 명령 전송]

      %% Arduino 처리 - 설정
      U --> V[Arduino: communication.cpp<br/>processSerialCommand]
      V --> W[Arduino: handleSetThresholdCommand<br/>센서 ID로 임계값 설정]
      W --> X{센서 설정이<br/>EEPROM에 존재?}

      X -->|No| X1[Arduino: DS18B20 사용자 ID 스캔<br/>자동 바인딩 시도]
      X1 --> X2{바인딩 성공?}
      X2 -->|No| X3[에러 응답:<br/>센서 ID 찾을 수 없음]
      X2 -->|Yes| Y

      X -->|Yes| Y[Arduino: config_manager.cpp<br/>setSensorThreshold by ID]

      %% EEPROM 저장
      Y --> Z[Arduino: EEPROM 설정 업데이트<br/>markConfigDirty 호출]
      Z --> AA[Arduino: processConfigAutosave<br/>디바운스 후 자동 저장]
      AA --> BB[Arduino: saveConfigToEEPROM<br/>실제 EEPROM 쓰기]

      BB --> CC{EEPROM 쓰기<br/>성공?}
      CC -->|No| CC1[에러 응답:<br/>저장 실패]
      CC -->|Yes| DD[성공 응답 전송<br/>Python으로]

      %% 결과 표시
      DD --> EE[Python: _process_threshold_response<br/>응답 처리]
      EE --> FF[성공 메시지 표시<br/>모달 닫기]

      %% 에러 경로들
      X3 --> GG[Python: 에러 메시지 표시]
      CC1 --> GG
      D1 --> HH[모달 열기 실패]
      Q1 --> II[입력값 수정 대기]

      %% 실시간 데이터 반영
      FF --> JJ[Arduino: sendMultiSensorData<br/>실시간 센서 데이터에<br/>새 TH/TL 값 포함하여 전송]
      JJ --> KK[Python: 대시보드 업데이트<br/>새로운 임계값 반영]

      %% 스타일링
      classDef userAction fill:#e1f5fe
      classDef pythonProcess fill:#f3e5f5
      classDef arduinoProcess fill:#e8f5e8
      classDef eepromProcess fill:#fff3e0
      classDef errorProcess fill:#ffebee

      class A userAction
      class B,C,E,M,N,O,P,S,EE,FF,KK pythonProcess
      class H,I,J,V,W,Y,Z,AA,BB,JJ arduinoProcess
      class Z,AA,BB eepromProcess
      class D1,X3,CC1,GG,HH,Q1 errorProcess


      이 플로우차트는 다음과 같은 주요 구성 요소들을 보여줍니다:

  주요 플로우:

  1. 사용자 인터페이스 (control_buttons.py)
    - TH/TL 버튼 렌더링
  2. Python 콜백 처리 (threshold_callbacks.py)
    - 모달 열기/닫기 로직
    - 입력값 유효성 검사
    - Arduino 통신 관리
  3. Arduino 통신 프로토콜 (communication.cpp)
    - JSON 기반 명령 처리
    - get_sensor_config / set_threshold 명령
  4. Arduino 설정 관리 (config_manager.cpp)
    - ID 기반 센서 설정 CRUD
    - EEPROM 자동 저장 시스템
  5. 실시간 데이터 반영 (sensor_manager.cpp)
    - 새로운 임계값이 센서 데이터에 실시간 반영

  핵심 특징:

  - ID 중심 아키텍처: 센서 ID (01-08)를 기반으로 한 설정 관리
  - EEPROM 수명 보호: 디바운스 메커니즘을 통한 자동 저장
  - 실시간 동기화: Arduino와 Python 간의 양방향 통신
  - 에러 처리: 각 단계별 검증 및 에러 핸들링
  - 호환성 지원: 주소 기반 방식과의 하위 호환성 유지
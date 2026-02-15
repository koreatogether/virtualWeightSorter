``` cpp
flowchart TD
    A[시작] --> B[setup 함수]
    B --> C[Serial 통신 시작]
    C --> D[initializeSensorSystem 호출]
    D --> D1[DS18B20 센서 초기화]
    D1 --> D2[센서 개수 확인]
    D2 --> D3[readSensorAddresses 호출]
    D3 --> D4[readSensorIDs 호출]
    D4 --> D5[sortSensors 호출]
    D5 --> E[initializeBLE 호출]

    E --> E1[BLE 초기화]
    E1 --> E2[로컬 이름 설정]
    E2 --> E3[특성 서비스 등록]
    E3 --> E4[초기값 설정]
    E4 --> E5[BLE 광고 시작]

    E5 --> F[loop 함수 시작]
    F --> G[updateSerial 호출]
    G --> G1{2초 경과?}
    G1 -- 예 --> G2[온도 요청]
    G2 --> G3[센서 정보 출력]
    G1 -- 아니오 --> H
    G3 --> H[updateBLETemperature 호출]

    H --> I{BLE 연결됨?}
    I -- 예 --> J[온도 요청]
    J --> K[각 센서 온도 읽기]
    K --> L[BLE 특성에 온도값 쓰기]
    I -- 아니오 --> F
    L --> F

    subgraph "센서 초기화 과정"
        D1
        D2
        D3
        D4
        D5
    end

    subgraph "BLE 초기화 과정"
        E1
        E2
        E3
        E4
        E5
    end

    subgraph "시리얼 출력 프로세스"
        G1
        G2
        G3
    end

    subgraph "BLE 업데이트 프로세스"
        I
        J
        K
        L
    end
```

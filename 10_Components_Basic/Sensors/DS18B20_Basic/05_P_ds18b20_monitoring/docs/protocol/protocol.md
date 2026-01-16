# DS18B20 Monitoring — Serial JSON Protocol

이 문서는 Arduino 펌웨어(DS18B20 모니터링 장치)와 Python 대시보드 간의 시리얼(Serial)로 주고받는 JSON 프로토콜을 정리합니다.

## 개요
- 모든 메시지는 한 줄의 JSON 문자열로 전송됩니다(엔터로 종료).
- Arduino는 최상위 `type` 필드를 보고 동작을 결정합니다.
- 주요 수신 `type` 값: `ping`, `command`.
- `command` 메시지는 내부 `command` 필드 값에 따라 다양한 동작을 수행합니다.

---

## 수신 메시지 (Dashboard → Arduino)

1) Ping

- 목적: 연결 확인
- 예시:

```
{"type":"ping"}
```

---

2) Command wrapper

- 기본 구조:

```
{"type":"command", "command":"<명령어>", ...}
```

- 지원되는 `command` 값들:

  - `get_status`
    - 예시: `{"type":"command","command":"get_status"}`
    - 동작: Arduino가 현재 시스템 상태를 `system_status` 타입으로 응답

  - `set_config`
    - 예시 (TH 설정):

```
{"type":"command","command":"set_config","config_type":"th","new_value":30.5}
```

    - 허용되는 `config_type` 및 `new_value` 타입/제약:
      - `th`: float, 범위 -55.0 .. 125.0
      - `tl`: float, 범위 -55.0 .. 125.0
      - `measurement_interval` (alias `interval`): integer(초 단위), 내부적으로 ms로 변환되어 적용. 범위: MIN_MEASUREMENT_INTERVAL_MS .. 60
      - `sensor_addr` 또는 `addr`: string (길이 16)

    - 동작: 값 검증 후 런타임에 반영, 가능한 경우 EEPROM에 저장하고 `system_status` 전송

  - `list_sensors`
    - 예시: `{"type":"command","command":"list_sensors"}`
    - 응답: `response` 타입에 `count` 및 `addresses` 필드 포함

  - `select_sensor_index`
    - 예시: `{"type":"command","command":"select_sensor_index","index":1}`
    - 필드: `index` 정수 (0 기반)
    - 동작: 인덱스 범위 체크 후 선택, 변경 시 config dirty 마킹 및 `system_status` 전송

  - `set_sensor_data`
    - 예시: `{"type":"command","command":"set_sensor_data","new_value":3}`
    - 용도: 사용자 정의 센서 ID(1..8) 설정
    - 동작: ID 유효성 후 EEPROM 저장 시도, 변경시 즉시 측정 실행

  - `force_measurement`
    - 예시: `{"type":"command","command":"force_measurement"}`
    - 동작: 즉시 온도 측정 수행

  - `commit_config`
    - 예시: `{"type":"command","command":"commit_config"}`
    - 동작: `config_dirty`이면 EEPROM에 쓰기 시도 후 응답

---

## Arduino가 전송하는 메시지 (Arduino → Dashboard)

- 공통: Arduino는 JSON을 한 줄로 직렬화해 전송(serializeJson + println)

1) `response`
- 예시:

```
{"type":"response","status":"success","message":"pong","timestamp":12345}
```
- 필드: `status` ("success" | "error"), `message` (string), `timestamp` (ms)

2) `sensor_data`
- 단일/멀티 센서용으로 사용되며 약간 다른 필드를 포함할 수 있음.
- 공통 필드:
  - `type`: "sensor_data"
  - `temperature`: float (소수 1자리 반올림)
  - `sensor_addr`: string (16자리 HEX)
  - `sensor_id`: string (예: "01".."08" 또는 "00")
  - `user_sensor_id`: int
  - `th_value`, `tl_value`: float
  - `measurement_interval`: ms (unsigned long)
  - `timestamp`: millis()

3) `system_status`
- 예시 필드: `sensor_connected`, `sensor_addr`, `user_sensor_id`, `th_value`, `tl_value`, `measurement_interval`, `last_temperature`, `uptime`, `config_version`, `config_dirty`, `eeprom_writes`, `debounce_ms`, `effective_interval_ms`, `sensor_resolution`

4) 기타: `info`, `warning`, `error`, `debug` 등은 `response` 형식을 차용해 전송

---

## 주의사항 및 구현상 발견된 점

- `measurement_interval`의 `new_value`는 초 단위로 보내야 하며 Arduino는 내부적으로 ms로 변환한다.
- `sensor_addr` 유효성은 길이 16만 검사(HEX 형식 자체는 엄격히 체크하지 않음).
- `list_sensors()` 응답 내 `addresses`는 펌웨어에서 이미 JSON 배열 문자열을 만들어 넣는 방식이라, Python 쪽에서 수신 시 추가 파싱이 필요할 수 있음(직렬화 중복 가능성).
- `handleConfigCommand()`에서 `debounce_ms`를 처리하는 블록이 현재 코드 흐름상 항상 도달하지 않을 가능성이 있어, `debounce_ms`를 원활히 사용하려면 코드 수정이 필요함.

---

## 예시 통신 시퀀스

1) 연결 확인

```
Client -> Arduino: {"type":"ping"}
Arduino -> Client: {"type":"response","status":"success","message":"pong","timestamp":...}
```

2) 측정 간격 변경(5초)

```
Client -> Arduino: {"type":"command","command":"set_config","config_type":"measurement_interval","new_value":5}
Arduino -> Client: {"type":"response","status":"success","message":"Measurement interval set to 5s and saved to EEPROM","timestamp":...}
Arduino -> Client: {"type":"system_status", ...}
```

---

문서 생성 완료. 필요하면 이 파일을 기반으로 Python 쪽 테스트 스크립트(`tests/` 또는 `tools/`)를 추가로 만들어 드리겠습니다.
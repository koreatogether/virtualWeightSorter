# Advanced PMS7003 Serial Control (Arduino R4 WiFi)

## 설계 개요
- **보드**: Arduino R4 WiFi (5 V) – 기존 Arduino Uno/MEGA 대신 사용
- **메모리**: RAM에 5 분(150 샘플) 데이터만 저장 → 약 900 byte 사용, 보드 메모리 충분
- **시리얼 속도**: PC와의 통신은 기존 115200 bps 유지
- **추가 기능**: 현재 요구사항에 따라 알람, LED, SD 카드 등은 구현하지 않음

## 목적
- **Passive Mode** 로 동작하는 PMS7003 센서를 Arduino R4 WiFi 보드에서 제어
- 2 초 간격으로 데이터를 수집하고 **5 분(150 샘플)** 정도를 RAM에 순환 저장
- PC와의 시리얼(`Serial`, 115200 bps) 로 명령을 주고 결과를 출력

## 하드웨어 연결
| PMS7003 핀 | Arduino R4 WiFi 핀 |
|------------|-------------------|
| VCC        | 5 V (전원)        |
| GND        | GND               |
| TX         | **Serial1 RX** (핀 0) |
| RX         | **Serial1 TX** (핀 1) – 명령 전송 시만 사용 (레벨 변환 필요 시 저항 분배) |

> **주의**: 센서 로직 전압은 3.3 V. Arduino R4 WiFi는 5 V 보드이므로 `RX` → 센서 `TX` 연결은 그대로 사용해도 무방하지만, `TX` → 센서 `RX` 은 레벨 시프터(또는 저항 분배) 권장.

## 사용 방법
1. 스케치를 보드에 업로드한다.
2. Arduino IDE 혹은 시리얼 모니터를 **115200 bps** 로 연다.
3. 아래 명령을 입력하면 동작한다.

| 명령 | 설명 |
|------|------|
| `GET 0` | 최신 **PM1.0** 값 출력 |
| `GET 1` | 최신 **PM2.5** 값 출력 |
| `GET 2` | 최신 **PM10** 값 출력 |
| `GET ALL` | 최신 3값을 한 줄에 출력 |
| `GET TABLE` | 5 분(150) 동안 저장된 데이터를 CSV‑like 표 형태로 출력 |
| `ANALYZE` | 현재 5 분 평균을 계산해 간단히 요약 (증감 판단은 현재 평균만 표시) |
| `RESET` | 저장된 모든 데이터를 0 으로 초기화 |

## 메모리 사용량
- `uint16_t pmHistory[3][150]` → 3 × 150 × 2 byte = 900 byte
- Arduino R4 WiFi(256 KB SRAM) 에 충분히 들어갑니다.

## 제한 사항
- `ANALYZE` 는 현재 구간 평균만 보여주며, 이전 구간과의 비교는 메모리 절약을 위해 구현되지 않았습니다. 필요 시 순환 버퍼를 두 개 사용해 확장할 수 있습니다.
- 추가 기능(알람, LED, SD 카드 등)은 구현되지 않았습니다.

## 참고 문서
- [docs/pms7003/pms7003_communication.md](file:///E:/arduino/01_project/5_%EA%B3%B5%EA%B8%B0%EC%A7%88_%EB%94%B0%EB%9D%BC%ED%95%98%EA%B8%B0/%EA%B3%B5%EA%B8%B0%EC%A7%88_pms7003_dht22/docs/pms7003/pms7003_communication.md) – 통신 프로토콜 상세
- [docs/pms7003/pms7003_attentions.md](file:///E:/arduino/01_project/5_%EA%B3%B5%EA%B8%B0%EC%A7%88_%EB%94%B0%EB%9D%BC%ED%95%98%EA%B8%B0/%EA%B3%B5%EA%B8%B0%EC%A7%88_pms7003_dht22/docs/pms7003/pms7003_attentions.md) – 회로·설치 주의사항

---

**Enjoy!** 🎉
Feel free to modify the command set or storage size to suit your own project.

 추천 저장소 (요약)
slavikb/BaiMon — ESP8266 기반 Vaillant E‑Bus 보일러 모니터 (온도·압력 데이터 게시).
https://github.com/slavikb/BaiMon 🔧
fredlcore/BSB-LAN — BSB(LPB) 보일러 시스템 버스용 모니터/제어 (Home Assistant 연동 사례).
https://github.com/fredlcore/BSB-LAN 🔧
diving91/Oil-Boiler-Monitoring — MySensors 스케치로 오일 보일러 수온 모니터링(DS18B20 등).
https://github.com/diving91/Oil-Boiler-Monitoring 💡
mdecoleman/boiler-temperature-monitor — Raspberry Pi Pico W + 온도센서 3개로 구형 보일러 디지털 표시 프로젝트.
https://github.com/mdecoleman/boiler-temperature-monitor 🔧
DimonSmart/BoilerMonitor — Arduino 기반 별장(다차) 보일러 온도 모니터링 예제.
https://github.com/DimonSmart/BoilerMonitor 💡
kszczupak/TemperatureReader — Raspberry Pi로 중앙 난방 보일러 온도 읽기/로깅.
https://github.com/kszczupak/TemperatureReader 🔧
HenryColdCZ/esphome-boiler-control — ESPHome + DS18B20를 이용한 보일러 제어/모니터링 구성.
https://github.com/HenryColdCZ/esphome-boiler-control 💡
nrallakis/wp-temperatures — 원격 센서에서 보일러 온도 데이터 수집 및 플롯(WordPress 플러그인).
https://github.com/nrallakis/wp-temperatures 🔍
dnekoz/WaterPressureSensor — Viessmann용 보일러 압력 및 온도 센서 프로젝트.
https://github.com/dnekoz/WaterPressureSensor 🔧
Majr98/pelletmon — PELLET 보일러 CANbus 모니터링(IoT 연동) 사례.
https://github.com/Majr98/pelletmon 💡


위 링크 중 선별 링크

https://github.com/mdecoleman/boiler-temperature-monitor

Raspberry Pi Pico W와 온도 센서 3개(DS18B20)를 사용하여 구형 보일러의 온도를 디지털로 표시하는 프로젝트입니다. 이 프로젝트는 보일러의 현재 온도, 설정 온도, 외부 온도를 모니터링할 수 있도록 설계되었습니다. Pico W의 Wi-Fi 기능을 활용하여 데이터를 원격으로 확인할 수도 있습니다. 프로젝트에는 필요한 하드웨어 구성, 소프트웨어 설치 및 설정 방법이 자세히 설명되어 있어 DIY 애호가들에게 유용합니다.

-> 위 링크가 최신이고 아두이노 보드용으로 변환해도 될 정도로 호환성은 높을 것으로 판단 됩니다.


위 저장소 분석 

간단히 요약하자면, 이 저장소는 **라즈베리 파이 피코 W(Raspberry Pi Pico W)**를 활용하여 구형 보일러의 상태를 실시간으로 모니터링하고 웹 대시보드로 시각화하는 프로젝트입니다.

질문하셨던 **ESP32 vs Pico W**, **열량 계산**, **DS18B20 센서** 등의 키워드가 모두 관통되는 매우 적절한 참고 사례입니다. 주요 특징을 분석해 드립니다.

---

### 1. 주요 하드웨어 구성

* **보드:** **Raspberry Pi Pico W** (Wi-Fi 기능 활용)
* **센서:** **DS18B20** 디지털 온도 센서 3개
* 보통 **출수(Flow)**, **입수(Return)**, **온수(Hot Water)** 파이프에 각각 부착합니다.


* **디스플레이:** **SSD1306 OLED** (128x64 I2C 방식)
* 질문하신 16x2 LCD 대신 더 작고 그래픽 표현이 가능한 OLED를 선택했습니다. (소비 전류는 백라이트 LCD보다 훨씬 적습니다.)



### 2. 소프트웨어 및 기능 특징 (MicroPython 기반)

* **데이터 시각화:** 보드 내부에 작은 웹 서버를 띄워 스마트폰이나 PC 브라우저에서 실시간 온도 그래프를 볼 수 있게 합니다.
* **InfluxDB 연동:** 데이터를 외부 시계열 데이터베이스(InfluxDB)로 보내 장기적인 통계를 낼 수 있도록 설계되었습니다.
* **설치 방식:** 보일러 배관 표면에 센서를 테이프나 타이로 고정하고 단열재로 감싸는 방식을 제안합니다. (앞서 말씀드린 DS18B20 프로브 형태가 사용됩니다.)

### 3. 질문하신 내용과의 연결점 (분석 의견)

#### ① 열량 계산 기능의 부재

이 코드는 현재 **'온도 모니터링'**에 집중되어 있습니다. 즉, 유량계(Flow Meter) 데이터가 없기 때문에 **실제 소모된 열량(kWh)은 계산하지 못하고 온도 변화 추이만 보여줍니다.** 열량계를 직접 구축하시려면 이 코드에 유량 펄스를 카운트하는 로직과 앞서 언급한 물리 공식()을 추가해야 합니다.

#### ② 왜 Pico W를 썼을까?

작성자(mdecoleman)는 MicroPython의 간결함과 Pico W의 안정적인 Wi-Fi 스택을 선호한 것으로 보입니다. 만약 이를 **ESP32**로 옮긴다면 훨씬 저렴하게 구현 가능하며, 아두이노 라이브러리를 통해 더 화려한 대시보드를 만들 수도 있습니다.

#### ③ 전력 소모 (CR 배터리 가능 여부)

이 프로젝트는 웹 서버를 상시 가동하고 OLED를 켜두기 때문에 **CR2032 같은 코인 배터리로는 며칠 버티지 못합니다.** 반드시 마이크로 5핀 USB 전원을 연결해야 하는 구조입니다. (배터리 구동을 원하신다면 Nordic 보드로 옮기고 화면을 E-Ink로 바꿔야 합니다.)

### 4. 이 저장소를 참고할 때의 팁

* **배관 부착:** DS18B20 프로브를 배관에 붙일 때 **서멀 구리스**를 살짝 바르고 금속 테이프로 감싸면 반응 속도와 정확도가 비약적으로 올라갑니다.
* **코드 활용:** `main.py` 파일에서 온도 데이터를 읽어오는 `ds18x20` 라이브러리 활용법을 참고하시면 좋습니다.

**이 프로젝트를 기반으로 유량계 로직을 추가하여 '진짜 열량계'로 업그레이드해 보시는 건 어떨까요?** 직접 구축하신다면 ESP32 버전으로 포팅하는 것도 좋은 공부가 될 것입니다.
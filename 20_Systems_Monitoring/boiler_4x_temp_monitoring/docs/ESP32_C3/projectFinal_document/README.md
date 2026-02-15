# Boiler 4x Temp Monitoring System (Rev08 - ESP32-C3 Edition)

## 📌 Project Overview
**Boiler 4x Temp Monitoring System**은 보일러 및 난방 시스템의 핵심 포인트 4곳(버너 입/출, 버퍼 상/하)의 온도를 실시간으로 정밀 모니터링하기 위한 고성능 임베디드 솔루션입니다.

기존 Arduino UNO R4 WiFi 버전에서 **Seeed XIAO ESP32-C3**로 성공적으로 마이그레이션되었으며, **비차단(Non-blocking) 아키텍처**와 **고성능 데이터 분석기(Python Integrated Analyzer)**를 통해 24/7 안정적인 운영을 보장합니다.

![System Architecture](https://via.placeholder.com/800x400?text=System+Architecture+Placeholder)

---

## ✨ Key Features

### 1. 🌡️ Real-time Monitoring
*   **4-Channel Precision**: DS18B20 센서 4개를 통해 0.0625°C 해상도로 온도 측정.
*   **Zero-Delay**: 비차단(Async) 센서 읽기 로직을 적용하여, 온도 측정 중에도 웹 서버와 HMI가 멈추지 않고 즉시 반응.

### 2. 🖥️ HMI & Web Dashboard
*   **Nextion Display**: 직관적인 터치 인터페이스로 현장에서 즉시 온도 확인 및 그래프 분석 가능.
*   **Responsive Web**: PC/Mobile 어디서든 접속 가능한 웹 대시보드 제공 (Chart.js 기반 실시간 그래프).

### 3. 🚀 High-Performance Analyzer
*   **Integrated Python Server**: 데이터 수집과 웹 서빙을 하나의 프로세스로 통합.
*   **Optimized Storage**: 일일 1.8만 개 이상의 데이터를 단일 **JSONL** 파일로 관리하여 로딩 속도 극대화.
*   **Smart Downsampling**: 대용량 데이터를 브라우저 부하 없이 부드럽게 렌더링.

---

## 🛠️ Tech Stack

| Category | Technology |
| :--- | :--- |
| **MCU** | Seeed XIAO ESP32-C3 (RISC-V) |
| **Sensors** | DS18B20 (OneWire) |
| **Display** | Nextion HMI (Enhanced Series) |
| **Firmware** | Arduino C++ (PlatformIO) |
| **Backend** | Python (http.server + Custom Logic) |
| **Frontend** | HTML5, Chart.js, CSS3 |

---

## 🚀 Quick Start

### 1. Hardware Setup
1.  **전원 연결**: XIAO ESP32-C3에 USB-C 전원을 공급합니다. (권장: 5V 1A 이상)
2.  **센서 연결**: DS18B20 센서 4개를 D2 핀에 연결합니다. (4.7kΩ 풀업 저항 필수)
3.  **디스플레이**: Nextion HMI를 D6(TX), D7(RX)에 연결합니다.

### 2. Software Run
프로젝트 루트에서 제공되는 배치 파일을 실행하면 자동으로 분석 서버와 브라우저가 열립니다.

```bash
run_analyzer.bat
```

*   **대시보드 접속**: `http://localhost:8080`
*   **기기 접속**: `http://192.168.0.150` (또는 설정된 IP)

---

## 📂 Documentation

더 자세한 기술 내용과 가이드는 다음 문서들을 참고하세요.

*   [🏗️ Architecture Guide](ARCHITECTURE.md): 시스템 설계 원칙과 데이터 흐름.
*   [🔌 Hardware Guide](HARDWARE_GUIDE.md): 회로도, 핀맵, 전력 분석.
*   [📊 Analyzer Guide](ANALYZER_GUIDE.md): 통합 분석 도구 사용법 및 성능 보고서.

---

## 📜 License
This project is licensed under the MIT License - see the LICENSE file for details.

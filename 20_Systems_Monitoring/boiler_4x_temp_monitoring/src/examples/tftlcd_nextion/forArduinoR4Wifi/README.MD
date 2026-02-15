# Arduino R4 WiFi Nextion LCD Examples

이 폴더는 **Arduino R4 WiFi**와 **Nextion TFT LCD(NX4832T035)** 간의 연동을 위한 최적화된 예제 코드들을 포함하고 있습니다. 모든 코드는 R4 WiFi의 하드웨어 시리얼(`Serial1`)과 고속 통신(115200bps)을 활용하도록 작성되었습니다.

## 📂 포함된 예제 파일 설명

### 1. [R4_Hardware_Connent_Test.ino](R4_Hardware_Connent_Test.ino)
- **목적**: 하드웨어 배선(RX/TX) 및 통신 정상 여부 확인 (가장 기초)
- **동작**: Nextion 화면의 밝기를 10%와 100% 사이에서 1초 간격으로 반복 변경합니다.
- **체크포인트**: 화면이 깜빡인다면 아두이노와 넥션 간의 연결이 물리적으로 완벽한 상태입니다.

### 2. [nextion_waveform_r4_wifi.ino](nextion_waveform_r4_wifi.ino)
- **목적**: 실시간 온도 모니터링 대시보드 구현 예제
- **주요 기능**:
    - 4채널 온도 데이터를 Waveform 컴포넌트에 매핑
    - `Xfloat` 컴포넌트를 사용하여 온도 수치 표시
    - 선형 보간(Linear Interpolation)을 통한 부드러운 그래프 드로잉
    - 115200bps 고속 통신 전환 포함

### 3. [nextion_7pixelLine_waveform_r4_wifi.ino](nextion_7pixelLine_waveform_r4_wifi.ino)
- **목적**: "두꺼운 선" 그래픽 효과 구현
- **동작**: 7개의 데이터 채널에 미세한 오프셋을 주어 전송함으로써, 0.1mm 단위의 얇은 선이 아닌 시각적으로 강조된 두꺼운 파형(7픽셀 두께)을 그립니다.

### 4. [nextion_18channel_waveform_r4_wifi.ino](nextion_18channel_waveform_r4_wifi.ino)
- **목적**: 통신 부하 스트레스 테스트 및 다채널 데이터 핸들링
- **주요 기능**:
    - 18개 채널의 데이터를 동시에 전송하여 R4 WiFi의 성능 확인
    - **Smart Bridge 모드**: D4 핀을 GND에 연결하고 부팅 시, 아두이노를 거쳐 Nextion Editor에서 직접 업로드가 가능한 브릿지 모드 진입

### 5. [nextion_4channel_max_fps_r4_wifi.ino](nextion_4channel_max_fps_r4_wifi.ino)
- **목적**: 물리적 한계 FPS(Frame Rate) 달성 테스트
- **주요 기능**:
    - 데이터 패킷 최소화(4채널)를 통해 **약 125 FPS** 달성 목표 (8ms 주기 갱신)
    - 115200bps에서 가장 부드러운 움직임을 보여주는 최적화 코드
    - Smart Bridge 모드 포함

---

## 🔌 하드웨어 연결 (Common Wiring)

| Arduino R4 WiFi | Nextion Pin | Color  |
| :-------------- | :---------- | :----- |
| **5V**          | 5V          | Red    |
| **GND**         | GND         | Black  |
| **Pin 0 (RX1)** | TX          | Blue   |
| **Pin 1 (TX1)** | RX          | Yellow |

> **주의**: Arduino R4 WiFi는 PC와의 통신(`Serial`)과 핀 0/1 통신(`Serial1`)이 독립적이므로, 업로드 시 넥션 배선을 분리할 필요가 없습니다.

## 🚀 시작 가이드
1. `R4_Hardware_Connent_Test.ino`를 먼저 업로드하여 배선을 확인하세요.
2. Nextion Editor에서 컴포넌트 ID(특히 Waveform ID)가 코드와 일치하는지 확인하세요. (기본 예제 ID: 11)
3. 고속 통신이 필요한 예제의 경우, 코드가 실행되면 Nextion의 `baud`도 함께 변경되므로 주의하세요.

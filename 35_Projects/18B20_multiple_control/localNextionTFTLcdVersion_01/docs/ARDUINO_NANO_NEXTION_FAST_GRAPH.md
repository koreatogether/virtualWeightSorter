# Arduino Nano + Nextion 하드웨어 시리얼 고속 그래프 그리기 가이드

이 문서는 Arduino Nano(또는 Uno)와 같이 하드웨어 시리얼 포트가 하나뿐인 보드에서 Nextion 디스플레이에 그래프(Waveform)를 **빠르고 부드럽게** 그리기 위한 설정 방법을 설명합니다.

## 1. 개요 및 문제점 해결

Arduino Nano는 하드웨어 시리얼 포트가 `Serial` (Pin 0, 1) 하나뿐이며, 이는 USB 통신(업로드 및 시리얼 모니터)과 공유됩니다. 
일반적으로 `SoftwareSerial`을 사용하지만, 이는 속도가 느려(9600bps 권장) 많은 양의 데이터를 전송해야 하는 그래프(Waveform) 기능에서는 그리는 속도가 느려지는 단점이 있습니다.

**해결책:**
1. Nextion 라이브러리가 **하드웨어 시리얼(`Serial`)**을 직접 제어하도록 설정합니다 (SoftwareSerial 미사용).
2. 통신 속도를 기본 **9600bps에서 115200bps**로 높여 데이터 전송 병목을 해결합니다.

---

## 2. 라이브러리 설정 수정 (`NexConfig.h`)

Nextion 공식 라이브러리는 기본적으로 Mega2560의 `Serial2`를 사용하도록 설정되어 있습니다. Nano에서 하드웨어 시리얼을 쓰기 위해 설정 파일을 수정해야 합니다.

**파일 위치:** `docs/nextionLib/NexConfig.h` (또는 프로젝트의 라이브러리 폴더 내 `NexConfig.h`)

**수정 내용:**

```cpp
// 1. 디버그 기능 비활성화 (Serial 포트를 Nextion과 공유하므로 충돌 방지)
//#define DEBUG_SERIAL_ENABLE  <-- 주석 처리

// 2. 디버그 포트 설정 (사용 안 함)
#define dbSerial Serial

// 3. Nextion 통신 포트를 'Serial'로 변경 (Nano의 하드웨어 시리얼)
#define nexSerial Serial       <-- Serial2에서 Serial로 변경
```

---

## 3. 아두이노 스케치 코드 작성 (속도 향상 로직)

Nextion은 기본적으로 9600bps로 통신을 시작하므로, 코드 상에서 **115200bps로 변경하는 명령**을 보내야 합니다.

**핵심 코드 로직:**

```cpp
#include <Nextion.h>

// Nano는 Serial1이 없으므로 Serial 사용
#define HMISerial Serial

void setup() {
    // 1. 라이브러리 초기화 (기본 9600bps로 시작)
    nexInit();

    // 2. [속도 향상] Nextion에게 통신 속도 변경 명령 전송 (baud=115200)
    // SoftwareSerial에서는 115200이 불안정하지만, HardwareSerial은 안정적임.
    // 기존 9600bps 대비 약 12배 빠른 전송이 가능.
    HMISerial.print("baud=115200");
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    HMISerial.write(0xFF);
    
    // 3. 디스플레이가 설정을 변경할 시간을 줌
    delay(100); 
    
    // 4. 아두이노측 시리얼 포트 속도 재설정 (Hardware Serial)
    HMISerial.begin(115200); 

    // 5. 이후 그래프 초기화 및 데이터 전송 시작
    HMISerial.print("cle ... ");
    // ...
}
```

---

## 4. 하드웨어 연결 및 업로드 주의사항

Arduino Nano의 `Serial`(Pin 0, 1)은 USB 업로드 라인과 공유되므로 아래 절차를 반드시 지켜야 합니다.

### 연결 (Wiring)
*   **Nano TX (Pin 1)** <--> **Nextion RX**
*   **Nano RX (Pin 0)** <--> **Nextion TX**
*   **GND** <--> **GND** (공통 접지 필수)
*   **5V** <--> **5V**

### 업로드 절차 (중요)
1.  **케이블 분리:** 코드 업로드 전, Nano의 **RX(0번), TX(1번) 핀에 연결된 Nextion 선을 뽑습니다.** (안 뽑으면 업로드 에러 `avrdude: stk500_getsync()` 발생)
2.  **업로드:** 아두이노 IDE/PlatformIO에서 코드를 업로드합니다.
3.  **재연결:** 업로드가 완료되면 USB 케이블은 전원 공급용으로 두고, **Nextion 선을 다시 연결**합니다.
4.  **확인:** Nextion 화면에 그래프가 빠르게 그려지는지 확인합니다.

### 문제 해결 (Troubleshooting)
*   **반응이 없을 때:** Nextion 디스플레이의 전원을 껐다 켜보세요 (Power Cycle). 이전에 변경된 baud rate 설정이 꼬였을 수 있습니다.
*   **시리얼 모니터:** 시리얼 모니터를 켜도 디버깅 메시지를 볼 수 없습니다 (Nextion 패킷만 깨져서 보임). 디버깅이 필요하면 LED 깜빡임 등을 이용해야 합니다.

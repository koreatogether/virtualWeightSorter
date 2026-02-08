# Hardware Configuration Guide

## 1. Wiring Diagram & Pin Map

### 1.1 Pin Assignments (Seeed XIAO ESP32-C3)

| Pin (XIAO) | GPIO | Function | Device Connection | Note |
| :--- | :--- | :--- | :--- | :--- |
| **D2** | GPIO 4 | OneWire | DS18B20 (Data) | 4.7kΩ Pull-up 필수 |
| **D6** | GPIO 21 | UART TX | Nextion (RX) | 레벨 시프터 권장 (3.3V -> 5V) |
| **D7** | GPIO 20 | UART RX | Nextion (TX) | 전압 분배 권장 (5V -> 3.3V) |
| **5V** | VBUS | Power In | USB Power | Nextion 전원 공급 (병렬) |
| **GND** | GND | Ground | Common Ground | 모든 장치 공통 접지 |

### 1.2 Connection Schematic (Conceptual)

```text
[XIAO ESP32-C3]        [DS18B20 x4]        [Nextion HMI]
    3.3V ---------------- VCC
    GND  ---------------- GND
    D2   ---------------- DATA (w/ 4.7kΩ Pull-up)

    5V   ------------------------------------- 5V (VCC)
    GND  ------------------------------------- GND
    D6 (TX) ------------> [Level Shifter] ---> RX
    D7 (RX) <------------ [Voltage Div] <----- TX
```

---

## 2. Power Budget Analysis

### 2.1 Power Consumption Estimates
시스템의 안정적인 동작을 위해 전력 소비량을 정확히 파악해야 합니다.

| Component | Voltage | Current (Max) | Note |
| :--- | :--- | :--- | :--- |
| **XIAO ESP32-C3** | 3.3V (LDO) | ~350mA | WiFi TX Peak 시 순간 전류 급증 |
| **Nextion LCD** | 5V | ~145mA | 화면 밝기 100% 기준 |
| **DS18B20 (x4)** | 3.3V | ~6mA | 변환 시 각 1.5mA 소모 |
| **Total** | **5V Input** | **~500mA + α** | **최소 1A 어댑터 권장** |

### ⚠️ Warning: Brown-out
노트북 USB 포트(500mA 제한) 사용 시, WiFi 통신과 Nextion 화면 갱신이 동시에 일어나면 전압 강하(Brown-out)로 인해 MCU가 리셋될 수 있습니다. **반드시 5V 1A 이상의 독립 전원 어댑터를 사용하세요.**

---

## 3. Logic Level Compatibility

XIAO ESP32-C3는 **3.3V Logic** 장치입니다. Nextion 디스플레이는 **5V Logic**을 사용하므로 주의가 필요합니다.

*   **Nextion TX (5V) -> XIAO RX (3.3V)**:
    *   **위험**: 5V 신호가 3.3V 핀에 직접 들어가면 MCU가 손상될 수 있습니다.
    *   **해결**: 저항 분배(Voltage Divider) 회로를 구성하여 전압을 낮춰야 합니다.
        *   R1 (Signal -> Pin): 2.2kΩ
        *   R2 (Pin -> GND): 3.3kΩ
*   **XIAO TX (3.3V) -> Nextion RX (5V)**:
    *   대부분의 경우 3.3V 신호를 High로 인식하므로 직결해도 동작하지만, 통신 안정성을 위해 레벨 시프터 사용을 권장합니다.

---

## 4. Components List (BOM)

*   **MCU**: Seeed Studio XIAO ESP32-C3
*   **Sensors**: Maxim DS18B20 Waterproof Probe (x4)
*   **Display**: Nextion Enhanced Series 3.5" (NX4832K035)
*   **Resistors**: 4.7kΩ (1ea), 2.2kΩ (1ea), 3.3kΩ (1ea)
*   **Power**: USB-C Cable & 5V 1A Adapter

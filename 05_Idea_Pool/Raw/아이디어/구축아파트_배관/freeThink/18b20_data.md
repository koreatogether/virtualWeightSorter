# How the DS18B20 Waterproof Digital Temperature Sensor Works: Complete Guide for Engineers & IoT Designers

## Introduction to the DS18B20 Waterproof Digital Temperature Sensor

Temperature measurement is widely required in modern electronic devices, including IoT devices, industrial automation systems, and many environmental monitoring devices. These devices required a precise and accurate measurement of temperature, and the DS18B20 waterproof digital temperature sensor is among the best digital temperature sensors available to date. The DS18B20 temperature sensor gained popularity due to its unique one-wire communication interface and ease of integration with any electronic application. Other available temperature sensors mostly require noise filtering, ADC calibration. Whereas the DS18B20 temperature sensor outputs already calibrated temperature data, which simplifies the design and provides ease of use to designers.

DS18B20 is a waterproof and digital temperature sensor that further increases its use in vast applications, including outdoor environments, HVAC systems, liquid temperature monitoring, food processing equipment, smart agriculture, and industrial systems. This temperature sensor follows the One-Wire protocol, which allows the designers to make multiple sensors to communicate over a single data line. This feature makes it a perfect choice for embedded systems because GPIO is limited such as ESP32, STM32 microncontrollers, and Arduino.

[DS18B20 digital temperature sensor](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-maxim-integrated-ds18b20-par-t-r-8b3f32a6) comes in stainless steel to make it a waterproof temperature sensor. This article will cover its complete guide, including one-wire communication protocol, working operation, real-world applications, and interfacing with different microcontrollers.

---

## Key Features and Technical Specifications of DS18B20

The [DS18B20 stands out among digital temperature sensors](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-maxim-integrated-ds18b20-t-r-a6d4c0a5) due to its robust electrical design, one-wire communication protocol, and industrial-grade accuracy. Understanding its core features and specifications is essential for engineers designing reliable sensing systems, especially in distributed or harsh environments.

### Key Features

#### Digital Output and No ADC required
The DS18B20 performs internal analog-to-digital conversion and transmits temperature data in digital format, eliminating the need for external ADCs and minimizing errors caused by noise or voltage variation.

#### One-Wire Communication Interface
All communication occurs over a single data line, which dramatically simplifies wiring. Multiple sensors can share the same bus, each identified by a unique 64-bit ROM code.

#### Wide Operating Temperature Range
DS18B20 temperature sensor has a wide operating temperature range, i.e., –55°C to +125°C. Therefore, it is suitable for refrigeration, industrial processes, defense, and avionics equipment, and outdoor deployments.

#### Programmable Resolution
The sensor supports 9-bit to 12-bit resolution, allowing designers to balance precision and conversion time:
- 9-bit: 0.5°C resolution (fastest conversion)
- 10-bit: 0.25°C
- 11-bit: 0.125°C
- 12-bit: 0.0625°C (maximum resolution)

### [Technical Specifications](https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf)

| Parameter | Max Value | Description |
| :--- | :--- | :--- |
| Operating Voltage | 5.5 V | Sensor can operate between 3.0 V and 5.5 V |
| Operating Temperature Range | 125°C | Measures temperatures from –55°C to +125°C |
| Accuracy | ±0.5°C | Typical accuracy in the –10°C to +85°C range |
| Resolution | 12 bits | Programmable from 9-bit to 12-bit (0.5°C to 0.0625°C) |
| Conversion Time | 750 ms | Time to complete a temperature measurement at 12-bit resolution |
| Output Data Format | 16-bit signed | Digital temperature data, ready to read by microcontroller |
| Unique ID | 64-bit | Factory-lasered ROM code for addressing multiple sensors on a bus |
| Standby Current | 1 µA | Current drawn when idle (low-power mode) |
| Active Current | 1.5 mA | Current drawn during temperature conversion |
| Package | TO-92 | Standard sensor die package; waterproof version encased in stainless steel |
| Waterproof Probe | N/A | Stainless steel enclosure with epoxy sealing for immersion |
| Cable Length | N/A | Typical 1–5 meters, can be customized |

---

## DS18B20 Pinout and Typical Packages

Understanding the pinout and packaging options of the DS18B20 is essential for proper wiring, mounting, and integration in your IoT or industrial projects. The sensor comes in multiple forms, including the standard TO-92 package and waterproof stainless steel probes, allowing flexible deployment in diverse environments.

### [DS18B20 Pinout (TO-92 Package)](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-maxim-integrated-ds18b20-25014185)

The classic TO-92 package has three pins, and it is not waterproof, which are usually arranged as follows (flat side facing you, pins pointing down):

| Pin Number | Name | Function |
| :--- | :--- | :--- |
| 1 | GND | Ground reference; connects to system ground |
| 2 | DQ (Data) | One-Wire data line for communication; supports parasitic power mode |
| 3 | VDD | Supply voltage (3.0 V to 5.5 V); optional in parasitic mode |

A 4.7 kΩ pull-up resistor is required between the DQ line and VDD to ensure proper One-Wire communication. In parasitic mode, VDD is connected to GND, and the sensor draws power directly from the DQ line during high logic levels.

![Pinout of DS18B20 TO-92 package and stainless steel probe](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/pinout-of-DS18B20.png)

### DS18B20 Typical Packages

| Package Type | Description | Applications |
| :--- | :--- | :--- |
| TO-92 | Standard through-hole package | Prototyping, air temperature measurement |
| Waterproof Stainless Steel Probe | Sensor die inside stainless steel tubing with epoxy sealing; includes cable (1–5 m) | Industrial, outdoor, liquid temperature monitoring |
| SOT-23 Surface Mount | Miniature PCB-mount package | Compact IoT devices, embedded systems |

![TO-92 and waterproof stainless steel DS18B20 temperature sensor](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/ds18b20-to-92-PACKAGE-AND-waterproof-stainless-steel-probe.png)

#### DS18B20 Symbol, Footprint, and 3D STEP Model
For IoT and hardware design engineers, the [DS18B20 symbol](https://www.snapeda.com/parts/DS18B20/Analog%20Devices/view-part/), footprint, and 3D model are important for PCB integration.

![DS18B20 altium symbol and footprint](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/ds18b20-footprint-and-symbol.png)

- [DS18B20-altium-aymbol-and-footprint.zip](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/DS18B20-altium-aymbol-and-footprint.zip)
- [DS18B20-3DModel-STEP-altium.zip](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/DS18B20-3DModel-STEP-altium.zip)

---

## Understanding One-Wire Communication Protocol

[One wire communication protocol](https://en.wikipedia.org/wiki/1-Wire) was originally developed by Dallas Semiconductor. It is a low-speed and half-duplex serial communication system. [Half-duplex](https://en.wikipedia.org/wiki/Duplex_(telecommunications)) means data can flow in both directions, but only one at a time. The [DS18B20 temperature sensor](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-maxim-integrated-ds18b20z-b2eb9782) uses this to enable communication via a single data line plus ground.

### What is One Wire Protocol?
One wire protocol is a basic serial communication protocol mostly used in consumer electronics, IoT, and medical devices. It works on the master-slave concept where a single microcontroller (master) controls the bus and one or more slave devices (DS18B20) respond. The data line (DQ) requires an external [pull-up resistor](https://en.wikipedia.org/wiki/Pull-up_resistor).

### One Wire Bus Hardware Configuration
The 1-wire bus uses an open-drain architecture, meaning devices can only pull the bus low. This allows the bus to be "released" when not transmitting.

![one wire hardware configuration of DS18B20](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/D18B20-hardware-configuration.png)

The idle state is high, requiring a 4.7kΩ resistor. If the bus is held low for more than 480µs, all components will reset.

### One-Wire Communication and Command Operation
Access requires a three-step sequence: initialization, ROM command, and function command.

#### Step 1: Initialization
Master transmits a reset pulse (low for 480us), then releases. DS18B20 responds with a presence pulse (low for 60us).

#### Step 2: ROM Commands (Device Selection)
The master (e.g., [STM32 microcontroller](https://www.flywing-tech.com/product-detail/embedded-microcontrollers-stmicroelectronics-stm32f401cbt6-5d66a8d9)) addresses devices:
- **Read ROM [33h]**: Read 64-bit code when only one device is present.
- **Match ROM [55h]**: Address a specific device.
- **Skip ROM [CCh]**: Broadcast to all devices.
- **Search ROM [F0h] / Alarm Search [ECh]**: Device discovery.

#### Step 3: Function Command (DS18B20 Operation)
- **Convert T [44h]**: Initiate temperature measurement.
- **Read Scratchpad [BEh]**: Read temperature data, alarm thresholds, etc.
- **Write Scratchpad [4Eh]**: Write alarm values and configuration.

### DS18B20 64-Bit Lasered ROM Code
Each sensor has a unique code: 8 bits family code (0x28 for DS18B20), 48 bits serial number, and 8 bits [CRC](https://en.wikipedia.org/wiki/Cyclic_redundancy_check).

---

## DS18B20 Wiring Guide: Schematics and Cable Considerations

### Standard Power Mode Wiring
VDD to 3.3/5V, GND to Ground, DQ to GPIO. 4.7 kΩ pull-up between DQ and VDD.

![Standard power mode wiring connection of DS18B20](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/Normal-operation-schematic-of-DS18B20-with-arduino.png)

### Parasitic Power Mode Wiring
VDD connected to GND. Sensor draws power from DQ line when high.

![parasitic power mode wiring connection of DS18B20](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/parasitic-mode-power-connection-diagram-of-DS18B20-with-arduino.png)

### Multi-Sensor Wiring
Connect all VDD, GND, and DQ pins in parallel. Use a single 4.7kΩ pull-up resistor.

![Standard power mode wiring connection of multi DS18B20 sensors](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/DS18B20-temperature-sensors-connection-with-Arduino-.png)

### Cable Length and Signal Integrity
For waterproof probes, use twisted pair (DQ + GND), minimize stubs, and place pull-up near the master.

[![DS18B20+PAR digital temperature sensor](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/ds18b20par.png)](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-maxim-integrated-ds18b20-par-ba8b5973)

---

## Measuring Operation of DS18B20

DS18B20 measures temperature using a silicon-based sensing element and an [ADC](https://www.flywing-tech.com/product-detail/data-acquisition-analog-to-digital-converters-adc-texas-instruments-ads1115idgsr-eacc0961). It performs internal digitization and outputs direct temperature data.

### Temperature Conversion Process
1. Master sends reset pulse.
2. Master selects target device using ROM command.
3. Master sends **Convert T [44h]**.
4. Sensor performs ADC conversion.
5. Temperature data stored in scratchpad.

### Resolution and Conversion Time
- 9-bit: 0.5 °C (93.75 ms)
- 10-bit: 0.25 °C (187.5 ms)
- 11-bit: 0.125 °C (375 ms)
- 12-bit (default): 0.0625 °C (750 ms)

---

## DS18B20 Interfacing With Microcontrollers and Example Codes

Interfacing is simple with platforms like STM32, [ESP32](https://www.flywing-tech.com/product-detail/rf-evaluation-and-development-kits-boards-espressif-systems-esp32-c3-devkitc-02-eb7cd38e), and Arduino.

### Interfacing with Arduino
![Arduino interfacing with DS18B20](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/Normal-operation-schematic-of-DS18B20-with-arduino.png)

```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" °C");
  delay(1000);
}
```

### Interfacing with ESP32
```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.printf("Temperature: %.2f °C\n", tempC);
  delay(2000);
}
```

---

## DS18B20 Digital Temperature Sensor Simulation with Arduino

Working simulation using Proteus software.

![Proteus simulation of DS18B20 with Arduino UNO](https://www.flywing-tech.com/blog/wp-content/uploads/2025/12/proteus-simulation-of-DS18B20-temperature-sensor.png)

```cpp
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 8

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float Celsius = 0;
float Fahrenheit = 0;

void setup() {
  sensors.begin();
  Serial.begin(9600);
}

void loop() {
  sensors.requestTemperatures();
  Celsius = sensors.getTempCByIndex(0);
  Fahrenheit = sensors.toFahrenheit(Celsius);
  
  if(Celsius > -127){
    Serial.print(Celsius);
    Serial.print(" C  ");
    Serial.print(Fahrenheit);
    Serial.println(" F");
    delay(1000);
  }
}
```

---

## DS18B20 vs Alternatives (NTC, PT100, DHT22, TMP36, TMP117)

Alternatives include [NTC thermistor](https://www.flywing-tech.com/blog/thermistor-vs-rtd-key-differences-accuracy-and-applications/), PT100, DHT22, [TMP36](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-analog-devices-inc-tmp36gsz-reel7-bbade24d), and [TMP117](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-texas-instruments-tmp117aiybgr-0c639805).

| Parameter | DS18B20 | NTC Thermistor | PT100 (RTD) | DHT22 | TMP36 | TMP117 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Output** | One-Wire | Analog | Analog | Digital | Analog | I²C |
| **Accuracy** | ±0.5 °C | ±1–5 °C | ±0.1–0.3 °C | ±0.5 °C | ±1–2 °C | ±0.1 °C |
| **Range (°C)** | –55 to +125 | –40 to +125 | –200 to +850 | –40 to +80 | –40 to +125 | –55 to +150 |
| **Waterproof** | Yes | Yes | Yes | Rare | No | No |

---

## Conclusion

The DS18B20 waterproof digital temperature sensor is ideal due to its ease of integration, one-wire interface, and no requirement for ADC calibration. While alternatives like [TMP117](https://www.flywing-tech.com/product-detail/temperature-sensors-analog-and-digital-output-texas-instruments-tmp117maiybgt-6660eba6) exist for ultra-precision, the DS18B20 excels in industrial IoT, HVAC, and smart agriculture.

---

## Frequently Asked Questions (FAQ)

**How do I wire multiple DS18B20 sensors on a single data line?**
Connect all DQ pins, all GND pins, and all VDD pins together. Add one 4.7 kΩ pull-up resistor between DQ and VDD.

**How accurate is the DS18B20 sensor?**
Typical accuracy is ±0.5 °C from –10 °C to +85 °C.

**Can the DS18B20 be used in liquids?**
Yes, the waterproof stainless steel probe variant is fully submersible.

**How long can the DS18B20 One-Wire bus be?**
Practically 30–100 meters, depending on cable type and pull-up resistor. Use twisted pair cables.

**How do I improve the accuracy of DS18B20 readings?**
Use 12-bit resolution and calibrate with a known reference thermometer.

**How fast does the DS18B20 respond to temperature changes?**
9-bit conversion takes 93.75 ms; 12-bit takes 750 ms.

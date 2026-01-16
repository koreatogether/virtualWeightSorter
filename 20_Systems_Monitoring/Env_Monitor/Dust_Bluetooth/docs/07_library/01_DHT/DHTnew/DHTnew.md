DHTNew

DHT11 및 DHT22(및 호환 센서)용 아두이노 라이브러리 – 자동 센서 타입 인식 지원

설명

DHTNew는 ARM 및 AVR 플랫폼에서 안정적으로 동작합니다. 이 라이브러리는 잘 검증된 DHTlib 코드를 기반으로 하며, 모든 DHT 계열 라이브러리의 메인 개발 버전입니다.

지원 센서: DHT11, DHT22, DHT33, DHT44, AM2301, AM2302, AM2303
(이들은 모두 동일한 프로토콜 사용. 단, 세부 차이가 있음 → 예: DHT11은 음수 온도 미지원, 소수점 미지원, 더 긴 웨이크업 시간 필요)

라이브러리는 온도를 **섭씨(°C)**로, 습도를 0.0–100.0% RH로 반환합니다.
화씨(°F) 또는 켈빈(K) 변환은 Temperature 라이브러리
 참조.

센서가 이상한 값을 반환할 경우, dhtnew_pulse_diag_ext.ino 예제를 사용해 데이터시트와 타이밍 비교 가능.

Sonoff Si7021 지원

v0.4.14부터 실험적 지원 추가.

직접 하드웨어 테스트는 안 했으나, 동작 보고 있음 (#79).

매우 민감한 웨이크업 타이밍 필요 → 향후 개선 예정.

사용법: setType(70) 호출.

AM2320, AM2321, AM2322 지원

v0.4.18부터 실험적 지원 추가.

AM2320은 정상 동작 확인 (#26).

AM2321, AM2322도 데이터시트상 거의 동일 → 동작 예상.

사용법: setType(22) (DHT22와 동일 프로토콜).

차이가 발견될 경우 타입 23 도입 예정 (현재는 22와 동일 처리).

KY-015 (DHT11 모듈)

v0.5.1부터 지원.

KY-015는 내부적으로 DHT11이지만, 웨이크업 타이밍이 다소 빨라서 DHT22로 잘못 인식 → 잘못된 값 반환 문제 발생.

현재 인식 알고리즘 수정 → 올바르게 인식됨.

인식 실패 시: setType(11) 강제 지정 가능.

음수 온도 처리 개선 (v0.5.0)

일부 DHT22 센서가 다른 방식으로 음수 온도를 표현.

v0.5.0부터 자동 감지하여 올바른 해석 가능.

사양 (DHT22 기준)

전원: 3.3–6.0 V DC

출력: 단일 버스 디지털 신호

감지 소자: 폴리머 커패시터

동작 범위:

습도: 0–100% RH

온도: -40° ~ 80°C

정확도:

습도: ±2% RH (최대 ±5% RH)

온도: ±0.5°C

분해능:

습도: 0.1% RH

온도: 0.1°C

반복성:

습도: ±1.0% RH

온도: ±0.2°C

습도 히스테리시스: ±0.3% RH

장기 안정성: ±0.5% RH/년

측정 주기: 평균 2초

핀 배열 (DHT 표준)

VCC

DATA

NC (Not Connected)

GND
(일부 센서는 3핀만 제공. 반드시 데이터시트 확인)

인터페이스
생성자
#include "dhtnew.h"
DHTNEW(uint8_t pin);  // 데이터 핀 지정

주요 함수

int read() → 온도, 습도 값 읽기

float getTemperature() → °C 단위 온도 반환

float getHumidity() → %RH 단위 습도 반환

uint8_t getType() → 센서 타입 반환 (11, 22, 70 등)

void setType(uint8_t type) → 센서 타입 강제 지정

오프셋 보정

setTemperatureOffset(float offset) → 온도 보정

setHumidityOffset(float offset) → 습도 보정

전력 제어

powerDown() → 센서 절전 모드

powerUp() → 센서 재시작 (2초 대기 필요)

에러 처리

기본 에러 값: -999

setSuppressError(true) 사용 시, 마지막 정상값 반환하도록 설정 가능

특징 및 개선 사항 (역사)

핀 번호 기반 객체 생성 → bathroom(4), kitchen(3) 등 관리 용이

read() 호출 시 센서 타입 자동 감지 및 수학 공식 선택

오프셋 기능 내장 (단, 오버/언더플로우 가능성 존재)

lastRead() → 마지막 읽은 시각(ms) 반환, 불필요한 재읽기 방지

인터럽트 제어 기능 추가 (타이밍 안정성 확보)

음수 온도 처리 개선 (여러 버전에서 지속 수정됨)

ESP8266, ESP32, Arduino R4 등 다양한 보드와 호환성 점검

미래 계획 (Future)

문서 업데이트

다양한 보드에서 테스트 확대

센서 타입 자동 인식 알고리즘 개선

타입별 온도 범위 제약 검토 (예: DHT11: 0–50°C, DHT22: -40–80°C)

지원 (Support)

이 라이브러리를 좋아한다면, 유지보수 및 개발을 위해 기여할 수 있습니다.

버그 리포트 및 PR 제공

PayPal 또는 GitHub Sponsors를 통한 후원 가능


1. examples/dhtnew_adaptive_delay — 환경/응답에 따라 읽기 지연을 자동으로 조절하는 방법을 시연

2. examples/dhtnew_array — 여러 DHT 센서를 배열로 관리하고 동시에 읽는 방법을 시연

3. examples/dhtnew_debug — 통신과 라이브러리 동작을 자세히 출력해 문제를 진단하는 방법을 시연

4. examples/dhtnew_dht11 — DHT11 센서에서 온도·습도 값을 읽어 출력하는 기본 사용법

5. examples/dhtnew_dht22 — DHT22 센서에서 온도·습도 값을 읽어 출력하는 기본 사용법

6. examples/dhtnew_endless — 무한 루프로 지속적으로 센서 값을 읽어 처리하는 예제

7. examples/dhtnew_endless_debug — 무한 루프 읽기 중 디버그 출력을 함께 보여주는 예제

8. examples/dhtnew_endless_insideFunction — 무한 루프 내에서 함수 호출 구조로 센서 읽기를 수행하는 예제

9. examples/dhtnew_minimum — 라이브러리의 최소 기능만으로 센서를 읽는 간단한 예제

10. examples/dhtnew_minimum_insideFunction — 최소 기능 예제를 함수 내부 구조로 구현한 예제

11. examples/dhtnew_powerDown — 전력 절약을 위해 장치/센서를 파워다운했다가 복구하는 방법을 시연

12. examples/dhtnew_pulse_diag — 펄스 타이밍을 진단해 센서 통신 파형을 분석하는 예제

13. examples/dhtnew_pulse_diag_ext — 확장된 펄스 진단으로 더 상세한 통신 분석을 제공하는 예제

14. examples/dhtnew_runtime — 런타임(실행 시간) 측정과 성능 통계를 수집해 동작을 분석하는 예제

15. examples/dhtnew_setReadDelay — 사용자가 읽기 지연을 직접 설정해 동작을 조정하는 방법을 시연

16. examples/dhtnew_simple — 가장 기본적인 센서 읽기 및 출력 예제(초심자용)

17. examples/dhtnew_suppressError — 오류 출력을 억제하거나 실패를 무시/처리하는 방법을 시연

18. examples/dhtnew_test — 다양한 테스트 케이스로 센서와 라이브러리 기능을 검증하는 예제

19. examples/dhtnew_waitForRead — 블로킹 방식으로 읽기 완료를 기다리는 예제

20. examples/dhtnew_waitForRead_nonBlocking — 논블로킹 방식으로 읽기 완료를 처리하는 예제


=====================  영문 원본 =====================
DHTNew
Arduino library for DHT11 and DHT22 (and compatible) with automatic sensor type recognition.

Description
DHTNEW is stable for both ARM and AVR. It is based upon the well tested DHTlib code. This is the main development library of all my DHT libraries.

Supports DHT11, DHT22, DHT33, DHT44, AM2301, AM2302, AM2303 as these all have the same protocol. Note there are differences e.g. DHT11 has no negative temperature, no decimals, and a longer wakeup time.

The DHTNew library returns Temperature in degrees Celsius and Humidity in 0.0 - 100.0 %RH. For converting temperature to Fahrenheit or Kelvin, see https://github.com/RobTillaart/Temperature.

For diagnosis if a DHT sensor gives strange readings one can use dhtnew_pulse_diag_ext.ino to compare timing with the datasheet.

Sonoff Si7021
Since 0.4.14 there is experimental support for the Sonoff Si7021. No hardware yet to test this myself, but it is confirmed to work. See #79. Seems the Sonoff Si7021 sensor is very sensitive in the wakeup timing. This behaviour needs to be investigated in the future.

To use the library one should call setType(70).

Feedback (both positive and negative) about the Sonoff Si7021 sensors is welcome.

AM2320, AM2321 and AM2322
Since 0.4.18 there is experimental support for the AM2320, AM2321 and AM2322. Not tested myself, but AM2320 is confirmed to work, see RobTillaart/AM232X#26 As the AM2321 and AM2322 are quite identical according to the datasheet, those are expected to work too.

To use the library one should call setType(22) as the protocol is identical to the DHT22. If there are differences in operation type (23) will be elaborated. The type 23 is now mapped upon type 22.

Feedback (both positive and negative) about the AM232X sensors is welcome.

Note: check the datasheet how to connect!

KY-015 (dht11)
Since 0.5.1 there is support for the KY-015. Although it is a DHT11, the KY-015 behaves slightly different (faster wakeup) and was recognized as a DHT22 resulting in faulty conversions. The library sensor recognition code has been adapted so it should be recognized now. In the case the recognition fails, one can use setType(11) to force the type.

The KY-015 is confirmed to work, see #102

Feedback about the KY-015 sensors is welcome.

Note: check the datasheet how to connect!

0.5.0 Negative temperature
Apparently there are DHT22's which use another representation for negative temperatures. Since 0.5.0 the library automatically detects which representation is used by the sensor and chooses the correct algorithm to decode the negative temperature.

See issue #100 (solution) and #57 and #52 before.
See also https://arduino.stackexchange.com/questions/86448/dht22-sensor-reading-code-interprets-negative-values-weirdly

Related
https://github.com/RobTillaart/DHTNew DHT11/22 etc
https://github.com/RobTillaart/DHTStable DHT11/22 etc
https://github.com/RobTillaart/DHT_Simulator
https://github.com/RobTillaart/DS18B20_INT OneWire temperature sensor
https://github.com/RobTillaart/DS18B20_RT OneWire temperature sensor
https://github.com/RobTillaart/SHT31 Sensirion humidity / temperature sensor
https://github.com/RobTillaart/SHT85 Sensirion humidity / temperature sensor
https://www.kandrsmith.org/RJS/Misc/Hygrometers/calib_many.html (interesting)
https://github.com/RobTillaart/Temperature (conversions, dewPoint, heat index etc.)
DHT PIN layout from left to right
Front		Description
pin 1		VCC
pin 2		DATA
pin 3		Not Connected
pin 4		GND
Note: check the datasheet of the sensor how to connect! as some have only 3 pins.

Specification DHT22
As most used model, for other sensors I refer to datasheet.

Model	DHT22	Notes
Power supply	3.3 - 6.0 V DC
Output signal	digital signal via single-bus
Sensing element	polymer capacitor
Operating range	humidity 0.0-100.0% RH	temperature -40° - 80° Celsius
Accuracy humidity	±2% RH(Max ±5% RH)	temperature < ±0.5° Celsius
Resolution or sensitivity	humidity 0.1% RH	temperature 0.1° Celsius
Repeatability humidity	±1.0% RH	temperature ±0.2° Celsius
Humidity hysteresis	±0.3% RH
Long-term Stability	±0.5% RH/year
Sensing period	average: 2 s
Interchangeability	fully interchangeable
Dimensions	small 14 x 18 x 5.5 mm	big 22 x 28 x 5 mm
Interface
#include "dhtnew.h"
Constructor
DHTNEW(uint8_t pin) defines the dataPin of the sensor.
void reset() might help to reset a sensor behaving badly. It resets the library internal settings to default, however it does not reset the sensor in a hardware way.
uint8_t getType() 0 = unknown, 11 or 22. In case of 0, getType() will try to determine type. Since 0.4.14 type 70 is added for experimental Sonoff Si7021 support.
void setType(uint8_t type = 0) allows to force the type of the sensor.
Type	Sensors	Notes
0	not defined
11	DHT11 DHT12, KY015	KY015 needs setType(11)
22	DHT22, DHT33, DHT44 a.o	most others
23	DHT23	mapped to 22 for now
70	Sonoff Si7021	experimental
other	sets to 0	0.4.20
Base interface
int read() reads a new temperature (Celsius) and humidity (%RH) from the sensor.
uint32_t lastRead() returns milliseconds since last read()
float getHumidity() returns last read humidity = 0.0 - 100.0 %RH. In case of an error it returns DHTLIB_INVALID_VALUE == -999. Note this error value can be suppressed by setSuppressError(bool).
float getTemperature() returns last read temperature in Celsius. Range depends on the sensor. In case of an error it returns DHTLIB_INVALID_VALUE == -999. Note this error value can be suppressed by setSuppressError(bool).
Offset
Adding offsets works well in normal range however they might introduce under- or overflow at the ends of the sensor range.

Humidity offset is in % RH and is constrained to 0.0 - 100.0 % in the code.

Temperature offset is in degrees Celsius. For temperature a constrain would be type dependant, so it is not done. Furthermore by setting the offset to -273.15 one get the Kelvin scale.

void setHumidityOffset(float offset) typical < ±5% RH.
void setTemperatureOffset(float offset) typical < ±2°C.
float getHumidityOffset() idem.
float getTemperatureOffset() idem.
The "short-named" offset functions will become obsolete in the future (0.6.0).

void setHumOffset(float offset) typical < ±5% RH.
void setTempOffset(float offset) typical < ±2°C.
float getHumOffset() idem.
float getTempOffset() idem.
Control
Functions to adjust the communication with the sensor.

void setDisableIRQ(bool b ) allows or suppresses interrupts during core read function to keep timing as correct as possible. Note AVR + MKR1010 + Arduino R4
bool getDisableIRQ() returns the above setting. Default true.
void setWaitForReading(bool b ) flag to enforce a blocking wait.
bool getWaitForReading() returns the above setting.
void setReadDelay(uint16_t rd = 0) To tune the time it waits before actual read. This reduces the blocking time. Default depends on type. 1000 ms (dht11) or 2000 ms (dht22). set readDelay to 0 will reset to datasheet values AFTER a call to read().
uint16_t getReadDelay() returns the above setting.
void powerDown() pulls dataPin down to reduce power consumption
void powerUp() restarts the sensor, note one must wait up to two seconds.
void setSuppressError(bool b) suppress error values of -999 => You need to check the return value of read() instead.
This is used to keep spikes out of your plotter / graphs / logs.
bool getSuppressError() returns the above setting.
Operation
See examples

TIME_OUT
If consistent TIMOUT_C or TIMEOUT_D occur during reading a sensor, one could try if allowing interrupts solves the issue DHT.setDisableIRQ(false).

This solved this problem at least on

AVR boards - is build into the constructor
MKR1010 Wifi - see #67 (added as comment in the examples)
Arduino R4 - see arduino/uno-r4-library-compatibility#38
In version 0.4.10 the TIMEOUT_C is extended from 70-90 us to even suppress the TIMEOUT_C even more. See discussion and tests in #67.

Serial
The MKR1010Wifi board need to wait for Serial at startup if you want to monitor it from the IDE. Adding the line while(!Serial): fixes this. (added to the examples).

There might be more boards that need this line to work properly.

DHT22 and ESP8266
The DHT22 sensor has some problems in combination with specific pins of the ESP8266. See more details
#31 (message Jan 3, 2021)
arendst/Tasmota#3522
Voltage AM2301 and ESP8266
In a test an AM2301 had problems giving no humidity (99.9% overflow) when the DHTStable library was used with an ESP8266. (Reported by mail, no GH issue). As this DHTStable library is strongly related to the DHTNew it is mentioned here too.

After days of testing and thinking and more testing the cause was found. The AM2301 was powered by a 5V3 power supply which was apparently too high while having the data handshakes at 3V3. When the VCC voltage was lowered to 5V1 it appeared to work as it should. (Kudos to Viktor for finding the cause)

History
DHTNEW has some new features compared to the DHTlib code.

The constructor has a pin number, so the one sensor - one object paradigm is chosen. So you can now make a DHTNEW object bathroom(4), kitchen(3), etc.
The read() function now reads both DHT11 and DHT22 sensors and selects the right math per sensor based upon the bit patterns.
An offset can be set for both temperature and humidity to have a first-order linear calibration in the class itself. Of course, this introduces a possible risk of under- or overflow. For a more elaborated or non-linear offset, I refer to my multimap class.
lastRead() keeps track of the last time the sensor is read. If this is not too long ago one can decide not to read the sensors but use the current values for temperature and humidity. This saves up to 20+ milliseconds for a DHT11 or 5+ milliseconds for a DHT22. Note that these sensors should have 1-2 seconds between reads according to specification. In the future, this functionality could be inside the library by setting a time threshold (e.g. 1 second by default) to give more stable results.
Added interrupt enable/disable flag to prevent interrupts disturb timing of DHT protocol. Be aware that this may affect other parts of your application.
(0.1.7) added an automatic check of lastRead in the read call. If request a read to fast it will just return OK.
(0.1.7) added waitForReading flag (kudos to Mr-HaleYa) to let the sensor explicitly wait until a new value can be read.
(0.2.0) Temperature and humidity are private now, use getTemperature() and getHumidity()
(0.2.1) Adjusted the bit timing threshold to work around issue #11
(0.2.2) added ERROR_SENSOR_NOT_READY and differentiated timeout errors.
(0.3.0) removed interrupt flag, now the library always disables interrupts during the clocking of the bits. Added getReadDelay & setReadDelay to tune reading interval. Check the example code. Adjusted the timing in the wake-up part of the protocol. Added more comments to describe the protocol.
(0.3.1) added powerDown() and powerUp() for low power applications. Note that after powerUp() the user must wait for two seconds before doing a read(). Just like after a (re)boot.
Note: The lib does not (yet) control the power pin of the sensor. Discussion see #13
(0.3.2) Added setSuppressError() and getSuppressError() so the library will not output -999 but the last known valid value for temperature and humidity. This flag is useful to suppress 'negative spikes' in graphs or logs. Default the error values are not suppressed to be backwards compatible.
Added #ifndef around DHTLIB_INVALID_VALUE so the default -999 can be overruled compile time to set another error value e.g. -127 or -1 whatever suits the project.
(0.3.3) Refactored the low level readSensor() as the BIT SHIFT ERROR issue #29 and issue #11 popped up again. It was reproduced "efficiently" with an ESP32 and by using long wires. Fixed with an explicit digitalWrite(dataPin, HIGH) + delayMicroseconds() to have enough time between pulling the line HIGH and polling for the line LOW.
(0.3.4) Added waitFor(state, timeout) to more precisely follow the datasheet in terms of timing. Reintroduced the interrupt enable/disable flag as forced noInterrupts() could break the timing of the DHT protocol / micros() - seen on AVR.
(0.4.0) Added DHTLIB_WAITING_FOR_READ as return value of read => minor break of interface
(0.4.1) Added Arduino-CI support + gettype() now tries to determine type if not known.
(0.4.2) Fix negative temperatures. Tested with DHTNew_debug.ino and hex dump in .cpp and a freezer.
Note: testing in a freezer is not so good for humidity readings.
(0.4.3) Added reset() to reset internal variables when a sensor blocks this might help. Added lastRead() to return time the sensor is last read. (in milliseconds).
(0.4.4) DO NOT USE incorrect negative temp.
(0.4.5) Prevent -0.0 when negative temp is 0; DO NOT USE as it maps every negative temp to zero.
(0.4.6) Fixed negative temperature (again).
(0.4.7) fix #60 negative temperatures below -25.5°C + readme.md.
(0.4.8) fixes to improve Arduino-lint.
(0.4.9) add optional flag DHTLIB_VALUE_OUT_OF_RANGE.
(0.4.10) updated build-CI to do compile test - UNO, due, zero, Leonardo, m4, esp32, esp8266, mega2560. updated readme.md - added badges and remarks after testing with MKR1010 Wifi. updated TIMEOUT_C from 70 -> 90 us to minimize its occurrence - See #67. added while(!Serial); in examples to they work for MKR1010 Wifi.
(0.4.11) update library.json, license, minor edits (clean up), unit tests
(0.4.12) Fix #72, delayMicroseconds() for wakeUp
(0.4.13) Fix #76, disable interrupts for ESP32.
(0.4.14) added experimental support for Si7021.
(0.4.15) Fix #81, recognize DHT22 as type 70. Add minimal wakeup delay.
(0.4.16) Fix #84 correct the reading of type 70 for Sonoff Si7021.
(0.4.17) fix #86, define constants explicit as float.
(0.4.18) Update readme.md and library.* about support for AM2320/21/22.
(0.4.19) Update readme.md
(0.4.20) Update GitHub actions and readme.md
(0.4.21) Add dhtnew_pulse_diag_ext.ino
(0.5.0) Fix negative values
(0.5.1) Support KY015 and more
(0.5.2) Support KY015 (again) Add DHT_endless_debug.ino develop example
(0.5.3) Update readme.md for Arduino R4
Future
Must
update documentation
Should
test on more boards
#if defined(MKR1010) // TODO find out real define #67
#ifdef ARDUINO_SAMD_MKRWIFI1010
#error found
#endif
Could
test compatibility => table.
investigate temperature constraining (type dependant)
if (type == 11) temp = constrain(temp,   0, 100);
if (type == 22) temp = constrain(temp, -40,  80);
etc.
type parameter in constructor, default 0
reimplement the recognition algorithm (separate class?)
read as 22 => check the hum / temp data range to determine type
read as 11 => check idem
read as 70 => check idem
split wakeup time from bit interpretation used.
Wont
move all code from .h to .cpp
derived classes for fixed type?
Support
If you appreciate my libraries, you can support the development and maintenance. Improve the quality of the libraries by providing issues and Pull Requests, or donate through PayPal or GitHub sponsors.

Thank you,
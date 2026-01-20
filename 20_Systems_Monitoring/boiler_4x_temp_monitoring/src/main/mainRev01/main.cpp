/*
  ds18b20_basic.ino
  DS18B20 기본 예제 (OneWire + DallasTemperature)
  - Arduino R4 WiFi 보드용(핀/라이브러리 호환)
  - 최대 4개의 DS18B20을 같은 1-Wire 버스에 연결
  - 20x4 I2C LCD에 출력 (주소 0x27 예시)

  코딩 규칙 준수:
   - 주석은 한국어로 작성
   - 상수는 const로 선언 (매직 넘버 금지)
   - loop()는 오케스트레이션 역할만 수행 (긴 블로킹 금지)
   - 들여쓰기: space 4
   - 중괄호 스타일: Allman

  Wiring (예시):
   - DS18B20 DATA -> D2
   - DS18B20 VCC -> 5V (또는 보드 전압에 맞춤)
   - DS18B20 GND -> GND
   - DATA와 VCC 사이에 4.7K 풀업 저항
   - 20x4 LCD (I2C) -> SDA/SCL, 주소 0x27(다를 수 있음)

  필요한 라이브러리:
   - OneWire
   - DallasTemperature
   - LiquidCrystal_I2C (I2C 기반 20x4 LCD)
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// 상수
const uint8_t ONE_WIRE_BUS = 2;                        // DS18B20 데이터 핀
const uint8_t MAX_SENSORS = 4;                         // 기대하는 센서 수 (최대)
const uint8_t LCD_I2C_ADDRESS = 0x27;                  // LCD I2C 주소 (환경에 따라 변경)
const unsigned long SAMPLE_INTERVAL_MS = 2000;         // 측정 주기 (밀리초)
const unsigned long DISPLAY_ROTATE_MS = 5000;          // 화면 전환 주기 (밀리초)
const unsigned long MISSING_SENSOR_REMINDER_MS = 5000; // 센서 미검출 시 알림 간격 (밀리초)

// 전역 객체 및 상태
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorAddress[MAX_SENSORS];
uint8_t foundSensors = 0;
float sensorTemps[MAX_SENSORS];
uint8_t currentDisplayMode = 0; // 0: 온도차이 모드, 1: 개별 온도 모드

LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);

unsigned long lastSampleMs = 0;
unsigned long lastDisplayRotateMs = 0;         // 마지막 화면 전환 시각
unsigned long lastMissingSensorReminderMs = 0; // 마지막 미검출 알림 시각

// 헬퍼: 주소 출력 (디버그용)
void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16)
        {
            Serial.print("0");
        }
        Serial.print(deviceAddress[i], HEX);
    }
}

// 센서 초기화
void initSensors()
{
    sensors.begin();
    uint8_t count = sensors.getDeviceCount();
    if (count > MAX_SENSORS)
    {
        count = MAX_SENSORS;
    }

    uint8_t stored = 0;
    for (uint8_t i = 0; i < sensors.getDeviceCount() && stored < MAX_SENSORS; i++)
    {
        if (sensors.getAddress(sensorAddress[stored], i))
        {
            Serial.print("Sensor #");
            Serial.print(stored);
            Serial.print(" addr: ");
            printAddress(sensorAddress[stored]);
            Serial.println();
            stored++;
        }
        else
        {
            Serial.print("Failed to read address for device index ");
            Serial.println(i);
        }
    }

    foundSensors = stored;
    for (uint8_t i = 0; i < foundSensors; i++)
    {
        sensorTemps[i] = DEVICE_DISCONNECTED_C; // 초기화
    }

    Serial.print("Found sensors: ");
    Serial.println(foundSensors);
    if (foundSensors == 0)
    {
        Serial.println("add sensor please");
    }
}

// LCD 초기화
void initDisplay()
{
    lcd.begin(20, 4);
    lcd.setBacklight(255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Boiler Temp Monitor");
}

// 센서 주소 짧은 표시 (마지막 4자리) - setup에서 4초간 표시
void showSensorShortAddresses(unsigned long durationMs = 4000)
{
    if (foundSensors == 0)
    {
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor addrs (last4):");

    for (uint8_t i = 0; i < foundSensors && i < MAX_SENSORS; i++)
    {
        // 마지막 2바이트 -> 4자리 16진수
        char buf[5] = {0};
        uint8_t b1 = sensorAddress[i][6];
        uint8_t b2 = sensorAddress[i][7];
        sprintf(buf, "%02X%02X", b1, b2);

        lcd.setCursor(0, 1 + i);
        lcd.print("S");
        lcd.print(i + 1);
        lcd.print(": ");
        lcd.print(buf);

        // Serial에도 출력
        Serial.print("S");
        Serial.print(i + 1);
        Serial.print(" short addr: ");
        Serial.println(buf);
    }

    unsigned long startMs = millis();
    while (millis() - startMs < durationMs)
    {
        delay(50);
    }

    // 복구: 기본 헤더로 되돌림
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Boiler Temp Monitor");
}

// 온도 읽기 (동기화 호출, 주기적으로 실행)
void readTemperatures()
{
    sensors.requestTemperatures(); // 변환 요청 (blocking until done)

    for (uint8_t i = 0; i < foundSensors; i++)
    {
        float tempC = sensors.getTempC(sensorAddress[i]);
        sensorTemps[i] = tempC;
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.println(" 연결 끊김");
        }
        else
        {
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(tempC, 1);
            Serial.println(" C");
        }
    }
}

// LCD 업데이트 (개별 온도 표시 모드)
void updateDisplayModeIndividual()
{
    for (uint8_t i = 0; i < MAX_SENSORS; i++)
    {
        lcd.setCursor(0, i);
        if (i < foundSensors)
        {
            if (sensorTemps[i] == DEVICE_DISCONNECTED_C)
            {
                lcd.print("S");
                lcd.print(i + 1);
                lcd.print(": err           ");
            }
            else
            {
                lcd.print("S");
                lcd.print(i + 1);
                lcd.print(": ");
                lcd.print(sensorTemps[i], 1);
                lcd.print(" C          ");
            }
        }
        else
        {
            lcd.print("S");
            lcd.print(i + 1);
            lcd.print(": -             ");
        }
    }
}

// LCD 업데이트 (온도차이 표시 모드 - Display 1)
void updateDisplayModeDiff()
{
    // 1~2번 센서 차이
    lcd.setCursor(0, 0);
    if (foundSensors >= 2 && sensorTemps[0] != DEVICE_DISCONNECTED_C && sensorTemps[1] != DEVICE_DISCONNECTED_C)
    {
        lcd.print("01: ");
        lcd.print(sensorTemps[0], 1);
        lcd.print("C ");
        lcd.setCursor(10, 0);
        lcd.print("02: ");
        lcd.print(sensorTemps[1], 1);
        lcd.print("C");

        lcd.setCursor(0, 1);
        lcd.print("01 - 02 : ");
        lcd.print(sensorTemps[0] - sensorTemps[1], 1);
        lcd.print(" C");
    }
    else
    {
        lcd.print("01-02: Check Sensors");
    }

    // 3~4번 센서 차이
    lcd.setCursor(0, 2);
    if (foundSensors >= 4 && sensorTemps[2] != DEVICE_DISCONNECTED_C && sensorTemps[3] != DEVICE_DISCONNECTED_C)
    {
        lcd.print("03: ");
        lcd.print(sensorTemps[2], 1);
        lcd.print("C ");
        lcd.setCursor(10, 2);
        lcd.print("04: ");
        lcd.print(sensorTemps[3], 1);
        lcd.print("C");

        lcd.setCursor(0, 3);
        lcd.print("03 - 04 : ");
        lcd.print(sensorTemps[2] - sensorTemps[3], 1);
        lcd.print(" C");
    }
    else if (foundSensors >= 2)
    {
        lcd.setCursor(0, 2);
        lcd.print("03-04: Need 4 Sensors");
    }
}

// 주기적 업데이트를 관리하는 오케스트레이션 함수
void updateSystem()
{
    unsigned long now = millis();

    // 센서 데이터 읽기
    if (now - lastSampleMs >= SAMPLE_INTERVAL_MS)
    {
        lastSampleMs = now;
        if (foundSensors > 0)
        {
            readTemperatures();
        }
    }

    // 화면 전환 로직
    if (now - lastDisplayRotateMs >= DISPLAY_ROTATE_MS)
    {
        lastDisplayRotateMs = now;
        currentDisplayMode = (currentDisplayMode + 1) % 2;
        lcd.clear(); // 모드 전환 시 화면 초기화
    }

    // LCD 업데이트
    if (foundSensors > 0)
    {
        if (currentDisplayMode == 0)
        {
            updateDisplayModeDiff();
        }
        else
        {
            updateDisplayModeIndividual();
        }
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    initDisplay();
    initSensors();

    if (foundSensors == 0)
    {
        lcd.setCursor(0, 1);
        lcd.print("No DS18B20 found");
    }
    else
    {
        // 연결된 센서가 있으면 각 센서의 마지막 4자리 주소를 4초간 표시
        showSensorShortAddresses(4000);
    }

    // 초기 샘플 시간 설정
    lastSampleMs = millis();
}

void loop()
{
    // loop()는 오케스트레이션만 수행합니다.
    updateSystem();

    // 다른 태스크가 있다면 여기에 비차단 방식으로 추가
}

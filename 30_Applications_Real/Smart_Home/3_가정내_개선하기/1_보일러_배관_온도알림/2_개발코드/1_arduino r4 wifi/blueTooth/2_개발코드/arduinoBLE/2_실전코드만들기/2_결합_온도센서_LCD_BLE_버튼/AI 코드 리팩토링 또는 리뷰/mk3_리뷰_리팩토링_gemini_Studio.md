## 코드 리뷰 및 리팩토링 제안 (아두이노 초보자 관점)

안녕하세요! 아두이노 초보자 입장에서 코드를 살펴보겠습니다. 훌륭한 프로젝트를 진행하고 계시네요! 전반적으로 코드가 잘 구성되어 있지만, 몇 가지 부분을 개선하면 초보자가 이해하기 쉽고 유지보수도 용이해질 것입니다.

### 1. 코드 구조 및 가독성

*   **주석**: 코드에 주석이 잘 달려 있어서 각 부분이 어떤 역할을 하는지 이해하기 좋습니다. 👍
    *   **개선 제안**:
        *   함수 상단에 함수가 하는 일, 받는 인자, 반환 값에 대한 간략한 설명을 추가하면 더 좋습니다.
        *   `// -------------` 와 같이 여러 줄 주석보다는 `/* ... */` 형태의 블록 주석을 사용하면 가독성이 더 좋아집니다.

*   **들여쓰기**: 코드가 깔끔하게 들여쓰기 되어 있어 가독성이 좋습니다.

*   **변수명**: 변수명이 의미를 잘 나타내고 있어 이해하기 쉽습니다.
    *   **개선 제안**:
        *   `tempC`와 같이 축약된 변수명보다는 `temperatureCelsius`와 같이 좀 더 명확하게 풀어서 쓰는 것이 좋습니다. (초보자는 축약어를 이해하기 어려울 수 있습니다.)

*   **함수 분리**: 코드가 여러 함수로 잘 분리되어 있어 각 함수의 역할을 파악하기 쉽습니다.
    *   **개선 제안**:
        *   `updateLCDDisplay()` 함수가 좀 긴 편입니다. LCD에 온도 값을 표시하는 부분을 별도의 함수로 분리하면 가독성이 더 좋아질 수 있습니다.

### 2. 코드 단순화 및 명확성

*   **매직 넘버**: 코드에 `8` (주소 길이), `20`, `4` (LCD 크기), `20000` (LCD 타임아웃), `9600` (시리얼 통신 속도) 등의 숫자가 직접 사용되고 있습니다. 이러한 숫자들을 "매직 넘버"라고 하는데, 의미를 파악하기 어렵고 나중에 수정하기 어렵게 만듭니다.
    *   **개선 제안**:
        *   `const int` 또는 `#define`을 사용하여 의미 있는 이름의 상수로 정의하는 것이 좋습니다.
        ```c++
        const int ADDRESS_LENGTH = 8;
        const int LCD_COLS = 20;
        const int LCD_ROWS = 4;
        const unsigned long LCD_TIMEOUT = 20000; // 20초
        const int SERIAL_BAUD_RATE = 9600;
        ```

*   **반복되는 코드**: `tempCharacteristic0`, `tempCharacteristic1`, ... 에 값을 쓰는 부분이 반복되고 있습니다.
    *   **개선 제안**:
        *   `tempCharacteristics` 배열을 이미 사용하고 있으므로, `for` 루프를 사용하여 코드를 간결하게 만들 수 있습니다.

*   **`sprintf` 대신 `String` 사용**: `sprintf` 함수는 C 스타일 문자열을 다루기 때문에 초보자가 사용하기 어려울 수 있습니다.
    *   **개선 제안**:
        *   Arduino의 `String` 클래스를 사용하면 문자열 처리를 더 쉽게 할 수 있습니다. (하지만 `String` 클래스는 메모리 단편화 문제를 일으킬 수 있으므로 주의해야 합니다. 이 프로젝트에서는 큰 문제가 되지 않을 것입니다.)

*   **`memcpy`**: `memcpy` 함수는 메모리 복사를 수행하는데, 초보자가 이해하기 어려울 수 있습니다.
    *    **개선 제안**:
    *   `DeviceAddress`가 단순한 바이트 배열이므로, `for` 루프를 사용하여 각 바이트를 복사하는 방식으로 변경할 수 있습니다.
*    **센서 에러처리**:
    *   **개선 제안**:
        센서 에러를 단순히 ERR, OVR 로만 표기하지 않고, LCD, 시리얼 디버깅시에 어떠한 에러가 발생했는지 알 수 있는 코드를 추가하면 좋습니다.
    *   에러코드를 전역변수로 선언해서 에러 핸들링 하면 더욱 좋습니다.

### 3. 리팩토링 제안 (초보자도 이해하기 쉬운 버전)

```c++
#include <ArduinoBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>
#include <ezButton.h>

/* ----- 설정 ----- */
const int ONE_WIRE_BUS = 2;
const int REQUIRED_SENSOR_COUNT = 4; // 필요한 센서 수
const int ADDRESS_LENGTH = 8;        // 센서 주소 길이
const int LCD_COLS = 20;             // LCD 가로 칸 수
const int LCD_ROWS = 4;              // LCD 세로 줄 수
const unsigned long LCD_TIMEOUT = 20000;  // LCD 꺼짐 시간 (20초)
const unsigned long SERIAL_INTERVAL = 2000; // 시리얼 출력 간격 (2초)
const int SERIAL_BAUD_RATE = 9600;      // 시리얼 통신 속도
const int BUTTON_PIN = 3;            // 버튼 핀 번호
const int I2C_ADDR = 0x27;

// BLE 서비스 및 특성 UUID
const char *SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_BASE = "19B10000-E8F2-537E-4F6C-D104768A1214"; // 기본 UUID

// 에러 코드 정의
enum ErrorCode {
  NO_ERROR = 0,
  SENSOR_NOT_FOUND,
  SENSOR_DISCONNECTED,
  TEMPERATURE_OUT_OF_RANGE,
  DUPLICATE_SENSOR_ID
};

/* ----- 전역 변수 ----- */
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

DeviceAddress sensorsAddress[REQUIRED_SENSOR_COUNT];
int sensorIDs[REQUIRED_SENSOR_COUNT];
bool sensorValid[REQUIRED_SENSOR_COUNT];
ErrorCode sensorErrors[REQUIRED_SENSOR_COUNT] = {NO_ERROR}; // 각 센서별 에러 코드

unsigned long lastSerialTime = 0;

BLEService tempService(SERVICE_UUID);
BLECharacteristic *tempCharacteristics[REQUIRED_SENSOR_COUNT];

LiquidCrystal_PCF8574 lcd(I2C_ADDR);
ezButton button(BUTTON_PIN);
unsigned long lcdOnTime = 0;
bool isFirstLCDRun = true;

//에러 메세지 처리 함수
const char *getErrorMessage(ErrorCode error)
{
  switch (error)
  {
  case SENSOR_NOT_FOUND:
    return "Sensor Not Found";
  case SENSOR_DISCONNECTED:
    return "Sensor Disconnected";
  case TEMPERATURE_OUT_OF_RANGE:
    return "Temp Out of Range";
  case DUPLICATE_SENSOR_ID:
    return "Duplicate Sensor ID";
  default:
    return "Unknown Error";
  }
}

/* ----- 함수 선언 ----- */
void initializeSensorSystem();
void readSensorAddresses();
void readSensorIDs();
void sortSensors();
void initializeBLE();
void updateBLETemperature();
void updateSerial();
void updateLCDDisplay();
void displaySensorData(int row, int sensorIndex); // LCD에 센서 데이터 표시
String formatTemperature(float tempC, ErrorCode error);    // 온도 값 포맷팅

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);

  // LCD 및 버튼 초기화
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.setBacklight(255);
  button.setDebounceTime(50);
  lcdOnTime = millis();

  initializeSensorSystem(); // 센서 초기화
  initializeBLE();          // BLE 초기화

  // BLE 특성 동적 생성
  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
  {
    // 특성 UUID 생성 (기본 UUID + 센서 번호)
    String characteristicUUID = String(CHARACTERISTIC_UUID_BASE);
    characteristicUUID.setCharAt(7, '1' + i); // 마지막 문자 변경 (0, 1, 2, 3)

    // BLE 특성 생성 및 서비스에 추가
    tempCharacteristics[i] = new BLECharacteristic(characteristicUUID.c_str(), BLERead | BLENotify, 8);
    tempService.addCharacteristic(*tempCharacteristics[i]);
  }

  // BLE 서비스 추가 및 광고 시작
  BLE.setLocalName("ArduinoR4Temp");
  BLE.setAdvertisedService(tempService);
  BLE.addService(tempService);
  BLE.advertise();
  Serial.println("BLE peripheral started, waiting for connections...");
}

void loop()
{
  updateLCDDisplay();
  updateBLETemperature();
  // updateSerial(); // 시리얼 디버깅 시 활성화
}

/* ----- 함수 정의 ----- */

// 센서 초기화 함수
void initializeSensorSystem()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing sensors...");
  Serial.println("Starting sensor initialization...");

  ds18b20.begin();
  uint8_t deviceCount = ds18b20.getDeviceCount();

  if (deviceCount == 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No sensors!");
    Serial.println("[Error] No sensors detected.");
    while (1)
      ;
  }

  if (deviceCount < REQUIRED_SENSOR_COUNT)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Insufficient sensors:");
    lcd.setCursor(0, 1);
    lcd.print(deviceCount);
    lcd.print(" / ");
    lcd.print(REQUIRED_SENSOR_COUNT);
    Serial.println("[Warning] Fewer sensors detected than configured.");
  }

  readSensorAddresses(); // 센서 주소 읽기
  delay(1000);
  readSensorIDs(); // 센서 ID 읽기
  delay(1000);
  sortSensors(); // 센서 정렬
  delay(1000);

  lcd.clear();
  lcd.print("Initialization complete");
  Serial.println("[Init] Initialization complete.");
  delay(1000);
}

// 센서 주소 읽기 함수
void readSensorAddresses()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading addresses...");
  Serial.println("Starting to read sensor addresses...");

  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
  {
    sensorValid[i] = ds18b20.getAddress(sensorsAddress[i], i);
    if (!sensorValid[i])
    {
      Serial.print("[Error] Failed to read address for sensor ");
      Serial.println(i);
      sensorErrors[i] = SENSOR_NOT_FOUND; // 에러 코드 설정
    }
  }
}

// 센서 ID 읽기 함수
void readSensorIDs()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading IDs...");
  Serial.println("Starting to read sensor IDs...");

  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
  {
    if (sensorValid[i])
    {
      sensorIDs[i] = ds18b20.getUserData(sensorsAddress[i]);

      // 중복 ID 확인
      for (int j = 0; j < i; j++)
      {
        if (sensorValid[j] && sensorIDs[i] == sensorIDs[j])
        {
          Serial.print("[Error] Duplicate ID found: ");
          Serial.println(sensorIDs[i]);
          sensorErrors[i] = DUPLICATE_SENSOR_ID; // 에러 코드 설정
        }
      }
    }
    else
    {
      sensorIDs[i] = -1; // 오류 표시
    }
  }
}

// 센서 정렬 함수 (ID 기준 오름차순)
void sortSensors()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sorting sensors...");
  Serial.println("Starting to sort sensors...");

  for (int i = 0; i < REQUIRED_SENSOR_COUNT - 1; i++)
  {
    for (int j = 0; j < REQUIRED_SENSOR_COUNT - i - 1; j++)
    {
      if (sensorIDs[j] > sensorIDs[j + 1])
      {
        // ID 교환
        int tempID = sensorIDs[j];
        sensorIDs[j] = sensorIDs[j + 1];
        sensorIDs[j + 1] = tempID;

        // 주소 교환 (간단한 방법으로 변경)
        DeviceAddress tempAddr;
        for (int k = 0; k < ADDRESS_LENGTH; k++)
        {
          tempAddr[k] = sensorsAddress[j][k];
          sensorsAddress[j][k] = sensorsAddress[j + 1][k];
          sensorsAddress[j + 1][k] = tempAddr[k];
        }

        // 유효성 교환
        bool tempValid = sensorValid[j];
        sensorValid[j] = sensorValid[j + 1];
        sensorValid[j + 1] = tempValid;

        // 에러 정보 교환
        ErrorCode tempError = sensorErrors[j];
        sensorErrors[j] = sensorErrors[j + 1];
        sensorErrors[j + 1] = tempError;
      }
    }
  }
}

// BLE 초기화 함수
void initializeBLE()
{
  Serial.println("Starting BLE initialization...");

  if (!BLE.begin())
  {
    Serial.println("Failed to start BLE.");
    while (1)
      ; // 무한 루프 (BLE 없이는 동작 불가)
  }
}

// BLE 온도를 업데이트하고 BLE 이벤트를 폴링하는 함수
void updateBLETemperature()
{
  BLE.poll();

  if (BLE.connected())
  {
    ds18b20.requestTemperatures();

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
      String temperatureString = "";

      if (!sensorValid[i])
      {
        temperatureString = "ERR";
      }
      else
      {
        float tempC = ds18b20.getTempC(sensorsAddress[i]);

        if (tempC == DEVICE_DISCONNECTED_C)
        {
          temperatureString = "ERR";
        }
        else if (tempC < -55 || tempC > 125)
        {
          temperatureString = "OVR";
        }
        else
        {
          temperatureString = String(tempC, 1); // 소수점 한 자리까지
        }
      }
      tempCharacteristics[i]->writeValue(temperatureString.c_str());
    }
  }
}

// 시리얼 포트에 센서 정보를 출력하는 함수
void updateSerial()
{
  if (millis() - lastSerialTime > SERIAL_INTERVAL)
  {
    lastSerialTime = millis();
    ds18b20.requestTemperatures();

    Serial.println("\n--- Sensor Information ---");
    Serial.println("Index\tID\tTemperature\tAddress\t\tStatus");
    Serial.println("----------------------------------------------");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
      Serial.print(i + 1); // 인덱스 (1부터 시작)
      Serial.print("\t");

      if (!sensorValid[i])
      {
        Serial.print("Sensor Error: ");
        Serial.println(getErrorMessage(sensorErrors[i]));
        continue; // 다음 센서로 넘어감
      }

      Serial.print(sensorIDs[i]); // 센서 ID
      Serial.print("\t");

      float tempC = ds18b20.getTempC(sensorsAddress[i]);
      String tempStatus;

      if (tempC == DEVICE_DISCONNECTED_C)
      {
        Serial.print("ERR\t");
        sensorErrors[i] = SENSOR_DISCONNECTED; // 에러 상태 업데이트
      }
      else if (tempC < -55 || tempC > 125)
      {
        Serial.print("OVR\t");
        sensorErrors[i] = TEMPERATURE_OUT_OF_RANGE;
      }
      else
      {
        Serial.print(tempC, 1); // 온도 값 (소수점 한 자리까지)
        Serial.print("C\t");
        sensorErrors[i] = NO_ERROR;
      }

      // 센서 주소 출력
      for (int j = 0; j < ADDRESS_LENGTH; j++)
      {
        if (sensorsAddress[i][j] < 16)
        {
          Serial.print("0"); // 한 자리 수면 앞에 0 붙임
        }
        Serial.print(sensorsAddress[i][j], HEX);
      }
      Serial.print("\t");

      // 센서 상태 출력
      Serial.println(sensorErrors[i] == NO_ERROR ? "OK" : getErrorMessage(sensorErrors[i]));
    }

    Serial.println("----------------------------------------------");
  }
}

// LCD 화면 업데이트 함수
void updateLCDDisplay()
{
  unsigned long currentMillis = millis();

  button.loop(); // 버튼 상태 업데이트

  if (button.isPressed())
  {
    Serial.println("Button pressed");
    lcdOnTime = currentMillis;
    isFirstLCDRun = true; // 화면 초기화
  }

  if (currentMillis - lcdOnTime < LCD_TIMEOUT)
  {
    lcd.setBacklight(255); // 백라이트 켜기

    if (isFirstLCDRun)
    {
      lcd.clear();
      isFirstLCDRun = false;
    }

    // 각 센서 데이터 표시
    for (int i = 0; i < REQUIRED_SENSOR_COUNT && i < LCD_ROWS; i++)
    {
      displaySensorData(i, i);
    }
  }
  else
  {
    // LCD 끄기
    lcd.setBacklight(0);
    lcd.clear();
  }
}

// LCD에 센서 데이터 표시하는 함수 (updateLCDDisplay에서 분리)
void displaySensorData(int row, int sensorIndex)
{
  lcd.setCursor(0, row);
  lcd.print("ID:");
  lcd.print(sensorValid[sensorIndex] ? sensorIDs[sensorIndex] : -1);
  lcd.print(" ");

  lcd.print("T:");
  float tempC = sensorValid[sensorIndex] ? ds18b20.getTempC(sensorsAddress[sensorIndex]) : DEVICE_DISCONNECTED_C;

  if (!sensorValid[sensorIndex])
  {
    lcd.print("ERR");
  }
  else if (tempC == DEVICE_DISCONNECTED_C)
  {
    lcd.print("--.-");
  }
  else if (tempC < -55 || tempC > 125)
  {
    lcd.print("OVR");
  }
  else
  {
    lcd.print(tempC, 1);
    lcd.print("C");
  }
  lcd.setCursor(15, row);

  // 센서 상태 출력
  lcd.print(sensorErrors[sensorIndex] == NO_ERROR ? "OK " : "ERR");
}
// 온도 값 포맷팅 함수
String formatTemperature(float tempC, ErrorCode error)
{
  if (error != NO_ERROR)
  {
    return "ERR";
  }

  if (tempC == DEVICE_DISCONNECTED_C)
  {
    return "--.-";
  }

  if (tempC < -55 || tempC > 125)
  {
    return "OVR"; // 범위를 벗어난 값
  }

  return String(tempC, 1); // 소수점 한 자리까지
}
```

**주요 변경 사항:**

*   **매직 넘버 제거**: 상수를 사용하여 코드의 가독성과 유지보수성을 높였습니다.
*   **`String` 클래스 사용**: `sprintf` 대신 `String`을 사용하여 문자열 처리를 간소화했습니다.
*   **`memcpy` 대신 `for` 루프 사용**: 메모리 복사를 더 이해하기 쉬운 방식으로 변경했습니다.
*   **LCD 출력 함수 분리**: `updateLCDDisplay` 함수에서 LCD에 데이터를 표시하는 부분을 `displaySensorData` 함수로 분리하여 가독성을 높였습니다.
*   **온도 포맷 함수**: 온도 값을 문자열로 변환하는 부분을 별도의 함수 (`formatTemperature`)로 분리했습니다.
*    **센서 에러처리 추가**:
    *   `enum ErrorCode`: 어떠한 에러가 발생했는지 알수 있도록 열거형 에러코드를 추가했습니다.
    *    `getErrorMessage` : 에러코드에 따른 에러 메세지를 처리하는 함수를 추가했습니다.
    *    `sensorErrors[REQUIRED_SENSOR_COUNT]`: 각 센서별 에러코드를 저장하는 변수를 추가 했습니다.

이 리팩토링된 코드는 아두이노 초보자가 더 쉽게 이해하고 수정할 수 있을 것입니다. 추가적으로 궁금한 점이나 개선하고 싶은 부분이 있다면 언제든지 질문해주세요!

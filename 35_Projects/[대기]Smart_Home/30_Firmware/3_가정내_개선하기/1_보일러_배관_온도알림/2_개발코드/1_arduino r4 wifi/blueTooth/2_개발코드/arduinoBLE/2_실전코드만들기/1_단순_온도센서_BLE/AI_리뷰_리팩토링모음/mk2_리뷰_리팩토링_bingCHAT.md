안녕하세요! 제공해주신 코드를 Arduino 초보자도 이해하기 쉽게 리뷰하고, 리팩토링할 수 있는 요소들을 알려드릴게요. 코드의 목적은 그대로 유지하면서, 더 읽기 쉽고 효율적으로 개선해보겠습니다.

---

### **코드 설명**

이 코드는 **DS18B20** 온도 센서 4개를 사용하여 **BLE(Bluetooth Low Energy)**를 통해 온도 값을 전송하는 프로그램입니다. 각 센서의 온도 값은 BLE 특성(characteristic)에 등록되어 전송됩니다.

---

### **코드 리뷰 및 개선 사항**

#### **1. 라이브러리 포함**

```cpp
#include <ArduinoBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>
```

- 필요한 라이브러리를 잘 포함하고 있습니다.

#### **2. 온도 센서 설정**

```cpp
const int ONE_WIRE_BUS = 2;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

const int REQUIRED_SENSOR_COUNT = 4; // 필요한 센서 수

DeviceAddress sensorsAddress[REQUIRED_SENSOR_COUNT];
int sensorIDs[REQUIRED_SENSOR_COUNT];
bool sensorValid[REQUIRED_SENSOR_COUNT];
```

- **개선 사항:**
  - `ONE_WIRE_BUS`를 `const byte`로 정의하면 메모리를 조금 더 효율적으로 사용할 수 있습니다.
  - `sensorIDs` 배열은 코드에서 사용되지 않으므로 제거해도 됩니다.
  - 변수명은 **의미를 명확하게** 하는 것이 좋습니다. 예를 들어, `sensorValid`를 `isSensorValid`로 변경하면 읽기 쉽습니다.

#### **3. 시리얼 출력 시간 변수**

```cpp
unsigned long lastSerialTime = 0;
const unsigned long SERIAL_INTERVAL = 2000; // 2초 간격
```

- 주기적으로 시리얼 모니터에 출력하기 위한 설정입니다.

#### **4. BLE 서비스 및 특성 UUID**

```cpp
const char *SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";

// 각 센서별 특성 UUID 선언
const char *CHARACTERISTIC_UUID_0 = "19B10001-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_1 = "19B10002-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_2 = "19B10003-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_3 = "19B10004-E8F2-537E-4F6C-D104768A1214";
```

- **개선 사항:**
  - UUID를 배열로 관리하면 코드가 간결해집니다.

    ```cpp
    const char *CHARACTERISTIC_UUIDS[REQUIRED_SENSOR_COUNT] = {
      "19B10001-E8F2-537E-4F6C-D104768A1214",
      "19B10002-E8F2-537E-4F6C-D104768A1214",
      "19B10003-E8F2-537E-4F6C-D104768A1214",
      "19B10004-E8F2-537E-4F6C-D104768A1214"
    };
    ```

#### **5. BLE 서비스 및 특성 객체 생성**

```cpp
BLEService tempService(SERVICE_UUID);

BLECharacteristic tempCharacteristic0(CHARACTERISTIC_UUID_0, BLERead | BLENotify, 8);
BLECharacteristic tempCharacteristic1(CHARACTERISTIC_UUID_1, BLERead | BLENotify, 8);
BLECharacteristic tempCharacteristic2(CHARACTERISTIC_UUID_2, BLERead | BLENotify, 8);
BLECharacteristic tempCharacteristic3(CHARACTERISTIC_UUID_3, BLERead | BLENotify, 8);

BLECharacteristic *tempCharacteristics[REQUIRED_SENSOR_COUNT] = {&tempCharacteristic0, &tempCharacteristic1, &tempCharacteristic2, &tempCharacteristic3};
```

- **개선 사항:**
  - 동적 할당이나 반복문을 사용하여 객체 생성 코드를 단순화할 수 있습니다.

    ```cpp
    BLECharacteristic *tempCharacteristics[REQUIRED_SENSOR_COUNT];

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
      tempCharacteristics[i] = new BLECharacteristic(CHARACTERISTIC_UUIDS[i], BLERead | BLENotify, 8);
    }
    ```

#### **6. 함수 구현 부분**

- 함수들이 현재 내용이 없습니다.

```cpp
void initializeSensorSystem() {
  Serial.println("센서 초기화 시작");
}

// 다른 함수들도 동일
```

- **개선 사항:**
  - 함수 내부에 실제 로직을 구현해야 합니다.
  - 예를 들어, `initializeSensorSystem()` 함수에서는 센서를 초기화하고 센서 수를 확인하는 코드를 추가합니다.

    ```cpp
    void initializeSensorSystem() {
      ds18b20.begin();
      int sensorCount = ds18b20.getDeviceCount();

      if (sensorCount != REQUIRED_SENSOR_COUNT) {
        Serial.println("센서 수가 일치하지 않습니다.");
      } else {
        Serial.println("센서 초기화 완료");
      }
    }
    ```

#### **7. BLE 초기화 및 준비**

```cpp
void initializeBLE() {
  Serial.println("BLE 초기화 시작");
}
```

- **개선 사항:**
  - BLE 초기화 코드를 추가해야 합니다.

    ```cpp
    void initializeBLE() {
      if (!BLE.begin()) {
        Serial.println("BLE 초기화 실패");
        while (1);
      }

      BLE.setLocalName("TemperatureSensor");
      BLE.setAdvertisedService(tempService);

      for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        tempService.addCharacteristic(*tempCharacteristics[i]);
      }

      BLE.addService(tempService);
      BLE.advertise();

      Serial.println("BLE 초기화 완료, 광고 시작");
    }
    ```

#### **8. 온도 업데이트 함수**

```cpp
void updateBLETemperature() {
  BLE.poll();

  if (BLE.connected()) {
    ds18b20.requestTemperatures();
    char buffer[8];
  }
}
```

- **개선 사항:**
  - 각 센서의 온도 값을 읽어서 BLE 특성에 업데이트하는 코드를 추가해야 합니다.

    ```cpp
    void updateBLETemperature() {
      BLE.poll();

      if (BLE.connected()) {
        ds18b20.requestTemperatures();

        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
          float temperature = ds18b20.getTempCByIndex(i);
          tempCharacteristics[i]->writeValue(temperature);
        }
      }
    }
    ```

#### **9. 시리얼에 센서 정보 업데이트**

```cpp
void updateSerial() {
  if (millis() - lastSerialTime > SERIAL_INTERVAL) {
    lastSerialTime = millis();
    ds18b20.requestTemperatures();
    Serial.println("\n--- 센서 정보 ---");
    Serial.println("순번\tID\t온도\t\t주소\t\t상태");
    Serial.println("----------------------------------------------");
  }
}
```

- **개선 사항:**
  - 센서의 온도 값을 실제로 출력하도록 수정합니다.

    ```cpp
    void updateSerial() {
      if (millis() - lastSerialTime > SERIAL_INTERVAL) {
        lastSerialTime = millis();
        ds18b20.requestTemperatures();

        Serial.println("\n--- 센서 정보 ---");

        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
          float temperature = ds18b20.getTempCByIndex(i);
          Serial.print("센서 ");
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(temperature);
          Serial.println(" °C");
        }
      }
    }
    ```

#### **10. setup() 함수**

```cpp
void setup() {
  Serial.begin(9600);
}
```

- **개선 사항:**
  - 센서와 BLE를 초기화하는 함수를 호출해야 합니다.

    ```cpp
    void setup() {
      Serial.begin(9600);
      initializeSensorSystem();
      initializeBLE();
    }
    ```

#### **11. 메인 루프(loop) 함수**

```cpp
void loop() {
  updateSerial();
  updateBLETemperature();
}
```

- 메인 루프에서는 시리얼 출력과 BLE 업데이트 함수를 호출합니다.

---

### **리팩토링된 코드**

위의 개선 사항을 반영하여 전체 코드를 리팩토링해보았습니다.

```cpp
#include <ArduinoBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ------------- 온도 센서 설정 -------------
const byte ONE_WIRE_BUS = 2;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

const int REQUIRED_SENSOR_COUNT = 4;

DeviceAddress sensorsAddress[REQUIRED_SENSOR_COUNT];
bool isSensorValid[REQUIRED_SENSOR_COUNT];

unsigned long lastSerialTime = 0;
const unsigned long SERIAL_INTERVAL = 2000; // 2초 간격

// BLE 서비스 UUID
const char *SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";

// 각 센서별 특성 UUID 배열
const char *CHARACTERISTIC_UUIDS[REQUIRED_SENSOR_COUNT] = {
  "19B10001-E8F2-537E-4F6C-D104768A1214",
  "19B10002-E8F2-537E-4F6C-D104768A1214",
  "19B10003-E8F2-537E-4F6C-D104768A1214",
  "19B10004-E8F2-537E-4F6C-D104768A1214"
};

// BLE 서비스 및 특성 객체 생성
BLEService tempService(SERVICE_UUID);
BLECharacteristic *tempCharacteristics[REQUIRED_SENSOR_COUNT];

void initializeSensorSystem() {
  ds18b20.begin();
  int sensorCount = ds18b20.getDeviceCount();

  if (sensorCount != REQUIRED_SENSOR_COUNT) {
    Serial.println("센서 수가 일치하지 않습니다.");
  } else {
    Serial.println("센서 초기화 완료");
  }
}

void initializeBLE() {
  if (!BLE.begin()) {
    Serial.println("BLE 초기화 실패");
    while (1);
  }

  BLE.setLocalName("TemperatureSensor");
  BLE.setAdvertisedService(tempService);

  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
    tempCharacteristics[i] = new BLECharacteristic(CHARACTERISTIC_UUIDS[i], BLERead | BLENotify, 8);
    tempService.addCharacteristic(*tempCharacteristics[i]);
  }

  BLE.addService(tempService);
  BLE.advertise();

  Serial.println("BLE 초기화 완료, 광고 시작");
}

void updateBLETemperature() {
  BLE.poll();

  if (BLE.connected()) {
    ds18b20.requestTemperatures();

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
      float temperature = ds18b20.getTempCByIndex(i);
      tempCharacteristics[i]->writeValue(temperature);
    }
  }
}

void updateSerial() {
  if (millis() - lastSerialTime > SERIAL_INTERVAL) {
    lastSerialTime = millis();
    ds18b20.requestTemperatures();

    Serial.println("\n--- 센서 정보 ---");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
      float temperature = ds18b20.getTempCByIndex(i);
      Serial.print("센서 ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(temperature);
      Serial.println(" °C");
    }
  }
}

void setup() {
  Serial.begin(9600);
  initializeSensorSystem();
  initializeBLE();
}

void loop() {
  updateSerial();
  updateBLETemperature();
}
```

---

### **추가 설명**

- **반복문과 배열 사용**: 센서와 BLE 특성에 대한 코드를 반복문으로 처리하여 코드의 길이를 줄이고 가독성을 높였습니다.
- **함수 구현**: 각 함수에 필요한 로직을 추가하여 프로그램이 제대로 동작하도록 했습니다.
- **변수명 개선**: 변수명을 명확히 하여 코드를 읽기 쉽게 만들었습니다.
- **에러 처리**: 센서 수가 일치하지 않는 경우 사용자에게 알리는 코드가 추가되었습니다.
- **메모리 관리**: `const`와 `byte`를 사용하여 메모리를 효율적으로 사용하였습니다.

---

### **결론**

이렇게 리팩토링된 코드는 Arduino 초보자도 이해하기 쉽게 작성되었으며, 유지 보수와 확장이 용이합니다. 각 단계에서 무엇을 하는지 주석을 추가하면 더욱 도움이 될 것입니다.

**추가로 궁금한 점이나 도움이 필요한 부분이 있다면 언제든지 말씀해주세요!**
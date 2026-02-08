## 코드 리뷰 및 리팩토링 제안 (아두이노 초보자 대상)

이 코드는 DS18B20 온도 센서 4개에서 측정한 온도를 BLE를 통해 전송하는 훌륭한 프로젝트입니다. 아두이노 초보자도 쉽게 이해할 수 있도록 코드 리뷰와 함께 리팩토링 요소를 제안해 드립니다.  코드의 주요 목적은 변경하지 않고, 가독성과 유지보수성을 높이는 데 초점을 맞추겠습니다.

### 1. 코드 리뷰 (주석 및 설명 추가)

```c++
#include <ArduinoBLE.h>  // BLE 기능을 사용하기 위한 라이브러리
#include <OneWire.h>     // 1-Wire 통신 (DS18B20 센서)을 위한 라이브러리
#include <DallasTemperature.h> // DS18B20 센서를 쉽게 사용하기 위한 라이브러리

// ------------- 온도 센서 설정 -------------

const int ONE_WIRE_BUS = 2; // DS18B20 센서의 데이터 핀 (아두이노 2번 핀에 연결)

OneWire oneWire(ONE_WIRE_BUS); // OneWire 객체 생성 (센서와 통신하기 위한 객체)

DallasTemperature ds18b20(&oneWire); // DallasTemperature 객체 생성 (센서를 제어하기 위한 객체)

const int REQUIRED_SENSOR_COUNT = 4; // 사용할 센서의 개수 (4개)

DeviceAddress sensorsAddress[REQUIRED_SENSOR_COUNT]; // 각 센서의 고유 주소(8바이트)를 저장할 배열
int sensorIDs[REQUIRED_SENSOR_COUNT];        // 각 센서의 ID (사용자가 지정하는 번호)를 저장할 배열
bool sensorValid[REQUIRED_SENSOR_COUNT];    // 각 센서가 정상적으로 연결되었는지 확인하는 배열 (true: 연결됨, false: 연결 안됨)

// ------------- 시리얼 출력 시간 변수 -------------

unsigned long lastSerialTime = 0;       // 마지막으로 시리얼 통신을 한 시간 저장
const unsigned long SERIAL_INTERVAL = 2000; // 시리얼 통신 간격 (2000ms = 2초)

// BLE 서비스 UUID (Universally Unique Identifier, 고유 식별자)
//  -  서비스와 특성은 UUID를 통해 구분합니다.  UUID는 128비트 숫자로,  다른 장치와 겹치지 않도록 생성됩니다.
const char *SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";

// 각 센서별 특성 UUID 선언 (센서 1, 2, 3, 4에 대한 UUID)
const char *CHARACTERISTIC_UUID_0 = "19B10001-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_1 = "19B10002-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_2 = "19B10003-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_3 = "19B10004-E8F2-537E-4F6C-D104768A1214";

// BLE 서비스 및 특성 객체 생성
BLEService tempService(SERVICE_UUID); // BLE 서비스를 위한 객체 (온도 서비스)

// 각 센서에 대한 BLE 특성 객체 (온도 값을 읽고, 알림을 받을 수 있는 특성)
BLECharacteristic tempCharacteristic0(CHARACTERISTIC_UUID_0, BLERead | BLENotify, 8); // 센서 0
BLECharacteristic tempCharacteristic1(CHARACTERISTIC_UUID_1, BLERead | BLENotify, 8); // 센서 1
BLECharacteristic tempCharacteristic2(CHARACTERISTIC_UUID_2, BLERead | BLENotify, 8); // 센서 2
BLECharacteristic tempCharacteristic3(CHARACTERISTIC_UUID_3, BLERead | BLENotify, 8); // 센서 3

// 각 센서의 특성을 배열로 묶어 관리 (나중에 쉽게 접근하기 위해)
BLECharacteristic *tempCharacteristics[REQUIRED_SENSOR_COUNT] = {&tempCharacteristic0, &tempCharacteristic1, &tempCharacteristic2, &tempCharacteristic3};

// ------------- 센서 초기화 함수 -------------
// 센서 시스템을 초기화하는 함수 (센서 개수 확인, 주소 읽기 등)
void initializeSensorSystem() {
    Serial.println("센서 초기화 시작");
    ds18b20.begin(); // DS18B20 라이브러리 초기화

    // 연결된 센서 개수 확인 및 필요한 개수와 비교
    int deviceCount = ds18b20.getDeviceCount();
    Serial.print("연결된 센서 개수: ");
    Serial.println(deviceCount);

    if (deviceCount < REQUIRED_SENSOR_COUNT) {
        Serial.println("필요한 센서 개수가 부족합니다.");
        //  여기서 에러 처리를 할 수 있습니다 (예: LED 깜빡임, 무한 루프).
        while (true); // 에러 발생 시 프로그램 정지 (무한 루프)
    }

    // 센서 유효성 초기화
    for(int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
        sensorValid[i] = false; // 초기에는 모든 센서를 유효하지 않다고 설정
    }
}
// ------------- 센서 주소 읽기 함수 -------------
// 각 센서의 고유 주소를 읽어와서 sensorsAddress 배열에 저장
void readSensorAddresses() {
    Serial.println("센서 주소 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
       if (ds18b20.getAddress(sensorsAddress[i], i)) { // 센서 주소 읽기 시도
           Serial.print("센서 ");
           Serial.print(i);
           Serial.print(" 주소: ");
           // 주소를 16진수로 출력 (예: 28FF437A8016042E)
           for (int j = 0; j < 8; j++) {
               if (sensorsAddress[i][j] < 16) Serial.print("0"); // 한 자리 수면 앞에 0 붙임
               Serial.print(sensorsAddress[i][j], HEX);
           }
            Serial.println();
           sensorValid[i] = true;
        } else {
            Serial.print("센서 ");
            Serial.print(i);
            Serial.println(" 주소를 읽을 수 없습니다.");
            sensorValid[i] = false;
        }
    }
}
// ------------- 센서 ID 읽기 함수 -------------
// 이 예제에서는 센서 ID를 주소의 마지막 바이트로 설정합니다.  (센서 구분 용도)
void readSensorIDs() {
    Serial.println("센서 ID 읽기 시작");
    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        if(sensorValid[i])
        {
        sensorIDs[i] = sensorsAddress[i][7]; // 주소의 마지막 바이트를 ID로 사용
        Serial.print("센서 ");
        Serial.print(i);
        Serial.print(" ID: ");
        Serial.println(sensorIDs[i]);
        }
    }
}

// ------------- 센서 정렬 함수 -------------
// 센서 ID를 기준으로 오름차순 정렬 (선택 정렬 알고리즘 사용)
//  -  센서를 물리적으로 정렬하기 어려울 때,  소프트웨어적으로 순서를 정할 수 있습니다.
void sortSensors() {
    Serial.println("센서 정렬 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT - 1; i++) {
        for (int j = i + 1; j < REQUIRED_SENSOR_COUNT; j++) {
            if (sensorIDs[i] > sensorIDs[j]) {
                // ID 값 교환 (swap)
                int tempID = sensorIDs[i];
                sensorIDs[i] = sensorIDs[j];
                sensorIDs[j] = tempID;

                // 주소 교환
                DeviceAddress tempAddress;
                memcpy(tempAddress, sensorsAddress[i], sizeof(DeviceAddress)); // 임시 변수에 복사
                memcpy(sensorsAddress[i], sensorsAddress[j], sizeof(DeviceAddress));
                memcpy(sensorsAddress[j], tempAddress, sizeof(DeviceAddress));

                 // sensorValid 상태 교환
                bool tempValid = sensorValid[i];
                sensorValid[i] = sensorValid[j];
                sensorValid[j] = tempValid;
            }
        }
    }
     Serial.println("센서 정렬 완료");
}
// ------------- BLE 초기화 및 준비 -------------

void initializeBLE() {
    Serial.println("BLE 초기화 시작");

    if (!BLE.begin()) { // BLE 초기화
        Serial.println("BLE 초기화 실패!");
        while (1); //  BLE 초기화 실패하면 여기서 정지 (무한 루프)
    }

    // BLE 장치 이름 설정 (다른 장치에서 이 장치를 찾을 때 사용)
    BLE.setLocalName("Nano 33 BLE Sense Temp"); // 예시 이름 (변경 가능)
    BLE.setAdvertisedService(tempService); //  이 장치가 제공하는 서비스(온도 서비스)를 알림

    // 특성들을 서비스에 추가
    for(int i = 0; i< REQUIRED_SENSOR_COUNT; i++)
    {
        tempService.addCharacteristic(*tempCharacteristics[i]);
    }

    BLE.addService(tempService); // 서비스를 BLE 스택에 추가

      // 초기값 설정 (센서 연결 전)
    for(int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
      tempCharacteristics[i]->writeValue("N/A"); //"N/A" Not Available
    }


    BLE.advertise(); //  BLE 연결을 위한 광고 시작 (다른 장치가 이 장치를 찾을 수 있도록)
    Serial.println("BLE 준비 완료. 연결 대기 중...");
}

// ------------- BLE 이벤트 폴링 및 온도 업데이트 함수 -------------
// BLE 이벤트를 처리하고, 연결된 장치에 온도 값을 보냅니다.
void updateBLETemperature() {
    BLE.poll(); // BLE 이벤트 처리 (연결, 연결 해제, 데이터 수신 등)

    if (BLE.connected()) { //  BLE 장치가 연결된 경우
        // 모든 센서의 온도를 한 번에 요청
        ds18b20.requestTemperatures();
        char buffer[8]; // 온도 값을 문자열로 저장하기 위한 버퍼

        // 각 센서의 온도를 읽어서 BLE 특성에 쓰기
        for(int i = 0 ; i < REQUIRED_SENSOR_COUNT; i++)
        {
            if(sensorValid[i])
            {
            float temperatureC = ds18b20.getTempC(sensorsAddress[i]); // 섭씨 온도 읽기
            // 온도를 문자열로 변환하여 버퍼에 저장
            dtostrf(temperatureC, 1, 2, buffer); // dtostrf(float 값, 최소 너비, 소수점 이하 자릿수, 저장할 버퍼)

            // BLE 특성에 온도 값 쓰기 (연결된 장치로 전송)
            tempCharacteristics[i]->writeValue(buffer);

             Serial.print("센서 ");
             Serial.print(i);
             Serial.print(" 온도: ");
             Serial.print(buffer); // 시리얼 모니터에도 출력
             Serial.println(" °C");
            }
        }
    }
}
// ------------- 시리얼에 센서 정보 업데이트 함수 -------------

void updateSerial() {
    // 시리얼 통신 간격(SERIAL_INTERVAL)이 지났는지 확인
    if (millis() - lastSerialTime > SERIAL_INTERVAL) {
        lastSerialTime = millis(); // 현재 시간으로 업데이트

        // 모든 센서의 온도를 한 번에 요청
        ds18b20.requestTemperatures();

        Serial.println("\n--- 센서 정보 ---");
        Serial.println("순번\tID\t온도\t\t주소\t\t상태");
        Serial.println("----------------------------------------------");

        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
            Serial.print(i); // 순번 출력
            Serial.print("\t");
            Serial.print(sensorIDs[i]); // 센서 ID 출력
            Serial.print("\t");

            if (sensorValid[i]) {
                // 섭씨 온도 읽기
                float tempC = ds18b20.getTempC(sensorsAddress[i]);
                 // 화씨 온도 읽기 (필요하면 사용)
                float tempF = ds18b20.getTempF(sensorsAddress[i]);

                Serial.print(tempC); // 섭씨 온도 출력
                Serial.print(" °C\t");
                // Serial.print(tempF); // 화씨 온도 출력 (필요하면 사용)
                // Serial.print(" °F\t");

            } else {
                Serial.print("N/A\t\t"); // 센서가 유효하지 않으면 "N/A" 출력
            }


             // 주소 출력
            for (int j = 0; j < 8; j++) {
                if (sensorsAddress[i][j] < 16) Serial.print("0"); // 한 자리 수면 앞에 0 붙임
                Serial.print(sensorsAddress[i][j], HEX); // 16진수로 출력
            }
            Serial.print("\t");

            //센서 상태
            if(sensorValid[i])
            {
            Serial.println("정상"); // 센서 상태 출력 (정상 또는 오류)
            }
            else
            {
            Serial.println("오류"); // 센서 상태 출력 (정상 또는 오류)
            }
        }
    }
}

// ------------- setup 함수 -------------
// 아두이노 보드가 시작될 때 한 번 실행되는 함수
void setup() {
    Serial.begin(9600); // 시리얼 통신 시작 (9600 bps)
    //while (!Serial); // 시리얼 연결 대기 (USB 시리얼 사용 시)

    initializeSensorSystem(); // 센서 시스템 초기화
    readSensorAddresses();    // 센서 주소 읽기
    readSensorIDs();          // 센서 ID 읽기
    sortSensors();           // 센서 정렬
    initializeBLE();         // BLE 초기화
}

// ------------- loop 함수 -------------
// setup 함수가 끝난 후 계속 반복 실행되는 함수
void loop() {
    updateSerial();        // 시리얼 통신으로 센서 정보 업데이트
    updateBLETemperature(); // BLE를 통해 온도 전송
}
```

**주요 변경 사항:**

*   **상세한 주석:**  각 코드 블록과 변수에 대한 설명을 추가하여 코드의 기능을 쉽게 이해할 수 있도록 했습니다.
*   **함수 설명:** 각 함수가 어떤 역할을 하는지 명확하게 설명했습니다.
*   **변수명 명확화:**  변수명을 좀 더 직관적으로 변경했습니다 (예: `deviceCount`, `sensorValid`).
*   **에러 처리:** 센서 개수가 부족할 경우 프로그램을 정지시키는 에러 처리 코드를 추가했습니다.
*  **센서 유효성 검사**: `sensorValid` 배열을 사용하여, 센서가 올바르게 연결되었는지 확인하고, 연결되지 않은 센서에 대해서는 "N/A"를 출력하도록 처리했습니다.
*   **BLE 초기값:** BLE 연결 전에 특성의 초기값을 "N/A"로 설정하여, 연결 전 잘못된 값이 표시되지 않도록 했습니다.
*  **코드 내 출력문 추가**: BLE 특성에 값을 쓰는 부분(`tempCharacteristics[i]->writeValue(buffer);`)에서 `Serial.print()` 문을 추가하여, 현재 전송되는 온도 값을 시리얼 모니터에서 확인할 수 있게 하였습니다.

### 2. 리팩토링 제안

1.  **`#define` 대신 `const` 사용:**

    ```c++
    // #define ONE_WIRE_BUS 2  //  이전 방식
    const int ONE_WIRE_BUS = 2; // 더 현대적인 C++ 방식 (타입 안전성)
    ```

    `#define`은 전처리기에 의해 처리되므로 타입 검사가 이루어지지 않습니다. `const`를 사용하면 컴파일러가 타입 검사를 수행하여 더 안전한 코드를 작성할 수 있습니다.

2.  **센서 ID를 위한 `enum` 사용 (선택 사항):**

    ```c++
    // 센서 ID를 좀 더 명확하게 관리 (선택 사항)
    enum SensorID {
        SENSOR_0, // 0
        SENSOR_1, // 1
        SENSOR_2, // 2
        SENSOR_3  // 3
    };

    // ...

    // 사용 예시
    sensorIDs[SENSOR_0] = sensorsAddress[SENSOR_0][7]; // 가독성이 좋아짐
    ```

    `enum`을 사용하면 숫자 대신 의미 있는 이름을 사용할 수 있어 코드의 가독성이 향상됩니다.

3.  **센서 정보 구조체 (선택 사항):**

    ```c++
    // 센서 정보를 묶어서 관리하는 구조체 (선택 사항)
    struct SensorData {
        DeviceAddress address;
        int id;
        bool isValid;
        float temperatureC; //  온도 값을 바로 저장 (계산된 값)
        // float temperatureF; // 화씨 온도 (필요한 경우)
    };

    SensorData sensors[REQUIRED_SENSOR_COUNT]; // 구조체 배열

    // ... 사용 예시 (updateBLETemperature 함수 내에서)
    sensors[i].temperatureC = ds18b20.getTempC(sensors[i].address);
    dtostrf(sensors[i].temperatureC, 1, 2, buffer);

    ```

    구조체를 사용하면 관련된 센서 데이터를 하나로 묶어 관리할 수 있어 코드가 더 깔끔해집니다. 또한, 계산된 온도 값을 구조체에 바로 저장하여 반복적인 온도 계산을 줄일 수 있습니다.

4. **`updateSerial()` 함수에서 `ds18b20.requestTemperatures()` 호출 제거(중복 제거)**
    * `updateBLETemperature()` 함수에서도 `ds18b20.requestTemperatures()`를 호출하고 있으므로 `updateSerial()`에서는 제거하는 것이 좋습니다. BLE 연결이 없을 때만 시리얼로 출력하도록 수정하는 것이 더 효율적입니다.

    ```c++
    void updateSerial() {
        if (millis() - lastSerialTime > SERIAL_INTERVAL) {
            lastSerialTime = millis();

            // BLE 연결이 없을 때만 온도 요청 및 시리얼 출력
            if (!BLE.connected()) {
                ds18b20.requestTemperatures();

                // ... (나머지 시리얼 출력 코드는 동일) ...
            }
        }
    }

    void updateBLETemperature() {
        BLE.poll();

        if (BLE.connected()) {
            ds18b20.requestTemperatures(); // 여기서는 반드시 필요
            // ... (나머지 코드는 동일) ...
        }
    }
    ```

5. **매직 넘버 제거:**
    * 코드 내에 직접적으로 사용되는 숫자들(매직 넘버)을 상수로 정의하여 가독성과 유지보수성을 높입니다.

    ```c++
    const int SENSOR_ADDRESS_LENGTH = 8; // 센서 주소 길이
    // ...

    for (int j = 0; j < SENSOR_ADDRESS_LENGTH; j++) { //  8 대신 SENSOR_ADDRESS_LENGTH
       // ...
    }
    ```

6. **함수 분리 (선택 사항):**
    * `updateBLETemperature()` 함수가 너무 길다면, 온도 읽기, BLE 특성 업데이트 부분을 별도의 함수로 분리하는 것을 고려해볼 수 있습니다.
    ```c++
    // 온도 읽기 함수
    void readTemperatures() {
        ds18b20.requestTemperatures();
         for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
            if (sensorValid[i]) {
                sensors[i].temperatureC = ds18b20.getTempC(sensors[i].address);
            }
        }
    }

    // BLE 특성 업데이트 함수
    void updateBLECharacteristics() {
       char buffer[8];
        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
            if (sensorValid[i]) {
                 dtostrf(sensors[i].temperatureC, 1, 2, buffer);
                tempCharacteristics[i]->writeValue(buffer);
            }
        }
    }

    void updateBLETemperature() {
      BLE.poll();
      if (BLE.connected()) {
        readTemperatures();        // 온도 읽기
        updateBLECharacteristics(); // BLE 특성 업데이트
      }
    }
    ```

이러한 리팩토링 제안을 통해 코드를 더 이해하기 쉽고, 관리하기 편하게 만들 수 있습니다.  초보자라면 한 번에 모든 것을 변경하기보다는,  하나씩 적용해 보면서 코드의 변화를 이해하는 것이 좋습니다.

# 아두이노 코드 리뷰 및 리팩토링 제안

이 코드는 DS18B20 온도 센서 4개를 사용하여 온도를 측정하고 BLE를 통해 전송하는 프로그램입니다. 초보자 관점에서 이해하기 쉽도록 코드를 리뷰하고 리팩토링 요소를 제안해 드리겠습니다.

## 주요 관찰 사항

1. **비어있는 함수들**: 여러 함수(`initializeSensorSystem`, `readSensorAddresses` 등)가 선언되었지만 내용이 비어 있습니다.
2. **불완전한 코드 구조**: `updateBLETemperature` 함수가 중괄호로 제대로 닫히지 않았습니다.
3. **setup 함수의 불완전성**: `setup` 함수에서 센서와 BLE 초기화가 누락되었습니다.

## 리팩토링 제안

### 1. 비어있는 함수 구현하기

```cpp
// ------------- 센서 초기화 함수 -------------
void initializeSensorSystem() {
  Serial.println("센서 초기화 시작");
  ds18b20.begin();  // DS18B20 센서 시작

  // 시스템 상에 존재하는 센서 수 확인
  int sensorCount = ds18b20.getDeviceCount();
  Serial.print("발견된 센서 수: ");
  Serial.println(sensorCount);

  if (sensorCount < REQUIRED_SENSOR_COUNT) {
    Serial.println("경고: 필요한 센서 수보다 적은 센서가 발견되었습니다.");
  }
}

// ------------- 센서 주소 읽기 함수 -------------
void readSensorAddresses() {
  Serial.println("센서 주소 읽기 시작");
  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
    if (ds18b20.getAddress(sensorsAddress[i], i)) {
      sensorValid[i] = true;
      Serial.print("센서 ");
      Serial.print(i);
      Serial.print(" 주소: ");
      printAddress(sensorsAddress[i]);
      Serial.println();
    } else {
      sensorValid[i] = false;
      Serial.print("센서 ");
      Serial.print(i);
      Serial.println(" 주소를 읽을 수 없습니다!");
    }
  }
}

// 주소 출력을 위한 도우미 함수
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// ------------- 센서 ID 읽기 함수 -------------
void readSensorIDs() {
  Serial.println("센서 ID 읽기 시작");
  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
    if (sensorValid[i]) {
      // 센서 ID를 주소의 마지막 두 바이트로 설정 (예시 목적)
      sensorIDs[i] = (sensorsAddress[i][6] << 8) | sensorsAddress[i][7];
      Serial.print("센서 ");
      Serial.print(i);
      Serial.print(" ID: ");
      Serial.println(sensorIDs[i]);
    } else {
      sensorIDs[i] = -1; // 유효하지 않은 ID
    }
  }
}
```

### 2. 불완전한 함수 완성

```cpp
// ------------- BLE 이벤트 폴링 및 온도 업데이트 함수 -------------
void updateBLETemperature() {
  BLE.poll();
  if (BLE.connected()) {
    ds18b20.requestTemperatures();
    char buffer[8];

    // 각 센서별로 온도 업데이트
    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
      if (sensorValid[i]) {
        float tempC = ds18b20.getTempC(sensorsAddress[i]);
        if (tempC != DEVICE_DISCONNECTED_C) {
          sprintf(buffer, "%.2f", tempC);
          tempCharacteristics[i]->writeValue(buffer);
        }
      }
    }
  }
}

// ------------- 시리얼에 센서 정보 업데이트 함수 -------------
void updateSerial() {
  if (millis() - lastSerialTime > SERIAL_INTERVAL) {
    lastSerialTime = millis();
    ds18b20.requestTemperatures();

    Serial.println("\n--- 센서 정보 ---");
    Serial.println("순번\tID\t온도\t\t주소\t\t상태");
    Serial.println("----------------------------------------------");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
      Serial.print(i);
      Serial.print("\t");

      if (sensorValid[i]) {
        float tempC = ds18b20.getTempC(sensorsAddress[i]);

        // ID 출력
        Serial.print(sensorIDs[i]);
        Serial.print("\t");

        // 온도 출력
        if (tempC != DEVICE_DISCONNECTED_C) {
          Serial.print(tempC);
          Serial.print("°C\t");
        } else {
          Serial.print("오류\t\t");
        }

        // 주소 출력
        printAddress(sensorsAddress[i]);
        Serial.print("\t");

        // 상태 출력
        Serial.println("정상");
      } else {
        Serial.println("-\t-\t\t-\t\t연결안됨");
      }
    }
  }
}
```

### 3. `setup` 및 `loop` 함수 완성

```cpp
// ------------- setup 함수 -------------
void setup() {
  Serial.begin(9600);
  while (!Serial);  // 시리얼 연결 대기

  Serial.println("DS18B20 온도 센서 & BLE 시작하는 중...");

  // 센서 초기화
  initializeSensorSystem();
  readSensorAddresses();
  readSensorIDs();
  sortSensors();

  // BLE 초기화
  initializeBLE();
}

// ------------- BLE 초기화 및 준비 -------------
void initializeBLE() {
  Serial.println("BLE 초기화 시작");

  if (!BLE.begin()) {
    Serial.println("BLE 시작 실패!");
    while (1);
  }

  // BLE 장치 이름 설정
  BLE.setLocalName("TempMonitor");
  BLE.setAdvertisedService(tempService);

  // 서비스에 특성 추가
  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
    tempService.addCharacteristic(*tempCharacteristics[i]);
  }

  // BLE에 서비스 추가
  BLE.addService(tempService);

  // 초기 특성 값 설정
  for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
    tempCharacteristics[i]->writeValue("0.0");
  }

  // 광고 시작
  BLE.advertise();
  Serial.println("BLE 서버 시작. 연결 대기 중...");
}

// ------------- 센서 정렬 함수 -------------
void sortSensors() {
  Serial.println("센서 정렬 시작");
  // 센서 ID에 따라 정렬하는 간단한 버블 정렬 (필요한 경우)
  for (int i = 0; i < REQUIRED_SENSOR_COUNT - 1; i++) {
    for (int j = 0; j < REQUIRED_SENSOR_COUNT - i - 1; j++) {
      if (sensorIDs[j] > sensorIDs[j + 1]) {
        // ID 교환
        int tempID = sensorIDs[j];
        sensorIDs[j] = sensorIDs[j + 1];
        sensorIDs[j + 1] = tempID;

        // 주소 교환
        DeviceAddress tempAddr;
        memcpy(tempAddr, sensorsAddress[j], sizeof(DeviceAddress));
        memcpy(sensorsAddress[j], sensorsAddress[j + 1], sizeof(DeviceAddress));
        memcpy(sensorsAddress[j + 1], tempAddr, sizeof(DeviceAddress));

        // 유효성 플래그 교환
        bool tempValid = sensorValid[j];
        sensorValid[j] = sensorValid[j + 1];
        sensorValid[j + 1] = tempValid;
      }
    }
  }

  Serial.println("센서 정렬 완료");
}
```

## 리팩토링 요소 요약 (초보자 친화적)

1. **코드 문서화 추가**
   - 각 함수의 목적과 동작 방식에 대한 주석 추가
   - 중요 변수와 상수에 대한 설명 추가

2. **에러 처리 개선**
   - 센서 연결 실패 시 명확한 에러 메시지 표시
   - BLE 연결 상태 확인 및 오류 표시

3. **초기화 보완**
   - `setup()` 함수에서 모든 초기화 함수 호출 추가
   - 센서와 BLE 모두 적절히 초기화되도록 개선

4. **가독성 향상**
   - 함수 이름을 더 직관적으로 변경 고려
   - 들여쓰기와 코드 포맷팅 일관성 유지

5. **코드 모듈화 개선**
   - 온도 측정과 BLE 통신 부분 명확히 분리
   - 도우미 함수 추가 (예: `printAddress()`)

## 결론

위 리팩토링을 적용하면 코드의 구조가 명확해지고 초보자도 이해하기 쉬운 형태가 됩니다. 특히 비어있던 함수들이 제대로 구현되어 코드의 목적을 달성할 수 있게 됩니다. 또한 에러 처리와 디버깅 출력이 개선되어 문제 발생 시 대응이 용이해집니다.
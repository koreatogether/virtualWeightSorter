이 코드는 DS18B20 온도 센서와 BLE(Bluetooth Low Energy)를 사용하여 온도 값을 게시하는 기능을 구현하고 있으며, 추가로 LCD와 버튼을 사용합니다. 아두이노 초보자 입장에서 코드를 이해하기 쉽고 유지보수하기 쉽게 만들기 위해, 코드 리뷰와 리팩토링 요소를 아래와 같이 정리했습니다.
코드 리뷰
먼저 주어진 코드를 분석하여 현재 코드의 구조와 동작 방식을 살펴보겠습니다. 이 코드는 센서 초기화, 주소 및 ID 읽기, 정렬, BLE 설정, 온도 전송, LCD 표시 등 여러 기능을 포함하고 있습니다. 각 주요 함수를 중심으로 리뷰합니다.
1. 센서 초기화 함수 (initializeSensorSystem)
기능: DS18B20 센서를 초기화하고, 필요한 센서 수를 확인하며, 센서 주소와 ID를 읽고 정렬합니다.
장점: 센서 초기화 과정을 한 함수에서 관리하며, 각 단계(주소 읽기, ID 읽기, 정렬)를 별도의 함수로 분리하여 가독성을 높였습니다.
문제점:
함수가 여러 단계를 포함하고 있어 길고 복잡해 보일 수 있습니다.
센서 수가 부족하거나 감지되지 않을 때 LCD와 시리얼에 경고를 표시하지만, 더 자세한 에러 처리가 필요합니다.
2. 센서 주소 읽기 함수 (readSensorAddresses)
기능: ds18b20.getAddress를 사용하여 센서의 주소를 읽습니다.
장점: 센서 주소 읽기 실패 시 오류 메시지를 출력하여 디버깅에 도움이 됩니다.
문제점: 센서 주소 읽기 실패 시 추가적인 에러 처리가 부족합니다 (예: 실패한 센서를 어떻게 처리할지 명확하지 않음).
3. 센서 ID 읽기 함수 (readSensorIDs)
기능: ds18b20.getUserData를 사용하여 센서의 ID를 읽습니다.
장점: 중복 ID를 확인합니다.
문제점:
중복 ID가 있을 경우 경고만 출력하고 별다른 처리가 없습니다.
중복 ID를 가진 센서를 어떻게 처리할지 명확하지 않습니다 (예: 무시하거나 사용자에게 알림을 제공).
4. 센서 정렬 함수 (sortSensors)
기능: 센서 ID를 기준으로 센서를 정렬합니다.
장점: 버블 정렬을 사용하여 구현이 간단합니다.
문제점:
버블 정렬은 센서 수가 적을 때는 괜찮지만, 센서 수가 많아지면 비효율적일 수 있습니다.
현재 센서 수는 4개로 적으므로 큰 문제가 되지 않지만, 더 효율적인 알고리즘을 고려할 수 있습니다.
5. BLE 초기화 함수 (initializeBLE)
기능: BLE를 시작하고, 서비스와 특성을 설정하며, 광고를 시작합니다.
장점: BLE 설정이 명확하게 구현되어 있습니다.
문제점: BLE 초기화 실패 시 더 자세한 에러 메시지가 필요할 수 있습니다.
6. BLE 이벤트 폴링 및 온도 업데이트 함수 (updateBLETemperature)
기능: BLE가 연결된 상태에서 센서 온도를 읽고, 각 특성에 온도 값을 전송합니다.
장점: 온도 값이 유효하지 않을 경우 "ERR"를 전송하여 에러를 표시합니다.
문제점:
온도 읽기와 전송 코드가 반복적으로 나타나며, 중복 코드를 줄일 여지가 있습니다.
에러 처리("ERR" 표시)가 단순하며, 더 자세한 정보를 제공할 수 있습니다.
7. 시리얼에 센서 정보 업데이트 함수 (updateSerial)
기능: 2초 간격으로 센서 정보를 시리얼에 출력합니다 (순번, ID, 온도, 주소, 상태).
장점: 디버깅에 유용하며, 센서 상태를 한눈에 확인할 수 있습니다.
문제점:
현재 주석 처리되어 있어 디버깅 시 활성화해야 합니다.
에러 상태를 "ERR"로 표시하지만, 더 구체적인 에러 정보를 제공할 수 있습니다.
8. LCD 업데이트 함수 (updateLCDDisplay)
기능: 버튼이 눌리면 LCD를 켜고, 20초 후에 끕니다. LCD에 센서 ID, 온도, 상태를 표시합니다.
장점: 버튼 입력에 따라 LCD를 켜고 끄는 로직이 잘 구현되었습니다.
문제점:
함수가 길고 복잡하며, 여러 기능을 한꺼번에 처리합니다.
센서 정보 표시 부분이 반복적이며, 더 작은 함수로 나눌 수 있습니다.
리팩토링 요소
초보자가 코드를 쉽게 이해하고 유지보수할 수 있도록 하기 위해 아래 리팩토링 요소를 제안합니다.
1. 센서 ID 중복 처리
현재 문제: 중복 ID가 있을 경우 경고만 출력하고 별다른 처리가 없습니다.
개선 방안:
중복 ID를 가진 센서를 무시하거나, 사용자에게 알림을 제공하는 로직을 추가합니다.
예를 들어, 중복 ID가 있는 경우 LCD에 경고 메시지를 표시하거나, 해당 센서를 사용하지 않도록 설정할 수 있습니다.
예시:
```cpp
void readSensorIDs() {
    // 기존 코드
    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        if (sensorValid[i]) {
            sensorIDs[i] = ds18b20.getUserData(sensorsAddress[i]);
            for (int j = 0; j < i; j++) {
                if (sensorValid[j] && sensorIDs[i] == sensorIDs[j]) {
                    Serial.print("[Error] 중복 ID: ");
                    Serial.println(sensorIDs[i]);
                    sensorValid[i] = false; // 중복된 센서를 무효화
                    lcd.clear();
                    lcd.print("Duplicate ID: ");
                    lcd.print(sensorIDs[i]);
                }
            }
        }
    }
}
2. 온도 읽기 및 전송 함수 추출
현재 문제: updateBLETemperature 함수에서 온도를 읽고 전송하는 코드가 반복적입니다.
개선 방안:
온도 읽기와 전송 로직을 별도의 함수로 추출하여 중복 코드를 줄입니다.
예시:
cpp
void readAndSendTemperature(int index, BLECharacteristic* characteristic) {
    char buffer[8];
    if (!sensorValid[index]) {
        sprintf(buffer, "ERR");
    } else {
        float tempC = ds18b20.getTempC(sensorsAddress[index]);
        if (tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125) {
            sprintf(buffer, "ERR");
        } else {
            sprintf(buffer, "%0.1f", tempC);
        }
    }
    characteristic->writeValue(buffer);
}

void updateBLETemperature() {
    BLE.poll();
    if (BLE.connected()) {
        ds18b20.requestTemperatures();
        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
            readAndSendTemperature(i, tempCharacteristics[i]);
        }
    }
}
3. LCD 표시 함수 분리
현재 문제: updateLCDDisplay 함수가 길고 여러 기능을 포함하고 있어 복잡합니다.
개선 방안:
버튼 처리, LCD 켜기/끄기, 센서 정보 표시를 별도의 함수로 나눕니다.
예시:
cpp
void handleButtonPress() {
    button.loop();
    if (button.isPressed()) {
        Serial.println("Button pressed");
        lcdOnTime = millis();
        isFirstLCDRun = true;
    }
}

void updateLCDBacklight(unsigned long currentMillis) {
    if (currentMillis - lcdOnTime < LCD_TIMEOUT) {
        lcd.setBacklight(255);
        if (isFirstLCDRun) {
            lcd.clear();
            isFirstLCDRun = false;
        }
    } else {
        lcd.setBacklight(0);
        lcd.clear();
    }
}

void displaySensorInfo() {
    ds18b20.requestTemperatures();
    for (int i = 0; i < REQUIRED_SENSOR_COUNT && i < LCD_ROWS; i++) {
        lcd.setCursor(0, i);
        lcd.print("ID:");
        lcd.print(sensorValid[i] ? sensorIDs[i] : -1);
        // 온도 표시 로직 (기존 코드 사용)
    }
}

void updateLCDDisplay() {
    unsigned long currentMillis = millis();
    handleButtonPress();
    updateLCDBacklight(currentMillis);
    if (currentMillis - lcdOnTime < LCD_TIMEOUT) {
        displaySensorInfo();
    }
}
4. 에러 처리 개선
현재 문제: 센서 감지 실패, 온도 값 오류 등에 대한 처리가 단순합니다("ERR" 표시).
개선 방안:
더 구체적인 에러 메시지를 LCD와 시리얼에 출력합니다.
예를 들어, 센서 주소 읽기 실패 시 실패 이유를 명확히 표시하거나, 온도 값 오류 시 가능한 원인을 설명합니다.
예시:
cpp
void readSensorAddresses() {
    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        sensorValid[i] = ds18b20.getAddress(sensorsAddress[i], i);
        if (!sensorValid[i]) {
            Serial.print("[Error] 센서 ");
            Serial.print(i);
            Serial.println("의 주소 읽기 실패: 센서가 연결되지 않았거나 손상됨");
            lcd.clear();
            lcd.print("Sensor ");
            lcd.print(i);
            lcd.print(" address fail");
        }
    }
}
5. 코드 구조 개선
현재 문제: 일부 함수가 길고 복잡하여 가독성이 떨어집니다.
개선 방안:
함수를 더 작은 단위로 나누어 가독성을 높입니다.
예를 들어, initializeSensorSystem 함수를 더 세부적인 함수로 나눌 수 있습니다.
예시:
cpp
void checkSensorCount() {
    ds18b20.begin();
    uint8_t deviceCount = ds18b20.getDeviceCount();
    if (deviceCount == 0) {
        lcd.clear();
        lcd.print("No sensors!");
        Serial.println("[Error] 센서가 감지되지 않음");
        while (1);
    }
    if (deviceCount < REQUIRED_SENSOR_COUNT) {
        lcd.clear();
        lcd.print("Insufficient sensors:");
        Serial.println("[Warning] 설정 센서 수보다 감지된 센서수 적음");
    }
}

void initializeSensorSystem() {
    lcd.clear();
    lcd.print("Initializing sensors...");
    checkSensorCount();
    readSensorAddresses();
    readSensorIDs();
    sortSensors();
    lcd.clear();
    lcd.print("Initialization complete");
}
6. 주석 추가
현재 문제: 코드에 주석이 있지만, 초보자가 이해하기 어렵거나 부족한 부분이 있습니다.
개선 방안:
각 함수와 주요 코드 블록에 대해 자세한 주석을 추가합니다.
예시:
cpp
// 센서 주소를 읽어 배열에 저장하는 함수
// 센서 주소 읽기 실패 시 오류 메시지를 출력
void readSensorAddresses() {
    lcd.clear();
    lcd.print("Reading addresses...");
    Serial.println("센서 주소 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        // 센서 주소 읽기 시도
        sensorValid[i] = ds18b20.getAddress(sensorsAddress[i], i);
        if (!sensorValid[i]) {
            Serial.print("[Error] 센서 ");
            Serial.print(i);
            Serial.println("의 주소 읽기 실패");
        }
    }
}
```
요약
이 코드는 DS18B20 온도 센서와 BLE를 사용하여 온도 값을 게시하며, LCD와 버튼을 추가로 사용합니다. 초보자가 이해하기 쉽고 유지보수하기 쉽게 만들기 위해 아래와 같은 리팩토링을 제안합니다:
센서 ID 중복 처리: 중복 ID를 무시하거나 사용자에게 알림 제공.
온도 읽기 및 전송 함수 추출: 중복 코드를 줄이기 위해 별도의 함수로 분리.
LCD 표시 함수 분리: 버튼 처리, LCD 켜기/끄기, 정보 표시를 별도의 함수로 나눔.
에러 처리 개선: 더 구체적인 에러 메시지를 LCD와 시리얼에 출력.
코드 구조 개선: 함수를 더 작은 단위로 나누어 가독성 향상.
주석 추가: 자세한 주석을 추가하여 초보자가 이해하기 쉽게 만듦.
이러한 리팩토링을 통해 코드의 가독성과 유지보수성을 높일 수 있으며, 초보자가 더 쉽게 코드를 이해하고 수정할 수 있습니다.
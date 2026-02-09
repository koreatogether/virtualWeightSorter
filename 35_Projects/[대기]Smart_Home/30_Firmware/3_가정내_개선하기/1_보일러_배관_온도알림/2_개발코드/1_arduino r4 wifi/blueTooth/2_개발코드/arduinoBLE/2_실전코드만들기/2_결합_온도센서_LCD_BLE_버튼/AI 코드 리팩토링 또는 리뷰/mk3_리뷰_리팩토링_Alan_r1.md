아두이노 초보자를 위한 코드 리뷰와 리팩토링 요소를 다음과 같이 정리해 보겠습니다:
1. 코드 구조 개선
코드가 길고 복잡해 보일 수 있으므로, 함수와 변수 이름을 더 직관적으로 변경하고, 주석을 추가하여 가독성을 높이는 것이 좋습니다.
2. 함수 분리 및 재사용성 향상
코드 내에서 반복되는 부분을 함수로 분리하여 재사용성을 높이고, 코드의 길이를 줄일 수 있습니다.
3. 에러 처리 개선
센서와 BLE 초기화 과정에서 발생할 수 있는 에러를 더 명확하게 처리하고, 사용자에게 알릴 수 있도록 개선합니다.
4. 매직 넘버 제거
코드 내의 매직 넘버를 상수로 정의하여 가독성을 높이고, 유지보수를 쉽게 합니다.
5. LCD 업데이트 최적화
LCD 업데이트 함수에서 중복되는 코드를 줄이고, 효율적으로 동작하도록 개선합니다.
리팩토링된 코드 예시


``` cpp
#include <ArduinoBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>
#include <ezButton.h>

// 상수 정의
const int ONE_WIRE_BUS = 2;
const int REQUIRED_SENSOR_COUNT = 4;
const int SERIAL_INTERVAL = 2000;
const int I2C_ADDR = 0x27;
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
const int BUTTON_PIN = 3;
const unsigned long LCD_TIMEOUT = 20000;

const char *SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUIDS[REQUIRED_SENSOR_COUNT] = {
    "19B10001-E8F2-537E-4F6C-D104768A1214",
    "19B10002-E8F2-537E-4F6C-D104768A1214",
    "19B10003-E8F2-537E-4F6C-D104768A1214",
    "19B10004-E8F2-537E-4F6C-D104768A1214"
};

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DeviceAddress sensorsAddress[REQUIRED_SENSOR_COUNT];
int sensorIDs[REQUIRED_SENSOR_COUNT];
bool sensorValid[REQUIRED_SENSOR_COUNT];

unsigned long lastSerialTime = 0;
unsigned long lcdOnTime = 0;
bool isFirstLCDRun = true;

LiquidCrystal_PCF8574 lcd(I2C_ADDR);
ezButton button(BUTTON_PIN);

BLEService tempService(SERVICE_UUID);
BLECharacteristic *tempCharacteristics[REQUIRED_SENSOR_COUNT];

void initializeSensorSystem();
void readSensorAddresses();
void readSensorIDs();
void sortSensors();
void initializeBLE();
void updateBLETemperature();
void updateSerial();
void showMessageOnLCD(const char *message);
void updateLCDDisplay();

void setup() {
    Serial.begin(9600);
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setBacklight(255);
    button.setDebounceTime(50);
    lcdOnTime = millis();

    initializeSensorSystem();
    initializeBLE();
}

void loop() {
    updateLCDDisplay();
    // updateSerial(); // 시리얼 디버깅할 때만 활성화 할 것!
    updateBLETemperature();
}

void initializeSensorSystem() {
    showMessageOnLCD("Initializing sensors...");
    Serial.println("센서 초기화 시작");

    ds18b20.begin();
    uint8_t deviceCount = ds18b20.getDeviceCount();

    if (deviceCount == 0) {
        showMessageOnLCD("No sensors!");
        Serial.println("[Error] 센서가 감지되지 않음");
        while (1);
    }

    if (deviceCount < REQUIRED_SENSOR_COUNT) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Insufficient sensors:");
        lcd.setCursor(0, 1);
        lcd.print(deviceCount);
        lcd.print(" / ");
        lcd.print(REQUIRED_SENSOR_COUNT);
        Serial.println("[Warning] 설정 센서 수보다 감지된 센서수 적음");
    }

    readSensorAddresses();
    delay(1000);
    readSensorIDs();
    delay(1000);
    sortSensors();
    delay(1000);

    showMessageOnLCD("Initialization complete");
    Serial.println("[Init] 초기화 완료");
    Serial.println("센서 초기화 완료");
}

void readSensorAddresses() {
    showMessageOnLCD("Reading addresses...");
    Serial.println("센서 주소 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        sensorValid[i] = ds18b20.getAddress(sensorsAddress[i], i);

        if (!sensorValid[i]) {
            Serial.print("[Error] 센서 ");
            Serial.print(i);
            Serial.println("의 주소 읽기 실패");
        }
    }
}

void readSensorIDs() {
    showMessageOnLCD("Reading IDs...");
    Serial.println("센서 ID 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        if (sensorValid[i]) {
            sensorIDs[i] = ds18b20.getUserData(sensorsAddress[i]);
            for (int j = 0; j < i; j++) {
                if (sensorValid[j] && sensorIDs[i] == sensorIDs[j]) {
                    Serial.print("[Error] 중복 ID: ");
                    Serial.println(sensorIDs[i]);
                }
            }
        } else {
            sensorIDs[i] = -1; // 오류 표시
        }
    }
}

void sortSensors() {
    showMessageOnLCD("Sorting sensors...");
    Serial.println("센서 정렬 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT - 1; i++) {
        for (int j = 0; j < REQUIRED_SENSOR_COUNT - i - 1; j++) {
            if (sensorIDs[j] > sensorIDs[j + 1]) {
                // Swap IDs
                int tempID = sensorIDs[j];
                sensorIDs[j] = sensorIDs[j + 1];
                sensorIDs[j + 1] = tempID;

                // Swap addresses
                DeviceAddress tempAddr;
                memcpy(tempAddr, sensorsAddress[j], sizeof(DeviceAddress));
                memcpy(sensorsAddress[j], sensorsAddress[j + 1], sizeof(DeviceAddress));
                memcpy(sensorsAddress[j + 1], tempAddr, sizeof(DeviceAddress));

                // Swap validity
                bool tempValid = sensorValid[j];
                sensorValid[j] = sensorValid[j + 1];
                sensorValid[j + 1] = tempValid;
            }
        }
    }
}

void initializeBLE() {
    Serial.println("BLE 초기화 시작");

    if (!BLE.begin()) {
        Serial.println("BLE 시작 실패");
    }

    BLE.setLocalName("ArduinoR4Temp");
    BLE.setAdvertisedService(tempService);

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        tempCharacteristics[i] = new BLECharacteristic(CHARACTERISTIC_UUIDS[i], BLERead | BLENotify, 8);
        tempService.addCharacteristic(*tempCharacteristics[i]);
        tempCharacteristics[i]->writeValue("0");
    }

    BLE.addService(tempService);
    BLE.advertise();
    Serial.println("BLE 주변 기기가 시작되었습니다, 연결 대기 중...");
}

void updateBLETemperature() {
    BLE.poll();
    if (BLE.connected()) {
        ds18b20.requestTemperatures();
        char buffer[8];

        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
            if (!sensorValid[i]) {
                sprintf(buffer, "ERR");
            } else {
                float tempC = ds18b20.getTempC(sensorsAddress[i]);
                if (tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125) {
                    sprintf(buffer, "ERR");
                } else {
                    sprintf(buffer, "%0.1f", tempC);
                }
            }
            tempCharacteristics[i]->writeValue(buffer);
        }
    }
}

void updateSerial() {
    if (millis() - lastSerialTime > SERIAL_INTERVAL) {
        lastSerialTime = millis();
        ds18b20.requestTemperatures();
        Serial.println("\n--- 센서 정보 ---");
        Serial.println("순번\tID\t온도\t\t주소\t\t상태");
        Serial.println("----------------------------------------------");

        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
            Serial.print(i + 1);
            Serial.print("\t");

            if (!sensorValid[i]) {
                Serial.println("센서 오류");
                continue;
            }

            Serial.print(sensorIDs[i]);
            Serial.print("\t");

            float tempC = ds18b20.getTempC(sensorsAddress[i]);
            if (tempC == DEVICE_DISCONNECTED_C) {
                Serial.print("ERR");
            } else if (tempC < -55 || tempC > 125) {
                Serial.print("OVR");
            } else {
                Serial.print(tempC, 1);
                Serial.print("C");
            }
            Serial.print("\t");

            for (uint8_t j = 0; j < 8; j++) {
                if (sensorsAddress[i][j] < 16)
                    Serial.print("0");
                Serial.print(sensorsAddress[i][j], HEX);
            }
            Serial.print("\t");

            if (!sensorValid[i] || tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125) {
                Serial.println("ERR");
            } else {
                Serial.println("OK");
            }
        }
        Serial.println("----------------------------------------------");
    }
}

void showMessageOnLCD(const char *message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}

void updateLCDDisplay() {
    unsigned long currentMillis = millis();

    button.loop();
    if (button.isPressed()) {
        Serial.println("Button pressed");
        lcdOnTime = currentMillis;
        isFirstLCDRun = true;
    }

    if (currentMillis - lcdOnTime < LCD_TIMEOUT) {
        lcd.setBacklight(255);
        if (isFirstLCDRun) {
            lcd.clear();
            isFirstLCDRun = false;
        }

        ds18b20.requestTemperatures();
        for (int i = 0; i < REQUIRED_SENSOR_COUNT && i < LCD_ROWS; i++) {
            lcd.setCursor(0, i);
            lcd.print("ID:");
            lcd.print(sensorValid[i] ? sensorIDs[i] : -1);
            lcd.print(" ");

            float tempC = sensorValid[i] ? ds18b20.getTempC(sensorsAddress[i]) : DEVICE_DISCONNECTED_C;
            lcd.print("T:");
            if (!sensorValid[i]) {
                lcd.print("ERR");
            } else if (tempC == DEVICE_DISCONNECTED_C) {
                lcd.print("--.-");
            } else if (tempC < -55 || tempC > 125) {
                lcd.print("OVR");
            } else {
                lcd.print(tempC, 1);
                lcd.print("C");
            }

            lcd.setCursor(15, i);
            if (!sensorValid[i] || tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125) {
                lcd.print("ERR");
            } else {
                lcd.print("OK ");
            }
        }
    } else {
        lcd.setBacklight(0);
        lcd.clear();
    }
}
```
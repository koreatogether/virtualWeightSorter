/*
이 코드는 ds18b20과 BLE를 이용해 온도 값을 게시합니다.
센서 4개에 맞는 특성 4개가 등록되며, 각각의 특성이 각 센서의 온도 값을 게시합니다.
*/

#include <ArduinoBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// 추가: LCD와 버튼을 위한 라이브러리 추가
#include <LiquidCrystal_PCF8574.h>
#include <ezButton.h>

// ------------- 온도 센서 설정 -------------
const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

const int REQUIRED_SENSOR_COUNT = 4; // 필요한 센서 수
DeviceAddress sensorsAddress[REQUIRED_SENSOR_COUNT];
int sensorIDs[REQUIRED_SENSOR_COUNT];
bool sensorValid[REQUIRED_SENSOR_COUNT];

// ------------- 시리얼 출력 시간 변수 -------------
unsigned long lastSerialTime = 0;
const unsigned long SERIAL_INTERVAL = 2000; // 2초 간격

// BLE 서비스 UUID
const char *SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";

// 각 센서별 특성 UUID 선언
const char *CHARACTERISTIC_UUID_0 = "19B10001-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_1 = "19B10002-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_2 = "19B10003-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID_3 = "19B10004-E8F2-537E-4F6C-D104768A1214";

// BLE 서비스 및 특성 객체 생성
BLEService tempService(SERVICE_UUID);
BLECharacteristic tempCharacteristic0(CHARACTERISTIC_UUID_0, BLERead | BLENotify, 8);
BLECharacteristic tempCharacteristic1(CHARACTERISTIC_UUID_1, BLERead | BLENotify, 8);
BLECharacteristic tempCharacteristic2(CHARACTERISTIC_UUID_2, BLERead | BLENotify, 8);
BLECharacteristic tempCharacteristic3(CHARACTERISTIC_UUID_3, BLERead | BLENotify, 8);

BLECharacteristic *tempCharacteristics[REQUIRED_SENSOR_COUNT] = {&tempCharacteristic0, &tempCharacteristic1, &tempCharacteristic2, &tempCharacteristic3};

// ------------- 추가: LCD 및 버튼 설정 -------------
const int I2C_ADDR = 0x27;
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
const unsigned long LCD_TIMEOUT = 20000; // 20초 후 LCD 끄기
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

const int BUTTON_PIN = 3;
volatile bool buttonPressed = false;          // 인터럽트에서 사용할 플래그
volatile unsigned long lastInterruptTime = 0; // 디바운싱을 위한 변수
const unsigned long DEBOUNCE_DELAY = 200;     // 디바운스 시간 (밀리초)

unsigned long lcdOnTime = 0; // LCD 켜진 시각
bool isFirstLCDRun = true;   // LCD 첫 구동 상태

// 인터럽트 서비스 루틴 (ISR)
void buttonInterrupt()
{
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY)
    {
        buttonPressed = true;
        lastInterruptTime = interruptTime;
    }
}

// ------------- 센서 초기화 함수 -------------
void initializeSensorSystem()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Initializing sensors...");
    Serial.println("센서 초기화 시작");

    ds18b20.begin();
    uint8_t deviceCount = ds18b20.getDeviceCount();

    if (deviceCount == 0)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No sensors!");
        Serial.println("[Error] 센서가 감지되지 않음");
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
        Serial.println("[Warning] 설정 센서 수보다 감지된 센서수 적음");
    }

    // 센서 주소, ID 읽기 및 정렬
    readSensorAddresses();
    delay(1000);

    // 센서 ID 읽기
    readSensorIDs();
    delay(1000);

    // 센서 정렬
    sortSensors();
    delay(1000);

    lcd.clear();
    lcd.print("Initialization complete");
    Serial.println("[Init] 초기화 완료");
    Serial.println("센서 초기화 완료");
    delay(1000);
}

// ------------- 센서 주소 읽기 함수 -------------
void readSensorAddresses()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reading addresses...");
    Serial.println("센서 주소 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
        sensorValid[i] = ds18b20.getAddress(sensorsAddress[i], i);

        if (!sensorValid[i])
        {
            Serial.print("[Error] 센서 ");
            Serial.print(i);
            Serial.println("의 주소 읽기 실패");
        }
    }
}

// ------------- 센서 ID 읽기 함수 -------------
void readSensorIDs()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reading IDs...");
    Serial.println("센서 ID 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
        if (sensorValid[i])
        {
            sensorIDs[i] = ds18b20.getUserData(sensorsAddress[i]);
            // 중복 확인: 앞의 센서와 비교
            for (int j = 0; j < i; j++)
            {
                if (sensorValid[j] && sensorIDs[i] == sensorIDs[j])
                {
                    Serial.print("[Error] 중복 ID: ");
                    Serial.println(sensorIDs[i]);
                }
            }
        }
        else
        {
            sensorIDs[i] = -1; // 오류 표시
        }
    }
}

// ------------- 센서 정렬 함수 -------------
void sortSensors()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sorting sensors...");
    Serial.println("센서 정렬 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT - 1; i++)
    {
        for (int j = 0; j < REQUIRED_SENSOR_COUNT - i - 1; j++)
        {
            if (sensorIDs[j] > sensorIDs[j + 1])
            {
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

// ------------- BLE 초기화 및 준비 -------------
void initializeBLE()
{
    Serial.println("BLE 초기화 시작");

    if (!BLE.begin())
    {
        Serial.println("BLE 시작 실패");
    }

    // BLE 지역 이름 및 서비스 광고 설정
    BLE.setLocalName("ArduinoR4Temp");
    BLE.setAdvertisedService(tempService);

    // 각 특성을 서비스에 추가
    tempService.addCharacteristic(tempCharacteristic0);
    tempService.addCharacteristic(tempCharacteristic1);
    tempService.addCharacteristic(tempCharacteristic2);
    tempService.addCharacteristic(tempCharacteristic3);

    BLE.addService(tempService);

    // 특성 초기값 설정
    tempCharacteristic0.writeValue("0");
    tempCharacteristic1.writeValue("0");
    tempCharacteristic2.writeValue("0");
    tempCharacteristic3.writeValue("0");

    // 광고 시작
    BLE.advertise();
    Serial.println("BLE 주변 기기가 시작되었습니다, 연결 대기 중...");
}

// ------------- BLE 이벤트 폴링 및 온도 업데이트 함수 -------------
void updateBLETemperature()
{
    // BLE.poll();
    if (BLE.connected())
    {
        ds18b20.requestTemperatures();
        char buffer[8];

        // 각 센서에 대해 온도를 읽어 해당 특성으로 전송
        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
        {
            if (!sensorValid[i])
            {
                sprintf(buffer, "ERR");
            }
            else
            {
                float tempC = ds18b20.getTempC(sensorsAddress[i]);
                if (tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125)
                {
                    sprintf(buffer, "ERR");
                }
                else
                {
                    sprintf(buffer, "%0.1f", tempC);
                }
            }
            tempCharacteristics[i]->writeValue(buffer);
        }
    }
}

// ------------- 시리얼에 센서 정보 업데이트 함수 -------------
void updateSerial()
{
    if (millis() - lastSerialTime > SERIAL_INTERVAL)
    {
        lastSerialTime = millis();
        ds18b20.requestTemperatures();
        Serial.println("\n--- 센서 정보 ---");
        Serial.println("순번\tID\t온도\t\t주소\t\t상태");
        Serial.println("----------------------------------------------");

        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
        {
            Serial.print(i + 1);
            Serial.print("\t");

            if (!sensorValid[i])
            {
                Serial.println("센서 오류");
                continue;
            }

            Serial.print(sensorIDs[i]);
            Serial.print("\t");

            float tempC = ds18b20.getTempC(sensorsAddress[i]);
            if (tempC == DEVICE_DISCONNECTED_C)
            {
                Serial.print("ERR");
            }
            else if (tempC < -55 || tempC > 125)
            {
                Serial.print("OVR");
            }
            else
            {
                Serial.print(tempC, 1);
                Serial.print("C");
            }
            Serial.print("\t");

            // 주소 출력
            for (uint8_t j = 0; j < 8; j++)
            {
                if (sensorsAddress[i][j] < 16)
                    Serial.print("0");
                Serial.print(sensorsAddress[i][j], HEX);
            }
            Serial.print("\t");

            if (!sensorValid[i] || tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125)
            {
                Serial.println("ERR");
            }
            else
            {
                Serial.println("OK");
            }
        }
        Serial.println("----------------------------------------------");
    }
}

// ------------ LCD에 간단한 메시지 출력 ------------
void showMessageOnLCD(const char *message)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}

// ------------- 추가: LCD 업데이트 함수 (5_rev3와 동일) -------------
void updateLCDDisplay()
{
    unsigned long currentMillis = millis();

    if (buttonPressed)
    {
        Serial.println("Button pressed (ISR)");
        lcdOnTime = currentMillis;
        isFirstLCDRun = true;  // 화면 초기화
        buttonPressed = false; // 플래그 리셋
    }

    if (currentMillis - lcdOnTime < LCD_TIMEOUT)
    {
        lcd.setBacklight(255);
        if (isFirstLCDRun)
        {
            lcd.clear();
            isFirstLCDRun = false;
        }

        ds18b20.requestTemperatures();
        for (int i = 0; i < REQUIRED_SENSOR_COUNT && i < LCD_ROWS; i++)
        {
            lcd.setCursor(0, i);
            lcd.print("ID:");
            lcd.print(sensorValid[i] ? sensorIDs[i] : -1);
            lcd.print(" ");

            float tempC = sensorValid[i] ? ds18b20.getTempC(sensorsAddress[i]) : DEVICE_DISCONNECTED_C;
            lcd.print("T:");
            if (!sensorValid[i])
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

            lcd.setCursor(15, i);
            if (!sensorValid[i] || tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125)
            {
                lcd.print("ERR");
            }
            else
            {
                lcd.print("OK ");
            }
        }
    }
    else
    {
        // LCD 끄기
        lcd.setBacklight(0);
        lcd.clear();
    }
}

// ------------- setup 함수 -------------
void setup()
{
    Serial.begin(9600);

    // 추가: LCD 및 버튼 초기화
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setBacklight(255);

    // 버튼 인터럽트 설정
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING);

    lcdOnTime = millis();

    // 센서 초기화 (주소, ID, 정렬)
    initializeSensorSystem();
    // BLE 초기화
    initializeBLE();
}

// ------------- loop 함수 -------------
void loop()
{
    // BLE 통신
    BLE.poll();

    // LCD 업데이트
    updateLCDDisplay();

    // 온도 업데이트
    updateBLETemperature();
}

/*
이 코드는 ds18b20과 BLE를 이용해 온도 값을 게시합니다.
센서 4개에 맞는 특성 4개가 등록되며, 각각의 특성이 각 센서의 온도 값을 게시합니다.
*/

#include <ArduinoBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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

// ------------- 센서 초기화 함수 -------------
void initializeSensorSystem()
{
    Serial.println("센서 초기화 시작");

    ds18b20.begin();
    uint8_t deviceCount = ds18b20.getDeviceCount();

    if (deviceCount == 0)
    {
        Serial.println("[Error] 센서가 감지되지 않음");
        while (1)
            ;
    }

    if (deviceCount < REQUIRED_SENSOR_COUNT)
    {
        Serial.println("[Warning] 설정 센서 수보다 감지된 센서수 적음");
    }

    // 센서 주소, ID 읽기 및 정렬
    readSensorAddresses();
    readSensorIDs();
    sortSensors();

    Serial.println("센서 초기화 완료");
    delay(1000);
}

// ------------- 센서 주소 읽기 함수 -------------
void readSensorAddresses()
{
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
        delay(1000);
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
    BLE.poll();
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

// ------------- setup 함수 -------------
void setup()
{
    Serial.begin(9600);

    // 센서 초기화 (주소, ID, 정렬)
    initializeSensorSystem();
    // BLE 초기화
    initializeBLE();
}

// ------------- loop 함수 -------------
void loop()
{
    updateSerial();
    updateBLETemperature();
}

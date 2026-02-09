/*
이 코드는 ds18b20 과 BLE를 통해서 온도 값을 게시하는 코드입니다.
*/

#include <ArduinoBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//--------------  온도 센서 설정 --------------
const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

const int REQUIRED_SENSOR_COUNT = 4; // 필요한 센서 수
DeviceAddress sensorsAddress[REQUIRED_SENSOR_COUNT];
int sensorIDs[REQUIRED_SENSOR_COUNT];
bool sensorValid[REQUIRED_SENSOR_COUNT];

// --------------- 시리얼 출력 시간 변수 ---------------
unsigned long lastSerialTime = 0;
const unsigned long SERIAL_INTERVAL = 2000; // 2초 간격

// 사용자 정의 서비스 및 특성
const char *SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char *CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214";

BLEService tempService(SERVICE_UUID);
BLECharacteristic tempCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify, 8); // 최대 8바이트로 설정 (숫자 문자열, 예: -45.0 또는 125.0)

// --------------- 센서 초기화 함수 ---------------
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

    if (deviceCount < REQUIRED_SENSOR_COUNT) // Fixed typo: changed RREQUIRED_SENSOR_COUNT to REQUIRED_SENSOR_COUNT
    {
        Serial.println("[Warning] 설정 센서 수보다 감지된 센서수 적음");
    }

    readSensorAddresses(); // 센서 주소 읽기 함수 호출
    readSensorIDs();       // 센서 ID 읽기 함수 호출
    sortSensors();         // 센서 정렬 함수 호출

    Serial.println("센서 초기화 완료");
    delay(1000);
}

// --------------- 센서 주소 읽기 함수 ---------------
void readSensorAddresses()
{
    Serial.println("센서 주소 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
        // Changed 'sensor' to 'ds18b20' and 'sensorAddresses' to 'sensorsAddress'
        sensorValid[i] = ds18b20.getAddress(sensorsAddress[i], i);

        if (!sensorValid[i])
        {
            Serial.print("[Error] 센서 ");
            Serial.print(i);
            Serial.println("의 주소 읽기 실패");
        }
    }
}

// --------------- 센서 ID 읽기 함수 ---------------
void readSensorIDs()
{
    Serial.println("센서 ID 읽기 시작"); // Fixed typo: pritnln -> println

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
        if (sensorValid[i])
        {
            // Changed 'sensors' to 'ds18b20' and 'sensorAddresses' to 'sensorsAddress'
            sensorIDs[i] = ds18b20.getUserData(sensorsAddress[i]);
            // 중복 확인 , 앞의 센서와 비교
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

// --------------- 센서 정렬 함수 ---------------
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

                // Swap addresses; changed 'sensorAddresses' to 'sensorsAddress'
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

// --------------- BLE 초기화 및 준비 ---------------
void initializeBLE()
{
    Serial.println("BLE 초기화 시작");

    if (!BLE.begin())
    {
        Serial.println("BLE 시작 실패");
        delay(1000);
    }

    // 지역 이름 설정 및 서비스 광고
    BLE.setLocalName("ArduinoR4Temp");
    BLE.setAdvertisedService(tempService);
    // 특성을 서비스에 추가
    tempService.addCharacteristic(tempCharacteristic);
    BLE.addService(tempService);
    // 특성 값을 초기화 (문자열 "0")
    tempCharacteristic.writeValue("0");
    // 광고 시작
    BLE.advertise();
    Serial.println("BLE 주변 기기가 시작되었습니다, 연결 대기 중...");
}

// --------------- BLE 이벤트 폴링 함수 ---------------
void updateBLETemperature()
{
    BLE.poll();
    if (BLE.connected())
    {
        // 센서 정보 업데이트
        ds18b20.requestTemperatures(); // Changed 'sensors' to 'ds18b20'
        char buffer[8];
        for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
        {
            if (!sensorValid[i])
            {
                sprintf(buffer, "ERR");
            }
            else
            {
                float tempC = ds18b20.getTempC(sensorsAddress[i]); // Changed 'sensors' to 'ds18b20' and 'sensorAddresses' to 'sensorsAddress'
                if (tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125)
                {
                    sprintf(buffer, "ERR");
                }
                else
                {
                    sprintf(buffer, "%0.1f", tempC);
                }
            }
            tempCharacteristic.writeValue(buffer);
        }
    }
}

// --------------- 시리얼에 센서 정보 업데이트 ---------------
void updateSerial()
{
    if (millis() - lastSerialTime > SERIAL_INTERVAL)
    {
        lastSerialTime = millis();

        ds18b20.requestTemperatures();
        // Changed 'sensors' to 'ds18b20'
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
            // Changed 'sensors' to 'ds18b20' and 'sensorAddresses' to 'sensorsAddress'
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
                if (sensorsAddress[i][j] < 16) // Changed 'sensorAddresses' to 'sensorsAddress'
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

// --------------- setup 함수 ---------------
void setup()
{
    Serial.begin(9600);

    // 센서 시스템 초기화 ( 주소 , ID , 정렬 )
    initializeSensorSystem();
    initializeBLE();
}

// --------------- loop 함수 ---------------
void loop()
{
    updateSerial();
    updateBLETemperature();
}
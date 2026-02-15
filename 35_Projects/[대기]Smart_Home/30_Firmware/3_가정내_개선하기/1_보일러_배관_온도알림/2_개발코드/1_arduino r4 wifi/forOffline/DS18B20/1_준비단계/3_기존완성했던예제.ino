/*
 * 목적:
 * 센서의 ID, 주소, 온도를 출력하면서 다양한 예외상황 처리
 * 원래 코드 위치 : sensors/온도센서/DS18B20/실제사용모양/06_예외처리추가.ino
 * Wokwi 주소 : https://wokwi.com/projects/422825706774615041  ( id 검증부분 비활성화시킴)
 */
#include <OneWire.h>
#include <DallasTemperature.h>

const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int arraySizeByUser = 4;
DeviceAddress deviceAddress[arraySizeByUser];
int sortID[arraySizeByUser];
bool isValidSensor[arraySizeByUser]; // 센서 유효성 체크용 배열 추가

// 센서 연결 확인 및 초기화 함수 추가
bool initializeSensors()
{
    sensors.begin();
    uint8_t actualDeviceCount = sensors.getDeviceCount();

    if (actualDeviceCount == 0)
    {
        Serial.println(F("Error: No sensors detected!"));
        return false;
    }

    if (actualDeviceCount < arraySizeByUser)
    {
        Serial.print(F("Warning: Expected "));
        Serial.print(arraySizeByUser);
        Serial.print(F(" sensors, but only found "));
        Serial.println(actualDeviceCount);
        return false;
    }

    return true;
}

void readAddress()
{
    for (int i = 0; i < arraySizeByUser; i++)
    {
        isValidSensor[i] = sensors.getAddress(deviceAddress[i], i);
        if (!isValidSensor[i])
        {
            Serial.print(F("Error: Failed to read address for sensor "));
            Serial.println(i);
        }
    }
}

void readIdFromDS18B20()
{
    bool foundDuplicate = false;
    for (int i = 0; i < arraySizeByUser; i++)
    {
        if (isValidSensor[i])
        {
            sortID[i] = sensors.getUserData(deviceAddress[i]);

            // ID 유효성 검사 (1~99 범위만 허용)
            if (sortID[i] < 1 || sortID[i] > 99)
            {
                Serial.print(F("Warning: Invalid ID detected for sensor "));
                Serial.print(i);
                Serial.print(F(": "));
                Serial.println(sortID[i]);
                isValidSensor[i] = false;
                continue;
            }

            // ID 중복 검사
            for (int j = 0; j < i; j++)
            {
                if (isValidSensor[j] && sortID[i] == sortID[j])
                {
                    Serial.print(F("Error: Duplicate ID detected: "));
                    Serial.println(sortID[i]);
                    foundDuplicate = true;
                }
            }
        }
    }

    if (foundDuplicate)
    {
        Serial.println(F("Please reprogram sensor IDs to unique values"));
    }
}

void sortIdByBubbleSort()
{
    // # 버블 정렬을 사용하여 ID와 주소를 함께 오름차순으로 정렬하는 함수
    DeviceAddress tempAddr;
    for (int i = 0; i < arraySizeByUser - 1; i++)
    {
        for (int j = 0; j < arraySizeByUser - i - 1; j++)
        {
            if (sortID[j] > sortID[j + 1]) // 오름차순 정렬을 위한 비교
            {
                // ID 교환
                int tempID = sortID[j];
                sortID[j] = sortID[j + 1];
                sortID[j + 1] = tempID;

                // 주소 교환
                memcpy(tempAddr, deviceAddress[j], sizeof(DeviceAddress));
                memcpy(deviceAddress[j], deviceAddress[j + 1], sizeof(DeviceAddress));
                memcpy(deviceAddress[j + 1], tempAddr, sizeof(DeviceAddress));

                // 유효성 상태도 함께 교환
                bool tempValid = isValidSensor[j];
                isValidSensor[j] = isValidSensor[j + 1];
                isValidSensor[j + 1] = tempValid;
            }
        }
    }
}

void printSensorInfo()
{
    // 온도 측정 요청
    sensors.requestTemperatures(); // void 함수이므로 반환값 검사 제거

    Serial.println(F("\n=== 센서 정보 ==="));
    Serial.println(F("순서\tID\t온도\t\t주소\t\t상태"));
    Serial.println(F("----------------------------------------------------"));

    for (int i = 0; i < arraySizeByUser; i++)
    {
        Serial.print(i + 1);
        Serial.print(F("\t"));

        if (!isValidSensor[i])
        {
            Serial.println(F("센서 오류 - 확인 필요"));
            readAddress();
            readIdFromDS18B20();
            sortIdByBubbleSort();
            continue;
        }

        // ID 출력
        Serial.print(sortID[i]);
        Serial.print(F("\t"));

        // 온도 출력 및 검증
        float tempC = sensors.getTempC(deviceAddress[i]);
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            Serial.print(F("통신오류"));
        }
        else if (tempC < -55 || tempC > 125)
        {
            Serial.print(F("온도범위초과"));
        }
        else
        {
            Serial.print(tempC, 1);
            Serial.print(F("°C"));
        }
        Serial.print(F("\t"));

        // 주소 출력
        for (uint8_t j = 0; j < 8; j++)
        {
            if (deviceAddress[i][j] < 16)
                Serial.print("0");
            Serial.print(deviceAddress[i][j], HEX);
        }

        // 상태 출력 - 조건에 따라 다른 상태 표시
        Serial.print(F("\t"));
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            Serial.print(F("통신오류"));
        }
        else if (tempC < -55 || tempC > 125)
        {
            Serial.print(F("온도범위초과"));
        }
        else
        {
            Serial.print(F("정상"));
        }
        Serial.println();
    }
    Serial.println(F("----------------------------------------------------"));
}

void setup()
{
    Serial.begin(9600);

    if (!initializeSensors())
    {
        Serial.println(F("System halted. Please check sensor connections."));
        // while (1)
        // {
        //     delay(1000);
        // } // 시스템 중단
    }

    for (int i = 0; i < arraySizeByUser; i++)
    {
        sensors.setResolution(deviceAddress[i], 12);
    }

    readAddress();
    readIdFromDS18B20();
    sortIdByBubbleSort();
    printSensorInfo();
}

void loop()
{
    printSensorInfo();
    delay(2000);
}

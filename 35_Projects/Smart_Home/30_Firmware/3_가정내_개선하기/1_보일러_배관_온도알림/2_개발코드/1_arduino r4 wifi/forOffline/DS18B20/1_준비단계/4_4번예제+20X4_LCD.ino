/*
 * 목적:
 * 센서의 ID, 주소, 온도를 출력하면서 다양한 예외상황 처리
 * 원래 코드 위치 : sensors/온도센서/DS18B20/실제사용모양/06_예외처리추가.ino
 * Wokwi (온도센서 연결안함) 주소 : https://wokwi.com/projects/422825706774615041  ( id 검증부분 비활성화시킴)
 * Wokwi ( 온도센서 정상) 주소 : https://wokwi.com/projects/422970904159711233
 */
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>

/*------------  20x4 LCD 변수 및 객체 -------------*/
const int I2C_ADDR = 0x27;
const int LCD_COLUNMS = 20;
const int LCD_ROWS = 4;
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

/*------------  온도 센서 변수 및 객체 -------------*/
const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/*------------  사용자 정의 변수 및 객체 -------------*/
const int arraySizeByUser = 4;
DeviceAddress deviceAddress[arraySizeByUser];
int sortID[arraySizeByUser];
bool isValidSensor[arraySizeByUser]; // 센서 유효성 체크용 배열 추가

/*------------ 센서 연결 활성 및 초기화 함수 추가 ----------------*/
bool initializeSensors()
{
    sensors.begin();
    uint8_t actualDeviceCount = sensors.getDeviceCount();

    if (actualDeviceCount == 0)
    {
        // to LCD display
        lcd.setCursor(0, 0);
        lcd.print("No sensors detected!");

        Serial.println(F("No sensors detected!"));
        delay(3000);
        return false;
    }

    if (actualDeviceCount < arraySizeByUser)
    {
        // to LCD display
        lcd.setCursor(0, 0);
        lcd.print("Warning:Expected");
        lcd.setCursor(0, 1);
        lcd.print(arraySizeByUser);
        lcd.setCursor(2, 1);
        lcd.print("sensors");
        lcd.setCursor(0, 2);
        lcd.print("but only ");
        lcd.setCursor(11, 2);
        lcd.print(actualDeviceCount);
        lcd.setCursor(13, 2);
        lcd.print("found");

        Serial.print(F("Warning: Expected "));
        Serial.print(arraySizeByUser);
        Serial.print(F(" sensors, but only found "));
        Serial.println(actualDeviceCount);
        delay(3000);
        return false;
    }

    return true;
}

/*------------ 센서 주소 읽기 및 ID 검증 함수 추가 ----------------*/
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

            // // ID 유효성 검사 (1~99 범위만 허용)
            // if (sortID[i] < 1 || sortID[i] > 99)
            // {
            //     Serial.print(F("Warning: Invalid ID detected for sensor "));
            //     Serial.print(i);
            //     Serial.print(F(": "));
            //     Serial.println(sortID[i]);
            //     isValidSensor[i] = false;
            //     continue;
            // }

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

/*------------ 센서 ID 및 주소 정렬 함수 추가 ----------------*/
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

/*------------ 센서 정보 출력 함수 추가 ----------------*/
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

// 수정: 20x4 LCD에 센서 정보(id/온도/상태) 출력 함수
void printSensorInforToLCD()
{
    sensors.requestTemperatures();
    lcd.clear();

    for (int i = 0; i < arraySizeByUser; i++)
    {
        lcd.setCursor(0, i);

        // ID 출력
        if (isValidSensor[i])
        {
            lcd.print("ID:");
            lcd.print(sortID[i]);
        }
        else
        {
            lcd.print("ID:-");
        }

        lcd.print(" ");

        // 온도와 상태 체크
        float tempC = isValidSensor[i] ? sensors.getTempC(deviceAddress[i]) : DEVICE_DISCONNECTED_C;
        String status;
        String tempDisplay;

        // 상태와 온도 표시 결정
        if (!isValidSensor[i])
        {
            status = "ERR";
            tempDisplay = "--.-";
        }
        else if (tempC == DEVICE_DISCONNECTED_C)
        {
            status = "COM";
            tempDisplay = "ERR ";
        }
        else if (tempC < -55 || tempC > 125)
        {
            status = "OVR";
            tempDisplay = "OVER";
        }
        else
        {
            status = "OK ";
            char tempStr[6];
            dtostrf(tempC, 4, 1, tempStr);
            tempDisplay = String(tempStr);
        }

        // 온도 출력
        lcd.print("T:");
        lcd.print(tempDisplay);
        lcd.print(" ");

        // 상태 출력
        lcd.setCursor(15, i);
        lcd.print(status);
    }
}

/**************************************** */
/*              setup 함수                */
/**************************************** */
void setup()
{
    Serial.begin(9600);
    lcd.begin(LCD_COLUNMS, LCD_ROWS); // 20x4 LCD 활성화
    lcd.setBacklight(255);            // LCD 백라이트 활성화

    if (!initializeSensors())
    {
        // to LCD display
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System halted.");
        lcd.setCursor(0, 1);
        lcd.print("Please check");
        lcd.setCursor(0, 2);
        lcd.print("sensor connections.");
        delay(3000);

        Serial.println(F("System halted. Please check sensor connections."));
    }

    for (int i = 0; i < arraySizeByUser; i++)
    {
        sensors.setResolution(deviceAddress[i], 12);
    }

    readAddress();
    readIdFromDS18B20();
    sortIdByBubbleSort();
}

/**************************************** */
/*              loop 함수                */
/**************************************** */
void loop()
{
    printSensorInfo();
    printSensorInforToLCD();
    delay(2000);
}

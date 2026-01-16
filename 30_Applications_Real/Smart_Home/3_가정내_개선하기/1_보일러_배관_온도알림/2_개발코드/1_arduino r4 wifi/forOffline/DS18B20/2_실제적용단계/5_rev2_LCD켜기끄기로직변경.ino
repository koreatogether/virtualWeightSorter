/*
 * 목적:
 * 센서의 ID, 주소, 온도를 출력하면서 다양한 예외상황 처리
 * 원래 코드 위치 : sensors/온도센서/DS18B20/실제사용모양/06_예외처리추가.ino
 * Wokwi (온도센서 연결안함) 주소 : https://wokwi.com/projects/422825706774615041  ( id 검증부분 비활성화시킴)
 * Wokwi ( 온도센서 정상) 주소 : https://wokwi.com/projects/422970904159711233
 * Wokwi #6 주소 : https://wokwi.com/projects/423006796521012225  , https://wokwi.com/projects/423006796521012225
 * 실제 적용 결과 , 가상 결과와 같음
 * 부팅 후 최종화면 +  20 초간 LCD 화면을 보여주고 별도의 버튼을 눌렀을때 + 20초간 보여주는 로직을 추가 해보자
 */

/*---------- 배선 ---------------*/
/*
아두이노  <---> DS18B20
 2       <---->  DQ
 5V      <---->  VCC
 GND     <---->  GND
-----------------------------
아두이노 <---> I2C LCD
 A4      <---->  SDA
 A5      <---->  SCL
 5V      <---->  VCC
 GND     <---->  GND

아두이노 <---->  버튼
3       <---->  버튼의 다릿발중 한쪽
GND     <---->  버튼의 다릿발중 다른 한쪽

*/

/*
 * 목적:
 * - LCD 켜기/끄기 로직 개선
 * - 버튼 처리 로직 통합
 * - 비차단 방식으로 변경
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>
#include <ezButton.h>

/*------------  LCD 관련 상수 및 객체 -------------*/
const int I2C_ADDR = 0x27;
const int LCD_COLUNMS = 20;
const int LCD_ROWS = 4;
const int LCD_TIMEOUT = 20000; // 20초 타임아웃
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

/*------------  온도 센서 관련 상수 및 객체 -------------*/
const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/*------------- 버튼 관련 상수 및 객체 -------------*/
const int BUTTON_PIN = 3;
ezButton button(BUTTON_PIN);

/*------------  LCD 상태 관리 변수 -------------*/
unsigned long lcdStartTime = 0;
bool isLcdOn = true;

/*------------  센서 관련 변수 -------------*/
const int arraySizeByUser = 4;
DeviceAddress deviceAddress[arraySizeByUser];
int sortID[arraySizeByUser];
bool isValidSensor[arraySizeByUser];

/*------------ 시리얼 출력 타이머 변수 -------------*/
unsigned long lastSerialPrintTime = 0;
const unsigned long SERIAL_PRINT_INTERVAL = 2000; // 2초

/*------------ LCD 상태 관리 함수 ----------------*/
void updateLCDState()
{
    static bool prevLcdState = true;
    bool shouldLcdBeOn = false;
    unsigned long currentTime = millis();

    // 버튼이 눌렸는지 확인
    if (button.isPressed())
    {
        lcdStartTime = currentTime;
        shouldLcdBeOn = true;
    }

    // 타이머 확인
    if (currentTime - lcdStartTime < LCD_TIMEOUT)
    {
        shouldLcdBeOn = true;
    }

    // LCD 상태가 변경되었을 때만 업데이트
    if (shouldLcdBeOn != prevLcdState)
    {
        if (shouldLcdBeOn)
        {
            lcd.setBacklight(255);
            extern bool firstRun; // printSensorInforToLCD의 정적 변수 참조
            firstRun = true;      // LCD가 켜질 때 firstRun 플래그를 리셋
            printSensorInforToLCD();
        }
        else
        {
            lcd.setBacklight(0);
            lcd.clear();
        }
        prevLcdState = shouldLcdBeOn;
    }

    // LCD가 켜져있을 때만 정보 업데이트
    if (shouldLcdBeOn)
    {
        printSensorInforToLCD();
    }
}

/*------------ 센서 초기화 및 설정 함수 ----------------*/
bool initializeSensors()
{
    sensors.begin();
    uint8_t actualDeviceCount = sensors.getDeviceCount();

    if (actualDeviceCount == 0)
    {
        lcd.setCursor(0, 0);
        lcd.print("No sensors detected!");
        Serial.println(F("No sensors detected!"));
        return false;
    }

    if (actualDeviceCount < arraySizeByUser)
    {
        lcd.setCursor(0, 0);
        lcd.print("Warning: Found only");
        lcd.setCursor(0, 1);
        lcd.print(actualDeviceCount);
        lcd.print(" of ");
        lcd.print(arraySizeByUser);
        lcd.print(" sensors");
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
            for (int j = 0; j < i; j++)
            {
                if (isValidSensor[j] && sortID[i] == sortID[j])
                {
                    Serial.print(F("Error: Duplicate ID: "));
                    Serial.println(sortID[i]);
                    foundDuplicate = true;
                }
            }
        }
    }
}

void sortIdByBubbleSort()
{
    DeviceAddress tempAddr;
    for (int i = 0; i < arraySizeByUser - 1; i++)
    {
        for (int j = 0; j < arraySizeByUser - i - 1; j++)
        {
            if (sortID[j] > sortID[j + 1])
            {
                // ID 교환
                int tempID = sortID[j];
                sortID[j] = sortID[j + 1];
                sortID[j + 1] = tempID;

                // 주소 교환
                memcpy(tempAddr, deviceAddress[j], sizeof(DeviceAddress));
                memcpy(deviceAddress[j], deviceAddress[j + 1], sizeof(DeviceAddress));
                memcpy(deviceAddress[j + 1], tempAddr, sizeof(DeviceAddress));

                // 유효성 상태 교환
                bool tempValid = isValidSensor[j];
                isValidSensor[j] = isValidSensor[j + 1];
                isValidSensor[j + 1] = tempValid;
            }
        }
    }
}

/*------------ 센서 정보 출력 함수 ----------------*/
void printSensorInfo()
{
    unsigned long currentTime = millis();

    // 2초가 지났는지 확인
    if (currentTime - lastSerialPrintTime < SERIAL_PRINT_INTERVAL)
    {
        return; // 2초가 지나지 않았으면 함수 종료
    }

    // 마지막 출력 시간 업데이트
    lastSerialPrintTime = currentTime;

    // 온도 측정 요청
    sensors.requestTemperatures();

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
            continue;
        }

        Serial.print(sortID[i]);
        Serial.print(F("\t"));

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

        for (uint8_t j = 0; j < 8; j++)
        {
            if (deviceAddress[i][j] < 16)
                Serial.print("0");
            Serial.print(deviceAddress[i][j], HEX);
        }

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

void printSensorInforToLCD()
{
    static bool firstRun = true; // 첫 실행 여부를 추적하는 정적 변수
    sensors.requestTemperatures();

    if (firstRun)
    {
        lcd.clear();
        firstRun = false;
    }

    for (int i = 0; i < arraySizeByUser; i++)
    {
        lcd.setCursor(0, i);

        // ID 출력
        lcd.print("ID:");
        lcd.print(isValidSensor[i] ? String(sortID[i]) : "-");
        lcd.print(" ");

        // 온도와 상태
        float tempC = isValidSensor[i] ? sensors.getTempC(deviceAddress[i]) : DEVICE_DISCONNECTED_C;
        String status, tempDisplay;

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

        lcd.print("T:");
        lcd.print(tempDisplay);
        lcd.print(" ");
        lcd.setCursor(15, i);
        lcd.print(status);
    }
}

// firstRun 변수를 전역 범위에서 정의
bool firstRun = true; // 전역 변수로 선언

// LCD 상태가 꺼질 때 firstRun 플래그를 리셋하는 함수 수정
void resetLCDFirstRun()
{
    static bool prevLcdState = true;
    bool shouldLcdBeOn = false;
    unsigned long currentTime = millis();

    if (currentTime - lcdStartTime >= LCD_TIMEOUT)
    {
        shouldLcdBeOn = false;
    }
    else
    {
        shouldLcdBeOn = true;
    }

    // LCD가 꺼질 때 firstRun 플래그를 리셋
    if (!shouldLcdBeOn && prevLcdState)
    {
        firstRun = true; // 전역 변수 직접 참조
    }

    prevLcdState = shouldLcdBeOn;
}

// 초기화 단계를 추적하기 위한 상태 변수 추가
enum InitState
{
    INIT_START,
    READ_ADDRESS,
    READ_ID,
    SORT_ID,
    INIT_COMPLETE
};

InitState currentInitState = INIT_START;
unsigned long initStateStartTime = 0;
const unsigned long STATE_DISPLAY_TIME = 3000; // 3초

void updateInitializationState()
{
    unsigned long currentTime = millis();

    // 상태가 변경된 직후에만 LCD 업데이트
    static InitState lastState = INIT_START;
    if (currentInitState != lastState)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        switch (currentInitState)
        {
        case READ_ADDRESS:
            lcd.print("Reading addresses...");
            break;
        case READ_ID:
            lcd.print("Reading sensor IDs...");
            break;
        case SORT_ID:
            lcd.print("Sorting sensors...");
            break;
        }
        lastState = currentInitState;
        initStateStartTime = currentTime;
    }

    // 각 상태가 3초 경과했는지 확인
    if (currentTime - initStateStartTime >= STATE_DISPLAY_TIME)
    {
        switch (currentInitState)
        {
        case READ_ADDRESS:
            readAddress();
            currentInitState = READ_ID;
            break;
        case READ_ID:
            readIdFromDS18B20();
            currentInitState = SORT_ID;
            break;
        case SORT_ID:
            sortIdByBubbleSort();
            currentInitState = INIT_COMPLETE;
            break;
        }
    }
}

void setup()
{
    Serial.begin(9600);
    lcd.begin(LCD_COLUNMS, LCD_ROWS);
    lcd.setBacklight(255);
    button.setDebounceTime(50);

    if (!initializeSensors())
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System halted.");
        lcd.setCursor(0, 1);
        lcd.print("Check sensors.");
        Serial.println(F("System halted. Check sensors."));
    }

    for (int i = 0; i < arraySizeByUser; i++)
    {
        sensors.setResolution(deviceAddress[i], 12);
    }

    // 초기화 시작
    currentInitState = READ_ADDRESS;
    initStateStartTime = millis();

    // 초기화가 완료될 때까지 대기
    while (currentInitState != INIT_COMPLETE)
    {
        updateInitializationState();
    }

    lcdStartTime = millis(); // 초기 LCD 타이머 설정
}

void loop()
{
    button.loop();    // 버튼 상태 업데이트
    updateLCDState(); // LCD 상태 관리

    // printSensorInfo(); // 시리얼 모니터 출력, 평상시 비활성화 , 사유 : 버튼인식에 방해됨
    //  시리얼 출력 함수를 활성화 후 LCD 화면을 켤때 버튼 1초간 눌렀다가 떼면 인식을 함.
}
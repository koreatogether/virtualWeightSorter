/*
 *  목적: 센서의 ID, 주소, 온도를 LCD와 시리얼 모니터에 출력하는 예제
 *  설명: 이 코드는 아두이노 초보자들이 이해하기 쉽도록 구조와 주석을 단순화하였습니다.
 *  하드웨어 연결:
 *    [DS18B20 온도 센서]
 *      - 데이터: 아두이노 핀 2 (OneWire 버스)
 *      - VCC: 5V
 *      - GND: GND
 *    [I2C LCD]
 *      - SDA: 아두이노 A4
 *      - SCL: 아두이노 A5
 *      - VCC: 5V
 *      - GND: GND
 *    [버튼]
 *      - 한쪽: 아두이노 핀 3
 *      - 다른쪽: GND
 *
 *  동작: 부팅 시 센서 초기화 후 LCD와 시리얼 모니터에 센서 정보를 주기적으로 표시합니다.
 *         LCD는 20초동안 켜지며, 버튼을 누르면 LCD 표시 시간이 갱신됩니다.
 *
 * 가상 시뮬레이터 주소 rev3용 : https://wokwi.com/projects/423193400100590593
 * 본 파일의 코드는 완료 코드 폴더에 offline_rev1.ino로 저장되어 있습니다.
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>
#include <ezButton.h>

// ------------ LCD 설정 ------------
const int I2C_ADDR = 0x27;
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
const unsigned long LCD_TIMEOUT = 20000; // 20초 후 LCD 끄기
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

// ------------ 온도 센서 설정 ------------
const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ------------ 버튼 설정 ------------
const int BUTTON_PIN = 3;
ezButton button(BUTTON_PIN);

// ------------ 센서 관련 변수 ------------
const int REQUIRED_SENSOR_COUNT = 4; // 필요한 센서 수
DeviceAddress sensorAddresses[REQUIRED_SENSOR_COUNT];
int sensorIDs[REQUIRED_SENSOR_COUNT];
bool sensorValid[REQUIRED_SENSOR_COUNT];

// ------------ 타이머 변수 ------------
unsigned long lcdOnTime = 0;                // LCD 켜진 시각
unsigned long lastSerialTime = 0;           // 시리얼 출력 갱신 시간
const unsigned long SERIAL_INTERVAL = 2000; // 2초 간격

// 전역 변수: LCD 처음 켜질 때 화면 초기화를 위해 사용
bool isFirstLCDRun = true;

// ------------ 함수 선언 ------------
void initializeSensorSystem();
void readSensorAddresses();
void readSensorIDs();
void sortSensors();
void showMessageOnLCD(const char *message);
void updateLCDDisplay();
void updateSerialDisplay();

// ------------ 센서 시스템 초기화 ------------
void initializeSensorSystem()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Initializing sensors...");
    Serial.println("[Init] 센서 초기화 시작");

    sensors.begin();
    uint8_t deviceCount = sensors.getDeviceCount();

    if (deviceCount == 0)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No sensors!");
        Serial.println("[Error] 센서가 감지되지 않음");
        while (1)
            ; // 더 이상 진행하지 않음
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
        Serial.println("[Warning] 필요한 센서 수 부족");
        // 계속 진행하지만 오류 메시지 출력
    }

    // 센서 주소 읽기
    readSensorAddresses();
    delay(1000); // 잠시 대기

    // 센서 ID 읽기
    readSensorIDs();
    delay(1000);

    // 센서 정렬
    sortSensors();
    delay(1000);

    lcd.clear();
    lcd.print("Initialization complete");
    Serial.println("[Init] 초기화 완료");
    delay(1000);
}

// ------------ 센서 주소 읽기 ------------
void readSensorAddresses()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reading addresses...");
    Serial.println("[Init] 센서 주소 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
        // sensors.getAddress returns true if address is read successfully
        sensorValid[i] = sensors.getAddress(sensorAddresses[i], i);
        if (!sensorValid[i])
        {
            Serial.print("[Error] 센서 ");
            Serial.print(i);
            Serial.println("의 주소 읽기 실패");
        }
    }
}

// ------------ 센서 ID 읽기 ------------
void readSensorIDs()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reading IDs...");
    Serial.println("[Init] 센서 ID 읽기 시작");

    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++)
    {
        if (sensorValid[i])
        {
            sensorIDs[i] = sensors.getUserData(sensorAddresses[i]);
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

// ------------ 센서 정렬 (Bubble Sort) ------------
void sortSensors()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sorting sensors...");
    Serial.println("[Init] 센서 정렬 시작");

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
                memcpy(tempAddr, sensorAddresses[j], sizeof(DeviceAddress));
                memcpy(sensorAddresses[j], sensorAddresses[j + 1], sizeof(DeviceAddress));
                memcpy(sensorAddresses[j + 1], tempAddr, sizeof(DeviceAddress));

                // Swap validity
                bool tempValid = sensorValid[j];
                sensorValid[j] = sensorValid[j + 1];
                sensorValid[j + 1] = tempValid;
            }
        }
    }
}

// ------------ LCD에 간단한 메시지 출력 ------------
void showMessageOnLCD(const char *message)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}

// ------------ LCD 정보 업데이트 ------------
void updateLCDDisplay()
{
    unsigned long currentMillis = millis();

    // 버튼을 누르면 LCD 타이머를 갱신
    if (button.isPressed())
    {
        lcdOnTime = currentMillis;
        isFirstLCDRun = true; // 다시 화면 초기화
    }

    // LCD 켜져야 하는 시간인지 확인
    if (currentMillis - lcdOnTime < LCD_TIMEOUT)
    {
        lcd.setBacklight(255);
        // 화면 초기화가 필요한 경우
        if (isFirstLCDRun)
        {
            lcd.clear();
            isFirstLCDRun = false;
        }

        // 센서 정보를 LCD에 출력 (최대 4개 센서)
        sensors.requestTemperatures();
        for (int i = 0; i < REQUIRED_SENSOR_COUNT && i < LCD_ROWS; i++)
        {
            lcd.setCursor(0, i);
            lcd.print("ID:");
            lcd.print(sensorValid[i] ? String(sensorIDs[i]) : "-");
            lcd.print(" ");

            float tempC = sensorValid[i] ? sensors.getTempC(sensorAddresses[i]) : DEVICE_DISCONNECTED_C;
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
            // 오른쪽에 상태 표시
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

// ------------ 시리얼에 센서 정보 업데이트 ------------
void updateSerialDisplay()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastSerialTime < SERIAL_INTERVAL)
        return;
    lastSerialTime = currentMillis;

    sensors.requestTemperatures();
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

        float tempC = sensors.getTempC(sensorAddresses[i]);
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
            if (sensorAddresses[i][j] < 16)
                Serial.print("0");
            Serial.print(sensorAddresses[i][j], HEX);
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

// ------------ setup 함수 ------------
void setup()
{
    Serial.begin(9600);
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setBacklight(255);
    button.setDebounceTime(50);

    // 초기 타이머 설정
    lcdOnTime = millis();

    // 센서 시스템 초기화 (주소, ID, 정렬)
    initializeSensorSystem();
}

// ------------ loop 함수 ------------
void loop()
{
    button.loop();         // 버튼 상태 업데이트
    updateLCDDisplay();    // LCD 화면 업데이트
    updateSerialDisplay(); // 시리얼 모니터 업데이트
}

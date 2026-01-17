/*
  MQ135 가스 센서, LCD, SD 카드 및 WiFi를 사용한 데이터 로깅 시스템
*/

#include <MQUnifiedsensor.h>
#include <LiquidCrystal_PCF8574.h>
#include <SPI.h>
#include <SdFat.h>
#include <WiFiS3.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// MQ135 센서 설정
#define SENSOR_PIN A0
#define MQ135_TYPE "MQ-135"
#define VOLTAGE_RESOLUTION 5
#define ADC_RESOLUTION 10
#define RATIO_CLEAN_AIR 3.6

MQUnifiedsensor gasSensor("Arduino UNO", VOLTAGE_RESOLUTION, ADC_RESOLUTION, SENSOR_PIN, MQ135_TYPE);

float co, alcohol, co2, toluen, nh4, aceton;

// LCD 설정
const int I2C_ADDR = 0x27;
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

// SD 카드 설정
SdFat SD;
const int SD_CS_PIN = 10;
const String FILE_NAME = "gas_data.csv";

// WiFi 및 NTP 설정
const char *SSID = "";
const char *PASSWORD = "";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 32400);

// 타이머 설정
unsigned long prevMillis = 0;
const int LOG_INTERVAL = 1000;
const unsigned long DISPLAY_CYCLE = 10;
const unsigned long MAIN_DISPLAY_TIME = DISPLAY_CYCLE / 2;
bool isMainDisplay = true;

// MQ135 센서 초기화 함수
void setupMQ135()
{
    gasSensor.setRegressionMethod(1);
    gasSensor.init();

    Serial.print("센서 보정 중...");
    float r0Sum = 0;
    for (int i = 0; i < 10; i++)
    {
        gasSensor.update();
        r0Sum += gasSensor.calibrate(RATIO_CLEAN_AIR);
        Serial.print(".");
    }
    gasSensor.setR0(r0Sum / 10);
    Serial.println(" 완료!");
}

// MQ135 센서 데이터 읽기
void readGasSensor()
{
    gasSensor.update();
    gasSensor.setA(605.18);
    gasSensor.setB(-3.937);
    co = gasSensor.readSensor();
    gasSensor.setA(77.255);
    gasSensor.setB(-3.18);
    alcohol = gasSensor.readSensor();
    gasSensor.setA(110.47);
    gasSensor.setB(-2.862);
    co2 = gasSensor.readSensor();
    gasSensor.setA(44.947);
    gasSensor.setB(-3.445);
    toluen = gasSensor.readSensor();
    gasSensor.setA(102.2);
    gasSensor.setB(-2.473);
    nh4 = gasSensor.readSensor();
    gasSensor.setA(34.668);
    gasSensor.setB(-3.369);
    aceton = gasSensor.readSensor();
}

// LCD 초기화
void setupLCD()
{
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setBacklight(80);
}

// LCD 출력 함수
void displayGasData(const char *label, float value, int row)
{
    lcd.setCursor(0, row);
    lcd.print(label);
    lcd.setCursor(10, row);
    lcd.print(value, 1);
    lcd.print(" ppm");
}

// LCD 메인 화면
void displayMainScreen()
{
    displayGasData("CO:", co, 0);
    displayGasData("CO2:", co2 + 400, 1);
    displayGasData("NH4:", nh4, 2);
}

// LCD 서브 화면
void displaySubScreen()
{
    displayGasData("Aceton:", aceton, 0);
    displayGasData("Alcohol:", alcohol, 1);
    displayGasData("Toluen:", toluen, 2);
}

// SD 카드 초기화
void setupSDCard()
{
    if (!SD.begin(SD_CS_PIN))
    {
        Serial.println("SD 카드 초기화 실패!");
        while (1)
            ;
    }
    Serial.println("SD 카드 초기화 성공");
    if (!SD.exists(FILE_NAME))
    {
        File dataFile = SD.open(FILE_NAME, FILE_WRITE);
        dataFile.println("DateTime,CO,Alcohol,CO2,Toluen,NH4,Aceton");
        dataFile.close();
    }
}

// SD 카드 데이터 저장
void logDataToSD()
{
    timeClient.update();
    File dataFile = SD.open(FILE_NAME, FILE_WRITE);
    if (dataFile)
    {
        dataFile.printf("%04d-%02d-%02d %02d:%02d:%02d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                        year(), month(), day(), hour(), minute(), second(),
                        co, alcohol, co2 + 400, toluen, nh4, aceton);
        dataFile.close();
        Serial.println("데이터 저장 완료");
    }
    else
    {
        Serial.println("SD 카드 쓰기 실패");
    }
}

// WiFi 연결 설정
void setupWiFi()
{
    Serial.print("WiFi 연결 중...");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi 연결 완료");
    timeClient.begin();
}

// 아두이노 초기화
void setup()
{
    Serial.begin(9600);
    setupWiFi();
    setupMQ135();
    setupLCD();
    setupSDCard();
}

// 메인 루프
void loop()
{
    if (millis() - prevMillis >= LOG_INTERVAL)
    {
        prevMillis = millis();
        readGasSensor();
        logDataToSD();
    }

    unsigned long timeInCycle = (millis() / 1000) % DISPLAY_CYCLE;
    bool shouldDisplayMain = (timeInCycle < MAIN_DISPLAY_TIME);
    if (shouldDisplayMain != isMainDisplay)
    {
        lcd.clear();
        isMainDisplay = shouldDisplayMain;
    }
    isMainDisplay ? displayMainScreen() : displaySubScreen();
}

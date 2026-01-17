// Include necessary libraries
#include <MQUnifiedsensor.h>
#include <LiquidCrystal_PCF8574.h>
#include <SPI.h>
#include <SdFat.h>
#include <WiFiS3.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//**** MQ135 Sensor Setup ******//
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A0                 // Analog input pin
#define type "MQ-135"          // MQ135
#define ADC_Bit_Resolution 10  // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm

// Create MQ135 sensor object
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

// Declare global variables for sensor values
float CO, Alcohol, CO2, Toluen, NH4, Aceton;

//**** LCD Setup ******//
const int I2C_ADDR = 0x27;
const int LCD_COLUMNS = 20;
const int LCD_ROWS = 4;

// Create LCD object
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

//***** SD CARD Setup ****//
SdFat SD;
const int chipSelect = 10;        // SD card CS pin
String fileName = "gas_data.csv"; // File name to save data

//**** Timing Variables ******//
unsigned long previousMillis = 0;
const int INTERVAL = 1000; // Interval for loop execution (1 second)
const unsigned long DISPLAY_CYCLE = 10;
const unsigned long MAIN_CYCLE = DISPLAY_CYCLE / 2;

// WiFi and NTP Client Setup
const char *ssid = "   ";        // WiFi SSID
const char *password = "      "; // WiFi Password

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//******************************/
//*  MQ135 Setup Function *
//*******************************/
void setup_MQ135()
{
    MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
    MQ135.init();

    Serial.print("Calibrating please wait.");
    float calcR0 = 0;
    for (int i = 1; i <= 10; i++)
    {
        MQ135.update();
        calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
        Serial.print(".");
    }

    MQ135.setR0(calcR0 / 10);
    Serial.println("  done!");

    if (isinf(calcR0))
    {
        Serial.println("Warning: Connection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
        while (1)
            ;
    }
    if (calcR0 == 0)
    {
        Serial.println("Warning: Connection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
        while (1)
            ;
    }
}

//******************************/
//*  MQ135 Loop Function *
//*******************************/
void loop_MQ135()
{
    MQ135.update(); // Update sensor data

    MQ135.setA(605.18);
    MQ135.setB(-3.937);
    CO = MQ135.readSensor();

    MQ135.setA(77.255);
    MQ135.setB(-3.18);
    Alcohol = MQ135.readSensor();

    MQ135.setA(110.47);
    MQ135.setB(-2.862);
    CO2 = MQ135.readSensor();

    MQ135.setA(44.947);
    MQ135.setB(-3.445);
    Toluen = MQ135.readSensor();

    MQ135.setA(102.2);
    MQ135.setB(-2.473);
    NH4 = MQ135.readSensor();

    MQ135.setA(34.668);
    MQ135.setB(-3.369);
    Aceton = MQ135.readSensor();
}

//******************************/
//*  LCD Display Setup Function *
//*******************************/
void setup_LCD()
{
    lcd.begin(LCD_COLUMNS, LCD_ROWS);
    lcd.setBacklight(80);
}

//******************************/
//*  LCD Value Alignment Function *
//*******************************/
void printAligned(float value, int row, const char *label)
{
    int intPart = (int)value;
    int intDigits = (intPart == 0) ? 1 : (log10(abs(intPart)) + 1); // Calculate integer part digits
    int decimalPos = 12 - intDigits;                                // Calculate decimal position

    lcd.setCursor(0, row);
    lcd.print(label);

    lcd.setCursor(decimalPos, row);
    lcd.print(value, 1);

    lcd.setCursor(16, row);
    lcd.print(" ppm");
}

//******************************/
//*  LCD Display Loop Functions *
//*******************************/
void loop_LCD_main()
{
    printAligned(CO, 0, "CO :");
    printAligned(CO2 + 400, 1, "CO2:"); // Add 400 offset as per library recommendation
    printAligned(NH4, 2, "NH4:");
}

void loop_LCD_sub()
{
    printAligned(Aceton, 0, "Aceton :");
    printAligned(Alcohol, 1, "Alcohol:");
    printAligned(Toluen, 2, "Toluen :");
}

//******************************/
//*  SD CARD Setup Function *
//******************************/
void setup_SdCard()
{
    if (!SD.begin(chipSelect))
    {
        Serial.println("SD card initialization failed!");
        while (1)
            ;
    }
    Serial.println("SD card initialization successful");

    // Write header if file does not exist
    if (!SD.exists(fileName))
    {
        File dataFile = SD.open(fileName, FILE_WRITE);
        if (dataFile)
        {
            dataFile.println("DateTime,CO,Alcohol,CO2,Toluen,NH4,Aceton");
            dataFile.close();
        }
    }
}

//******************************/
//*  WiFi Setup Function *
//******************************/
void setup_WiFi()
{
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    timeClient.begin();
    timeClient.setTimeOffset(32400); // UTC+9 (9 hours = 9 * 60 * 60 seconds)
    timeClient.update();
}

//******************************/
//*  SD CARD Loop Function *
//******************************/
void loop_SdCard()
{
    timeClient.update();

    File dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile)
    {
        // DateTime format: YYYY-MM-DD HH:MM:SS
        time_t epochTime = timeClient.getEpochTime();
        struct tm *ptm = localtime((time_t *)&epochTime);

        dataFile.print(ptm->tm_year + 1900);
        dataFile.print("-");
        if (ptm->tm_mon + 1 < 10)
            dataFile.print("0");
        dataFile.print(ptm->tm_mon + 1);
        dataFile.print("-");
        if (ptm->tm_mday < 10)
            dataFile.print("0");
        dataFile.print(ptm->tm_mday);
        dataFile.print(" ");
        if (ptm->tm_hour < 10)
            dataFile.print("0");
        dataFile.print(ptm->tm_hour);
        dataFile.print(":");
        if (ptm->tm_min < 10)
            dataFile.print("0");
        dataFile.print(ptm->tm_min);
        dataFile.print(":");
        if (ptm->tm_sec < 10)
            dataFile.print("0");
        dataFile.print(ptm->tm_sec);
        dataFile.print(",");

        dataFile.print(CO);
        dataFile.print(",");
        dataFile.print(Alcohol);
        dataFile.print(",");
        dataFile.print(CO2 + 400);
        dataFile.print(",");
        dataFile.print(Toluen);
        dataFile.print(",");
        dataFile.print(NH4);
        dataFile.print(",");
        dataFile.println(Aceton);
        dataFile.close();

        Serial.println("Data saved");
    }
    else
    {
        Serial.println("Failed to open file");
    }
}

//******************************/
//*  Setup Function *
//******************************/
void setup()
{
    Serial.begin(9600);

    setup_WiFi();
    setup_MQ135();
    setup_LCD();
    setup_SdCard();
}

//******************************/
//*  Loop Function *
//******************************/
static bool displayingMain = true;

void loop()
{
    if (millis() - previousMillis >= INTERVAL)
    {
        previousMillis = millis();
        loop_MQ135();
        loop_SdCard(); // Call SD card save function
    }

    unsigned long currentInterval = (millis() / 1000) % DISPLAY_CYCLE;
    bool shouldDisplayMain = (currentInterval < MAIN_CYCLE);

    // Clear LCD only on display switch
    if (shouldDisplayMain != displayingMain)
    {
        lcd.clear();
        displayingMain = shouldDisplayMain;
    }
    if (displayingMain)
    {
        loop_LCD_main();
    }
    else
    {
        loop_LCD_sub();
    }
}
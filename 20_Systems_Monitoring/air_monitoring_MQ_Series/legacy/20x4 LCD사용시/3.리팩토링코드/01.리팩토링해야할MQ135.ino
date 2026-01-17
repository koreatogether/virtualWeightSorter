/*
  SdFat library
*/

// Include the library
#include <MQUnifiedsensor.h>
#include <LiquidCrystal_PCF8574.h>
#include <SPI.h>
#include <SdFat.h>
#include <WiFiS3.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//**** MQ135 ******//
// Definitions
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A0                 // Analog input 0 of your arduino
#define type "MQ-135"          // MQ135
#define ADC_Bit_Resolution 10  // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm
// #define calibration_button 13 //Pin to calibrate your sensor

// Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

// 다른 함수에서 MQ135 변수들을 사용하기 위해서 전역변수로 선언
float CO, Alcohol, CO2, Toluen, NH4, Aceton;

//**** LCD ******//
// LCD 객체
const int I2C_ADDR = 0x27;
const int LCD_COLUNMS = 20;
const int LCD_ROWS = 4;

LiquidCrystal_PCF8574 lcd(I2C_ADDR); // set the LCD address to 0x27 for a 16(20) chars and 2(4) line display

//***** SD CARD ****//
SdFat SD;
const int chipSelect = 10;        // SD 카드 CS 핀
String fileName = "gas_data.csv"; // 저장할 파일 이름

//*****/ 루프함수내 반복 실행에 관련된 변수 ****** *//
unsigned long previousMillis = 0; // 실행의 시작 지점이었던 millis 값
const int INTERVAL = 1000;
// 루프 함수에서 non blocking 코드기법을 사용하기 위한 시간 간격 ,1초 (ms)

// 모든 화면이 유지되는 시간
const unsigned long DISPLAY_CYCLE = 10;
// 각 화면이 유지되는 시간
const unsigned long MAIN_CYCLE = DISPLAY_CYCLE / 2;

// WiFi 및 NTP 클라이언트 설정
// ############################
const char *ssid = "   ";        // WiFi 이름
const char *password = "      "; // WiFi 비밀번호
// ############################

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org"); // 기본 UTC+0로 설정

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
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0)
  {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1)
      ;
  }
}

//******************************/
//*  MQ135 Loop Function *
//*******************************/

void loop_MQ135()
{
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin

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

///******************************/
//*  LCD Display Setup Function *
//*******************************/

void setup_LCD()
{
  lcd.begin(LCD_COLUNMS, LCD_ROWS);
  lcd.setBacklight(80);
}

//******************************/
//*  LCD Value Alingn Function *
//*******************************/

void printAligned(float value, int row, const char *label)
{
  int intPart = (int)value;
  int intDigits = (intPart == 0) ? 1 : (log10(abs(intPart)) + 1); // 정수 부분 자릿수 계산
  int decimalPos = 12 - intDigits;                                // 소수점 위치 계산

  lcd.setCursor(0, row);
  lcd.print(label);

  lcd.setCursor(decimalPos, row);
  lcd.print(value, 1);

  lcd.setCursor(16, row);
  lcd.print(" ppm");
}

//******************************/
//*  LCD Display Loop Function *
//*******************************/

void loop_LCD_main()
{
  printAligned(CO, 0, "CO :");
  printAligned(CO2 + 400, 1, "CO2:"); // 라이브러리에서 400의 offset을 더하라고 함.
  printAligned(NH4, 2, "NH4:");
}

void loop_LCD_sub()
{
  printAligned(Aceton, 0, "Aceton :");
  printAligned(Alcohol, 1, "Alcohol:");
  printAligned(Toluen, 2, "Toluen :");
}

//******************************/
// SD CARD Setup Function
//******************************/
void setup_SdCard()
{
  if (!SD.begin(chipSelect))
  {
    Serial.println("SD 카드 초기화 실패!");
    while (1)
      ;
  }
  Serial.println("SD 카드 초기화 성공");

  // 파일이 없을 경우 헤더 작성
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

// WiFi Setup Function
void setup_WiFi()
{
  Serial.print("WiFi 연결 중...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 연결됨");

  timeClient.begin();
  timeClient.setTimeOffset(32400); // UTC+9 (9시간 = 9 * 60 * 60초)
  timeClient.update();
}

//******************************/
// SD CARD Loop Function
//******************************/
void loop_SdCard()
{
  timeClient.update();

  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile)
  {
    // DateTime 포맷: YYYY-MM-DD HH:MM:SS
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = localtime((time_t *)&epochTime); // gmtime 대신 localtime 사용

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

    Serial.println("데이터 저장 완료");
  }
  else
  {
    Serial.println("파일 열기 실패");
  }
}

//******************************/
//*  Setup Function *
//*******************************/
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
//*******************************/
static bool displayingMain = true;

void loop()
{
  if (millis() - previousMillis >= INTERVAL)
  {
    previousMillis = millis();
    loop_MQ135();
    loop_SdCard(); // SD 카드 저장 함수 호출
  }
  unsigned long currentInterval = (millis() / 1000) % DISPLAY_CYCLE;
  bool shouldDisplayMain = (currentInterval < MAIN_CYCLE);

  // 화면 전환 시에만 lcd.clear() 실행
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
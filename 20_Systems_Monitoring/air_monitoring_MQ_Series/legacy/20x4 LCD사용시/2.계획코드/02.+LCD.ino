/*
  MQUnifiedsensor Library - reading an MQ135

  Demonstrates the use a MQ135 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez

  Added example
  modified 23 May 2019
  by Miguel Califa

  Updated library usage
  modified 26 March 2020
  by Miguel Califa

  Wiring:
  https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/static/img/MQ_Arduino.PNG
  Please make sure arduino A0 pin represents the analog input configured on #define pin

 This example code is in the public domain.

*/

// Include the library
#include <MQUnifiedsensor.h>
#include <LiquidCrystal_PCF8574.h>

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

//* 루프함수내 반복 실행에 관련된 변수  *//
unsigned long previousMillis = 0; // 실행의 시작 지점이었던 millis 값
const int INTERVAL = 1000;        // 루프 함수에서 non blocking 코드기법을 사용하기 위한 시간 간격 ,1초 (ms)

// 모든 화면이 유지되는 시간
const unsigned long DISPLAY_CYCLE = 10;
// 각 화면이 유지되는 시간
const unsigned long MAIN_CYCLE = DISPLAY_CYCLE / 2;

//******************************/
//*  MQ135 Setup Function *
//*******************************/
void setup_MQ135()
{
  // Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b

  /*****************************  MQ Init ********************************************/
  // Remarks: Configure the pin of arduino as input.
  /************************************************************************************/
  MQ135.init();
  /*
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ135.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/
  // Explanation:
  // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recomend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!.");

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
  MQ135.setB(-3.937);      // Configure the equation to calculate CO concentration value
  CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(77.255);
  MQ135.setB(-3.18);            // Configure the equation to calculate Alcohol concentration value
  Alcohol = MQ135.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(110.47);
  MQ135.setB(-2.862);       // Configure the equation to calculate CO2 concentration value
  CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(44.947);
  MQ135.setB(-3.445);          // Configure the equation to calculate Toluen concentration value
  Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(102.2);
  MQ135.setB(-2.473);       // Configure the equation to calculate NH4 concentration value
  NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(34.668);
  MQ135.setB(-3.369);          // Configure the equation to calculate Aceton concentration value
  Aceton = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  // Serial.print("|   ");
  // Serial.print(CO);
  // Serial.print("   |   ");
  // Serial.print(Alcohol);
  // Note: 400 Offset for CO2 source: https://github.com/miguel5612/MQSensorsLib/issues/29
  /*
  Motivation:
  We have added 400 PPM because when the library is calibrated it assumes the current state of the
  air as 0 PPM, and it is considered today that the CO2 present in the atmosphere is around 400 PPM.
  https://www.lavanguardia.com/natural/20190514/462242832581/concentracion-dioxido-cabono-co2-atmosfera-bate-record-historia-humanidad.html
  */
  // Serial.print("   |   ");
  // Serial.print(CO2 + 400);
  // Serial.print("   |   ");
  // Serial.print(Toluen);
  // Serial.print("   |   ");
  // Serial.print(NH4);
  // Serial.print("   |   ");
  // Serial.print(Aceton);
  // Serial.println("   |");
  /*
    Exponential regression:
  GAS      | a      | b
  CO       | 605.18 | -3.937
  Alcohol  | 77.255 | -3.18
  CO2      | 110.47 | -2.862
  Toluen  | 44.947 | -3.445
  NH4      | 102.2  | -2.473
  Aceton  | 34.668 | -3.369
  */
}

///******************************/
//*  LCD Display Setup Function *
//*******************************/

void setup_LCD()
{
  lcd.begin(LCD_COLUNMS, LCD_ROWS); // initialize the lcd
  lcd.setBacklight(80);             // set the backlight on full
}

//******************************/
//*  LCD Display Loop Function *
//*******************************/

void loop_LCD_main()
{
  /* LCD 화면 배치 ( 0 ~ 3초 )
  CO : 0.0  ppm
  CO2: 0.0  ppm
  NH4: 0.0  ppm
  */

  // 첫번째줄 왼쪽
  lcd.setCursor(0, 0);
  lcd.print("CO :");
  lcd.setCursor(6, 0);
  lcd.print(CO, 1);
  lcd.setCursor(16, 0);
  lcd.print(" ppm");
  // 첫번째줄 오른쪽
  lcd.setCursor(0, 1);
  lcd.print("CO2:");
  lcd.setCursor(6, 1);
  lcd.print(CO2 + 400, 1); // 라이브러리의 조언에 따라서 400을 더해줌
  lcd.setCursor(16, 1);
  lcd.print(" ppm");
  // 두번째줄 왼쪽
  lcd.setCursor(0, 2);
  lcd.print("NH4:");
  lcd.setCursor(6, 2);
  lcd.print(NH4, 1);
  lcd.setCursor(16, 2);
  lcd.print(" ppm");
}

void loop_LCD_sub()
{
  /* LCD 화면 배치 (4 ~ 6초)
  Aceton : 0.0  ppm
  Alcohol: 0.0  ppm
  Toluen : 0.0  ppm
  */

  lcd.setCursor(0, 0);
  lcd.print("Aceton :");
  lcd.setCursor(10, 0);
  lcd.print(Aceton, 1);
  lcd.setCursor(16, 0);
  lcd.print(" ppm");
  // 세번째줄
  lcd.setCursor(0, 1);
  lcd.print("Alcohol: ");
  lcd.setCursor(10, 1);
  lcd.print(Alcohol, 1);
  lcd.setCursor(16, 1);
  lcd.print(" ppm");
  // 네번째줄
  lcd.setCursor(0, 2);
  lcd.print("Toluen :");
  lcd.setCursor(10, 2);
  lcd.print(Toluen, 1);
  lcd.setCursor(16, 2);
  lcd.print(" ppm");
}

/*
 * 각 가스 PPM 정수값 에 따라서 표시자리를 달리 달리 함수
 * 정수부분 값이 1자리인경우 14번째 열부터 출력
 * 정수부분 값이 2자리인경우 13번째 열부터 출력
 * 정수부분 값이 3자리인경우 12번째 열부터 출력
 * 소수부분 값이 1자리인경우 10번째 열부터 출력
 */

//******************************/
//*  Setup Function *
//*******************************/
void setup()
{

  Serial.begin(9600);

  // MQ135 셋업 함수 호출
  setup_MQ135();

  // LCD 셋업 함수 호출
  setup_LCD();
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
  }

  unsigned long currentInterval = (millis() / 1000) % DISPLAY_CYCLE; // 설정된 전역 변수 사용
  bool shouldDisplayMain = (currentInterval < MAIN_CYCLE);           // 설정된 전역 변수 사용

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

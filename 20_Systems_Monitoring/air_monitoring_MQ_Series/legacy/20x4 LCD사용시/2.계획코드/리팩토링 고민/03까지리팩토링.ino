#include <MQUnifiedsensor.h>
#include <LiquidCrystal_PCF8574.h>

// Definitions
#define PLACA "Arduino UNO"
#define VOLTAGE_RESOLUTION 5
#define MQ135_PIN A0
#define SENSOR_TYPE "MQ-135"
#define ADC_BIT_RESOLUTION 10
#define RATIO_MQ135_CLEAN_AIR 3.6
#define I2C_ADDR 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4
#define INTERVAL 1000
#define DISPLAY_CYCLE 10
#define MAIN_CYCLE (DISPLAY_CYCLE / 2)

// Declare Sensor
MQUnifiedsensor MQ135(PLACA, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, MQ135_PIN, SENSOR_TYPE);

// LCD 객체
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

// Function Prototypes
void setup_MQ135();
void calculateGasConcentrations(float &CO, float &Alcohol, float &CO2, float &Toluen, float &NH4, float &Aceton);
void setup_LCD();
void printAligned(float value, int row, const char *label);
void displayMainScreen(float CO, float CO2, float NH4);
void displaySubScreen(float Aceton, float Alcohol, float Toluen);
void setup();
void loop();

void setup_MQ135() {
  MQ135.setRegressionMethod(1);
  MQ135.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0 += MQ135.calibrate(RATIO_MQ135_CLEAN_AIR);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println(" done!");
  if (isinf(calcR0) || calcR0 == 0) {
    Serial.println("Warning: Connection issue detected. Please check your wiring and supply.");
    while (1);
  }
}

void calculateGasConcentrations(float &CO, float &Alcohol, float &CO2, float &Toluen, float &NH4, float &Aceton) {
  MQ135.update();
  MQ135.setA(605.18); MQ135.setB(-3.937); CO = MQ135.readSensor();
  MQ135.setA(77.255); MQ135.setB(-3.18); Alcohol = MQ135.readSensor();
  MQ135.setA(110.47); MQ135.setB(-2.862); CO2 = MQ135.readSensor();
  MQ135.setA(44.947); MQ135.setB(-3.445); Toluen = MQ135.readSensor();
  MQ135.setA(102.2); MQ135.setB(-2.473); NH4 = MQ135.readSensor();
  MQ135.setA(34.668); MQ135.setB(-3.369); Aceton = MQ135.readSensor();
}

void setup_LCD() {
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.setBacklight(80);
}

void printAligned(float value, int row, const char *label) {
  int intPart = (int)value;
  int intDigits = (intPart == 0) ? 1 : (log10(abs(intPart)) + 1);
  int decimalPos = 12 - intDigits;
  lcd.setCursor(0, row);
  lcd.print(label);
  lcd.setCursor(decimalPos, row);
  lcd.print(value, 1);
  lcd.setCursor(16, row);
  lcd.print(" ppm");
}

void displayMainScreen(float CO, float CO2, float NH4) {
  printAligned(CO, 0, "CO :");
  printAligned(CO2 + 400, 1, "CO2:");
  printAligned(NH4, 2, "NH4:");
}

void displaySubScreen(float Aceton, float Alcohol, float Toluen) {
  printAligned(Aceton, 0, "Aceton :");
  printAligned(Alcohol, 1, "Alcohol:");
  printAligned(Toluen, 2, "Toluen :");
}

void setup() {
  Serial.begin(9600);
  setup_MQ135();
  setup_LCD();
}

void loop() {
  static unsigned long previousMillis = 0;
  static bool displayingMain = true;
  float CO, Alcohol, CO2, Toluen, NH4, Aceton;

  if (millis() - previousMillis >= INTERVAL) {
    previousMillis = millis();
    calculateGasConcentrations(CO, Alcohol, CO2, Toluen, NH4, Aceton);
  }

  unsigned long currentInterval = (millis() / 1000) % DISPLAY_CYCLE;
  bool shouldDisplayMain = (currentInterval < MAIN_CYCLE);

  if (shouldDisplayMain != displayingMain) {
    lcd.clear();
    displayingMain = shouldDisplayMain;
  }

  if (displayingMain) {
    displayMainScreen(CO, CO2, NH4);
  } else {
    displaySubScreen(Aceton, Alcohol, Toluen);
  }
}
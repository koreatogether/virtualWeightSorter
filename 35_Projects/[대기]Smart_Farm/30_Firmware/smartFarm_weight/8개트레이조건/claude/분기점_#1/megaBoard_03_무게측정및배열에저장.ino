// https://wokwi.com/projects/406754858717524993
// https://wokwi.com/projects/406756784320601089
// https://wokwi.com/projects/406760179081512961  (핀 충돌 해결한 버전)

/* INFO

가싱시뮬레이터 지금까지 확왼한것들

객체 선언시 배열 가능

라이브러리 안에서 pinMode 선언이 있으나 코드상에 다시한번선언을 해주어야 핀이 제대로 동작함
--> 원인 파악해야함  가상 시뮬레이터가 옛날 버전일수도

객체 배열 선언이 먹히므로 나머지 코드에서도 배열을 사용가능해서 코드가 간결해짐
https://wokwi.com/projects/406757678203406337

개선한 점
- 객체 배열 선언으로 코드가 간결해짐
*/
#include <HX711.h> // Rob Tillaart's HX711 library  https://github.com/RobTillaart/HX711
// #include <Adafruit_NeoPixel.h>

const int DOUT_SCK_PIN[8][2] = {
    {23, 25}, // DOUT_PIN[0], SCK_PIN[0]
    {27, 29}, // DOUT_PIN[1], SCK_PIN[1]
    {31, 33}, // DOUT_PIN[2], SCK_PIN[2]
    {34, 37}, // DOUT_PIN[3], SCK_PIN[3]
    {39, 41}, // DOUT_PIN[4], SCK_PIN[4]
    {7, 6},   // DOUT_PIN[5], SCK_PIN[5]  // 핀 바궈보기 , 43 핀부터 쓰지 말것 충돌남
    {5, 4},   // DOUT_PIN[6], SCK_PIN[6]
    {3, 2}    // DOUT_PIN[7], SCK_PIN[7]
};

const float CALIBRATION_FACTOR = 0.420f; // calibration factor
const int TARE_COUNT = 5;                // number of tare readings
const int UNITS_COUNT = 1;               // number of get_units readings
const int DECIMAL_PLACES = 1;

// 배열 객체 선언
HX711 scale[8];

// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  // pixels.begin();

  for (int i = 0; i < 8; i++)
  {
    Serial.println("array start");
    pinMode(DOUT_SCK_PIN[i][0], INPUT);
    digitalWrite(DOUT_SCK_PIN[i][0], LOW);
    pinMode(DOUT_SCK_PIN[i][1], OUTPUT);
    Serial.println("array end");
  }

  for (int i = 0; i < 8; i++)
  {
    Serial.println("begin start");
    scale[i].begin(DOUT_SCK_PIN[i][0], DOUT_SCK_PIN[i][1], false);
    delay(100);
    scale[i].set_scale(CALIBRATION_FACTOR); // Corrected: Moved inside the loop
    scale[i].tare(TARE_COUNT);              // Corrected: Moved inside the loop
    Serial.println("begin end");
    delay(1000);
  }

  // pixels.clear();
  Serial.println("end");
}

void loop()
{

  for (int i = 0; i < 2; i++)
  {
    Serial.print("Channel ");
    Serial.print(i + 1);
    Serial.print("\t");
    Serial.print("raw value: ");
    Serial.print(scale[i].read_average(UNITS_COUNT));
    Serial.print("\t");
    Serial.print("units: ");
    Serial.println(scale[i].get_units(UNITS_COUNT), DECIMAL_PLACES);
  }

  Serial.println("");
  Serial.println("=============================================");
  Serial.println("");
  delay(3000);
}

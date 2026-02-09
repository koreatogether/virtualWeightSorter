// https://wokwi.com/projects/406754858717524993
// https://wokwi.com/projects/406756784320601089

/* INFO

가싱시뮬레이터 지금까지 확왼한것들

객체 선언시 배열 가능

라이브러리 안에서 pinMode 선언이 있으나 코드상에 다시한번선언을 해주어야 핀이 제대로 동작함
--> 원인 파악해야함  가상 시뮬레이터가 옛날 버전일수도

객체 배열 선언이 먹히므로 나머지 코드에서도 배열을 사용가능해서 코드가 간결해짐
https://wokwi.com/projects/406757678203406337
*/

#include <HX711.h> // Rob Tillaart's HX711 library  https://github.com/RobTillaart/HX711
// #include <Adafruit_NeoPixel.h>

const int DOUT_PIN[] = {23, 27, 31, 34, 39, 43, 47, 51}; // clock pin
const int SCK_PIN[] = {25, 29, 33, 37, 41, 44, 49, 53};

const float CALIBRATION_FACTOR = 0.420f; // calibration factor
const int TARE_COUNT = 5;                // number of tare readings
const int UNITS_COUNT = 1;               // number of get_units readings
const int DECIMAL_PLACES = 1;

// 배열 객체 선언
HX711 scale[8];

// HX711 scale0;
// HX711 scale1;
// HX711 scale2;
// HX711 scale3;
// HX711 scale4;
// HX711 scale5;
// HX711 scale6;
// HX711 scale7;
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  // pixels.begin();

  // for (int i = 0; i < 8 ; i++)
  // {
  //   scale[i].begin(DOUT_PIN[i], SCK_PIN[i]);
  //   scale[i].set_scale(CALIBRATION_FACTOR);  // Corrected: Moved inside the loop
  //   scale[i].tare(TARE_COUNT);               // Corrected: Moved inside the loop
  // }

  for (int i = 0; i < 7; i++)
  {
    pinMode(DOUT_PIN[i], INPUT);
    pinMode(DOUT_PIN[i], LOW);
  }

  for (int i = 0; i < 7; i++)
  {
    pinMode(SCK_PIN[i], OUTPUT);
  }

  scale[0].begin(23, 25, false);
  scale[0].set_scale(CALIBRATION_FACTOR); // Corrected: Moved inside the loop
  scale[0].tare(TARE_COUNT);              // Corrected: Moved inside the loop

  scale[1].begin(27, 29, false);
  scale[1].set_scale(CALIBRATION_FACTOR); // Corrected: Moved inside the loop
  scale[1].tare(TARE_COUNT);              // Corrected: Moved inside the loop

  // pixels.clear();
  Serial.println("end");
}

void loop()
{

  // for (int i = 0; i < 7; i ++)
  // {
  //   Serial.print("Channel ");
  //   Serial.print(i);
  //   Serial.print("\t");
  //   Serial.print("raw value: ");
  //   Serial.println(scale[i].read_average(UNITS_COUNT));
  //   Serial.print("\t");
  //   Serial.print("units: ");
  //   Serial.println(scale[i].get_units(UNITS_COUNT), DECIMAL_PLACES);
  // }

  //  if (!scale0.is_ready())
  // {
  //   Serial.println("not working ");
  //   delay(1000);
  // }

  Serial.print("Channel ");
  Serial.print("1");
  Serial.print("\t");
  Serial.print("raw value: ");
  Serial.print(scale[0].read_average(UNITS_COUNT));
  Serial.print("\t");
  Serial.print("units: ");
  Serial.println(scale[0].get_units(UNITS_COUNT), DECIMAL_PLACES);

  Serial.print("Channel ");
  Serial.print("2");
  Serial.print("\t");
  Serial.print("raw value: ");
  Serial.print(scale[1].read_average(UNITS_COUNT));
  Serial.print("\t");
  Serial.print("units: ");
  Serial.println(scale[1].get_units(UNITS_COUNT), DECIMAL_PLACES);

  Serial.println("");
  Serial.println("=============================================");
  Serial.println("");
  delay(3000);
}

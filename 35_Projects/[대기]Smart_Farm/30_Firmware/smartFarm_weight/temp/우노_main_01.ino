// https://wokwi.com/projects/406661174095835137
// https://wokwi.com/projects/406661332725479425
// https://wokwi.com/projects/406662999135199233 (네오 픽셀추가한것 )
// https://wokwi.com/projects/406663637732743169 (색상 정리 한것 )

// wokwi 네오픽셀 예제는 https://wokwi.com/projects/384650890071021569

#include <HX711.h> // Rob Tillaart's HX711 library  https://github.com/RobTillaart/HX711
#include <Adafruit_NeoPixel.h>

const int SCK_PIN = 2;                                  // clock pin
const int ARRAY_DOUT_PIN[] = {3, 4, 5, 6, 7, 8, 9, 10}; // data out pin
const int NUM_SCALES = sizeof(ARRAY_DOUT_PIN) / sizeof(ARRAY_DOUT_PIN[0]);

const float CALIBRATION_FACTOR = 420.5f; // calibration factor
const int TARE_COUNT = 5;                // number of tare readings
const int UNITS_COUNT = 5;               // number of get_units readings
const int DECIMAL_PLACES = 1;

const int NEO_PIXEL_PIN = 10;
const int NUM_PIXELS = 8;

// 배열 객체 선언
HX711 scale[8];

void setup()
{
    Serial.begin(115200);
    Serial.println("HX711 Multi Channel Example");

    for (int i = 0; i < NUM_SCALES; i++)
    {
        scale[i].begin(ARRAY_DOUT_PIN[i], SCK_PIN);
        scale[i].set_scale(CALIBRATION_FACTOR);
        scale[i].tare(TARE_COUNT);
    }

    // 배열 저울 무게 얻기
    for (int i = 0; i < NUM_SCALES; i++)
    {
        Serial.print("Channel ");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(scale[i].get_units(UNITS_COUNT), DECIMAL_PLACES);
    }
}

void loop()
{
}
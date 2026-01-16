// https://wokwi.com/projects/406661174095835137
// https://wokwi.com/projects/406661332725479425
// https://wokwi.com/projects/406662999135199233 (네오 픽셀추가한것 )
// https://wokwi.com/projects/406663637732743169 (색상 정리 한것 )
// https://wokwi.com/projects/406664383443382273 ( 저울쪽 코드 손봄 )
// https://wokwi.com/projects/406751213494683649

// 작동 안되어서 별도 테스트 링크 https://wokwi.com/projects/406751979631581185
// ** 저울 한개당 한개의 datapin , 한개의 sck 핀이필요한 것으로 파악됨
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

const int NEO_PIXEL_PIN = 11;
const int NUM_PIXELS = 8;

// 배열 객체 선언
HX711 scale[NUM_SCALES];
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    Serial.begin(115200);
    pixels.begin();
    Serial.println("HX711 Multi Channel Example");

    for (int i = 0; i < NUM_SCALES; i++)
    {
        scale[i].begin(ARRAY_DOUT_PIN[i], SCK_PIN);
        scale[i].set_scale(CALIBRATION_FACTOR); // Corrected: Moved inside the loop
        scale[i].tare(TARE_COUNT);              // Corrected: Moved inside the loop
    }

    for (int i = 0; i < NUM_SCALES; i++)
    {
        Serial.print("Channel ");
        Serial.print(i);
        Serial.print(" raw value: ");
        Serial.println(scale[i].read_average(UNITS_COUNT));
        Serial.print(" units: ");
        Serial.println(scale[i].get_units(UNITS_COUNT), DECIMAL_PLACES);
    }

    pixels.clear();
}

void loop()
{
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));   // RED (빨)
    pixels.setPixelColor(1, pixels.Color(255, 165, 0)); // Orange(주)
    pixels.setPixelColor(2, pixels.Color(255, 255, 0)); // Yellow(노)
    pixels.setPixelColor(3, pixels.Color(0, 255, 0));   // Green(초)
    pixels.setPixelColor(4, pixels.Color(0, 0, 255));   // Blue (파)
    pixels.setPixelColor(5, pixels.Color(75, 0, 130));  // Indigo (남)
    pixels.setPixelColor(6, pixels.Color(148, 0, 211)); // Violet (보)
    pixels.setPixelColor(7, pixels.Color(0, 100, 48));  // 임의의 색
    pixels.show();
    delay(100);

    for (int i = 0; i < NUM_SCALES; i++)
    {
        Serial.print("Channel ");
        Serial.print(i);
        Serial.print(" raw value: ");
        Serial.println(scale[i].read_average(UNITS_COUNT));
        Serial.print(" units: ");
        Serial.println(scale[i].get_units(UNITS_COUNT), DECIMAL_PLACES);
    }

    Serial.println("");
    Serial.println("=============================================");
    Serial.println("");
    delay(3000);
}

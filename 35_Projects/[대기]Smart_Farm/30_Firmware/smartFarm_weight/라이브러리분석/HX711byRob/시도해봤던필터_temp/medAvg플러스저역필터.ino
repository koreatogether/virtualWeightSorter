//
//    FILE: HX_set_mode.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711

// 24-08-09 테스트 완료 했고 느리지만 안정적으로 1g 단위까지는 잘 인식함

#include "HX711.h"

HX711 scale;

uint8_t dataPin = 12;
uint8_t clockPin = 11;

uint32_t start, stop;
volatile float f;

float filteredValue = 0.0;
float alpha = 0.1; // Smoothing factor, adjust between 0.0 and 1.0 for desired filtering effect
                   // 아두이노 나노 또는 우노는 0.1만으로도 벅차다.

// 저역필터 적용
float applyLowPassFilter(float newValue)
{
    filteredValue = alpha * newValue + (1 - alpha) * filteredValue;
    return filteredValue;
}

void setup()
{
    Serial.begin(115200);
    Serial.println(__FILE__);
    Serial.print("LIBRARY VERSION: ");
    Serial.println(HX711_LIB_VERSION);
    Serial.println();

    scale.begin(dataPin, clockPin);

    // TODO find a nice solution for this calibration..
    // loadcell factor
    scale.set_scale(466.768432);
    scale.set_gain(128);
    Serial.print("now gain is ");
    Serial.println(scale.get_gain());
    Serial.println();
    scale.set_average_mode();
    Serial.print(scale.get_mode());
    delay(1000);
    scale.tare();
}

void loop()
{
    Serial.println();
    scale.set_medavg_mode();
    Serial.print(scale.get_mode());

    // Get the raw weight value from the scale
    float rawValue = scale.get_units(3);

    // Apply the low-pass filter to stabilize the reading
    float stableValue = applyLowPassFilter(rawValue);

    // Print the filtered (stable) weight value
    Serial.print("  weight: ");
    Serial.println(stableValue, 3);
}

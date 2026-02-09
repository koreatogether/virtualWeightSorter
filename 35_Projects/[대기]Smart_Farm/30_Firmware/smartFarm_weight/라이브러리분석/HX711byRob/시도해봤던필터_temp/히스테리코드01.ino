// 이 코드의 단점은 0.5기준으로 왔다갔다할때 처리 하는 코드가 없다.

#include "HX711.h"

HX711 scale;

uint8_t dataPin = 12;
uint8_t clockPin = 11;

float calibration_factor = 466.768432;
float deadBand = 0.5; // Changed to 0.5 as per requirement

float referenceWeight = 0; // Global variable to store reference weight

int get_ABS_weight()
{
    float weight = scale.get_units(3);
    // Get the fractional part of the weight
    float fractionalPart = weight - static_cast<int>(weight);

    Serial.print("    ");
    Serial.print(fractionalPart);
    Serial.print("    ");

    // Check if the first decimal place is greater than 0.5
    if (fractionalPart >= 0.5)
    {
        weight = round(weight);
    }
    else
    {
        weight = floor(weight);
    }

    return static_cast<int>(weight);
}

void setup()
{
    Serial.begin(115200);
    Serial.println(__FILE__);
    Serial.print("LIBRARY VERSION: ");
    Serial.println(HX711_LIB_VERSION);
    Serial.println();

    scale.begin(dataPin, clockPin);

    scale.set_scale(calibration_factor);
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
    float rawWeight = scale.get_units(1);
    Serial.print("rawWeight :");
    Serial.print(rawWeight);
    Serial.print("\t");
    int stableWeight = get_ABS_weight();
    Serial.print("Stable weight: ");
    Serial.println(stableWeight);
}
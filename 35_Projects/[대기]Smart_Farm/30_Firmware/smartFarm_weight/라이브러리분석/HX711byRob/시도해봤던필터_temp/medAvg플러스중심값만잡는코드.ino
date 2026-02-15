#include "HX711.h"

HX711 scale;

uint8_t dataPin = 12;
uint8_t clockPin = 11;

float calibration_factor = 466.768432;
float deadBand = 0.5; // Changed to 0.5 as per requirement

float referenceWeight = 0; // Global variable to store reference weight

int get_stabilized_weight()
{
    float weight = scale.get_units(3);
    float diff = weight - referenceWeight;

    if (abs(diff) < deadBand)
    {
        // Weight is stable
        if (referenceWeight >= 99.5)
        {
            // Round up if 99.5 or greater
            return round(referenceWeight);
        }
        else
        {
            // Otherwise, truncate to integer
            return (int)referenceWeight;
        }
    }
    else
    {
        // Weight is not stable, update reference
        referenceWeight = weight;
        if (weight >= 99.5)
        {
            return round(weight);
        }
        else
        {
            return (int)weight;
        }
    }
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
    int stableWeight = get_stabilized_weight();
    Serial.print("Stabilized weight: ");
    Serial.println(stableWeight);
    delay(1000); // Delay for readability, adjust as needed
}
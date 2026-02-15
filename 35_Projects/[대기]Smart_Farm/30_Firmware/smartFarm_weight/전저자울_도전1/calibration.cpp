#include <HX711.h>

// Define the calibration factor
const float calibration_factor = 123.45; // Replace with your own calibration factor

// Create an instance of the HX711 class
HX711 scale;

void setup()
{
    // Initialize the HX711 module
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    // Set the calibration factor
    scale.set_scale(calibration_factor);
}

void loop()
{
    // Your code here
}
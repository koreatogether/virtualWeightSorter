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

// Kalman filter parameters (adjust these based on your system)
const float Q = 0.01; // Process noise covariance
const float R = 0.1;  // Measurement noise covariance
float P = 0.0;        // Estimated error covariance
float x_hat = 0.0;    // Estimated state (weight)
float K = 0.0;        // Kalman gain

// 저역필터 적용
float applyKalmanFilter(float newValue)
{
    // Prediction step
    P = P + Q;
    x_hat = x_hat; // No control input

    // Update step
    K = P / (P + R);
    x_hat = x_hat + K * (newValue - x_hat);
    P = (1 - K) * P;

    return x_hat;
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

    // Get the raw weight value from the scale
    float rawValue = scale.get_units(3);

    // Apply the Kalman filter to stabilize the reading
    float filteredValue = applyKalmanFilter(rawValue);

    // Print the filtered (stable) weight value
    Serial.print(" weight: ");
    Serial.println(filteredValue, 3);
}

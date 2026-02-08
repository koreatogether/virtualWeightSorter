// 칼만 필터에 초기값을 부여 하도록 개선된 코드이다.

#include "HX711.h"

HX711 scale;

uint8_t dataPin = 12;
uint8_t clockPin = 11;

// Kalman filter parameters
float Q = 0.01;    // Process noise covariance
float R = 0.1;     // Measurement noise covariance
float P = 1.0;     // Initial error covariance
float x_hat = 0.0; // Initial estimated state (weight)
float K = 0.0;     // Kalman gain

float applyKalmanFilter(float measurement)
{
  // Prediction step
  float x_hat_minus = x_hat;
  float P_minus = P + Q;

  // Update step
  K = P_minus / (P_minus + R);
  x_hat = x_hat_minus + K * (measurement - x_hat_minus);
  P = (1 - K) * P_minus;

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

  // Calibration
  scale.set_scale(466.768432);
  scale.set_gain(128);
  Serial.print("Current gain: ");
  Serial.println(scale.get_gain());
  Serial.println();

  scale.set_average_mode();
  Serial.print("Current mode: ");
  Serial.println(scale.get_mode());
  delay(1000);
  scale.tare();

  // Initialize Kalman filter with first measurement
  x_hat = scale.get_units(3);
}

void loop()
{
  // Get the raw weight value from the scale
  float rawValue = scale.get_units(15);

  // Apply the Kalman filter to stabilize the reading
  float filteredValue = applyKalmanFilter(rawValue);

  // Print both raw and filtered weight values
  Serial.print("Raw weight: ");
  Serial.print(rawValue, 3);
  Serial.print(" | Filtered weight: ");
  Serial.println(filteredValue, 3);

  delay(100); // Short delay to prevent too rapid readings
}
Here is an example of how you could extract a `Sensor` class from the original code:
```cpp
// Sensor.h
#ifndef Sensor_h
#define Sensor_h

#include <Adafruit_MPU6050.h>

class Sensor {
public:
  Sensor();
  void begin();
  void readSensorValues(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ, float &sensorTemperature);
  void setAccelerometerRange(MPU6050_RANGE range);
  void setGyroRange(MPU6050_RANGE range);
  void setFilterBandwidth(MPU6050_BANDWIDTH bandwidth);

private:
  Adafruit_MPU6050 mpu_;
};

#endif // Sensor_h
```

```cpp
// Sensor.cpp
#include "Sensor.h"

Sensor::Sensor() : mpu_() {}

void Sensor::begin() {
  if (!mpu_.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      // error handling code here
    }
  } else {
    Serial.println("Success MPU6050 chip ~");
  }
}

void Sensor::readSensorValues(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ, float &sensorTemperature) {
  sensors_event_t a, g, temp;
  mpu_.getEvent(&a, &g, &temp);

  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;

  gyroX = g.gyro.x;
  gyroY = g.gyro.y;
  gyroZ = g.gyro.z;

  sensorTemperature = temp.temperature;
}

void Sensor::setAccelerometerRange(MPU6050_RANGE range) {
  mpu_.setAccelerometerRange(range);
}

void Sensor::setGyroRange(MPU6050_RANGE range) {
  mpu_.setGyroRange(range);
}

void Sensor::setFilterBandwidth(MPU6050_BANDWIDTH bandwidth) {
  mpu_.setFilterBandwidth(bandwidth);
}
```

In this example, the `Sensor` class encapsulates the MPU6050 sensor and provides methods for initializing the sensor, reading sensor values, and setting various configuration options. This makes the code more modular and easier to maintain.

You can then use the `Sensor` class in your main code like this:
```cpp
Sensor sensor;

void setup() {
  sensor.begin();
  //...
}

void loop() {
  float accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature;
  sensor.readSensorValues(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature);
  //...
}
```
This is just one way to extract a `Sensor` class from the original code. Depending on your specific requirements, you may need to modify the class to fit your needs.
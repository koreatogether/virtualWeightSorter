//* from 코파일럿으로부터 얻은 코드입니다.

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_accel, *mpu_gyro;

void setup()
{
    Serial.begin(115200);
    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1)
        {
            delay(10);
        }
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu_accel = mpu.getAccelerometerSensor();
    mpu_gyro = mpu.getGyroSensor();
}

void loop()
{
    sensors_event_t accel_event, gyro_event;
    mpu_accel->getEvent(&accel_event);
    mpu_gyro->getEvent(&gyro_event);

    float Ax = accel_event.acceleration.x;
    float Ay = accel_event.acceleration.y;
    float Az = accel_event.acceleration.z;

    float pitch = atan2(Ay, sqrt(pow(Ax, 2) + pow(Az, 2))) * 180 / PI;
    float roll = atan2(Ax, sqrt(pow(Ay, 2) + pow(Az, 2))) * 180 / PI;

    if (abs(pitch) > 45 || abs(roll) > 45)
    {
        Serial.println("Banner has fallen!");
    }

    delay(100);
}

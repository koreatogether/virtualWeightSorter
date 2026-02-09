#include <Aduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu; // 객체

const int serialComBaudrate = 9600;

void setup(void)
{
  Serial.begin(serialComBaudrate);

  // Try to initialize!
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(20);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("~~~");
  delay(100);
}

void loop()
{
  float ax, ay, az, gx, gy, gz, newTemp;
  getValueMPU6050(ax, ay, az, gx, gy, gz, newTemp);
  // Do something with ax, ay, az...
  Serial.print("ax = ");
  Serial.print(ax);
  Serial.print(", ay = ");
  Serial.print(ay);
  Serial.print(", az = ");
  Serial.println(az);
  Serial.println(" ");
  Serial.print("    ");
  Serial.print("gx = ");
  Serial.print(gx);
  Serial.print(", gy = ");
  Serial.print(gy);
  Serial.print(", gz = ");
  Serial.print(gz);
  Serial.print("   temp= ");
  Serial.println(newTemp);
  Serial.println(" ");
  Serial.println(" ");
  delay(500);
}

void getValueMPU6050(float &ax, float &ay, float &az, float &gx, float &gy, float &gz, float &newTemp)
{
  // Read sensor data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Store accelerometer values
  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;

  // Store gyroscope values
  gx = g.gyro.x;
  gy = g.gyro.y;
  gz = g.gyro.z;

  // Store temperature
  newTemp = temp.temperature;

  delay(10); //  ==> 이거 나중에 변경해야 할지 모른다. 내부 인터럽트 사용???
}
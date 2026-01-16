// 005 버전은 led 와 연계 할것
// 가상시뮬레이터 https://wokwi.com/projects/396934077598831617

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
  else
  {
    Serial.println("Succese MPU 6050 chip ~");
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  delay(100);
}

void loop()
{
  float ax, ay, az, gx, gy, gz, newTemp;

  // 센서 값 받아오기
  getValueMPU6050(ax, ay, az, gx, gy, gz, newTemp);

  // 경고 메시지 출력 조건 확인
  checkTiltWarning(gx, gy, gz);

  // 센서 값 출력
  printSensorValues(ax, ay, az, gx, gy, gz, newTemp);

  delay(1000);
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

void checkTiltWarning(float gx, float gy, float gz)
{
  // gx, gy, gz 값을 각도로 변환
  float angleX = radiansToDegrees(gx);
  float angleY = radiansToDegrees(gy);
  float angleZ = radiansToDegrees(gz);

  // 각 축에 대해 60도 이상 기울었는지 확인
  if (abs(angleX) > 60 || abs(angleY) > 60 || abs(angleZ) > 60)
  {
    Serial.println("경고: 막대기가 60도 이상 기울어졌습니다!");
  }
}

float radiansToDegrees(float radians)
{
  return radians * (180.0 / PI);
}

void printSensorValues(float ax, float ay, float az, float gx, float gy, float gz, float newTemp)
{
  // Do something with ax, ay, az...
  Serial.print("ax = ");
  Serial.print(ax);
  Serial.print(", ay = ");
  Serial.print(ay);
  Serial.print(", az = ");
  Serial.print(az);
  Serial.print(" gx = ");
  Serial.print(gx);
  Serial.print(", gy = ");
  Serial.print(gy);
  Serial.print(", gz = ");
  Serial.print(gz);
  Serial.print(",,,  ");
  Serial.println(newTemp);
}
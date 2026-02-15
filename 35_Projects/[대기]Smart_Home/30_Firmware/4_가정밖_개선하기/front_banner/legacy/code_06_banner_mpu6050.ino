// 006 버전은 부저 울리게 할것 .
// 005 버전 가상시뮬레이터 : https://wokwi.com/projects/396938415717420033
// 006 버전 가상시시뮬레이터 : https://wokwi.com/projects/397040344779356161
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu; // 객체

const int serialComBaudrate = 9600;         // 시리얼 통신 속도
const int delayTime = 300;                  // 전체 동작간에 딜레이 시간
const float TILT_WARNING_THRESHOLD = 60.0f; // 쓰러짐 각도 한계 각도
const int buzzerPin = 8;                    // 부저 핀

const byte ledPins[] = {
    2,
    3,
    4,
    5,
    6,
    7,
};

// 명시적 함수 리스트
float convertRadiansToDegrees(float radians);
void readMPU6050SensorValue(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ, float &sensorTemperature);
void adjustLEDsBasedOnTiltAngle(float gyroX, float gyroY, float gyroZ);
void controlBuzzerByAngle(float angle);

void readMPU6050SensorValue(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ, float &sensorTemperature)
{
  // Read sensor data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Store accelerometer values
  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;

  // Store gyroscope values
  gyroX = g.gyro.x;
  gyroY = g.gyro.y;
  gyroZ = g.gyro.z;

  // Store temperature
  sensorTemperature = temp.temperature;

  delay(10); //  ==> 이거 나중에 변경해야 할지 모른다. 내부 인터럽트 사용???
}

void adjustLEDsBasedOnTiltAngle(float gyroX, float gyroY, float gyroZ)
{
  // gyroX, gyroY, gyroZ 값을 각도로 변환
  float angleX = convertRadiansToDegrees(gyroX);
  float angleY = convertRadiansToDegrees(gyroY);
  float angleZ = convertRadiansToDegrees(gyroZ);

  // 각 축에 대해 60도 이상 기울었는지 확인
  if (abs(angleX) > TILT_WARNING_THRESHOLD || abs(angleY) > TILT_WARNING_THRESHOLD || abs(angleZ) > TILT_WARNING_THRESHOLD)
  {
    Serial.println("경고: 막대기가 60도 이상 기울어졌습니다!");
  }

  // 최대 각도 계산
  float maxAngle = max(max(abs(angleX), abs(angleY)), abs(angleZ));

  // 각도에 따라 LED 제어
  controlLEDsByAngle(maxAngle);
}

void controlBuzzerByAngle(float angle)
{
  // 각도에 따라 부저 울리기
  if (angle <= 10)
  {
    noTone(buzzerPin); // 각도가 10도 이하일 때는 부저 소리를 내지 않음
  }
  else
  {
    // 각도에 따라 음높이를 조절하여, 기울어짐이 심할수록 음높이가 높아지도록 설정
    int toneFrequency = map(angle, 10, 90, 500, 2500); // 각도 10도에서 90도 사이를 500Hz에서 2500Hz로 매핑
    tone(buzzerPin, toneFrequency);                    // 설정된 톤으로 부저를 울림
    delay(100);                                        // 톤이 재생될 수 있도록 충분한 시간 제공
    noTone(buzzerPin);                                 // 다음 톤 재생을 위해 부저를 끔
  }
}

void controlLEDsByAngle(float angle)
{
  // 각도에 따라 LED 순차적으로 켜기
  for (int i = 0; i < 6; i++)
  {
    if (angle > (10 * (i + 1)))
    {
      digitalWrite(ledPins[i], HIGH);
    }
    else
    {
      digitalWrite(ledPins[i], LOW);
    }
  }
}

float convertRadiansToDegrees(float radians)
{
  return radians * (180.0 / PI);
}

void displaySensorReadings(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ, float sensorTemperature)
{
  // Do something with accelX, accelY, accelZ...
  Serial.print("accelX = ");
  Serial.print(accelX);
  Serial.print(", accelY = ");
  Serial.print(accelY);
  Serial.print(", accelZ = ");
  Serial.print(accelZ);
  Serial.print(" gyroX = ");
  Serial.print(gyroX);
  Serial.print(", gyroY = ");
  Serial.print(gyroY);
  Serial.print(", gyroZ = ");
  Serial.print(gyroZ);
  Serial.print(",,,  ");
  Serial.println(sensorTemperature);
}

// --------------------------------------------------------//

void setup(void)
{
  Serial.begin(serialComBaudrate);

  // LED 핀 모드 설정
  for (int i : ledPins)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  // 부저 핀 모드 설정
  pinMode(buzzerPin, OUTPUT);

  // 부저 테스트
  Serial.println("Starting buzzer tone test...");
  tone(buzzerPin, 1000, 500); // 1000Hz에서 500ms 동안 톤 재생
  delay(600);                 // 다음 톤 재생 전에 잠시 대기
  tone(buzzerPin, 1500, 500); // 1500Hz에서 500ms 동안 톤 재생
  delay(600);
  tone(buzzerPin, 2000, 500); // 2000Hz에서 500ms 동안 톤 재생
  delay(600);
  Serial.println("Buzzer tone test completed.");

  delay(100);

  // MPU6050 모듈 초기화
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      // 실패 시 모든 LED를 깜박이게 하여 사용자에게 알림
      for (int i : ledPins)
      {
        digitalWrite(i, HIGH); // 모든 LED 켜기
      }
      delay(500); // 0.5초 대기
      for (int i : ledPins)
      {
        digitalWrite(i, LOW); // 모든 LED 끄기
      }
      delay(500); // 0.5초 대기
    }
  }
  else
  {
    Serial.println("Success MPU6050 chip ~");
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  delay(100);
}

void loop()
{
  float accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature;

  // 센서로부터 값 읽어오기
  readMPU6050SensorValue(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature);

  // 기울기 각도에 의한 LED 표시
  adjustLEDsBasedOnTiltAngle(gyroX, gyroY, gyroZ);

  // 자이로스코프 값들을 도 단위로 변환
  float angleX = convertRadiansToDegrees(gyroX);
  float angleY = convertRadiansToDegrees(gyroY);
  float angleZ = convertRadiansToDegrees(gyroZ);

  // 변환된 각도를 사용하여 최대 각도 계산
  float maxAngle = max(max(abs(angleX), abs(angleY)), abs(angleZ));

  // 각도에 따라 부저 울리기
  controlBuzzerByAngle(maxAngle);

  // 디버깅용 센서값 시리얼 출력
  // displaySensorReadings(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature);

  // 디버깅용 최대 각도 시리얼 출력
  // Serial.print("Max angle for buzzer control: ");
  // Serial.println(maxAngle);

  delay(delayTime); // 필요에 따라서 300ms정도로 줄이면 됩니다.
}
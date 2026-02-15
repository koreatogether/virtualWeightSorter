//* from 코파일럿코드를 시작으로 편집한 코드입니다.

/*
 * 기존 006 오프라인 코드로부터 기능을 가져온다. ( O )
 * delay( ) 함수들을 제외 하고 millis () 기반의 동작으로 교체한다. ( O )
 * 1. mpu6050 모듈 의 동작부터 가져오기(O)
 *  https://wokwi.com/projects/397836111698211841
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const int repeatTime[] = {300, 500, 1000}; // 숫자는 동작 주기(ms)
unsigned long previousMillis = 0;          // 활용중
unsigned long previousMillis2 = 0;         // 필요시 활용
unsigned long previousMillis3 = 0;         // 필요시 활용

const byte TILT_WARNING_THRESHOLD[] = {45, 60, 75, 90}; // 숫자는 각도를 뜻함 , 임의 조정 가능
float pitch = 0;
float roll = 0;
Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_accel, *mpu_gyro;

const byte ledPins[] = {
    2,
    3,
    4,
    5,
    6,
    7,
};

const byte buzzerPin = 8; // 부저 핀

/**=====================================================
 * *       SETUP
 *======================================================**/
void setup()
{
  Serial.begin(115200); //* 시리얼 통신 초기화
  Serial.println(" 111111 ");
  initMPU6050(); //*  MPU6050 모듈 초기화
  initLED();     //*  LED 초기화
  initBuzzer();  //*  부저 초기화
}

/**=====================================================
 * *       LOOP
 *======================================================**/

void loop()
{

  if (millis() - previousMillis > repeatTime[0]) // 300ms마다 실행
  {
    previousMillis = millis();
  }

  if (millis() - previousMillis2 > repeatTime[1]) // 500ms마다 실행
  {
    previousMillis2 = millis();

    checkOrientation();
    printWarning();
    controlLEDsByAngle();
    controlBuzzerByAngle();
  }

  if (millis() - previousMillis3 > repeatTime[2]) // 1000ms마다 실행
  {
    previousMillis3 = millis();
  }
}

/**=====================================================
 * *       MPU6050 모듈 초기화
 *======================================================**/
void initMPU6050()
{

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
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu_accel = mpu.getAccelerometerSensor();
  mpu_gyro = mpu.getGyroSensor();
}

/**=====================================================
 * *       LED 출력 설정 및 초기 동작 설정(LED꺼짐)
 *======================================================**/
void initLED()
{
  for (int i : ledPins)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
}

/**=====================================================
 * *       부저 출력 설정 및 동작 테스트 설정
 *======================================================**/
void initBuzzer()
{
  pinMode(buzzerPin, OUTPUT);
  Serial.println("Starting buzzer tone test...");
  tone(buzzerPin, 1000, 500); // 1000Hz에서 500ms 동안 톤 재생
  delay(600);                 // 다음 톤 재생 전에 잠시 대기
  tone(buzzerPin, 1500, 500); // 1500Hz에서 500ms 동안 톤 재생
  delay(600);
  tone(buzzerPin, 2000, 500); // 2000Hz에서 500ms 동안 톤 재생
  delay(600);
  Serial.println("Buzzer tone test completed.");
}

/**=====================================================
 * *       배너 자세 계산
 *======================================================**/
void checkOrientation()
{
  sensors_event_t accel_event, gyro_event;
  mpu_accel->getEvent(&accel_event);
  mpu_gyro->getEvent(&gyro_event);

  float Ax = accel_event.acceleration.x;
  float Ay = accel_event.acceleration.y;
  float Az = accel_event.acceleration.z;

  pitch = atan2(Ay, sqrt(pow(Ax, 2) + pow(Az, 2))) * 180 / PI;
  roll = atan2(Ax, sqrt(pow(Ay, 2) + pow(Az, 2))) * 180 / PI;
}

/**=====================================================
 * *       배너 쓰러짐 알림
 *======================================================**/
void printWarning()
{
  if (abs(pitch) > TILT_WARNING_THRESHOLD[0] || abs(roll) > TILT_WARNING_THRESHOLD[0])
  {
    Serial.println("Banner has fallen!");
  }
}

/**=====================================================
 * *       LED 표시 제어
 *======================================================**/
void controlLEDsByAngle()
{
  // 각도에 따라 LED 순차적으로 켜기
  for (int i = 0; i < 6; i++)
  {
    if (abs(pitch) > (10 * (i + 1)) || abs(roll) > (10 * (i + 1)))
    {
      digitalWrite(ledPins[i], HIGH);
    }
    else
    {
      digitalWrite(ledPins[i], LOW);
    }
  }
}

/**=====================================================
 * *       부저 소리 제어
 *======================================================**/
void controlBuzzerByAngle()
{
  // 각도에 따라 부저 울리기
  if (abs(pitch) <= 10 && abs(roll) <= 10)
  {
    noTone(buzzerPin); // 각도가 10도 이하일 때는 부저 소리를 내지 않음
  }
  else
  {
    // 각도에 따라 음높이를 조절하여, 기울어짐이 심할수록 음높이가 높아지도록 설정
    int toneFrequency = map(max(abs(pitch), abs(roll)), 10, 90, 500, 2500); // 각도 10도에서 90도 사이를 500Hz에서 2500Hz로 매핑
    tone(buzzerPin, toneFrequency);                                         // 설정된 톤으로 부저를 울림
    delay(100);                                                             // 톤이 재생될 수 있도록 충분한 시간 제공
    noTone(buzzerPin);                                                      // 다음 톤 재생을 위해 부저를 끔
  }
}
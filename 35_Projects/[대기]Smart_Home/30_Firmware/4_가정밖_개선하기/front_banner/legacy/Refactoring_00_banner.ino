// 리팩토링 00 버전은 ai 들로부터 얻은 리팩토링 리스트 및 개선안을 참고하여 수정하는 프로그램이다.
/**-----------------------
 * *       INFORMATION
 *
 * * ////함수 및 변수 이름이 길다.
 * * ////변수 자료형 체크
 * * ////상수 이름을 대문자로 변경
 * * 코드 구조 개선 및 일부 함수 분리 및 재구성 가능성을 높일 것 .
 * * ////mpu6050 초기화 실패시 무한 루프에 빠지지 않고 재차 초기화 시도 하도록 수정할 것
 * * ////주석 내용 체크
 * * ////mpu6050이름 들어간 함수 및 변수명은 센서가 1개이므로 빼고 작성 해볼것
 * * ////loop() 함수 길지 않는지 체크
 * * 함수 코드들의 일반화 ???
 * * 사용자의 뜻에 따라서 원하는 각도 원하는 부저를 변경하도록 수정 해볼 것
 * * 가독성 과 유지 보수를 위한 리팩토링이 되도록 할것
 *------------------------**/

// 006 버전 가상시시뮬레이터 : https://wokwi.com/projects/397040344779356161
// 중간 수정한 가상시뮬레이터 : https://wokwi.com/projects/397064037131520001
// 최종 수정한 가상시뮬레이터 : https://wokwi.com/projects/397066292944757761
#include <Adafruit_MPU6050.h> // mpu6050 라이브러리
#include <Adafruit_Sensor.h>  // mpu6050 라이브러리
#include <Wire.h>             // i2c 라이브러리

Adafruit_MPU6050 mpu; // MPU6050 객체 생성

const int SERIAL_BAUD_RATE = 9600;          // 시리얼 통신 속도
const int LOOP_DELAY_TIME = 300;            // 전체 동작간에 딜레이 시간
const float TILT_WARNING_THRESHOLD = 60.0f; // 쓰러짐 각도 한계 각도
const int BUZZER_PIN = 8;                   // 부저 핀

const byte LED_PINS[] = {
    // LED 핀 배열
    2,
    3,
    4,
    5,
    6,
    7,
};

// 함수 프로토타입 선언
void initMPU6050();                           // MPU6050 초기화 함수
void testBuzzerTone();                        // 부저 테스트 함수
float convertRadiansToDegrees(float radians); // 라디안을 도로 변환하는 함수
void readSensorValue(float &accelX, float &accelY, float &accelZ,
                     float &gyroX, float &gyroY, float &gyroZ, float &sensorTemperature); // 센서 값 읽기 함수
void adjustLEDsBasedOnTiltAngle(float gyroX, float gyroY, float gyroZ);                   // 기울기 각도에 따라 LED 조정 함수
void controlBuzzerByAngle(float angle);                                                   // 각도에 따라 부저 제어 함수
void controlLEDsByAngle(float angle);                                                     // 각도에 따라 LED 제어 함수
void displaySensorReadings(float accelX, float accelY, float accelZ,
                           float gyroX, float gyroY, float gyroZ, float sensorTemperature); // 센서 읽기 값 표시 함수

// MPU6050 초기화
void initMPU6050()
{
  bool mpuInitialized = false;
  while (!mpuInitialized)
  {
    // MPU6050 모듈 초기화 시도
    if (!mpu.begin())
    {
      Serial.println("Failed to find MPU6050 chip, retrying in 3 seconds...");
      // 실패 시 모든 LED를 깜박이게 하여 사용자에게 알림
      for (int i : LED_PINS)
      {
        digitalWrite(i, HIGH); // 모든 LED 켜기
      }
      delay(500); // 0.5초 대기
      for (int i : LED_PINS)
      {
        digitalWrite(i, LOW); // 모든 LED 끄기
      }
      delay(3000); // 추가 대기 시간 (총 3초 대기)
      Serial.println("Retrying...");
    }
    else
    {
      Serial.println("Success MPU6050 chip ~");
      mpuInitialized = true; // 초기화 성공, 루프 종료
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  delay(100);
}

void testBuzzerTone()
{
  // 부저 테스트 : 다양한 주파수로 부저를 울려서 작동 확인
  Serial.println("Starting buzzer tone test...");
  tone(BUZZER_PIN, 1000, 500); // 1000Hz에서 500ms 동안 톤 재생
  delay(600);
  tone(BUZZER_PIN, 1500, 500); // 1500Hz에서 500ms 동안 톤 재생
  delay(600);
  tone(BUZZER_PIN, 2000, 500); // 2000Hz에서 500ms 동안 톤 재생
  delay(600);
  Serial.println("Buzzer tone test completed.");

  delay(100);
}

// 센서로부터 값 읽기
void readSensorValue(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ, float &sensorTemperature)
{
  // 센서 데이타 읽기 : 가속도계와 자이로스코프 값 , 그리고 온도를 읽어옴
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 가속도 값 저장
  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;

  // 자이로스코프 값 저장
  gyroX = g.gyro.x;
  gyroY = g.gyro.y;
  gyroZ = g.gyro.z;

  // 온도 값 저장
  sensorTemperature = temp.temperature;
}

// 각도값에 따라 LED 켜지는 갯수 조정
void adjustLEDsBasedOnTiltAngle(float gyroX, float gyroY, float gyroZ)
{
  // X , Y , Z 축의 원본 값을 각도로 변환해서 저장
  float angleX = convertRadiansToDegrees(gyroX);
  float angleY = convertRadiansToDegrees(gyroY);
  float angleZ = convertRadiansToDegrees(gyroZ);

  // 위  변수들 각도값을 기준으로 정해진 각도 이상이면 경고메세지를 출력
  if (abs(angleX) > TILT_WARNING_THRESHOLD || abs(angleY) > TILT_WARNING_THRESHOLD || abs(angleZ) > TILT_WARNING_THRESHOLD)
  {
    Serial.println("경고: 막대기가 60도 이상 기울어졌습니다!");
  }

  // 내장함수를 이용하여  X 축과 Y축을 먼저 비교하고 나중에 Z축을 비교 해서 값을 추출
  float maxAngle = max(max(abs(angleX), abs(angleY)), abs(angleZ));

  controlLEDsByAngle(maxAngle);
}

// 각도에 따라 부저 울리기
void controlBuzzerByAngle(float angle)
{

  if (angle <= 10)
  {
    noTone(BUZZER_PIN); // 각도가 10도 이하일 때는 부저 소리를 내지 않음
  }
  else
  {
    // 각도에 따라 음높이를 조절하여, 기울어짐이 심할수록 음높이가 높아지도록 설정
    int toneFrequency = map(angle, 10, 90, 500, 2500); // 각도 10도에서 90도 사이를 500Hz에서 2500Hz로 매핑
    tone(BUZZER_PIN, toneFrequency);                   // 설정된 톤으로 부저를 울림
    delay(100);                                        // 톤이 재생될 수 있도록 충분한 시간 제공
    noTone(BUZZER_PIN);                                // 다음 톤 재생을 위해 부저를 끔
  }
}

// 각도에 따라 LED 켜지는 갯수 조정
void controlLEDsByAngle(float angle)
{
  // 각도에 따라 LED 순차적으로 켜기
  for (int i = 0; i < 6; i++)
  {
    if (angle > (10 * (i + 1)))
    {
      digitalWrite(LED_PINS[i], HIGH);
    }
    else
    {
      digitalWrite(LED_PINS[i], LOW);
    }
  }
}

// 라디안값을 각도값으로 변환
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
  Serial.begin(SERIAL_BAUD_RATE); // 시리얼 통신 초기화

  // LED 핀 모드 설정
  for (int i : LED_PINS)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  // 부저 핀 모드 설정 및 부저 테스트
  pinMode(BUZZER_PIN, OUTPUT);
  testBuzzerTone(); // 부저 작동 테스트

  // mpu6050 모듈 초기화
  initMPU6050();
}

void loop()
{
  float accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature;

  // 센서로부터 가속도 , 자이로스코프 , 온도 값  읽어오기
  readSensorValue(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature);

  // 읽어온 자이로스코프 값을 기반으로 기울기 각도에 따라 LED 표현을 조정
  adjustLEDsBasedOnTiltAngle(gyroX, gyroY, gyroZ);

  // 자이로스코프 라디안값을 각도로 변환
  float angleX = convertRadiansToDegrees(gyroX);
  float angleY = convertRadiansToDegrees(gyroY);
  float angleZ = convertRadiansToDegrees(gyroZ);

  // 변환된 각도를 사용하여 최대 각도 계산
  float maxAngle = max(max(abs(angleX), abs(angleY)), abs(angleZ));

  // 각도에 따라 경고 부저 울리기
  controlBuzzerByAngle(maxAngle);

  // 디버깅용 센서값 시리얼 출력
  // displaySensorReadings(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, sensorTemperature);

  // 디버깅용 최대 각도 시리얼 출력
  // Serial.print("Max angle for buzzer control: ");
  // Serial.println(maxAngle);

  delay(LOOP_DELAY_TIME); // 메인 루프의 실행 간격을 조정
}
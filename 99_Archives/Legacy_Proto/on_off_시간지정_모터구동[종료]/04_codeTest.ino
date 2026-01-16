/*
  Power On/Off Repeat Control - Test Version

  이 스케치는 가변저항 값에 따라 ON/OFF 시간을 지정하여
  서보모터를 일정한 간격으로 제어합니다.

  테스트 목적으로 시간 단위를 분->초로 변경하여 더 빠르게 테스트할 수 있습니다.

  회로 구성:
  - 입력:
    * DHT22 온습도 센서 (핀 12)
    * 가변저항 1 (핀 A0) - ON 시간 조절용 (초 단위)
    * 가변저항 2 (핀 A1) - OFF 시간 조절용 (초 단위)

  - 출력:
    * 서보모터 1 (핀 10)
    * 서보모터 2 (핀 11)
    * OLED 디스플레이 (I2C 인터페이스)

  Created: 2023
  By: iot발걸음
  Modified: 2023

  V04: 테스트 용도로 시간을 초 단위로 변경
  https://wokwi.com/projects/424263110753710081
*/

#include <U8glib.h>
#include <DHT.h>
#include <Servo.h>

// OLED 디스플레이 설정
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

// DHT 센서 설정
#define DHTTYPE DHT22
const int DHTPIN = 12;
DHT dht(DHTPIN, DHTTYPE);
float Humidity;
float Temperature;

// 서보모터 설정
Servo servo, servo2;
int servoPin = 10, servoPin2 = 11;
int servoAngle, servo2Angle;

// 가변저항 설정
int potentioPin = A0, potentioPin2 = A1;
int potentioValue, potentioValue2;
int potentioMappingValue, potentioMappingValue2;

// 타이머 관련 변수
unsigned long previousMillis = 0, previousMillis2 = 0, previousMillis3 = 0;
unsigned long currentMillis = 0, currentMillis2 = 0;
long intervalOn = 3000;  // 기본 ON 상태 지속 시간 (ms)
long intervalOff = 3000; // 기본 OFF 상태 지속 시간 (ms)

// 시간 계산 관련 변수 (초 단위로 변경)
int matchingTime = 30; // 최대 30초
int selectTime, selectTime2;
int minTimeValue = 1; // 최소 1초

// 테스트 모드 관련 정보
bool testMode = true;
int cycleCount = 0;
int maxCycles = 10; // 최대 테스트 사이클 수

// 모터 상태 변수
int motorState = LOW;

void setup()
{
  // 시리얼 통신 초기화
  Serial.begin(115200);

  // DHT 센서 초기화
  dht.begin();

  // 서보모터 초기화
  servo.attach(servoPin);
  servo2.attach(servoPin2);

  // 서보모터 초기 위치 설정
  servo.write(0);
  delay(500);
  servo2.write(0);
  delay(500);

  Serial.println("==== 테스트 모드 시작 ====");
  Serial.println("가변저항 1: ON 시간 설정 (초)");
  Serial.println("가변저항 2: OFF 시간 설정 (초)");
}

void loop()
{
  // 현재 시간 업데이트
  currentMillis = millis();
  currentMillis2 = millis();

  // 가변저항 값 읽기 및 매핑 (루프의 시작 부분으로 이동)
  potentiometerMapping();
  potentiometerMapping2();

  // 온습도 측정
  temp_humidity();

  // 서보모터 제어
  water_motor_control();

  // OLED 디스플레이 업데이트
  oled_Display();

  // 디버깅 정보 출력
  serialPrint();
}

// 온습도 측정 함수
void temp_humidity()
{
  if (currentMillis2 - previousMillis2 >= 2500)
  {
    Humidity = dht.readHumidity();
    Temperature = dht.readTemperature();

    if (isnan(Temperature) || isnan(Humidity))
    {
      Serial.println("Failed to read from DHT sensor!");
    }

    previousMillis2 = currentMillis2;
  }
}

// 서보모터 제어 함수
void water_motor_control()
{
  if ((motorState == HIGH && currentMillis - previousMillis >= intervalOn) ||
      (motorState == LOW && currentMillis - previousMillis >= intervalOff))
  {

    previousMillis = currentMillis;

    if (motorState == LOW)
    {
      motorState = HIGH;
      servoAngle = 0;
      servo2Angle = 0;
      cycleCount++;
      Serial.print("사이클 #");
      Serial.print(cycleCount);
      Serial.println(" - 모터 ON");
    }
    else
    {
      motorState = LOW;
      servoAngle = 180;
      servo2Angle = 180;
      Serial.print("사이클 #");
      Serial.print(cycleCount);
      Serial.println(" - 모터 OFF");
    }

    servo.write(servoAngle);
    servo2.write(servo2Angle);

    // 테스트 모드에서 최대 사이클 도달 시 알림
    if (testMode && cycleCount >= maxCycles)
    {
      Serial.println("==== 테스트 사이클 완료 ====");
      Serial.print("총 사이클 수: ");
      Serial.println(cycleCount);

      // 필요하면 여기에 추가 종료 코드 작성
      // 테스트가 끝났다는 것을 표시
    }
  }
}

// OLED 디스플레이 업데이트 함수
void oled_Display()
{
  u8g.firstPage();
  do
  {
    u8g.setRot180(); // 화면 반전

    // 상단 제목
    u8g.setFont(u8g_font_unifont);
    u8g.drawStr(25, 10, "Test Mode");

    // ON/OFF 텍스트
    u8g.drawStr(10, 30, "ON");
    u8g.drawStr(90, 30, "OFF");

    // 온습도 표시
    u8g.setFont(u8g_font_5x7);
    u8g.setPrintPos(5, 56);
    u8g.print(Humidity, 0);
    u8g.print("%");
    u8g.setPrintPos(70, 56);
    u8g.print(Temperature, 0);
    u8g.print("C");

    // 현재 상태 표시
    u8g.setFont(u8g_font_unifont);
    if (motorState == HIGH)
    {
      u8g.drawFrame(5, 13, 25, 25);
    }
    else
    {
      u8g.drawFrame(85, 13, 35, 25);
    }

    // 현재 설정 시간 표시 (초)
    u8g.setFont(u8g_font_5x7);
    u8g.setPrintPos(5, 42);
    u8g.print(selectTime);
    u8g.print("s");

    u8g.setPrintPos(90, 42);
    u8g.print(selectTime2);
    u8g.print("s");

    // 사이클 카운트 표시
    u8g.setPrintPos(40, 50);
    u8g.print("Cycle: ");
    u8g.print(cycleCount);
    u8g.print("/");
    u8g.print(maxCycles);

  } while (u8g.nextPage());
}

// 첫 번째 가변저항 값 읽기 및 시간 변환 (초 단위)
void potentiometerMapping()
{
  potentioValue = analogRead(potentioPin);

  // 가변저항 값을 1~30초 범위로 매핑
  potentioMappingValue = map(potentioValue, 0, 1023, 1, matchingTime);

  // 최소 1초 보장
  selectTime = potentioMappingValue;
  if (selectTime < minTimeValue)
  {
    selectTime = minTimeValue;
  }

  // ON 시간 간격 설정 (밀리초 단위)
  intervalOn = (long)selectTime * 1000L; // 초 단위를 밀리초로 변환
}

// 두 번째 가변저항 값 읽기 및 시간 변환 (초 단위)
void potentiometerMapping2()
{
  potentioValue2 = analogRead(potentioPin2);

  // 가변저항 값을 1~30초 범위로 매핑
  potentioMappingValue2 = map(potentioValue2, 0, 1023, 1, matchingTime);

  // 최소 1초 보장
  selectTime2 = potentioMappingValue2;
  if (selectTime2 < minTimeValue)
  {
    selectTime2 = minTimeValue;
  }

  // OFF 시간 간격 설정 (밀리초 단위)
  intervalOff = (long)selectTime2 * 1000L; // 초 단위를 밀리초로 변환
}

// 디버깅 정보 출력 함수
void serialPrint()
{
  if (currentMillis - previousMillis3 >= 1000)
  { // 1초마다 출력 (테스트 모드에서는 더 빠르게)
    previousMillis3 = currentMillis;

    // 모터 상태 및 서보 각도 출력
    Serial.print("상태: ");
    Serial.print(motorState == HIGH ? "ON" : "OFF");
    Serial.print("    ");
    Serial.print("각도1: ");
    Serial.print(servoAngle);
    Serial.print("°    ");
    Serial.print("각도2: ");
    Serial.print(servo2Angle);
    Serial.println("°");

    // 가변저항 값 및 시간 설정값 출력
    Serial.print("ON 시간: ");
    Serial.print(selectTime);
    Serial.print("초 (");
    Serial.print(intervalOn);
    Serial.println("ms)");

    Serial.print("OFF 시간: ");
    Serial.print(selectTime2);
    Serial.print("초 (");
    Serial.print(intervalOff);
    Serial.println("ms)");

    // 남은 시간 표시
    unsigned long remainingTime;
    if (motorState == HIGH)
    {
      remainingTime = (intervalOn - (currentMillis - previousMillis)) / 1000;
      Serial.print("남은 ON 시간: ");
    }
    else
    {
      remainingTime = (intervalOff - (currentMillis - previousMillis)) / 1000;
      Serial.print("남은 OFF 시간: ");
    }
    Serial.print(remainingTime);
    Serial.println("초");

    Serial.println("-----------------------");
  }
}
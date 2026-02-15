/*
  Power On/Off Repeat Control with Servo Motors

  이 스케치는 가변저항 값에 따라 ON/OFF 시간을 지정하여
  서보모터를 일정한 간격으로 제어합니다.

  DHT 온습도 센서로 주변 환경을 측정하고 OLED 디스플레이에 표시합니다.

  회로 구성:
  - 입력:
    * DHT22 온습도 센서 (핀 12)
    * 가변저항 1 (핀 A0) - ON 시간 조절용
    * 가변저항 2 (핀 A1) - OFF 시간 조절용

  - 출력:
    * 서보모터 1 (핀 10)
    * 서보모터 2 (핀 11)
    * OLED 디스플레이 (I2C 인터페이스)

  Created: 2023
  By: iot발걸음

  https://wokwi.com/projects/424205141114092545
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
long intervalOn = 3000;   // ON 상태 지속 시간 (ms)
long intervalOff = 10000; // OFF 상태 지속 시간 (ms)

// 시간 계산 관련 변수
int matchingTime = 3 * 60 / 5; // 3시간을 5분 단위로 나눔
int selectTime, selectTime2;
float selectTimeMinute, selectTimeMinute2;
int selectTimeHour, selectTimeHour2;

int motorState = LOW; // 모터 상태 변수

void setup()
{

    Serial.begin(115200); // 시리얼 통신 초기화

    dht.begin(); // DHT 센서 초기화

    servo.attach(servoPin); // 서보모터 초기화
    servo2.attach(servoPin2);

    servo.write(0); // 서보모터 초기 위치 설정
    servo2.write(0);
}

void loop()
{
    // 현재 시간 업데이트
    currentMillis = millis();
    currentMillis2 = millis();

    temp_humidity(); // 온습도 측정

    water_motor_control(); // 서보모터 제어

    potentiometerMapping(); // 가변저항 값 읽기 및 매핑
    potentiometerMapping2();

    oled_Display(); // OLED 디스플레이 업데이트

    serialPrint(); // 디버깅 정보 출력
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
        else
        {
            Serial.print("Humidity: ");
            Serial.print(Humidity);
            Serial.print(" %\t");
            Serial.print("Temperature: ");
            Serial.print(Temperature);
            Serial.println(" *C ");
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
        }
        else
        {
            motorState = LOW;
            servoAngle = 180;
            servo2Angle = 180;
        }

        servo.write(servoAngle);
        servo2.write(servo2Angle);
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
        u8g.drawStr(25, 10, "Water Pump");

        // ON/OFF 텍스트
        u8g.drawStr(10, 30, "ON");
        u8g.drawStr(90, 30, "OFF");

        // 온습도 표시
        u8g.setFont(u8g_font_courB14);
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
    } while (u8g.nextPage());
}

// 첫 번째 가변저항 값 읽기 및 시간 변환
void potentiometerMapping()
{
    potentioValue = analogRead(potentioPin);
    potentioMappingValue = map(potentioValue, 0, 1023, 1, matchingTime);
    selectTime = potentioMappingValue * 5; // 5분 단위로 변환

    // 시간과 분으로 변환
    if (selectTime >= 60)
    {
        selectTimeHour = selectTime / 60;
        selectTimeMinute = (selectTime % 60); // 수정된 부분: % 연산자 사용
    }
    else
    {
        selectTimeHour = 0;
        selectTimeMinute = selectTime;
    }

    // ON 시간 간격 설정 (밀리초 단위)
    intervalOn = selectTime * 60 * 1000; // 분 단위를 밀리초로 변환
}

// 두 번째 가변저항 값 읽기 및 시간 변환
void potentiometerMapping2()
{
    potentioValue2 = analogRead(potentioPin2);
    potentioMappingValue2 = map(potentioValue2, 0, 1023, 1, matchingTime);
    selectTime2 = potentioMappingValue2 * 5; // 5분 단위로 변환

    // 시간과 분으로 변환
    if (selectTime2 >= 60)
    {
        selectTimeHour2 = selectTime2 / 60;
        selectTimeMinute2 = (selectTime2 % 60); // 수정된 부분: % 연산자 사용
    }
    else
    {
        selectTimeHour2 = 0;
        selectTimeMinute2 = selectTime2;
    }

    // OFF 시간 간격 설정 (밀리초 단위)
    intervalOff = selectTime2 * 60 * 1000; // 분 단위를 밀리초로 변환
}

// 디버깅 정보 출력 함수
void serialPrint()
{
    if (currentMillis - previousMillis3 >= 2500)
    {
        previousMillis3 = currentMillis;

        // 모터 상태 및 서보 각도 출력
        Serial.print("motorState = ");
        Serial.print(motorState);
        Serial.print("    ");
        Serial.print("servoAngle = ");
        Serial.print(servoAngle);
        Serial.print("    ");
        Serial.print("servo2Angle = ");
        Serial.print(servo2Angle);
        Serial.println("    ");

        // 가변저항 값 및 매핑된 값 출력
        Serial.print("potentioValue = ");
        Serial.print(potentioValue);
        Serial.print("    ");
        Serial.print("potentioMappingValue = ");
        Serial.print(potentioMappingValue);
        Serial.println("    ");
        Serial.print("potentioValue2 = ");
        Serial.print(potentioValue2);
        Serial.print("    ");
        Serial.print("potentioMappingValue2 = ");
        Serial.print(potentioMappingValue2);
        Serial.println("    ");

        // 시간 설정값 출력
        Serial.print("ON Time: ");
        if (selectTimeHour > 0)
        {
            Serial.print(selectTimeHour);
            Serial.print("h ");
        }
        Serial.print(selectTimeMinute);
        Serial.print("m (");
        Serial.print(intervalOn / 1000);
        Serial.println("s)");

        Serial.print("OFF Time: ");
        if (selectTimeHour2 > 0)
        {
            Serial.print(selectTimeHour2);
            Serial.print("h ");
        }
        Serial.print(selectTimeMinute2);
        Serial.print("m (");
        Serial.print(intervalOff / 1000);
        Serial.println("s)");
    }
}
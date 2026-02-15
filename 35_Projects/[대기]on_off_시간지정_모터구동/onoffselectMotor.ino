/*
  Example testing sketch for various DHT humidity/temperature sensors
  DHT CODE Written by ladyada, public domain

  밀리함수 컨트롤 코드는 https://blog.naver.com/damtaja/220992246602
  by 해바우

  화면 반전은 u8g.setRot180() 함수를 사용하여 할 수 있습니다.

  아두이노를 이용한 모터 OON / OFF 작동하기

  가변저항 값에 따라서 ON 시간 , OFF 시간을 지정하여
  모터 가동 시간을 정할수 있습니다.

  The circuit :
    각 input에 연결되어있는 구성 요소의 목록

    각 output에 연결되어있는 구성 요소의 목록
    수중펌프 ap201
    모터 드라이버 - L9110 모듈 2채널  ,  채널당 최대 700mA

  pwm 제어
  Created 연월일
  By iot발걸음
  Modified 날짜
  By iot발걸음

  V01 스케치에서
  V02 스케치 테스트중 , 차이는 가변 저항을 2개이용한다.
*/

#include <U8glib.h>
#include <DHT.h>
#include <Servo.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

#define DHTTYPE DHT22
const int DHTPIN = 12; // what pin we're connected to
DHT dht(DHTPIN, DHTTYPE);
float Humidity;
float Temperature;

Servo servo, servo2;

int servoPin = 10, servoPin2 = 11;
int servoAngle, servo2Angle;
int potentioPin = A0, potentioPin2 = A1;
int potentioValue, potentioValue2;
int potentioMappingValue, potentioMappingValue2;

// 10 = 서버모터 1, 11 = 서보모터2 , 12 = dht22 , 14 = 가변저항1 , 15 = 가변저항2
// int pinArray[] = {10 , 11 ,12 ,14 ,15};  // for wokki 가상시뮬레이터용

/* 5= 모터드라이버 A1 , 6 = 모터드라이버 A2 , 10 = 모터드라이버 B1 , 11 = 모터드라이버 B2
   12 = dht22 , 14 = 가변저항1 , 15 = 가변저항2 */
// int pinArray[] = {5,6,10,11,12 ,14,15}  // for 실제 회로도에 따라서라면

// const int pump_A1 = 5;
// const int pump_A2 = 6;
// const int pump_B1 = 10;
// const int pump_B2 = 11;

unsigned long previousMillis, previousMillis2, previousMillis3;
unsigned long currentMillis, currentMillis2;
long intervalOn = 3000;      // 28000;  // 28초 는 28000 , 60초는 60000
long intervalOff = 10000;    // 900000; // 15분은 900000
int matchingTime = 3*60/5 ; // 3은 최대시간 , 60은 분으로계산 , 5는 5분씩 
int selectTime, selectTime2;
float selectTimeMinute, selectTimeMinute2;
int selectTimeHour, selectTimeHour2; 
int motorState = LOW;
int motorSpeed1, motorSpeed2;

void setup()
{
    Serial.begin(115200);
    dht.begin();
    servo.attach(10);
    servo2.attach(11);
    servo.write(0);
    delay(1000);
    servo2.write(0);
    delay(1000);

    // pinMode(pump_A1, OUTPUT); // 일반모터용
    // pinMode(pump_A2, OUTPUT); // 일반모터용
    // pinMode(pump_B1, OUTPUT); // 일반모터용
    // pinMode(pump_B2, OUTPUT); // 일반모터용
}

void loop()
{
    currentMillis = millis();
    currentMillis2 = millis();
    temp_humidity();
    // water_motor_control();
    water_motor_control2(); // 서보모터용
    potentiometerMapping();
    potentiometerMapping();
    oled_Display();
    serialPrint();
}

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

void water_motor_control() // 일반모터용
{
    currentMillis = millis();
    if ((motorState == HIGH && currentMillis - previousMillis >= intervalOn) ||
        (motorState == LOW && currentMillis - previousMillis >= intervalOff))
    { // 수정한 부분
        previousMillis = currentMillis;
        if (motorState == LOW)
        {
            motorState = HIGH;
            motorSpeed1 = 255;
            motorSpeed2 = 0;
        }
        else
        {
            motorState = LOW;
            motorSpeed1 = 0;
            motorSpeed2 = 0;
        }

        // analogWrite(pump_A1, motorSpeed1);
        // analogWrite(pump_A2, motorSpeed2);
        // analogWrite(pump_B1, motorSpeed1);
        // analogWrite(pump_B2, motorSpeed2);
        // Serial.print("motorState = "); Serial.print(motorState); Serial.println("    ");
        // Serial.print("motorSpeed1 = "); Serial.print(motorSpeed1); Serial.println("    ");
        // Serial.print("motorSpeed2 = "); Serial.print(motorSpeed2); Serial.println("    ");
    }
}

void water_motor_control2() // 서보모터용
{
    currentMillis = millis();
    if ((motorState == HIGH && currentMillis - previousMillis >= intervalOn) ||

        (motorState == LOW && currentMillis - previousMillis >= intervalOff))
    { // 수정한 부분
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

void oled_Display()
{
    u8g.firstPage();
    do
    {
        u8g.setRot180(); //화면 반전
        u8g.setFont(u8g_font_unifont);
        u8g.drawStr(25, 10, "Water Pump");
        u8g.drawStr(10, 30, "ON");
        u8g.drawStr(90, 30, "OFF");
        u8g.setFont(u8g_font_courB14);
        u8g.setPrintPos(5, 56);
        u8g.print(Humidity, 0);
        u8g.print("%");
        u8g.setPrintPos(70, 56);
        u8g.print(Temperature, 0);
        u8g.print("C");
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

void potentiometerMapping() // read a0 and map() to 24
{
    potentioValue = analogRead(potentioPin);
    potentioMappingValue = map(potentioValue, 0, 1023, 1, matchingTime);
    selectTime = potentioMappingValue * 5;
    if(selectTime >= 60) 
    {
        selectTimeHour = selectTime / 60;
        selectTimeMinute = (selectTime - 60);
    }
}

void potentiometerMapping2()
{
    potentioValue2 = analogRead(potentioPin2);
    potentioMappingValue2 = map(potentioValue2, 0, 1023, 1, matchingTime); 
    selectTime2 = potentioMappingValue2 * 5;
    if(selectTime2 >= 60) 
    {
        selectTimeHour2 = selectTime2 / 60;
        selectTimeMinute2 = (selectTime2 - 60);
    }
}

void serialPrint() // 디버깅용
{
    if (currentMillis - previousMillis3 >= 2500)
    {
        previousMillis3 = currentMillis;
        Serial.print("motorState = ");Serial.print(motorState);Serial.print("    ");
        Serial.print("servoAngle = ");Serial.print(servoAngle);Serial.print("    ");
        Serial.print("servo2Angle = ");Serial.print(servo2Angle);Serial.println("    ");
        Serial.print("potentioValue = ");Serial.print(potentioValue);Serial.print("    ");
        Serial.print("potentioMappingValue = ");Serial.print(potentioMappingValue);Serial.println("    ");
        Serial.print("potentioValue2 = ");Serial.print(potentioValue2);Serial.print("    ");
        Serial.print("potentioMappingValue2 = ");Serial.print(potentioMappingValue2);Serial.println("    ");

        Serial.print("selectTime = ");Serial.print(selectTime);Serial.print("    ");
        Serial.print("selectTimeHour = ");Serial.print(selectTimeHour);Serial.print("    ");
        Serial.print("selectTimeMinute = ");Serial.print(selectTimeMinute);Serial.println("    ");
    }
}

# 코드 변경이력






### 006

부저를 이용한 각도에 따른 음높이 변화를 + 해주었다.



### 005버전 기능별 소개

### MPU6050 센서를 이용한 기울기 감지 및 LED 제어 시스템

#### 개요
이 프로젝트는 MPU6050 가속도계 및 자이로스코프 센서를 사용하여 객체의 기울기를 감지하고, 감지된 기울기에 따라 LED를 제어하는 시스템입니다. 기울기가 특정 임계값을 초과할 경우 LED가 순차적으로 켜지며, 이는 경고 신호로 사용될 수 있습니다.

#### 구성 요소
- **MPU6050 센서**: 가속도 및 자이로스코프 값을 제공합니다.
- **LED 배열**: 기울기 각도에 따라 제어됩니다.
- **Arduino 보드**: 센서 데이터를 읽고 LED를 제어합니다.

#### 기능 설명
1. **센서 초기화 및 설정**
   - MPU6050 센서가 성공적으로 초기화되면 시리얼 모니터에 성공 메시지가 표시됩니다.
   - 초기화에 실패할 경우, 모든 LED가 깜박이며 오류를 알립니다.

2. **기울기 각도 계산 및 LED 제어**
   - 센서로부터 가속도 및 자이로스코프 데이터를 읽어 기울기 각도를 계산합니다.
   - 계산된 각도가 설정된 임계값을 초과하면 LED가 순차적으로 켜집니다.

3. **디버깅 정보 출력**
   - 가속도, 자이로스코프, 온도 데이터를 시리얼 모니터에 출력하여 디버깅을 용이하게 합니다.

#### 주의 사항
- 센서의 정확도 및 반응 속도는 `mpu.setFilterBandwidth()` 함수를 통해 조정할 수 있습니다.
- LED 제어 로직은 `controlLEDsByAngle()` 함수 내에서 수정할 수 있습니다.
- 시스템의 반응 속도는 메인 루프의 `delay()` 함수를 조정하여 변경할 수 있습니다.

#### 확장 가능성
- 기울기 각도에 따라 다른 유형의 경고(소리, 진동 등)를 추가할 수 있습니다.
- 다양한 센서와 결합하여 더 복잡한 시스템을 구축할 수 있습니다.

---

이 문서는 프로젝트의 핵심 기능과 구성 요소를 요약한 것으로, 프로젝트의 이해를 돕고, 필요에 따라 시스템을 수정하거나 확장하는 데 도움이 될 것입니다. 프로젝트의 성공적인 마무리를 기원합니다.












### 004

에러 검출을 위한 과정 
분석한 코드를 바탕으로 에러 검출을 위한 계획 또는 리스트를 작성하면 다음과 같습니다:

1. **하드웨어 연결 확인**: MPU6050 센서가 아두이노 보드에 올바르게 연결되어 있는지 확인합니다. 연결이 불안정하거나 잘못된 핀에 연결되어 있으면 센서를 인식하지 못할 수 있습니다.

2. **라이브러리 버전 확인**: `Adafruit_MPU6050` 및 `Adafruit_Sensor` 라이브러리의 최신 버전을 사용하고 있는지 확인합니다. 라이브러리가 오래되었거나 호환되지 않는 버전을 사용하고 있다면 업데이트가 필요할 수 있습니다.

3. **시리얼 통신 속도 일치**: 아두이노 IDE의 시리얼 모니터와 코드 내에서 설정한 시리얼 통신 속도(`serialComBaudrate`)가 일치하는지 확인합니다. 불일치할 경우 데이터가 제대로 전송되지 않을 수 있습니다.

4. **센서 초기화 실패 처리**: 센서 초기화에 실패했을 때 무한 루프에 빠지는 대신, 재시도 메커니즘을 구현하거나, 시리얼을 통해 구체적인 에러 메시지를 출력하여 문제 해결에 도움이 되는 정보를 제공할 수 있습니다.

5. **센서 데이터 읽기 지연**: `getValueMPU6050` 함수 내에서 `delay(10)`을 사용하고 있는데, 이는 센서 데이터를 읽는 데 필요한 최적의 지연 시간인지 확인합니다. 너무 짧거나 긴 지연 시간은 데이터 수집에 영향을 줄 수 있습니다.

6. **각도 변환 정확성 검증**: `radiansToDegrees` 함수를 사용하여 계산한 각도가 실제 기울기와 정확히 일치하는지 검증합니다. 변환 공식이 정확하게 적용되었는지 확인하고, 필요한 경우 보정할 수 있습니다.

7. **경고 조건의 적절성**: `checkTiltWarning` 함수에서 설정한 60도 이상 기울었을 때 경고를 출력하는 조건이 프로젝트의 요구 사항에 적합한지 검토합니다. 기울기 감지의 민감도를 조정해야 할 수도 있습니다.

8. **온도 데이터 활용**: 코드에서 온도 데이터(`newTemp`)를 읽고 있지만, 특별한 처리를 하지 않고 있습니다. 온도 데이터를 활용하는 로직이 필요한 경우 추가 개발이 필요할 수 있습니다.

9. **에러 로깅 및 디버깅**: 문제 발생 시 시리얼 모니터를 통해 충분한 정보를 로깅하여 문제를 쉽게 진단할 수 있도록 합니다. 예를 들어, 센서 값이 예상 범위를 벗어났을 때 추가적인 에러 메시지를 출력할 수 있습니다.

이러한 검토 사항들을 통해 코드의 안정성을 높이고, 잠재적인 문제를 사전에 발견하여 수정할 수 있습니다.

### 003
- 완전 분리형 성공  , 딜레이 (10) => 안쓰게 수정해야함 
```c++
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
  Serial.println("MPU6050 IS initialized");
  delay(100);
}

void loop()
{
   // getValueMPU6050 함수로부터 값들을 받을 변수 선언 
   float ax, ay, az ,gx ,gy , gz, newTemp;
   // ax , ay , az 등의 실수 변수를 쓸 준비가 완료됨 
   getValueMPU6050(ax, ay, az, gx , gy, gz, newTemp);
   // Do something with ax, ay, az...
  // Serial.print("ax = ");
  //  Serial.print(ax);
  //  Serial.print(", ay = ");
  //  Serial.print(ay);
  //  Serial.print(", az = ");
  //  Serial.print(az);
  //  Serial.print("gx = ");
  //  Serial.print(gx);
  //  Serial.print(", gy = ");
  //  Serial.print(gy);
  //  Serial.print(", gz = ");
  //  Serial.println(gz);
  //Serial.println(newTemp);


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
  
  gx = g.gyro.x;
  gy = g.gyro.y;
  gz = g.gyro.z;

  newTemp = temp.temperature;

  delay(10);  // ==> 딜레이 말고 다른 것으로 번경해야함 
}
```
### 002
- 완전 분리형 코드로 시도중
```c++
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;        // 객체

const int serialComBaudrate = 9600;


void setup(void) {
  Serial.begin(serialComBaudrate);

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(20);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  delay(100);
}

void loop() {

  /* Get new sensor events with the readings */
  readSensorData();  
  
  

  /* Print out the values */
  
}

void readSensorData() {
  // Read sensor data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print sensor data
  
  Serial.print(a.acceleration.x);
  Serial.print(", ");
  Serial.print(a.acceleration.y);
  Serial.print(", ");
  Serial.print(a.acceleration.z);
  Serial.print("    ");

  
  Serial.print(g.gyro.x);
  Serial.print(", ");
  Serial.print(g.gyro.y);
  Serial.print(", ");
  Serial.println(g.gyro.z);  
  
  delay(10);
 
}
```
### 001 
- 시리얼출력부분을 별도 함수로 
```c++
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial.begin(115200);

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  delay(100);
}

void loop() {
// Read sensor data and store in variables
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Call the separate function to print the values
  printSensorData(a, g);

  delay(10);
}

// New function to print sensor data in a reusable way
void printSensorData(sensors_event_t accel, sensors_event_t gyro) {
  Serial.print(accel.acceleration.x);
  Serial.print(",");
  Serial.print(accel.acceleration.y);
  Serial.print(",");
  Serial.print(accel.acceleration.z);
  Serial.print(", ");
  Serial.print(gyro.gyro.x);
  Serial.print(",");
  Serial.print(gyro.gyro.y);
  Serial.print(",");
  Serial.print(gyro.gyro.z);
  Serial.println("");
}
```

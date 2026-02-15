// 가상시뮬레이터 주소 : https://wokwi.com/projects/422825257497440257

#include <OneWire.h>
#include <DallasTemperature.h>

const int DS18B20_PIN = 2;
const int TEMPERATURE_PRECISION = 9; // 9, 10, 11, 12 비트 해상도 가능
const int deviceCount = 4;           // 센서 총 갯수

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// 4개의 센서 주소를 저장할 배열 크기 지정 ( 기본 4개 )
DeviceAddress sensorAddresses[4];

// BUS 라인에 물린 장치 수를 시리얼 출력 , LCD로 + 출력 수정할것
void printDeviceCount()
{
    Serial.print("Locating devices...");
    Serial.print("Found ");
    Serial.print(sensors.getDeviceCount(), DEC);
    Serial.println(" devices.");
}

// 센서의 기생전력 상태 체크 , LCD로 + 출력 수정할것
void printParasitePowerStatus()
{
    Serial.print("Parasite power is: ");
    if (sensors.isParasitePowerMode())
        Serial.println("ON");
    else
        Serial.println("OFF");
}

// 센서들의 주소를 읽어 정상 주소인지 체크 , LCD로 + 출력 수정할것
void findSensorAddress()
{
    for (int i = 0; i < deviceCount; i++)
    {
        if (!sensors.getAddress(sensorAddresses[i], i))
        {
            Serial.print("Unable to find address for Device ");
            Serial.println(i);
        }
        else
        {
            Serial.print("Found address for Device ");
            Serial.println(i);
        }
    }
}

// 센서들의 주소를 시리얼 출력 , LCD로 + 출력 수정할것
void printSensorsAddress()
{
    // 센서들의 주소를 출력
    for (int i = 0; i < deviceCount; i++)
    {
        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" Address: ");
        printAddress(sensorAddresses[i]);
        Serial.println();
    }
}

// 센서들의 해상도를 설정 , LCD로 + 출력 수정할것
void setSensorsResolution()
{
    for (int i = 0; i < deviceCount; i++)
    {
        sensors.setResolution(sensorAddresses[i], TEMPERATURE_PRECISION);
    }
}

// 설정된 센서들의 해상도를 시리얼 출력 , LCD로 + 출력 수정할것
void printSensorsResolution()
{
    for (int i = 0; i < deviceCount; i++)
    {
        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" Resolution: ");
        Serial.print(sensors.getResolution(sensorAddresses[i]), DEC);
        Serial.println();
    }
}

// 주소 인자변수를 받아서 8바이트 주소를 16진수로 출력 , LCD로 + 출력 수정할것 , 정확히 어떤 역활인지 공부 할것
void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16)
            Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

// 센서들의 raw 값이 지정된 에러값인지 체크하고 이상없다면 섭씨와 화씨를 출력 , LCD로 + 출력 수정할것
void printTemperature(DeviceAddress deviceAddress)
{
    float tempC = sensors.getTempC(deviceAddress);
    if (tempC == DEVICE_DISCONNECTED_C)
    {
        Serial.println("Error: Could not read temperature");
        return;
    }
    Serial.print("Temp C: ");
    Serial.print(tempC);
    Serial.print(" Temp F: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// 센서들의 주소와 온도 (섭씨, 화씨)를 출력 , LCD로 + 출력 수정할것
void printAllTemperatures()
{
    for (int i = 0; i < deviceCount; i++)
    {
        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" ");
        printTemperature(sensorAddresses[i]);
        Serial.println();
    }
}

/* ------------   setup ( ) --------------*/
void setup()
{
    Serial.begin(9600);
    sensors.begin();
    printDeviceCount();
    printParasitePowerStatus();
    findSensorAddress();
    printSensorsAddress();
    setSensorsResolution();
    printSensorsResolution();
}

/* ------------   loop ( ) --------------*/

void loop()
{
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures();
    Serial.println("DONE");

    for (int i = 0; i < 4; i++)
    {
        printAllTemperatures();
    }

    Serial.println();
    delay(3000); // 1초마다 온도 측정을 반복합니다.
}
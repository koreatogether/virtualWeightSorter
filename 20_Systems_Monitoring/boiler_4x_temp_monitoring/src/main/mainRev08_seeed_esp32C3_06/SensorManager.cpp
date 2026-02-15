#include "SensorManager.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress sensorAddress[MAX_SENSORS];
uint8_t foundSensors = 0;
float sensorTemps[MAX_SENSORS];

// 비차단 읽기를 위한 상태 변수
static unsigned long lastRequestTime = 0;
static bool isWaitingForConversion = false;
constexpr uint32_t CONVERSION_DELAY_MS = 750; // 12비트 해상도 기준 최대 대기 시간

void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16)
        {
            Serial.print(F("0"));
        }
        Serial.print(deviceAddress[i], HEX);
    }
}

static bool newDataAvailable = false;

bool isNewDataAvailable()
{
    if (newDataAvailable)
    {
        newDataAvailable = false;
        return true;
    }
    return false;
}

void initSensors()
{
    sensors.begin();
    // 비차단 모드 설정: requestTemperatures() 호출 시 완료될 때까지 기다리지 않음
    sensors.setWaitForConversion(false);
    
    delay(150);
    uint8_t count = sensors.getDeviceCount();
    if (count > MAX_SENSORS)
    {
        count = MAX_SENSORS;
    }

    uint8_t stored = 0;
    for (uint8_t i = 0; i < sensors.getDeviceCount() && stored < MAX_SENSORS; i++)
    {
        if (sensors.getAddress(sensorAddress[stored], i))
        {
            Serial.print(F("[Sensor] 센서 #"));
            Serial.print(stored);
            Serial.print(F(" 주소: "));
            printAddress(sensorAddress[stored]);
            Serial.println();
            stored++;
        }
    }
    foundSensors = stored;
    for (uint8_t i = 0; i < MAX_SENSORS; i++)
    {
        sensorTemps[i] = (i < foundSensors) ? 0.0 : DEVICE_DISCONNECTED_C;
    }
}

void readTemperatures()
{
    // 초기화 시 등 즉시 읽기가 필요할 때 사용 (차단 방식)
    sensors.setWaitForConversion(true);
    sensors.requestTemperatures();
    for (uint8_t i = 0; i < foundSensors; i++)
    {
        float tempC = sensors.getTempC(sensorAddress[i]);
        if (tempC != DEVICE_DISCONNECTED_C)
        {
            sensorTemps[i] = tempC;
        }
    }
    sensors.setWaitForConversion(false);
}

void updateSensors()
{
    unsigned long currentMillis = millis();

    // 1. 변환 요청이 없고 샘플링 주기가 되었을 때 요청
    if (!isWaitingForConversion)
    {
        if (currentMillis - lastRequestTime >= SAMPLE_INTERVAL_MS)
        {
            sensors.requestTemperatures();
            lastRequestTime = currentMillis;
            isWaitingForConversion = true;
        }
    }
    // 2. 변환 대기 중이고 충분한 시간이 흘렀을 때 데이터 읽기
    else
    {
        if (currentMillis - lastRequestTime >= CONVERSION_DELAY_MS)
        {
            for (uint8_t i = 0; i < foundSensors; i++)
            {
                float tempC = sensors.getTempC(sensorAddress[i]);
                // 연결 오류가 아닐 때만 업데이트
                if (tempC != DEVICE_DISCONNECTED_C && tempC != 85.0) 
                {
                    sensorTemps[i] = tempC;
                }
            }
            isWaitingForConversion = false;
            newDataAvailable = true;
        }
    }
}

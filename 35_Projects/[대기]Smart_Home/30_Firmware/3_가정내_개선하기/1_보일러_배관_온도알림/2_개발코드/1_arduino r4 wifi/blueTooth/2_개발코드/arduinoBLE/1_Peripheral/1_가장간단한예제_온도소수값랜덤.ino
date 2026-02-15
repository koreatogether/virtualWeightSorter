#include <ArduinoBLE.h>

// 사용자 정의 서비스 및 특성 UUID 정의
// 서비스와 특성에 고유한 UUID를 사용할 수 있습니다
#define SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

BLEService tempService(SERVICE_UUID);                                              // 서비스 생성
BLECharacteristic tempCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify, 8); // 최대 8바이트로 설정 (숫자 문자열, 예: -45.0 또는 125.0)

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;

    // BLE 초기화
    if (!BLE.begin())
    {
        Serial.println("Starting BLE failed!");
        while (1)
            ;
    }

    // 지역 이름 설정 및 서비스 광고
    BLE.setLocalName("ArduinoR4Temp");
    BLE.setAdvertisedService(tempService);

    // 특성을 서비스에 추가
    tempService.addCharacteristic(tempCharacteristic);
    BLE.addService(tempService);

    // 특성 값을 초기화 (문자열 "0")
    tempCharacteristic.writeValue("0");

    // 광고 시작
    BLE.advertise();
    Serial.println("BLE 주변 기기가 시작되었습니다, 연결 대기 중...");

    // 랜덤 숫자 생성기 초기화
    randomSeed(analogRead(0));
}

void loop()
{
    // BLE 이벤트 폴링
    BLEDevice central = BLE.central();

    // 중앙 기기가 연결되어 있으면
    if (central)
    {
        Serial.print("중앙 기기에 연결됨: ");
        Serial.println(central.address());

        // 중앙 기기가 연결되어 있는 동안, 1초마다 특성을 업데이트
        while (central.connected())
        {
            // -45.0 ~ +125.0 까지의 값을 0.1 단위로 랜덤하게 생성
            int randomTenths = random(-450, 1251); // -450부터 1250까지 생성
            float temperature = randomTenths / 10.0;
            char buffer[8];
            sprintf(buffer, "%0.1f", temperature); // 소수점 1자리 문자열로 변환
            tempCharacteristic.writeValue(buffer);
            Serial.print("업데이트된 온도: ");
            Serial.println(buffer);
            delay(1000);
            BLE.poll();
        }

        Serial.print("중앙 기기 연결 해제: ");
        Serial.println(central.address());
    }
}

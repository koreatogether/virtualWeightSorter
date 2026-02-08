#include <ArduinoBLE.h>

// 사용자 정의 서비스 및 특성 UUID 정의
// 서비스와 특성에 고유한 UUID를 사용할 수 있습니다
#define SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

BLEService tempService(SERVICE_UUID);                                              // 서비스 생성
BLECharacteristic tempCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify, 4); // 최대 4바이트로 설정 (숫자 최대 3자리 + null)

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
            int randomValue = random(10, 21); // 10~20 사이의 랜덤 값을 생성 (양끝 포함)
            char buffer[4];
            sprintf(buffer, "%d", randomValue); // 값을 문자열로 변환
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

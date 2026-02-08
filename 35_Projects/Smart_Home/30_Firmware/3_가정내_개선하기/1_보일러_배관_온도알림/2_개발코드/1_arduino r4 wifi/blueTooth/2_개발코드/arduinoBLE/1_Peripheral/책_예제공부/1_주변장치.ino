#include <ArduinoBLE.h>

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ; // 시리얼 모니터가 열릴때까지 기다린다.

    pinMode(LED_BUILTIN, OUTPUT);

    if (!BLE.begin())
    {
        Serial.println("BLE 초기화 실패했음");
        while (1)
            ; // 다음으로 진행안하고 무한 루프에 빠짐
    }

    // 주변 장치(arduino R4 WIFI 보드) 정보 설정
    BLE.setLocalName("BLE_BRODCASTER");
    BLE.setAdvertisedServiceUuid("8CD4AF2D-5AD8-4ECF-A7F2-D3E462BCB262");

    BLE.advertise(); // 주변 장치로 브로드캐스트 시작
    Serial.println("BLE 시작됨 , 중앙 장치가 연결되길 기다리고 있음");
}

void loop()
{
    BLEDevice central = BLE.central(); // 연결되는 중앙 장치를 감지하고 해당 중앙장치에 대한 정보를
                                       // BLEDevice 객체를반환한다.

    if (central)
    {
        Serial.print(" => 연결된 중앙 장치 : ");
        Serial.println(central.address()); // BLEDevice 객체(연결된 중앙장치)의 고유한 BLE 주소(일종의 식별자)
                                           // 를 문자열로 반환한다. 이 주소는 디바이스를 구분하는데 목적이 있다.
        digitalWrite(LED_BUILTIN, HIGH);   // LED 켜기 , 중앙장치가 연결됨을 시각적으로 알려주는 용도

        while (central.connected())
        {
            // 중앙 장치가 연결된 동안 할 코드 작성 하면됨
        }

        digitalWrite(LED_BUILTIN, LOW); // LED 끄기 , 중앙장치 연결이 끊김을 시각적으로 알려주는 용도
        Serial.println(" 중앙장치 연결 종료");
    }
}
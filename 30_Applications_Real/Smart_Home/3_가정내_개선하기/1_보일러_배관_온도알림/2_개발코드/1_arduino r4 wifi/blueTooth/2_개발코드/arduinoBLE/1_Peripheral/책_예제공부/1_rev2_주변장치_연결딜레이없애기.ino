#include <ArduinoBLE.h>

// setup(): 프로그램 초기화 및 BLE 설정
// 1. 시리얼 통신 초기화: 디버깅 메시지를 출력하기 위해 사용
// 2. LED_BUILTIN 핀 설정: 연결 상태를 시각적으로 표시하기 위해 사용
// 3. BLE 초기화 및 설정: BLE.begin()으로 초기화 후, 로컬 이름과 서비스 UUID를 설정하여 중앙 장치와의 연결 준비
// 4. BLE.advertise(): 주변 브로드캐스트를 시작하여 연결 요청을 대기
void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        ; // 시리얼 모니터가 열릴 때까지 대기
    }

    pinMode(LED_BUILTIN, OUTPUT); // 내장 LED 핀을 출력 모드로 설정

    // BLE 초기화 실패 시 무한 루프에 머무르며 오류를 알림
    if (!BLE.begin())
    {
        Serial.println("BLE 초기화 실패!");
        while (1)
        {
        }
    }

    // BLE 주변장치 정보 설정
    BLE.setLocalName("BLE_BRODCASTER");                                   // 장치의 이름 지정
    BLE.setAdvertisedServiceUuid("8CD4AF2D-5AD8-4ECF-A7F2-D3E462BCB262"); // 광고할 서비스 UUID 지정

    BLE.advertise(); // BLE 브로드캐스트 시작
    Serial.println("BLE 시작됨, 중앙 장치 연결 대기중...");
}

// loop(): 메인 실행 루프
// 1. BLE.central()을 통해 연결 요청이 들어온 중앙 장치를 감지
// 2. 연결된 중앙 장치의 주소를 출력하고 LED를 켬
// 3. 중앙 장치와 연결된 동안 필요한 작업을 수행할 수 있도록 while 루프 사용
// 4. 중앙 장치 연결 종료 시 LED를 끄고 종료 메시지 출력
void loop()
{
    // 주변에서 연결 요청이 있는 중앙 장치 탐색
    BLEDevice central = BLE.central();

    // 중앙 장치가 연결되었는지 체크
    if (central)
    {
        Serial.print("연결된 중앙 장치 주소: ");
        Serial.println(central.address()); // 연결된 중앙 장치의 고유 BLE 주소 출력

        digitalWrite(LED_BUILTIN, HIGH); // 연결 상태를 시각적으로 표시하기 위해 LED 켜기

        // 연결 시작 시간 기록
        unsigned long connectionStartTime = millis();

        // 기존 연결 유지 코드에 10초 후 자동 연결 해제 추가
        while (central.connected())
        {
            if (millis() - connectionStartTime > 10000)
            {
                central.disconnect();
                break;
            }
            // ... 연결 유지 중 수행할 작업 구현 ...
        }

        // 중앙 장치 연결 종료 후 실행
        digitalWrite(LED_BUILTIN, LOW); // LED 끄기
        Serial.println("중앙 장치 연결 종료");
    }
}

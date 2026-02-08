// ArduinoBLE 라이브러리 포함 - BLE 기능 구현에 사용
#include <ArduinoBLE.h>

// BLE 서비스 객체 생성: UUID를 통해 고유 식별, LED 제어를 위한 서비스
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");
// BLE Byte 특성 객체 생성: 읽기와 쓰기 기능 제공, LED 상태 제어용
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void setup()
{
    // 시리얼 통신 초기화: 9600 baud rate를 사용하여 시리얼 모니터와 통신
    Serial.begin(9600);
    // 시리얼 포트가 준비될 때까지 대기
    while (!Serial)
    {
        Serial.println("Serial is not ready");
    }

    // 내장 LED 핀을 출력 모드로 설정
    pinMode(LED_BUILTIN, OUTPUT);

    // BLE 초기화: 실패 시 메시지 출력 후 무한 루프에 진입
    if (!BLE.begin())
    {
        Serial.println("BLE 초기화에 실패를 했습니다.");
        while (1) // 무한 대기
        {
            ;
        }
    }

    // BLE 장치 이름 설정, 연결 시 식별용
    BLE.setLocalName("LED Service");

    // 사용자 정의 서비스에 특성(LED 상태 제어용)을 추가
    ledService.addCharacteristic(switchCharacteristic);

    // BLE 스택에 사용자 정의 서비스를 등록
    BLE.addService(ledService);
    // 광고(advertising) 시 제공할 서비스로 설정
    BLE.setAdvertisedService(ledService);
    // 특성의 초기 값 설정 (0: LED OFF)
    switchCharacteristic.writeValue(0);

    // BLE 광고 시작: 주변 장치가 검색할 수 있도록 광고 신호 전송
    BLE.advertise();
    // 시리얼 모니터에 BLE 모듈 시작 및 연결 대기 메시지 출력
    Serial.println("BLE 모듈을 시작 했습니다. 중앙 장치 연결을 기다리고 있습니다.");
}

void loop()
{
    // 연결된 중앙 장치(스마트폰 등)를 검사 및 반환
    BLEDevice central = BLE.central();

    // 중앙 장치가 연결되었을 경우
    if (central)
    {
        // 연결된 중앙 장치의 주소를 시리얼 모니터에 출력
        Serial.print(" => 연결된 중앙 장치");
        Serial.println(central.address());

        // 중앙 장치와의 연결이 유지되는 동안 반복 실행
        while (central.connected())
        {
            // 특성 값이 변경되었을 때 감지
            if (switchCharacteristic.written())
            {
                // 특성의 값에 따라 LED 상태 제어 (참이면 LED 켜기, 거짓이면 LED 끄기)
                // 주의: 아래의 변수명에 오타가 있을 수 있으므로 실제 작동 전에 확인 필요
                if (switchChracteristic.value())
                {
                    Serial.println(" => LED ON");
                    digitalWrite(LED_BUILTIN, HIGH);
                }
                else
                {
                    Serial.println(" => LED OFF");
                    digitalWrite(LED_BUILTIN, LOW);
                }
            }
        }

        // 중앙 장치의 연결이 해제되었을 때 메시지 출력
        Serial.print(" => 중앙 장치 연결 끊김");
    }
}
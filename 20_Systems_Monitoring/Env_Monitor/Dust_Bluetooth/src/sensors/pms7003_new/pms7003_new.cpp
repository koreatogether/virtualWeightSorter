#include <Arduino.h>
#include "pms7003_new.h"

// PMS7003 센서 객체 정의 (Hardware Serial 사용)
PMS pms7003_sensor(Serial1);
PMS::DATA pms7003_data;

// PMS7003 센서 초기화 함수
void init_pms7003_sensor()
{
    // PMS7003 Hardware Serial 초기화
    Serial1.begin(PMS7003_BAUD_RATE);

    Serial.println("PMS7003 센서 초기화 시작...");
    Serial.print("Hardware Serial 사용 - Serial1");
    Serial.print(", RX 핀: 23, TX 핀: 22");
    Serial.print(", Baudrate: ");
    Serial.println(PMS7003_BAUD_RATE);

    // PMS7003 센서를 슬립 모드로 시작 (팬 정지)
    pms7003_sensor.sleep();
    Serial.println("PMS7003 센서 슬립 모드 시작");

    Serial.println("PMS7003 센서 초기화 완료 - millis 기반 팬 수명 연장 사이클 시작");
}

// PMS7003 센서 메인 루프 처리 함수
void pms7003_loop()
{
    static unsigned long cycleStartTime = 0;
    static bool isFanOn = false;
    static bool isInitialized = false;
    static unsigned long initStartTime = 0;
    static int countdownValue = 20;
    static unsigned long lastCountdownTime = 0;

    unsigned long currentTime = millis();

    // 초기화 과정 (millis 기반 논블로킹)
    if (!isInitialized)
    {
        if (initStartTime == 0)
        {
            initStartTime = currentTime;
            Serial.println("PMS7003 센서 팬 안정화 대기 중... (20초)");
        }

        // 1초마다 카운트다운 (논블로킹)
        if (currentTime - lastCountdownTime >= 1000 && countdownValue > 0)
        {
            Serial.print("팬 안정화까지 남은 시간: ");
            Serial.print(countdownValue);
            Serial.println("초");
            countdownValue--;
            lastCountdownTime = currentTime;
        }

        // 20초 경과 시 초기화 완료
        if (currentTime - initStartTime >= 20000)
        {
            isInitialized = true;
            Serial.println("PMS7003 센서 초기화 완료 - 팬 수명 연장을 위한 10초 ON/OFF 사이클 시작");
        }
        return; // 초기화 중에는 다른 작업하지 않음
    }

    // 초기화 완료 후 팬 제어 시작
    if (cycleStartTime == 0)
    {
        cycleStartTime = currentTime;
        isFanOn = true;
        pms7003_sensor.wakeUp(); // 팬 ON (라이브러리 메소드 사용)
        Serial.println("=== PMS7003 팬 시작 (10초 작동) ===");
    }

    // 10초 ON / 10초 OFF 사이클 관리
    unsigned long cycleElapsed = currentTime - cycleStartTime;

    // 디버깅 정보 출력 (1초마다)
    static unsigned long lastDebugTime = 0;
    if (currentTime - lastDebugTime >= 1000)
    {
        lastDebugTime = currentTime;
        Serial.print("팬 상태: ");
        Serial.print(isFanOn ? "ON" : "OFF");
        Serial.print(", 경과 시간: ");
        Serial.print(cycleElapsed / 1000);
        Serial.println("초");
    }

    if (isFanOn && cycleElapsed >= 10000) // 10초 경과 시 팬 OFF
    {
        isFanOn = false;
        pms7003_sensor.sleep(); // 팬 OFF (라이브러리 메소드 사용)
        Serial.println("=== PMS7003 팬 정지 (10초 휴식) ===");
        cycleStartTime = currentTime;
    }
    else if (!isFanOn && cycleElapsed >= 10000) // 10초 경과 시 팬 ON
    {
        isFanOn = true;
        pms7003_sensor.wakeUp(); // 팬 ON (라이브러리 메소드 사용)
        Serial.println("=== PMS7003 팬 시작 (10초 작동) ===");
        cycleStartTime = currentTime;
    }

    // 팬이 켜진 상태에서만 데이터 읽기 시도 (팬 시작 후 3초부터)
    if (isFanOn && cycleElapsed >= 3000 && cycleElapsed < 10000)
    {
        static unsigned long lastReadAttempt = 0;

        // 2초마다 데이터 읽기 시도
        if (currentTime - lastReadAttempt >= 2000)
        {
            lastReadAttempt = currentTime;

            Serial.println("PMS7003 데이터 읽기 시도...");

            // PMS7003 센서 데이터 읽기 (millis() 기반 타임아웃)
            unsigned long readStartTime = millis();
            bool dataReceived = false;

            while (millis() - readStartTime < 3000) // 3초 타임아웃
            {
                if (pms7003_sensor.readUntil(pms7003_data, 100)) // 100ms 타임아웃
                {
                    dataReceived = true;
                    break;
                }
                delay(10); // 짧은 지연
            }

            if (dataReceived)
            {
                // 데이터 유효성 확인
                if (pms7003_data.PM_AE_UG_1_0 >= 0 && pms7003_data.PM_AE_UG_2_5 >= 0 && pms7003_data.PM_AE_UG_10_0 >= 0)
                {
                    // 데이터 출력
                    Serial.println("=== PMS7003 데이터 읽기 성공 ===");
                    Serial.print("PM 1.0 (ug/m3): ");
                    Serial.println(pms7003_data.PM_AE_UG_1_0);

                    Serial.print("PM 2.5 (ug/m3): ");
                    Serial.println(pms7003_data.PM_AE_UG_2_5);

                    Serial.print("PM 10.0 (ug/m3): ");
                    Serial.println(pms7003_data.PM_AE_UG_10_0);

                    Serial.println("================================");
                    Serial.println();
                }
                else
                {
                    Serial.println("PMS7003 센서 데이터가 유효하지 않습니다!");
                    Serial.print("원시 데이터 - PM1.0: ");
                    Serial.print(pms7003_data.PM_AE_UG_1_0);
                    Serial.print(", PM2.5: ");
                    Serial.print(pms7003_data.PM_AE_UG_2_5);
                    Serial.print(", PM10: ");
                    Serial.println(pms7003_data.PM_AE_UG_10_0);
                }
            }
            else
            {
                Serial.println("PMS7003 센서에서 데이터를 읽을 수 없습니다!");
                Serial.println("팬이 충분히 회전하지 않았거나 센서 연결을 확인하세요.");
            }
        }
    }
}
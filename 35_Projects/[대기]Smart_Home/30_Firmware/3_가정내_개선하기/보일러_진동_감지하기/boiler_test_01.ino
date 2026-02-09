/*

SW-420 진동 센서를 이용한 보일러 작동 횟수 카운터

보드 : 아두이노 나노
프로세서 : ATmega328P
프로그래밍 언어 : C++

*/

// SW-420 변수 선언
const int SW420_Pin = 2; // SW-420 핀 번호
int SW420_Value = 0;     // SW-420 센서 입력값 저장 변수
int sensorMoveCount = 0;           // 진동 센서 흔들림 카운트 변수
int sensorNotMoveCount = 0;        // 진동 센서 흔들림 없음 카운트 변수
int boilerWorkCount = 0;           // 보일러 작동 횟수 카운트 변수

// 보일러 작동 시간 체크 변수
unsigned long boilerStartTime = 0; // 보일러 작동 시작 시간 변수
unsigned long boilerEndTime = 0;   // 보일러 작동 종료 시간 변수
unsigned long boilerWorkTime = 0;  // 보일러 작동 시간 변수

// millis() 변수 선언
unsigned long previousMillis = 0; // 이전 시간 저장 변수
const long interval = 1000;       // 1초

// 셋업 함수 선언 및 시리얼 통신 설정
void setup()
{
    Serial.begin(9600);        // 시리얼 통신 속도 설정
    pinMode(SW420_Pin, INPUT); // SW-420 핀을 입력으로 설정
}

// 루프 함수 선언
void loop()
{
    // millis 사용
    unsigned long currentMillis = millis(); // 현재 시간 저장 변수

    int SW420_Value = digitalRead(SW420_Pin); // SW-420 값 읽기
    // millis() 사용 하여 1초마다  SW-420 값이 HIGH일 때 카운트 증가 및 카운트 값 출력
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis; // 이전 시간 저장 변수에 현재 시간 저장
        if (SW420_Value == HIGH)        // SW-420 값이 HIGH일 때
        {
            sensorMoveCount++;               // 카운트 증가
            Serial.println(sensorMoveCount); // 카운트 값 출력
        }
    }

    // 카운터가 4회이상 이면 보일러 작동으로 간주
    if (sensorMoveCount >= 4)
    {
        Serial.println("Boiler is working"); // 보일러 작동 출력
        sensorMoveCount = 0;                           // 카운트 초기화
        boilerWorkCount++;                             // 보일러 작동 횟수 카운트 증가
        // 보일러 작동 시작 시간 저장
        boilerStartTime = millis();
    }

    // 1초마다 진동 센서 값이 LOW일 때 sensorNotMoveCount 증가 및 카운트 값 출력
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis; // 이전 시간 저장 변수에 현재 시간 저장
        if (SW420_Value == LOW)         // SW-420 값이 LOW일 때
        {
            sensorNotMoveCount++;               // 카운트 증가
            Serial.println(sensorNotMoveCount); // 카운트 값 출력
        }
    }

    // sensorNotMoveCount가 4회이상 이면 보일러 작동 종료로 간주
    if (sensorNotMoveCount >= 4)
    {
        Serial.println("Boiler is not working"); // 보일러 작동 종료 출력
        sensorNotMoveCount = 0;                            // 카운트 초기화
        // 보일러 작동 종료 시간 저장
        boilerEndTime = millis();
        // 보일러 작동 시간 계산
        boilerWorkTime = boilerEndTime - boilerStartTime;
        // 보일러 작동 시간 출력
        Serial.print("Boiler work time : ");
        Serial.println(boilerWorkTime);
        // 보일러 가동 횟수 출력
        Serial.print("Boiler work count : ");
        Serial.println(boilerWorkCount);
    }
}


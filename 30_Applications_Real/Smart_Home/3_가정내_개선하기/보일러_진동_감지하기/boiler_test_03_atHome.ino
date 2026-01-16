/*
보일러 작동시 카운터 ++
보일러 비작동시 카운터 ++
보일러 작동시 현재 시간 측정 
보일러 비작동시 현재 시간 측정
보일러 작동 과 비작동간 총 작동시간 측정
작동 시간 측정 방향은 작동 -> 비작동시 측정
*/

const int SW420_Pin = 2;
int SW420_Value = 0;
int sensorMoveCount = 0;
int sensorNotMoveCount = 0;
int boilerWorkCount = 0;

unsigned long previousMillis = 0; // noDelay를 위한 밀리스함수 사용 변수
const long interval = 1000;       // 1초마다 센싱 및 행동 주기

// 보일러 작동시 시간을 측정 변수 및 비작동시 시간 측정 변수
unsigned long boilerStartTime = 0;
unsigned long boilerStopTime = 0;
unsigned long boilerWorkedTime = 0;
unsigned long previousBoilerStopTime = 0;
bool boilerStartFlag = false;
bool boilerStopFlag = false;

void setup()
{
    Serial.begin(9600);
    pinMode(SW420_Pin, INPUT_PULLUP);
}

void loop()
{
    unsigned long currentMillis = millis();
    int SW420_Value = digitalRead(SW420_Pin);

    if (currentMillis - previousMillis >= interval) // every 1 second
    {
        previousMillis = currentMillis;
        if (SW420_Value == HIGH)
        {
            sensorMoveCount++;
            sensorNotMoveCount = 0;
        }

        if (SW420_Value == LOW)
        {
            sensorNotMoveCount++;
            sensorMoveCount = 0;
        }

        checkBoilerWorkedTime();
        SerialPrint_1s();
    }

    if (sensorMoveCount >= 4)
    {

        sensorMoveCount = 0;
        boilerWorkCount++;
        boilerStartTime = millis();
        boilerStartFlag = true;

        SerialPrintforMoveCount();
    }

    if (sensorNotMoveCount >= 4)
    {

        sensorNotMoveCount = 0;
        previousBoilerStopTime = boilerStopTime;
        boilerStopTime = millis();
        boilerStopFlag = true;

        SerialPrintforNotMoveCount();
    }
}

// 보일러 총 작동 시간 측정 코드
void checkBoilerWorkedTime()
{

    if (boilerStartFlag == true && boilerStopFlag == true)
    {
        if (boilerStopTime >= boilerStartTime)
        {
            boilerWorkedTime = boilerStartTime - previousBoilerStopTime;
            Serial.print("   Boiler Worked Time = ");
            Serial.println(boilerWorkedTime);
            boilerStartFlag = false;
            boilerStopFlag = false;
        }
    }
}

// 1초마다 시리얼 출력이 필요한 출력 펑션코드
void SerialPrint_1s()
{

    Serial.print("MoveCount = ");
    Serial.print(sensorMoveCount);
    Serial.print("   ");
    Serial.print("NotMoveCount =  ");
    Serial.print(sensorNotMoveCount);
    Serial.print("   ");
    Serial.print("workCount = ");
    Serial.println(boilerWorkCount);
    Serial.print("startFlag = ");
    Serial.print(boilerStartFlag);
    Serial.print("   stoptFlag = ");
    Serial.println(boilerStopFlag);

    Serial.print("   preStopTime = ");
    Serial.println(previousBoilerStopTime);
}

void SerialPrintforMoveCount()
{
    Serial.print("Boiler is working   ");
    Serial.print("boilerStartTime =  ");
    Serial.print(boilerStartTime);
    Serial.print("   boilerStopTime = ");
    Serial.println(boilerStopTime);
}

void SerialPrintforNotMoveCount()
{
    Serial.print("Boiler is not working   ");
    Serial.print("boilerStartTime =  ");
    Serial.print(boilerStartTime);
    Serial.print("   boilerStopTime = ");
    Serial.println(boilerStopTime);
}
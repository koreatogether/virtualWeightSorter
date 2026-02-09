//* 시리얼 통신연결

#include <SoftwareSerial.h>

int variable = 0;
void setup()
{
    Serial.begin(9600);
}

void loop()
{

    variable = variable + 20;

    Serial.println(variable);
    delay(1000);
}

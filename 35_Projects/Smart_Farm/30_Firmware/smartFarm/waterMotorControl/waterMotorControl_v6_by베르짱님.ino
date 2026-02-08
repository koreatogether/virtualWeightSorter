// 가상시뮬레이터 : https://wokwi.com/projects/343575973103403602


#include <ezOutput.h>
#include <ezButton.h>


#define relayPin 2
#define startButon 3
#define stopButon 4
#define pot1Pin A0
#define pot2Pin A1

// ezButton 클래스 객체 생성
ezButton start_button(startButon); // 시작 버튼
ezButton stop_button(stopButon);  // 멈춤 버튼
ezOutput relay(relayPin);

int isRunFlag = false;

// 시리얼통신 115200 ,  ezButton and u8glib 초기화 없음
void setup()
{
  Serial.begin(115200);

  // ezButton debounce time 45ms
  start_button.setDebounceTime(45);
  stop_button.setDebounceTime(45);
}

void loop()
{
  start_button.loop();
  stop_button.loop();

  // on button pushed
  if (start_button.isPressed()) {
    unsigned long onTime  = map(analogRead(pot1Pin),0,1023,1,180)*50;
    unsigned long offTime = map(analogRead(pot2Pin),0,1023,1,180)*50;
    relay.blink(offTime, onTime);

    Serial.print("OnTime  : "); Serial.println(onTime);
    Serial.print("OffTime : "); Serial.println(offTime);

    isRunFlag = true;  
    Serial.println("Running"); 
  }

  // off button pushed
  if (stop_button.isPressed()) {
    isRunFlag = false;
    Serial.println("Stopped");
  }

  if (isRunFlag) {
    relay.loop();
  } else {
    relay.low();
   
  }
}



/* 싸인 보드 컨트롤 https://koskomro.com/shop/view.php?index_no=36974&gclid=EAIaIQobChMI4YqU_-jh-QIVT6mWCh3NqA_HEAkYAyABEgIOnPD_BwE
   싸인 보드 차량 부착 예 http://www.nofire.co.kr/m/product.html?branduid=143282
   해외 검색어
   1. Direction arrow
   2. arrow amber
   3. arrow led indicator light
   4. traffic advisor

    제목 : 차량 방향 지시등 따라 해보기

    제어 내용을 간결하게 표현한다. 각 핀에 연결된 구성 요소를 참조한다.
    !! 주의점 !!
    실제 led를 회로도처럼 연결하면 우노 , 나노 보드 탑니다.!

    The circuit :

      *        *
    *            *
  *   *  *  *  *   *
    *            *
      *        *

    * 각 input에 연결되어있는 구성 요소의 목록
          - tack button 3ea

    * 각 output에 연결되어있는 구성 요소의 목록
     - LED + Register 220 ohm 8ea
     https://wokwi.com/projects/424318992734007297
*/

// use ezButton library and ezOutput library
#include <ezButton.h>
#include <ezOutput.h>

ezButton button1(2); // right arrow
ezButton button2(3); //  bothside arrow
ezButton button3(4); // left arrow

ezOutput led1(12);
ezOutput led2(11);
ezOutput led3(10);
ezOutput led4(9);
ezOutput led5(8);
ezOutput led6(7);
ezOutput led7(6);
ezOutput led8(5);

void setup()
{
    Serial.begin(9600);
    button1.setDebounceTime(50); // 디바운스 처리
    button2.setDebounceTime(50);
    button3.setDebounceTime(50);
    button1.setCountMode(COUNT_FALLING); // 카운팅 하기 위한 것
    button2.setCountMode(COUNT_FALLING); // 카운팅 하기 위한 것
    button3.setCountMode(COUNT_FALLING); // 카운팅 하기 위한 것
}

void loop()
{
    button1.loop();
    button2.loop();
    button3.loop();

    led1.loop();
    led2.loop();
    led3.loop();
    led4.loop();
    led5.loop();
    led6.loop();
    led7.loop();
    led8.loop();

    if (button1.isPressed())
    {
        Serial.print("Left Arrow Button");
        Serial.print("   ");
        Serial.print(" Count = ");
        Serial.println(button1.getCount());

        button2.resetCount();
        button3.resetCount();

        led5.blink(300, 300, 100);
        led4.blink(300, 300, 200);
        led3.blink(300, 300, 300);
        led2.blink(300, 300, 400);
        led1.blink(300, 300, 500);

        led6.low();
        led7.low();
        led8.low();
        if (button1.getCount() == 2) //  버튼이 2번 눌리면 모든 led 끄기
        {
            everyLedOff();
            button1.resetCount();
        }
    }

    if (button2.isPressed())
    {
        Serial.print("Bothside Arrow Button");
        Serial.print("   ");
        Serial.print(" Count = ");
        Serial.println(button2.getCount());

        everyLedOff();
        button1.resetCount();
        button3.resetCount();

        led4.blink(300, 300, 100);
        led5.blink(300, 300, 100);

        led3.blink(300, 300, 200);
        led6.blink(300, 300, 200);

        led2.blink(300, 300, 300);
        led7.blink(300, 300, 300);

        led1.blink(300, 300, 400);
        led8.blink(300, 300, 400);
        if (button2.getCount() == 2) //  버튼이 2번 눌리면 모든 led 끄기
        {
            everyLedOff();
            button2.resetCount();
        }
    }

    if (button3.isPressed())
    {
        Serial.print("Right Arrow Button");
        Serial.print("   ");
        Serial.print(" Count = ");
        Serial.println(button3.getCount());

        button1.resetCount();
        button2.resetCount();

        led4.blink(300, 300, 100);
        led5.blink(300, 300, 200);
        led6.blink(300, 300, 300);
        led7.blink(300, 300, 400);
        led8.blink(300, 300, 500);

        led1.low();
        led2.low();
        led3.low();

        if (button3.getCount() == 2) //  버튼이 2번 눌리면 모든 led 끄기
        {
            everyLedOff();
            button3.resetCount();
        }
    }
}

void everyLedOff()
{
    led1.low();
    led2.low();
    led3.low();
    led4.low();
    led5.low();
    led6.low();
    led7.low();
    led8.low();
}
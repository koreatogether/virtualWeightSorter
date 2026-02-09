// https://wokwi.com/projects/405103458540744705
// https://wokwi.com/projects/405175283901065217  // 02

/**------------------------------------------------------------------------
 * *                                INFO
 *
 *
 *
 *
 *------------------------------------------------------------------------**/

#include <U8glib.h>
#include <HX711.h>  // BY robtillaart
#include <Keypad.h> //Mark Stanley, Alexander Brevig

// 핀 번호 정의
const byte dataPin = 12;
const byte clockPin = 11;

// Oled 객체 생성
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

// HX711 객체 생성
HX711 myScale;

// 전역변수 선언
float calibration_factor = 466.768432; // 켈리브레이션 통해서 얻은 값
byte times = 5;
void setup()
{
    Serial.begin(9600); // 시리얼 통신 시작

    // OLED 인식 체크
    checkOled();

    // Load cell 인식 체크
    checkLoadCell();
}

void loop()
{
    displayOled_1(); // Add your main code here
    displayOled_2(); // Add your main code here
    displayOled_3(); // Add your main code here

    caculateWeight();
}

void checkOled()
{
    if (!u8g.begin())
    {
        Serial.println(F("U8glib not started"));
        // for (;;)
        Serial.println("check OLED");
        delay(5000);
    }
}

void initLoadCell()
{
    myScale.begin(dataPin, clockPin); // HX711 시작
    Serial.println(scale.is_ready() ? F("Load cell are working.") : F("Load cell are not working."));
    myScale.set_scale(calibration_factor); // 측정 범위 설정 , 켈리브레이션 통해서 얻은 값

    // myScale.set_raw_mode(times);
    // myScale.set_average_mode(times);   // 평균값
    // myScale.set_median_mode(times);   // 양극단의 값을 제외한 중간값
    // myScale.set_medavg_mode(times);  // 중간값을 가지고 평균값
    // myScale.set_runavg_mode(times); // 런닝 평균값 , 실시간용

    myScale.tare(); // 초기화
}

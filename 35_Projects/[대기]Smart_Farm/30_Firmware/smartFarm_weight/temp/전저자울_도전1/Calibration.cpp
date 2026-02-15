#include <Calibration.h>

void calibration(HX711 myScale)
{
    Serial.println("\nStart calibration");
    Serial.println("Remove all weight from scale");
     //flush Serial input
     while (Serial.available() > 0) Serial.read();

     Serial.println("and press enter\n");
     while(Serial.available() == 0);

     Serial.println("Determine zero weight offset");
     myScale.tare(20); // 20 회 측정 후 평균값을 0으로 설정
     uini32_t offset = myScale.get_offset();

     Serial.print("Offfset:");
     Serial.println(offset);
     Serial.println();
}

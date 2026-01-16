//
//    FILE: HX_set_mode.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711

// Todo 1. 센서 연결 실패 또는 예외 상황에 대한 처리 추가 필요  [v]
// Todo 2. 무게 단위 표시 필요 [ ]
// Todo 3. 런타임중 켈리브레이션 기능 추가 필요 [ ] > 코드가 너무 길어서 분리 해야 할것같음
// Todo 4. 런타임중 Gain 변경 기능 추가 필요 [ ] -> 코드가 너무 길어서 분리 해야 할것같음
// Todo 5. 런타임중 Mode 변경 기능 추가 필요 [ ] -> 코드가 너무 길어서 분리 해야 할것같음
// Todo 6. 셋업 함수에서 기능별로 코드를 다시 정리 필요 [v]

#include "HX711.h"

HX711 scale;

const int dataPin = 12;
const int clockPin = 11;

const int NUMBER_OF_RETRY = 3;
const int WAIT_TIME = 1000;

const int NUMBER_OF_SAMPLES = 3;
const int GAIN_VALUE = 128;
const float calibration_factor = 466.768432f;

float previousWeight = 0.0f;
const float UPPER_HYSTERESIS = 0.6f;
const float LOWER_HYSTERESIS = 0.4f;

const int DELAY_TIME = 1000;
const long SERIAL_BAUD_RATE = 115200L;

void hx711_begin()
{
    scale.begin(dataPin, clockPin);

    if (!scale.wait_ready_retry(NUMBER_OF_RETRY, WAIT_TIME))
    {
        Serial.println("HX711 not found.");
    }
    else
    {
        Serial.println("HX711 is ready.");
    }
    delay(DEALY_TIME);

    scale.set_scale(calibration_factor);
    scale.set_gain(GAIN_VALUE);
    scale.tare();
    Serial.print(" set IS done , scale , gain and tare !");
    delay(DELAY_TIME);
}

float applyWeightHysteresis(float weight)
{
    float fractionalPart = weight - static_cast<int>(weight);

    if (weight != previousWeight)
    {
        if (fractionalPart >= UPPER_HYSTERESIS)
        {
            weight = ceil(weight);
        }
        else if (fractionalPart <= LOWER_HYSTERESIS)
        {
            weight = floor(weight);
        }
        else
        {
            weight = previousWeight;
        }
    }

    previousWeight = weight;
    return weight;
}

int getStableWeight()
{
    float weight = scale.get_units(NUMBER_OF_SAMPLES);
    weight = weight < 0 ? 0 : weight;
    weight = applyWeightHysteresis(weight);
    return static_cast<int>(weight);
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    isReadyHX711();
    hx711_begin();
    delay(2000);
}

void loop()
{
    float rawWeight = scale.get_units(1);
    int stableWeight = getStableWeight();

    Serial.print("rawWeight :");
    Serial.print(rawWeight);
    Serial.print("\t");
    Serial.print("Stable weight: ");
    Serial.println(stableWeight);
}
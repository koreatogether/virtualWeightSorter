
// https://wokwi.com/projects/406760615655119873 ( 04버전 _ 네오픽셀추가될것)
// https://wokwi.com/projects/406761093471327233 ( 04개선 - 네오픽셀 추가한것)
// https://wokwi.com/projects/406856366957485057 ( 05버전 - 자잘한 버그 수정 및 선오류 해결)
// https://wokwi.com/projects/406858734731378689 ( 06버전 - 등급별 네오픽셀 색상 표현)
/* INFO

객체 선언시 배열 가능

라이브러리 안에서 pinMode 선언이 있으나 코드상에 다시한번선언을 해주어야 핀이 제대로 동작함
--> 원인 파악해야함  가상 시뮬레이터가 옛날 버전일수도

객체 배열 선언이 먹히므로 나머지 코드에서도 배열을 사용가능해서 코드가 간결해짐
https://wokwi.com/projects/406757678203406337

참고할 데이타 정렬 방법 링크
https://blog.naver.com/njuhb/140157823478

선택 정렬에 대한 동영상
https://youtu.be/dyEEXdLTl9M?si=TAqCCQZBIs9wjg3y

선택 정렬 + 좀더 간단하게 계산을 알려주는 동영상
https://youtu.be/yrXvv1H9VRE?si=ssx1DN5Tn72dbNjU

질문자님의 등급에 맞추어서 기존 코드 추가하기

샤인 포도는 보통 2kg,4kg 중량으로 출하됩니다
2kg 3송이 기준으로 해서 4송이 2kg 이 가장 당도올리기가좋고 품질도 좋습니다
당도가 가장 중요 합니다
당도올리는 기술이 좋은분은 5송이4kg 으로 만들어 판매합니다
저희는 2kg으로 기준해서 출하합니다

*/

#include <HX711.h> // Rob Tillaart's HX711 library  https://github.com/RobTillaart/HX711
#include <Adafruit_NeoPixel.h>

const int DOUT_SCK_PIN[8][2] = {
    {23, 25}, // DOUT_PIN[0], SCK_PIN[0]
    {27, 29}, // DOUT_PIN[1], SCK_PIN[1]
    {31, 33}, // DOUT_PIN[2], SCK_PIN[2]
    {35, 37}, // DOUT_PIN[3], SCK_PIN[3]
    {39, 41}, // DOUT_PIN[4], SCK_PIN[4]
    {7, 6},   // DOUT_PIN[5], SCK_PIN[5]  // 핀 바궈보기 , 43 핀부터 쓰지 말것 충돌남
    {5, 4},   // DOUT_PIN[6], SCK_PIN[6]
    {3, 2}    // DOUT_PIN[7], SCK_PIN[7]
};

const float CALIBRATION_FACTOR = 0.420f; // calibration factor
const int TARE_COUNT = 5;                // number of tare readings
const int UNITS_COUNT = 1;               // number of get_units readings
const int DECIMAL_PLACES = 1;
int SAVE_WEIGHT[] = {0, 0, 0, 0, 0, 0, 0, 0}; // 저장된 무게값
int SAVE_GRADE[] = {0, 0, 0, 0, 0, 0, 0, 0};  // 저장된 등급값

const int NEO_PIXEL_PIN = 12;
const int NUM_PIXELS = 8;

// 배열 객체 선언
HX711 scale[8];

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);

  pixels.begin();
  pixels.clear();

  // 라이브러리 안에서 핀모드 설정이 안 먹혀 직접 선언함 , 다시한번 체크할것
  for (int i = 0; i < 8; i++)
  {
    Serial.println("array start");
    pinMode(DOUT_SCK_PIN[i][0], INPUT);
    digitalWrite(DOUT_SCK_PIN[i][0], LOW);
    pinMode(DOUT_SCK_PIN[i][1], OUTPUT);
    Serial.println("array end");
  }

  // 각 저울마다 초기화하고 영점 조정
  for (int i = 0; i < 8; i++)
  {
    Serial.println("begin start");
    scale[i].begin(DOUT_SCK_PIN[i][0], DOUT_SCK_PIN[i][1], false);
    scale[i].set_scale(CALIBRATION_FACTOR); // Corrected: Moved inside the loop
    scale[i].tare(TARE_COUNT);              // Corrected: Moved inside the loop
    Serial.println("begin end");
  }
}

// =========   loop 함수   =========
void loop()
{
  saveWeightToArray();
  checkRanges();
  printScaleValues();
  showPixel();
  delay(2000); // 추후엔 주석처리하면 됩니다.
}

void printScaleValues()
{
  for (int i = 0; i < 8; i++)
  {
    Serial.print("Channel ");
    Serial.print(i + 1);
    Serial.print("\t");
    Serial.print("raw value: ");
    Serial.print(scale[i].read_average(UNITS_COUNT));
    Serial.print("\t");
    Serial.print("units: ");
    Serial.println(scale[i].get_units(UNITS_COUNT), DECIMAL_PLACES);
  }

  Serial.println("");
  Serial.println("=============================================");
  Serial.println("");
}

void saveWeightToArray()
{
  for (int i = 0; i < 8; i++)
  {
    SAVE_WEIGHT[i] = scale[i].get_units(UNITS_COUNT);
  }
}

void checkRanges() // 저장된 무게값을 범위 구간과 비교하는 함수
{

  for (int i = 0; i < 8; i++)
  {
    if (SAVE_WEIGHT[i] >= 670 && SAVE_WEIGHT[i] < 710)
    {
      Serial.println("1구간입니다.");
      SAVE_GRADE[i] = 1;
    }
    else if (SAVE_WEIGHT[i] >= 490 && SAVE_WEIGHT[i] < 530)
    {
      Serial.println("2구간입니다.");
      SAVE_GRADE[i] = 2;
    }
    else if (SAVE_WEIGHT[i] >= 380 && SAVE_WEIGHT[i] < 430)
    {
      Serial.println("3구간입니다.");
      SAVE_GRADE[i] = 3;
    }
    else if (SAVE_WEIGHT[i] >= 2020 && SAVE_WEIGHT[i] < 2150)
    {
      Serial.println("4구간입니다.");
      SAVE_GRADE[i] = 4;
    }
    else
    {
      // 기타 코드 삽입 할것
      SAVE_GRADE[i] = 0;
    }
  }
  // 등급 출력
  for (int i = 0; i < 8; i++)
  {
    Serial.print("Channel ");
    Serial.print(i + 1);
    Serial.print("\t");
    Serial.print("Grade: ");
    Serial.println(SAVE_GRADE[i]);
  }
}

void collectGrade()
{
}

void showPixel()
{

  // SAVE_GRADE[]의 배열 값에 따라서 픽셀 색상을 1등급 빨강 , 2등급 노랑 , 3등급 파랑 , 4등급 임의의 색으로 표현
  // 0등급은 표현하지 않음
  for (int i = 0; i < 8; i++)
  {
    if (SAVE_GRADE[i] == 1)
    {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // RED (빨)
    }
    else if (SAVE_GRADE[i] == 2)
    {
      pixels.setPixelColor(i, pixels.Color(255, 255, 0)); // Yellow(노)
    }
    else if (SAVE_GRADE[i] == 3)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // Blue (파)
    }
    else if (SAVE_GRADE[i] == 4)
    {
      pixels.setPixelColor(i, pixels.Color(0, 100, 48)); // 임의의 색
    }
    else
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // 0등급은 표현하지 않음
    }
  }
  pixels.show();
}

// pixels.setPixelColor(1, pixels.Color(255, 165, 0)); // Orange(주)
// pixels.setPixelColor(3, pixels.Color(0, 255, 0));   // Green(초)
// pixels.setPixelColor(5, pixels.Color(75, 0, 130));  // Indigo (남)
// pixels.setPixelColor(6, pixels.Color(148, 0, 211)); // Violet (보
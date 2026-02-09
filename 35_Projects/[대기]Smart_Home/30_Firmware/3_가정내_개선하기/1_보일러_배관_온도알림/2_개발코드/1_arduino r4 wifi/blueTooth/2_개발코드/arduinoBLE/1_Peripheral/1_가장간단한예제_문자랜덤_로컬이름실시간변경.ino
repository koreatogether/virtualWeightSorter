/*
이 코드는 ArduinoBLE 라이브러리를 활용해 BLE 주변장치(Peripheral) 역할을 수행하는 예제입니다.
기기 이름(광고 이름)을 주기적으로 변경하며 ("hi", "hello", "good", "bad", "poll" 단어 배열 순환) BLE 광고를 갱신합니다.
또한 시리얼 모니터에 현재 광고되고 있는 단어를 출력합니다.
*/

#include <ArduinoBLE.h>

// Array of words to advertise
const char *words[] = {"hi", "hello", "good", "bad", "poll"};
const int numWords = sizeof(words) / sizeof(words[0]);
int currentIndex = 0;

// Update interval in milliseconds (1 second)
unsigned long lastUpdate = 0;
const unsigned long interval = 1000;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  // Initialize BLE
  if (!BLE.begin())
  {
    Serial.println("Starting BLE failed!");
    while (1)
      ;
  }

  // Set the initial advertisement local name with the first word
  BLE.setLocalName(words[currentIndex]);
  BLE.advertise();

  Serial.println("BLE advertisement started");
  lastUpdate = millis();
}

void loop()
{
  // Check if there is a central connecting (optional for this example)
  BLEDevice central = BLE.central();

  // Check if it's time to update the advertised word
  if (millis() - lastUpdate >= interval)
  {
    lastUpdate += interval;
    // Move to the next word in the array cyclically
    currentIndex = (currentIndex + 1) % numWords;

    // To update advertisement, stop and restart advertising with the new word
    BLE.stopAdvertise();
    BLE.setLocalName(words[currentIndex]);
    BLE.advertise();

    Serial.print("Advertising: ");
    Serial.println(words[currentIndex]);
  }
}
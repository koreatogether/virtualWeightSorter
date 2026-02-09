Here is an example of how you could extract a `Config` class from the original code:
```cpp
// Config.h
#ifndef Config_h
#define Config_h

class Config {
public:
  static const int SERIAL_COM_BAUDRATE = 9600;
  static const int DELAY_TIME = 300;
  static const float TILT_WARNING_THRESHOLD = 60.0f;
  static const int BUZZER_PIN = 8;
  static const int LED_PIN_COUNT = 6;
  static const int LED_PINS[];

  static void init();
};

#endif // Config_h
```

```cpp
// Config.cpp
#include "Config.h"

const int Config::LED_PINS[] = {2, 3, 4, 5, 6, 7};

void Config::init() {
  // Initialize any config values that need to be set at runtime
  // For example, you could read config values from EEPROM or a file here
}
```

In this example, the `Config` class encapsulates various configuration values that were previously scattered throughout the code. This makes it easier to manage and modify these values in a single place.

You can then use the `Config` class in your main code like this:
```cpp
void setup() {
  Config::init();
  Serial.begin(Config::SERIAL_COM_BAUDRATE);
  //...
}

void loop() {
  //...
  delay(Config::DELAY_TIME);
  //...
  if (angle > Config::TILT_WARNING_THRESHOLD) {
    //...
  }
  //...
  tone(Config::BUZZER_PIN, 1000);
  //...
  for (int i = 0; i < Config::LED_PIN_COUNT; i++) {
    digitalWrite(Config::LED_PINS[i], HIGH);
  }
  //...
}
```
By using the `Config` class, you can easily modify configuration values without having to search through the entire codebase.

Note that in this example, I've made the config values `static` members of the `Config` class, which means they can be accessed without creating an instance of the class. This is just one way to implement a `Config` class, and you may need to modify it to fit your specific requirements.
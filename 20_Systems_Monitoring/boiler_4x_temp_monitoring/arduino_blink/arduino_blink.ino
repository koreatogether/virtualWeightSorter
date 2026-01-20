// Basic Blink example for Arduino UNO R4 WiFi
// LED uses LED_BUILTIN

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }
    Serial.println("Blink sketch started");
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}

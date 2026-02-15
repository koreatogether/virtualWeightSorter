#include "dht22_new_validation.h"

bool isDht22ReadingValid(float humidity, float temperature)
{
    const bool humidity_ok = humidity >= DHT22_MIN_HUMIDITY && humidity <= DHT22_MAX_HUMIDITY;
    const bool temperature_ok = temperature >= DHT22_MIN_TEMPERATURE_C && temperature <= DHT22_MAX_TEMPERATURE_C;
    return humidity_ok && temperature_ok;
}

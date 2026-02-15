#include "Utils.h"
#ifdef UNIT_TEST
#include <cstdio>
#else
#include <stdio.h>
#endif

// log_ 함수들은 항상 컴파일되도록 #ifndef TEST 블록 밖으로 이동
void log_error(const char *message)
{
    Serial.print(F("[ERROR] "));
    Serial.println(message);
}

void log_error(const __FlashStringHelper *message)
{
    Serial.print(F("[ERROR] "));
    Serial.println(message);
}

void log_info(const char *message)
{
    Serial.print(F("[INFO] "));
    Serial.println(message);
}

void log_info(const __FlashStringHelper *message)
{
    Serial.print(F("[INFO] "));
    Serial.println(message);
}

void log_debug(const char *message)
{
    Serial.print(F("[DEBUG] "));
    Serial.println(message);
}

void log_debug(const __FlashStringHelper *message)
{
    Serial.print(F("[DEBUG] "));
    Serial.println(message);
}

void addrToString(DeviceAddress deviceAddress, char *addrStr)
{
    int idx = 0;
    const int maxLen = 24; // "XX:XX:XX:XX:XX:XX:XX:XX\0" = 24 characters

    for (uint8_t j = 0; j < 8; j++)
    {
        // 안전한 snprintf 사용 - 버퍼 오버플로우 방지
        int written = snprintf(&addrStr[idx], maxLen - idx, "%02X", deviceAddress[j]);
        if (written < 0 || idx + written >= maxLen)
        {
            log_error(F("Address string buffer overflow"));
            addrStr[idx] = '\0';
            return;
        }
        idx += written;

        if ((j % 2 == 1) && (j != 7))
        {
            if (idx + 1 >= maxLen)
            {
                log_error(F("Address string colon overflow"));
                addrStr[idx] = '\0';
                return;
            }
            addrStr[idx++] = ':';
        }
    }
    addrStr[idx] = '\0';
}

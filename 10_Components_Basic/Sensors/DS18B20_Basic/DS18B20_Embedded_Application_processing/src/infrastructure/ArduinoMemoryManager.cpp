#include "ArduinoMemoryManager.h"

size_t ArduinoMemoryManager::getFreeHeap() const
{
    return getFreeMemory();
}

size_t ArduinoMemoryManager::getTotalHeap() const
{
    // Arduino Uno R4 WiFi has 32KB SRAM
    return 32768;
}

float ArduinoMemoryManager::getMemoryUsagePercent() const
{
    size_t free = getFreeHeap();
    size_t total = getTotalHeap();

    if (total == 0)
        return 100.0f;

    return ((float)(total - free) / total) * 100.0f;
}

bool ArduinoMemoryManager::isMemoryLow() const
{
    return getFreeHeap() < MEMORY_WARNING_THRESHOLD;
}

void ArduinoMemoryManager::optimizeMemory()
{
    // Arduino에서는 명시적 가비지 컬렉션이 없으므로
    // 메모리 정리 힌트만 제공
    // 실제로는 애플리케이션 레벨에서 불필요한 객체 정리 등을 수행
}

size_t ArduinoMemoryManager::getFreeMemory() const
{
    // Arduino에서 사용 가능한 메모리 계산
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

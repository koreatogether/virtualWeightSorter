#pragma once
#include <Arduino.h>
#include "../domain/IMemoryManager.h"

/**
 * @brief Arduino 메모리 관리자 구현체
 *
 * Arduino 플랫폼에서 메모리 관리를 수행하는 구체적 구현
 */
class ArduinoMemoryManager : public IMemoryManager
{
private:
    static const size_t MEMORY_WARNING_THRESHOLD = 1024; // 1KB

public:
    ArduinoMemoryManager() = default;
    ~ArduinoMemoryManager() = default;

    size_t getFreeHeap() const override;
    size_t getTotalHeap() const override;
    float getMemoryUsagePercent() const override;
    bool isMemoryLow() const override;
    void optimizeMemory() override;

private:
    size_t getFreeMemory() const;
};

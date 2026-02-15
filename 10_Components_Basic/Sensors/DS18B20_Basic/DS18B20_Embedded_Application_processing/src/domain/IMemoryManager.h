#pragma once
#include <cstddef>

/**
 * @brief 메모리 관리 인터페이스
 *
 * 메모리 사용량 모니터링 및 최적화를 위한 추상 인터페이스
 */
class IMemoryManager
{
public:
    virtual ~IMemoryManager() = default;

    virtual size_t getFreeHeap() const = 0;
    virtual size_t getTotalHeap() const = 0;
    virtual float getMemoryUsagePercent() const = 0;
    virtual bool isMemoryLow() const = 0;
    virtual void optimizeMemory() = 0;
};

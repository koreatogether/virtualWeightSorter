#pragma once
#include <Arduino.h>
#include "../domain/IMemoryManager.h"
#include "../domain/ILogger.h"

/**
 * @brief 시스템 모니터링 서비스
 *
 * 메모리, 성능 등 시스템 상태를 모니터링하는 어플리케이션 서비스
 */
class SystemMonitorService
{
private:
    IMemoryManager *memoryManager;
    ILogger *logger;
    unsigned long lastCheckTime;
    static const unsigned long CHECK_INTERVAL = 5000; // 5초

public:
    SystemMonitorService(IMemoryManager *memMgr, ILogger *log);
    ~SystemMonitorService() = default;

    void initialize();
    void update();
    void printSystemStatus() const;
    bool isSystemHealthy() const;
    void handleMemoryWarning();
};

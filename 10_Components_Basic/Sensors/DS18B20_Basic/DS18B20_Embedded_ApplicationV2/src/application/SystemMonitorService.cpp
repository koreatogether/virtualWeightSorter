#include <Arduino.h>
#include "SystemMonitorService.h"

SystemMonitorService::SystemMonitorService(IMemoryManager *memMgr, ILogger *log)
    : memoryManager(memMgr), logger(log), lastCheckTime(0)
{
}

void SystemMonitorService::initialize()
{
    if (logger)
    {
        logger->info("System Monitor Service initialized");
    }
    lastCheckTime = millis();
}

void SystemMonitorService::update()
{
    unsigned long currentTime = millis();

    if (currentTime - lastCheckTime >= CHECK_INTERVAL)
    {
        if (memoryManager && memoryManager->isMemoryLow())
        {
            handleMemoryWarning();
        }

        lastCheckTime = currentTime;
    }
}

void SystemMonitorService::printSystemStatus() const
{
    if (!memoryManager)
        return;

    Serial.println("=== System Status ===");
    Serial.print("Free Heap: ");
    Serial.print(memoryManager->getFreeHeap());
    Serial.println(" bytes");
    Serial.print("Memory Usage: ");
    Serial.print(memoryManager->getMemoryUsagePercent());
    Serial.println("%");
    Serial.print("System Health: ");
    Serial.println(isSystemHealthy() ? "OK" : "WARNING");
}

bool SystemMonitorService::isSystemHealthy() const
{
    if (!memoryManager)
        return false;
    return !memoryManager->isMemoryLow();
}

void SystemMonitorService::handleMemoryWarning()
{
    if (logger)
    {
        logger->warning("Low memory detected - optimizing");
    }

    if (memoryManager)
    {
        memoryManager->optimizeMemory();
    }
}

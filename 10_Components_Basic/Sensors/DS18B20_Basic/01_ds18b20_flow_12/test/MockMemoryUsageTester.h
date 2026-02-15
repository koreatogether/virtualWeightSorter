#ifndef MOCK_MEMORY_USAGE_TESTER_H
#define MOCK_MEMORY_USAGE_TESTER_H

#include "FakeArduino.h"

// Mock MemoryUsageTester 클래스
class MockMemoryUsageTester
{
private:
    bool logMemoryUsageCalled;
    bool printMemoryStatusCalled;
    bool printMemoryStatusCSVCalled;
    String lastContext;
    String lastPhase;

    // 메모리 시뮬레이션용 변수들
    int mockUsedMemory;
    int mockTotalMemory;

public:
    MockMemoryUsageTester() : logMemoryUsageCalled(false),
                              printMemoryStatusCalled(false),
                              printMemoryStatusCSVCalled(false),
                              mockUsedMemory(1000), // 기본값: 1KB 사용
                              mockTotalMemory(8192)
    {
    } // 기본값: 8KB 총 메모리

    // Mock 메소드들
    void logMemoryUsage()
    {
        logMemoryUsageCalled = true;
    }

    void printMemoryStatus(const String &context)
    {
        printMemoryStatusCalled = true;
        lastContext = context;
    }

    void printMemoryStatusCSV(const String &context, const String &phase)
    {
        printMemoryStatusCSVCalled = true;
        lastContext = context;
        lastPhase = phase;
    }

    // 테스트용 검증 메소드들
    bool wasLogMemoryUsageCalled() const { return logMemoryUsageCalled; }
    bool wasPrintMemoryStatusCalled() const { return printMemoryStatusCalled; }
    bool wasPrintMemoryStatusCSVCalled() const { return printMemoryStatusCSVCalled; }
    String getLastContext() const { return lastContext; }
    String getLastPhase() const { return lastPhase; }

    // 전체 회귀 테스트용 추가 메서드들
    int getUsedMemory() const
    {
        return mockUsedMemory;
    }

    int getTotalMemory() const
    {
        return mockTotalMemory;
    }

    void setUsedMemory(int used)
    {
        mockUsedMemory = used;
    }

    void setTotalMemory(int total)
    {
        mockTotalMemory = total;
    }

    // 리셋 메소드
    void reset()
    {
        logMemoryUsageCalled = false;
        printMemoryStatusCalled = false;
        printMemoryStatusCSVCalled = false;
        lastContext = "";
        lastPhase = "";
        mockUsedMemory = 1000;  // 기본값으로 리셋
        mockTotalMemory = 8192; // 기본값으로 리셋
    }
};

#endif // MOCK_MEMORY_USAGE_TESTER_H

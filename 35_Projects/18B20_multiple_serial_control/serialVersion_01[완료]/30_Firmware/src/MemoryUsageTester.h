// 비활성화된 파일: MemoryUsageTester.h
#ifndef MEMORY_USAGE_TESTER_H
#define MEMORY_USAGE_TESTER_H

// 메모리 분석 기능 비활성화됨
class MemoryUsageTester {
public:
    inline int freeMemory() { return 0; }
    inline void runBasicTest() {}
    inline void runStressTest(int = 0) {}
    inline void printMemoryStatus(const char*) {}
    inline void testDataStructures() {}
    inline void printMemoryStatusCSV(const char*, const char*) {}
    inline void logMemoryUsage() {}
};

#endif // MEMORY_USAGE_TESTER_H

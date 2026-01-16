/**
 * Random Number Generator - C++ Implementation
 * 
 * 조건:
 * - 3개의 숫자(0, 1, 2)를 랜덤으로 추출
 * - 이전 숫자와 동일하지 않도록 추출
 * - 반복문(for, while) 사용 불가
 * - 논리연산자(||, &&) 사용 불가
 */

#include <iostream>
#include <random>
#include <chrono>
#include <map>
#include <iomanip>
#include <vector>

class RandomNumberGenerator {
private:
    int previousNumber;
    bool hasPrevious;
    
    // 통계 정보
    struct Stats {
        unsigned long totalCount = 0;
        std::chrono::steady_clock::time_point startTime;
        std::map<int, int> numberFrequency;
        std::map<std::string, int> transitionMatrix;
    } stats;
    
    // 룩업 테이블: [이전숫자][후보숫자] = 결과숫자
    int lookupTable[3][3] = {
        {1, 1, 2},  // 이전이 0일 때: 0->1, 1->1, 2->2
        {0, 0, 2},  // 이전이 1일 때: 0->0, 1->0, 2->2
        {0, 1, 0}   // 이전이 2일 때: 0->0, 1->1, 2->0
    };
    
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    
    void updateStats(int number) {
        stats.totalCount++;
        stats.numberFrequency[number]++;
        
        // 전이 행렬 업데이트 (이전 -> 현재)
        if (hasPrevious) {
            std::string transition = std::to_string(previousNumber) + "->" + std::to_string(number);
            stats.transitionMatrix[transition]++;
        }
    }

public:
    RandomNumberGenerator() : 
        previousNumber(-1), 
        hasPrevious(false),
        gen(rd()),
        dis(0, 2) {
        
        stats.startTime = std::chrono::steady_clock::now();
        stats.numberFrequency = {{0, 0}, {1, 0}, {2, 0}};
    }
    
    /**
     * 조건을 만족하는 랜덤 숫자 생성
     * 반복문과 논리연산자 사용 불가
     */
    int generateNumber() {
        int candidate = dis(gen);  // 0, 1, 2 중 랜덤 선택
        
        int result;
        // 이전 숫자가 없으면 바로 반환 (삼항 연산자 사용)
        result = hasPrevious ? lookupTable[previousNumber][candidate] : candidate;
        
        updateStats(result);
        previousNumber = result;
        hasPrevious = true;
        
        return result;
    }
    
    /**
     * 성능 및 분포 통계 반환
     */
    struct Statistics {
        unsigned long totalGenerated;
        double elapsedTimeSeconds;
        double generationRate;
        double averageTimePerGeneration;
        std::map<int, std::pair<int, double>> frequencyAnalysis;
        std::map<std::string, int> transitionAnalysis;
        int currentNumber;
    };
    
    Statistics getStatistics() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - stats.startTime);
        double elapsedTimeSeconds = elapsed.count() / 1000000.0;
        
        Statistics result;
        result.totalGenerated = stats.totalCount;
        result.elapsedTimeSeconds = elapsedTimeSeconds;
        result.generationRate = elapsedTimeSeconds > 0 ? stats.totalCount / elapsedTimeSeconds : 0;
        result.averageTimePerGeneration = stats.totalCount > 0 ? elapsedTimeSeconds / stats.totalCount : 0;
        result.currentNumber = hasPrevious ? previousNumber : -1;
        
        // 빈도 분석
        for (int i = 0; i < 3; i++) {
            int count = stats.numberFrequency[i];
            double percentage = stats.totalCount > 0 ? (count * 100.0) / stats.totalCount : 0;
            result.frequencyAnalysis[i] = std::make_pair(count, percentage);
        }
        
        result.transitionAnalysis = stats.transitionMatrix;
        
        return result;
    }
    
    /**
     * 생성기 상태 초기화
     */
    void reset() {
        previousNumber = -1;
        hasPrevious = false;
        stats.totalCount = 0;
        stats.startTime = std::chrono::steady_clock::now();
        stats.numberFrequency = {{0, 0}, {1, 0}, {2, 0}};
        stats.transitionMatrix.clear();
    }
};

/**
 * 생성기 데모 함수 (재귀 사용으로 반복문 대체)
 */
void demonstrateGenerator(RandomNumberGenerator& generator, int count, std::vector<int>& generatedNumbers) {
    if (count <= 0) {
        return;
    }
    
    int number = generator.generateNumber();
    generatedNumbers.push_back(number);
    
    std::string prevDisplay = generatedNumbers.size() > 1 ? 
        std::to_string(generatedNumbers[generatedNumbers.size() - 2]) : "none";
    std::cout << "Generated: " << number << " (Previous: " << prevDisplay << ")" << std::endl;
    
    // 재귀 호출로 반복문 대체
    demonstrateGenerator(generator, count - 1, generatedNumbers);
}

/**
 * 성능 벤치마크 함수 (재귀 사용)
 */
void runBenchmarkRecursive(RandomNumberGenerator& generator, int remaining) {
    if (remaining <= 0) {
        return;
    }
    
    generator.generateNumber();
    runBenchmarkRecursive(generator, remaining - 1);
}

/**
 * 성능 벤치마크 실행
 */
struct BenchmarkResult {
    int iterations;
    double totalTime;
    double timePerIteration;
    double iterationsPerSecond;
};

BenchmarkResult runBenchmark(int iterations = 10000) {
    RandomNumberGenerator generator;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    runBenchmarkRecursive(generator, iterations);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double totalTime = elapsed.count() / 1000000.0;
    
    BenchmarkResult result;
    result.iterations = iterations;
    result.totalTime = totalTime;
    result.timePerIteration = totalTime / iterations;
    result.iterationsPerSecond = iterations / totalTime;
    
    return result;
}

int main() {
    std::cout << "Random Number Generator - C++ Implementation" << std::endl;
    std::cout << "Numbers: 0, 1, 2" << std::endl;
    std::cout << "Constraint: No consecutive identical numbers" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    RandomNumberGenerator generator;
    std::vector<int> generatedNumbers;
    
    // 데모 실행 (20개 생성)
    demonstrateGenerator(generator, 20, generatedNumbers);
    
    // 통계 출력
    auto stats = generator.getStatistics();
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "PERFORMANCE STATISTICS" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    std::cout << "Total generated: " << stats.totalGenerated << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Elapsed time: " << stats.elapsedTimeSeconds << " seconds" << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "Generation rate: " << stats.generationRate << " numbers/second" << std::endl;
    std::cout << std::setprecision(3);
    std::cout << "Average time per generation: " << (stats.averageTimePerGeneration * 1000) << " milliseconds" << std::endl;
    
    std::cout << "\nFREQUENCY ANALYSIS:" << std::endl;
    for (const auto& pair : stats.frequencyAnalysis) {
        std::cout << "  Number " << pair.first << ": " << pair.second.first 
                  << " times (" << std::setprecision(1) << pair.second.second << "%)" << std::endl;
    }
    
    std::cout << "\nTRANSITION ANALYSIS:" << std::endl;
    for (const auto& pair : stats.transitionAnalysis) {
        std::cout << "  " << pair.first << ": " << pair.second << " times" << std::endl;
    }
    
    std::cout << "\nGenerated sequence: [";
    for (size_t i = 0; i < generatedNumbers.size(); ++i) {
        std::cout << generatedNumbers[i];
        if (i < generatedNumbers.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // 성능 벤치마크
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "PERFORMANCE BENCHMARK" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    auto benchmarkResults = runBenchmark(10000);
    std::cout << "Iterations: " << benchmarkResults.iterations << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Total time: " << benchmarkResults.totalTime << " seconds" << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "Time per iteration: " << (benchmarkResults.timePerIteration * 1000000) << " microseconds" << std::endl;
    std::cout << "Iterations per second: " << static_cast<int>(benchmarkResults.iterationsPerSecond) << std::endl;
    
    return 0;
}
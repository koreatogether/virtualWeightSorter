#ifndef MOCK_DATA_PROCESSOR_H
#define MOCK_DATA_PROCESSOR_H

#include "../DataProcessor.h"
#include <vector>
#include <string>
#include <map>

class MockDataProcessor : public DataProcessor
{
public:
    // 생성자는 실제 클래스와 동일하게 정의합니다. (2개의 매개변수 필요)
    MockDataProcessor(DS18B20_Sensor *sensor, Communicator *comm) : DataProcessor(sensor, comm)
    {
        reset(); // 테스트마다 상태를 초기화합니다.
    }

    // 테스트를 위한 상태 초기화 함수
    void reset()
    {
        _printSensorTableCalled = false;
        _printInvalidSensorTableCalled = false;
        _printAvailableIdsCalled = false;
        _hasInvalidSensorsResult = false;
        _setThresholdCalled = false;
        _getThresholdHighCalled = false;
        _getThresholdLowCalled = false;
        _clearAllThresholdsCalled = false;
        _lastSensorId = 0;
        _lastHighThreshold = 0;
        _lastLowThreshold = 0;
        // 센서별 임계값 저장을 위한 맵 초기화
        _sensorThresholds.clear();
    }

    // Mock 함수들
    void printSensorTable() override
    {
        _printSensorTableCalled = true;
    }

    void printInvalidSensorTable() override
    {
        _printInvalidSensorTableCalled = true;
    }

    void printAvailableIds() override
    {
        _printAvailableIdsCalled = true;
    }

    bool hasInvalidSensors() override
    {
        return _hasInvalidSensorsResult;
    }

    // 임계값 관련 Mock 함수들
    void setThreshold(int sensorId, int8_t high, int8_t low) override
    {
        _setThresholdCalled = true;
        _lastSensorId = sensorId;
        _lastHighThreshold = high;
        _lastLowThreshold = low;
        // 센서별 임계값 저장
        _sensorThresholds[sensorId] = {high, low};
    }

    int8_t getTh(int sensorId) override
    {
        _getThresholdHighCalled = true;
        auto it = _sensorThresholds.find(sensorId);
        if (it != _sensorThresholds.end())
        {
            return it->second.first; // high threshold
        }
        return 125; // 기본값
    }

    int8_t getTl(int sensorId) override
    {
        _getThresholdLowCalled = true;
        auto it = _sensorThresholds.find(sensorId);
        if (it != _sensorThresholds.end())
        {
            return it->second.second; // low threshold
        }
        return -55; // 기본값
    }

    void clearAllThresholds() override
    {
        _clearAllThresholdsCalled = true;
        _sensorThresholds.clear();
    }

    // 테스트에서 Mock의 상태를 설정하거나 확인할 수 있는 헬퍼 함수들
    void setHasInvalidSensorsResult(bool result)
    {
        _hasInvalidSensorsResult = result;
    }

    // Mock 함수 호출 확인 함수들
    bool wasSetThresholdCalled() const { return _setThresholdCalled; }
    bool wasGetThresholdHighCalled() const { return _getThresholdHighCalled; }
    bool wasGetThresholdLowCalled() const { return _getThresholdLowCalled; }
    bool wasClearAllThresholdsCalled() const { return _clearAllThresholdsCalled; }

    int getLastSensorId() const { return _lastSensorId; }
    int8_t getLastHighThreshold() const { return _lastHighThreshold; }
    int8_t getLastLowThreshold() const { return _lastLowThreshold; }

    // Mock 함수 호출 여부 확인을 위한 플래그
    bool _printSensorTableCalled;
    bool _printInvalidSensorTableCalled;
    bool _printAvailableIdsCalled;
    bool _setThresholdCalled;
    bool _getThresholdHighCalled;
    bool _getThresholdLowCalled;
    bool _clearAllThresholdsCalled;

private:
    bool _hasInvalidSensorsResult;
    int _lastSensorId;
    int8_t _lastHighThreshold;
    int8_t _lastLowThreshold;
    // 센서별 임계값 저장을 위한 맵 (센서ID -> {high, low})
    std::map<int, std::pair<int8_t, int8_t>> _sensorThresholds;
};

#endif // MOCK_DATA_PROCESSOR_H
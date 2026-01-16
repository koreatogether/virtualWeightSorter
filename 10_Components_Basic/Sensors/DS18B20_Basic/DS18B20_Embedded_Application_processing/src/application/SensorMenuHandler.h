#pragma once
#include <Arduino.h>
#include <vector>

// 센서 관련 메뉴 처리를 위한 클래스
class SensorMenuHandler
{
public:
    SensorMenuHandler();
    
    // 센서 인덱스 파싱 및 검증
    static std::vector<int> parseSensorIndices(const String& input);
    static bool validateSensorIndices(const std::vector<int>& indices);
    
    // 센서 선택 처리
    bool processSensorSelection(const std::vector<int>& indices, 
                               std::vector<int>& selectedIndices,
                               int& currentSensorIdx);
    
    // 센서 ID 변경 처리
    bool processSensorIdChange(int sensorIdx, int newId);
    
    // 메뉴 출력
    void printSensorIdMenu();
    void printSensorSelectionPrompt();
    void printIdInputPrompt(int sensorIdx);
    
private:
    // 헬퍼 메서드
    bool isIdDuplicated(int newId, int exceptIdx);
    void printSensorInfo(int idx);
};
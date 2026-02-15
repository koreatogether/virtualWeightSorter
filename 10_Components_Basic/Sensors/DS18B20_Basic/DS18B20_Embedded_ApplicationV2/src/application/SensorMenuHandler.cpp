#include <Arduino.h>
#include <vector>
#include "SensorMenuHandler.h"
#include "SensorController.h"

extern SensorController sensorController;

SensorMenuHandler::SensorMenuHandler()
{
    // 생성자
}

std::vector<int> SensorMenuHandler::parseSensorIndices(const String &input)
{
    std::vector<int> indices;
    bool used[9] = {false}; // 1~8만 사용
    int len = input.length();

    for (int i = 0; i < len; ++i)
    {
        char c = input.charAt(i);
        if (c >= '1' && c <= '8')
        {
            int idx = c - '0';
            if (!used[idx])
            {
                indices.push_back(idx);
                used[idx] = true;
            }
        }
    }
    return indices;
}

bool SensorMenuHandler::validateSensorIndices(const std::vector<int> &indices)
{
    if (indices.empty())
    {
        Serial.println("오류: 유효한 센서 번호가 없습니다.");
        return false;
    }

    if (indices.size() > 8)
    {
        Serial.println("오류: 너무 많은 센서가 선택되었습니다.");
        return false;
    }

    return true;
}

bool SensorMenuHandler::processSensorSelection(const std::vector<int> &indices,
                                               std::vector<int> &selectedIndices,
                                               int &currentSensorIdx)
{
    if (!validateSensorIndices(indices))
    {
        return false;
    }

    selectedIndices = indices;
    currentSensorIdx = 0;

    Serial.print("선택된 센서: ");
    for (size_t i = 0; i < indices.size(); ++i)
    {
        Serial.print(indices[i]);
        if (i < indices.size() - 1)
            Serial.print(", ");
    }
    Serial.println();

    return true;
}

bool SensorMenuHandler::processSensorIdChange(int sensorIdx, int newId)
{
    if (newId < 1 || newId > 8)
    {
        Serial.println("오류: ID는 1~8 사이여야 합니다.");
        return false;
    }

    if (isIdDuplicated(newId, sensorIdx - 1))
    {
        Serial.println("오류: 이미 사용 중인 ID입니다.");
        return false;
    }

    sensorController.setSensorLogicalId(sensorIdx - 1, newId);
    Serial.print("센서 ");
    Serial.print(sensorIdx);
    Serial.print("의 ID를 ");
    Serial.print(newId);
    Serial.println("로 변경했습니다.");

    return true;
}

void SensorMenuHandler::printSensorIdMenu()
{
    Serial.println();
    Serial.println("--- 센서 ID 조정 메뉴 ---");
    Serial.println("1. 개별 센서 ID 변경");
    Serial.println("2. 복수의 센서 ID 변경");
    Serial.println("3. 주소순 자동 ID 할당");
    Serial.println("4. 전체 ID 초기화");
    Serial.println("5. 이전 메뉴 이동");
    Serial.println("6. 상태창으로 돌아가기");
    Serial.print("메뉴 번호를 입력하세요: ");
}

void SensorMenuHandler::printSensorSelectionPrompt()
{
    Serial.println();
    Serial.println("변경할 센서 번호를 입력하세요 (예: 1,3,5 또는 135):");
    Serial.print("> ");
}

void SensorMenuHandler::printIdInputPrompt(int sensorIdx)
{
    Serial.print("센서 ");
    Serial.print(sensorIdx);
    Serial.print("의 새 ID (1-8)를 입력하세요: ");
}

bool SensorMenuHandler::isIdDuplicated(int newId, int exceptIdx)
{
    return sensorController.isIdDuplicated(newId, exceptIdx);
}

void SensorMenuHandler::printSensorInfo(int idx)
{
    // 센서 정보 출력 로직
    Serial.print("센서 ");
    Serial.print(idx);
    Serial.print(" (현재 ID: ");
    Serial.print(sensorController.getSensorLogicalId(idx - 1));
    Serial.println(")");
}
#pragma once
#include <cstdint>
#include <string>

struct SensorStatus
{
    int id = 0;
    uint64_t address = 0;
    float temperature = 0.0f;
    float upperThreshold = 0.0f;
    std::string upperState;
    float lowerThreshold = 0.0f;
    std::string lowerState;
    std::string status;
};

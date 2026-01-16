#pragma once
#include <cstdint>

class ITemperatureSensor
{
public:
    virtual ~ITemperatureSensor() = default;
    // 센서 초기화
    virtual void begin() = 0;
    // 단일 온도 읽기(기본)
    virtual float readTemperature() = 0;
    // 아래 함수들은 Mock 및 매니저 테스트에서 사용됨
    virtual int getSensorCount() const { return 0; }
    virtual bool getAddress(uint8_t /*index*/, uint64_t & /*address*/) { return false; }
    virtual float getTemperature(uint8_t /*index*/) { return 0.0f; }
};

#pragma once
#include "ITemperatureSensor.h"

/**
 * @brief 테스트용 온도 센서 인터페이스 구현체
 *
 * 실제 하드웨어 없이 온도 센서 기능을 시뮬레이션하는 구현체
 */
class TestTemperatureSensor : public ITemperatureSensor
{
private:
    float simulatedTemperature;
    bool connected;

public:
    TestTemperatureSensor();
    ~TestTemperatureSensor() = default;

    float readTemperature() override;
    bool isConnected() const override;
    void setSimulatedTemperature(float temp);
    void setConnectionStatus(bool status);
};

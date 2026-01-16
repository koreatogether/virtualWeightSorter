#ifndef DALLAS_TEMPERATURE_H
#define DALLAS_TEMPERATURE_H

#include <stdint.h>
#include "OneWire.h"     // OneWire 객체가 필요할 수 있으므로 포함
#include "FakeArduino.h" // DeviceAddress를 위해 추가

// DallasTemperature 클래스의 최소한의 정의
class DallasTemperature
{
public:
    DallasTemperature(OneWire *ow) {}
    void begin() {}
    int getDeviceCount() { return 0; }
    bool getAddress(DeviceAddress address, uint8_t index) { return false; }
    void requestTemperatures() {}
    float getTempC(DeviceAddress address) { return 0.0f; }

    // getUserData 및 setUserData 추가
    int getUserData(DeviceAddress address) { return 0; }     // Mock 구현
    void setUserData(DeviceAddress address, int userData) {} // Mock 구현

    // 알람 온도 관련 메서드 추가
    bool setHighAlarmTemp(DeviceAddress address, int8_t highTemp) { return true; }
    bool setLowAlarmTemp(DeviceAddress address, int8_t lowTemp) { return true; }
    int8_t getHighAlarmTemp(DeviceAddress address) { return 125; }
    int8_t getLowAlarmTemp(DeviceAddress address) { return -55; }
    void saveScratchPad(DeviceAddress address) {} // Mock 구현
};

#endif // DALLAS_TEMPERATURE_H
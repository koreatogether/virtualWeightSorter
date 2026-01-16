#ifndef MOCK_DS18B20_SENSOR_H
#define MOCK_DS18B20_SENSOR_H

#include "../DS18B20_Sensor.h"
#include "FakeArduino.h" // For DeviceAddress
#include <vector>
#include <map>
#include <string>
#include <array> // std::array를 위해 추가

// Mock 클래스는 실제 클래스의 인터페이스를 구현합니다.
class MockDS18B20_Sensor : public DS18B20_Sensor
{
public:
    // 생성자는 실제 클래스와 동일하게 정의합니다.
    MockDS18B20_Sensor(OneWire *oneWire = nullptr) : DS18B20_Sensor(oneWire)
    {
        reset(); // 테스트마다 상태를 초기화합니다。
    }

    // 테스트를 위한 상태 초기화 함수
    void reset()
    {
        _beginCalled = false;
        _deviceCount = 0;
        _addresses.clear();
        _userDatas.clear();
        _requestTemperaturesCalled = false;
        _tempCValue = 0.0f;
        _isIdUsedResult = false;
        _setUserDataCalled = false;
        _setUserDataAddr.clear();
        _setUserDataId = 0;
        _sensorTemperatures.clear();
        _deviceErrorSimulated = false;
        _retryCount = 0;
    }

    // Mock 함수들: 실제 로직 대신 테스트를 위한 동작을 정의합니다.
    void begin() override
    {
        _beginCalled = true;
    }

    int getDeviceCount() override
    {
        return _deviceCount;
    }

    bool getAddress(DeviceAddress address, uint8_t index) override
    {
        if (index < _addresses.size())
        {
            memcpy(address, _addresses[index].data(), 8); // std::array의 data() 메소드 사용
            return true;
        }
        return false;
    }

    void requestTemperatures() override
    {
        _requestTemperaturesCalled = true;
    }

    float getTempC(DeviceAddress address) override
    {
        // 주소로 센서 인덱스 찾기
        for (int i = 0; i < _addresses.size(); i++)
        {
            if (memcmp(address, _addresses[i].data(), 8) == 0)
            {
                // 개별 센서 온도가 설정되어 있으면 반환
                if (_sensorTemperatures.find(i) != _sensorTemperatures.end())
                {
                    return _sensorTemperatures[i];
                }
                break;
            }
        }
        // 개별 온도가 없으면 기본값 반환
        return _tempCValue;
    }

    float getTempCByIndex(uint8_t index) override
    {
        // 인덱스로 직접 온도 반환
        if (_sensorTemperatures.find(index) != _sensorTemperatures.end())
        {
            return _sensorTemperatures[index];
        }
        return _tempCValue;
    }

    int getUserData(DeviceAddress address) override
    {
        // 주어진 주소에 대한 사용자 데이터를 반환합니다.
        // 실제 구현에서는 EEPROM에서 읽어오지만, Mock에서는 미리 설정된 값을 반환합니다.
        std::string addrStr(reinterpret_cast<const char *>(address), 8);
        if (_userDatas.count(addrStr))
        {
            return _userDatas[addrStr];
        }
        return 0; // 기본값
    }

    void setUserData(DeviceAddress address, int userData) override
    {
        _setUserDataCalled = true;
        _setUserDataAddr.assign(reinterpret_cast<const char *>(address), 8);
        _setUserDataId = userData;
        // Mock에서는 실제 EEPROM에 쓰는 대신 내부 맵에 저장합니다.
        _userDatas[std::string(reinterpret_cast<const char *>(address), 8)] = userData;
    }

    int getUserDataByIndex(uint8_t index) override
    {
        if (index < _addresses.size())
        {
            return getUserData(_addresses[index].data()); // std::array의 data() 메소드 사용
        }
        return 0;
    }

    bool isIdUsed(int id) override
    {
        // Mock에서는 미리 설정된 결과를 반환하거나, 내부 _userDatas 맵을 기반으로 판단합니다.
        for (auto it = _userDatas.begin(); it != _userDatas.end(); ++it)
        {
            if (it->second == id)
            {
                return true;
            }
        }
        return false;
    }

    // 임계값 관련 기능은 DS18B20_Sensor 클래스에서 제거되었으므로
    // Mock에서도 제거합니다. 임계값은 DataProcessor에서 관리됩니다.

    // 테스트에서 Mock의 상태를 설정하거나 확인할 수 있는 헬퍼 함수들
    void setDeviceCount(int count)
    {
        _deviceCount = count;
    }

    void addDevice(const DeviceAddress address, int initialId = 0)
    {
        std::array<uint8_t, 8> newAddrArray;
        memcpy(newAddrArray.data(), address, 8);
        _addresses.push_back(newAddrArray);
        _userDatas[std::string(reinterpret_cast<const char *>(address), 8)] = initialId;
        _deviceCount = _addresses.size();
    }

    void setTempCValue(float temp)
    {
        _tempCValue = temp;
    }

    void setSensorTempC(int sensorId, float temp)
    {
        if (sensorId >= 0 && sensorId < _addresses.size())
        {
            _sensorTemperatures[sensorId] = temp;
        }
    }

    void setSensorTempCByIndex(int index, float temp)
    {
        if (index >= 0 && index < _addresses.size())
        {
            _sensorTemperatures[index] = temp;
        }
    }

    // 테스트용 추가 메소드들
    void setTempCByIndex(int index, float temp)
    {
        setSensorTempCByIndex(index, temp);
    }

    void setDeviceAddress(int index, const DeviceAddress addr)
    {
        if (index >= 0 && index < _addresses.size())
        {
            for (int i = 0; i < 8; i++)
            {
                _addresses[index][i] = addr[i];
            }
        }
    }

    // 전체 회귀 테스트용 추가 메서드들
    void addMockSensor(uint8_t addr0, uint8_t addr1, uint8_t addr2, uint8_t addr3,
                       uint8_t addr4, uint8_t addr5, uint8_t addr6, uint8_t addr7)
    {
        std::array<uint8_t, 8> newAddress = {addr0, addr1, addr2, addr3, addr4, addr5, addr6, addr7};
        _addresses.push_back(newAddress);
        _deviceCount++;

        // 기본 온도값 설정
        _sensorTemperatures[_deviceCount - 1] = 20.0;
    }

    void simulateDisconnection()
    {
        _deviceErrorSimulated = true;
        _retryCount = 0;
    }

    void simulateReconnection()
    {
        _deviceErrorSimulated = false;
        _retryCount = 0;
    }

    void simulateError()
    {
        _deviceErrorSimulated = true;
    }

    void clearError()
    {
        _deviceErrorSimulated = false;
        _retryCount = 0;
    }

    int getRetryCount() const
    {
        return _retryCount;
    }

    bool isErrorSimulated() const
    {
        return _deviceErrorSimulated;
    }

    void setIsIdUsedResult(bool result)
    {
        _isIdUsedResult = result;
    }

    void setUserDataByIndex(uint8_t index, int userData)
    {
        if (index < _addresses.size())
        {
            std::string addrStr(reinterpret_cast<const char *>(_addresses[index].data()), 8);
            _userDatas[addrStr] = userData;
        }
    }

    // Mock 함수 호출 여부 확인을 위한 플래그
    bool _beginCalled;
    bool _requestTemperaturesCalled;
    bool _setUserDataCalled;
    std::string _setUserDataAddr;
    int _setUserDataId;

private:
    int _deviceCount;
    std::vector<std::array<uint8_t, 8>> _addresses; // std::array로 변경
    std::map<std::string, int> _userDatas;          // 주소(string)를 키로 사용자 ID 저장
    float _tempCValue;
    bool _isIdUsedResult;
    std::map<int, float> _sensorTemperatures; // 센서별 온도 저장

    // 에러 시뮬레이션용 변수들
    bool _deviceErrorSimulated = false;
    int _retryCount = 0;
};

#endif // MOCK_DS18B20_SENSOR_H

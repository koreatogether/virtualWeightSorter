#include <Arduino.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include "SensorController.h"
#include <OneWire.h>
#include <DallasTemperature.h>

extern OneWire oneWire;
extern DallasTemperature sensors;

SensorRowInfo SensorController::g_sortedSensorRows[SENSOR_MAX_COUNT];

SensorController::SensorController()
{
    // 생성자에서는 기본 초기화만 수행
    // EEPROM 초기화는 setup()에서 명시적으로 호출
    measurementInterval = DEFAULT_MEASUREMENT_INTERVAL;
}

uint8_t SensorController::getSensorLogicalId(int idx)
{
    int id = sensors.getUserDataByIndex(idx);
    if (id < 1 || id > SENSOR_MAX_COUNT)
    {
        return 0; // ID가 설정되지 않은 센서는 0 반환 (미할당 상태)
    }
    return id;
}

void SensorController::setSensorLogicalId(int idx, uint8_t newId)
{
    // EEPROM 수명 보호: 값이 변경된 경우에만 쓰기
    uint8_t currentId = sensors.getUserDataByIndex(idx);

    if (currentId != newId)
    {
        sensors.setUserDataByIndex(idx, newId);
        delay(30); // EEPROM write 여유 대기

        int verify = sensors.getUserDataByIndex(idx);
        Serial.print("[진단] setSensorLogicalId idx:");
        Serial.print(idx);
        Serial.print(" userData(변경: ");
        Serial.print(currentId);
        Serial.print(" → ");
        Serial.print(verify);
        Serial.print(", 기대값:");
        Serial.print(newId);
        Serial.println(")");
    }
    else
    {
        Serial.print("[진단] setSensorLogicalId idx:");
        Serial.print(idx);
        Serial.print(" userData 변경 없음 (현재값: ");
        Serial.print(currentId);
        Serial.println(")");
    }
}

bool SensorController::isIdDuplicated(int newId, int exceptIdx)
{
    int deviceCount = sensors.getDeviceCount();
    for (int i = 0; i < deviceCount; ++i)
    {
        if (i == exceptIdx)
            continue;
        if (getSensorLogicalId(i) == newId)
            return true;
    }
    return false;
}

void SensorController::assignIDsByAddress()
{
    int count = sensors.getDeviceCount();
    std::vector<int> idxs;
    idxs.reserve(count);
    for (int i = 0; i < count; ++i)
        idxs.push_back(i);
    std::sort(idxs.begin(), idxs.end(), [&](int a, int b)
              {
        DeviceAddress addrA, addrB;
        sensors.getAddress(addrA, a);
        sensors.getAddress(addrB, b);
        return memcmp(addrA, addrB, sizeof(DeviceAddress)) < 0; });
    for (int j = 0; j < (int)idxs.size(); ++j)
    {
        setSensorLogicalId(idxs[j], j + 1);
    }
}

void SensorController::resetAllSensorIds()
{
    int deviceCount = sensors.getDeviceCount();
    Serial.println();
    Serial.println("=== 전체 센서 ID 초기화 시작 ===");

    if (deviceCount == 0)
    {
        Serial.println("연결된 센서가 없습니다.");
        return;
    }

    Serial.print("총 ");
    Serial.print(deviceCount);
    Serial.println("개의 센서 ID를 초기화합니다...");

    int resetCount = 0;
    for (int i = 0; i < deviceCount; i++)
    {
        uint8_t currentId = getSensorLogicalId(i);

        // 현재 ID가 있는 센서만 초기화 (EEPROM 수명 보호)
        if (currentId >= 1 && currentId <= SENSOR_MAX_COUNT)
        {
            // ID를 0으로 설정하여 미할당 상태로 만듦 (setSensorLogicalId 사용으로 중복 쓰기 방지)
            setSensorLogicalId(i, 0);

            Serial.print("센서 ");
            Serial.print(i + 1);
            Serial.print(" (기존 ID: ");
            Serial.print(currentId);
            Serial.println(") → 미할당 상태로 초기화");

            resetCount++;
        }
        else if (currentId == 0)
        {
            Serial.print("센서 ");
            Serial.print(i + 1);
            Serial.println(" → 이미 미할당 상태 (건너뜀)");
        }
    }

    Serial.println();
    Serial.print("초기화 완료: ");
    Serial.print(resetCount);
    Serial.print("/");
    Serial.print(deviceCount);
    Serial.println("개 센서 ID가 초기화되었습니다.");
    Serial.println("=== 전체 센서 ID 초기화 완료 ===");
    Serial.println();
}

const char *SensorController::getUpperState(float temp)
{
    // 기존 메서드는 기본 임계값 사용 (하위 호환성)
    if (temp == DEVICE_DISCONNECTED_C)
        return "-";
    return (temp > DEFAULT_UPPER_THRESHOLD) ? "초과" : "정상";
}

const char *SensorController::getLowerState(float temp)
{
    // 기존 메서드는 기본 임계값 사용 (하위 호환성)
    if (temp == DEVICE_DISCONNECTED_C)
        return "-";
    return (temp < DEFAULT_LOWER_THRESHOLD) ? "초과" : "정상";
}

const char *SensorController::getSensorStatus(float temp)
{
    // 기존 메서드는 기본 임계값 사용 (하위 호환성)
    if (temp == DEVICE_DISCONNECTED_C)
        return "오류";

    if (temp > DEFAULT_UPPER_THRESHOLD || temp < DEFAULT_LOWER_THRESHOLD)
    {
        return "경고";
    }

    return "정상";
}

void SensorController::printSensorAddress(const DeviceAddress &addr)
{
    Serial.print("0x");
    for (uint8_t j = 0; j < 8; j++)
    {
        if (addr[j] < 16)
            Serial.print("0");
        Serial.print(addr[j], HEX);
    }
}

void SensorController::printSensorRow(int idx, int id, const DeviceAddress &addr, float temp)
{
    Serial.print("| ");
    Serial.print(id);
    Serial.print("    | ");
    uint8_t logicalId = getSensorLogicalId(idx);
    bool idValid = (logicalId >= 1 && logicalId <= SENSOR_MAX_COUNT);
    if (id == -1)
    {
        Serial.print("NONE   | NONE         | N/A     | N/A       | N/A         | N/A         | N/A       | N/A         | N/A     |");
    }
    else
    {
        if (logicalId == 0)
        {
            Serial.print("미할당");
            Serial.print(" | ");
        }
        else if (idValid)
        {
            Serial.print(logicalId);
            Serial.print("   | ");
        }
        else
        {
            Serial.print("ERR");
            Serial.print("   | ");
        }
        printSensorAddress(addr);
        Serial.print(" | ");
        if (temp == DEVICE_DISCONNECTED_C)
            Serial.print("N/A   ");
        else
        {
            Serial.print(temp, 1);
            Serial.print("°C   ");
        }
        Serial.print(" | ");
        // 센서별 임계값 사용 - 표시 행 번호(id-1)를 인덱스로 사용
        int displayRowIdx = id - 1; // 표시되는 행 번호를 0-based 인덱스로 변환
        if (logicalId >= 1 && logicalId <= SENSOR_MAX_COUNT)
        {
            // ID가 할당된 센서: 표시 행 인덱스로 임계값 조회
            Serial.print(getUpperThreshold(displayRowIdx), 1);
            Serial.print("°C       | ");
            Serial.print(getUpperState(displayRowIdx, temp));
            Serial.print("         | ");
            Serial.print(getLowerThreshold(displayRowIdx), 1);
            Serial.print("°C       | ");
            Serial.print(getLowerState(displayRowIdx, temp));
            Serial.print("         | ");
            Serial.print(getSensorStatus(displayRowIdx, temp));
            Serial.print("     |");
        }
        else
        {
            // ID가 없는 센서도 표시 행 인덱스로 임계값 조회 (일관성 유지)
            Serial.print(getUpperThreshold(displayRowIdx), 1);
            Serial.print("°C       | ");
            Serial.print(getUpperState(displayRowIdx, temp));
            Serial.print("         | ");
            Serial.print(getLowerThreshold(displayRowIdx), 1);
            Serial.print("°C       | ");
            Serial.print(getLowerState(displayRowIdx, temp));
            Serial.print("         | ");
            Serial.print(getSensorStatus(displayRowIdx, temp));
            Serial.print("     |");
        }
    }
    Serial.println();
}

void SensorController::updateSensorRows()
{
    sensors.requestTemperatures();
    std::vector<SensorRowInfo> sensorRows;

    collectSensorData(sensorRows);
    sortSensorRows(sensorRows);
    storeSortedResults(sensorRows);
}

void SensorController::printSensorStatusTable()
{
    Serial.println("| 번호 | ID  | 센서 주소           | 현재 온도 | 상한임계값   | 상한초과상태 | 하한임계값   | 하한초과상태 | 센서상태 |");
    Serial.println("| ---- | --- | ------------       | ---------  | ------------ | ------------ | ------------ | ------------ | -------- |");

    updateSensorRows();

    bool idErrorFound = false;
    String idErrorList = "";

    // 1~8번 행을 모두 출력: 정렬된 센서, 그 뒤 미연결 센서
    for (int i = 0; i < SENSOR_MAX_COUNT; ++i)
    {
        const auto &row = g_sortedSensorRows[i];
        if (row.connected)
        {
            if (row.logicalId < 1 || row.logicalId > SENSOR_MAX_COUNT)
            {
                idErrorFound = true;
                idErrorList += String(row.idx + 1) + "(0x";
                for (uint8_t j = 0; j < 8; j++)
                {
                    if (row.addr[j] < 16)
                        idErrorList += "0";
                    idErrorList += String(row.addr[j], HEX);
                }
                idErrorList += ") ";
            }
            printSensorRow(row.idx, i + 1, row.addr, row.temp);
        }
        else
        {
            DeviceAddress dummy = {0};
            printSensorRow(-1, i + 1, dummy, DEVICE_DISCONNECTED_C);
        }
    }
    Serial.println("=================================================================================================================");
    Serial.println("=================================================================================================================");
    if (idErrorFound)
    {
        Serial.print("[경고] 유효하지 않은 센서 ID(1~8 범위 밖) 감지: 센서 번호/주소: ");
        Serial.println(idErrorList);
        Serial.println("각 센서의 논리 ID(alarmHigh)는 반드시 1~8 범위여야 합니다. 메뉴에서 ID를 재설정하세요.");
    }
    Serial.println("센서 제어 메뉴 진입: 'menu' 또는 'm' 입력");
    Serial.println("(센서 ID/임계값/상태 관리 등은 메뉴에서 설정 가능)");
}
void SensorController::collectSensorData(std::vector<SensorRowInfo> &sensorRows)
{
    int deviceCount = sensors.getDeviceCount();

    for (int i = 0; i < SENSOR_MAX_COUNT; ++i)
    {
        SensorRowInfo rowInfo = createSensorRowInfo(i, deviceCount);
        sensorRows.push_back(rowInfo);
    }
}

SensorRowInfo SensorController::createSensorRowInfo(int idx, int deviceCount)
{
    DeviceAddress addr = {0};
    float temp = DEVICE_DISCONNECTED_C;
    bool connected = false;

    if (idx < deviceCount && sensors.getAddress(addr, idx))
    {
        temp = sensors.getTempC(addr);
        connected = true;
    }

    int logicalId = getSensorLogicalId(idx);
    SensorRowInfo rowInfo = {idx, logicalId, {0}, temp, connected};

    // Copy address
    for (size_t k = 0; k < sizeof(DeviceAddress); ++k)
    {
        rowInfo.addr[k] = addr[k];
    }

    return rowInfo;
}

void SensorController::sortSensorRows(std::vector<SensorRowInfo> &sensorRows)
{
    // ID 할당된 센서 → ID 미할당 센서 → 미연결 센서 순으로 정렬
    std::sort(sensorRows.begin(), sensorRows.end(), [](const SensorRowInfo &a, const SensorRowInfo &b)
              {
        // 연결 상태가 다르면 연결된 센서를 앞으로
        if (a.connected != b.connected) {
            return a.connected > b.connected;
        }
        
        // 둘 다 미연결이면 인덱스 순으로 정렬
        if (!a.connected) {
            return a.idx < b.idx;
        }
        
        // 둘 다 연결된 경우: ID 할당 상태에 따라 분류
        bool aHasId = (a.logicalId >= 1 && a.logicalId <= SENSOR_MAX_COUNT);
        bool bHasId = (b.logicalId >= 1 && b.logicalId <= SENSOR_MAX_COUNT);
        
        // ID 할당 상태가 다르면 ID가 있는 센서를 앞으로
        if (aHasId != bHasId) {
            return aHasId > bHasId;
        }
        
        // 둘 다 ID가 할당된 경우: 논리 ID 순으로 정렬
        if (aHasId && bHasId) {
            return a.logicalId < b.logicalId;
        }
        
        // 둘 다 ID가 미할당인 경우: 인덱스 순으로 정렬
        return a.idx < b.idx; });
}

void SensorController::storeSortedResults(const std::vector<SensorRowInfo> &sensorRows)
{
    // 정렬 결과를 전역 배열에 저장
    for (int i = 0; i < SENSOR_MAX_COUNT; ++i)
    {
        g_sortedSensorRows[i] = sensorRows[i];
    }
}

// ========== 센서 임계값 관리 메서드들 ==========

void SensorController::initializeThresholds()
{
    Serial.print("EEPROM 임계값 로드 중");

    for (int i = 0; i < SENSOR_MAX_COUNT; i++)
    {
        // 안전한 초기화: 먼저 기본값으로 설정
        sensorThresholds[i].upperThreshold = DEFAULT_UPPER_THRESHOLD;
        sensorThresholds[i].lowerThreshold = DEFAULT_LOWER_THRESHOLD;
        sensorThresholds[i].isCustomSet = false;

        // EEPROM에서 로드 시도
        loadSensorThresholds(i);
        Serial.print(".");
        delay(5); // 각 센서 로드 후 짧은 지연
    }

    Serial.println(" 완료");

    // 측정 주기도 함께 초기화
    initializeMeasurementInterval();
}

void SensorController::loadSensorThresholds(int sensorIdx)
{
    if (sensorIdx < 0 || sensorIdx >= SENSOR_MAX_COUNT)
        return;

    int addr = getEEPROMAddress(sensorIdx);

    // EEPROM에서 임계값 읽기
    EEPROM.get(addr, sensorThresholds[sensorIdx].upperThreshold);
    EEPROM.get(addr + 4, sensorThresholds[sensorIdx].lowerThreshold);

    // 유효성 검사 (초기값 또는 손상된 데이터 처리)
    bool needsReset = false;

    if (isnan(sensorThresholds[sensorIdx].upperThreshold) ||
        !isValidTemperature(sensorThresholds[sensorIdx].upperThreshold))
    {
        sensorThresholds[sensorIdx].upperThreshold = DEFAULT_UPPER_THRESHOLD;
        needsReset = true;
    }

    if (isnan(sensorThresholds[sensorIdx].lowerThreshold) ||
        !isValidTemperature(sensorThresholds[sensorIdx].lowerThreshold))
    {
        sensorThresholds[sensorIdx].lowerThreshold = DEFAULT_LOWER_THRESHOLD;
        needsReset = true;
    }

    // 논리 검증: 상한값이 하한값보다 작으면 기본값으로 리셋
    if (sensorThresholds[sensorIdx].upperThreshold <= sensorThresholds[sensorIdx].lowerThreshold)
    {
        sensorThresholds[sensorIdx].upperThreshold = DEFAULT_UPPER_THRESHOLD;
        sensorThresholds[sensorIdx].lowerThreshold = DEFAULT_LOWER_THRESHOLD;
        needsReset = true;
    }

    // 손상된 데이터가 있었다면 EEPROM에 기본값 저장 (조용히)
    if (needsReset)
    {
        saveSensorThresholds(sensorIdx, false); // verbose = false
        sensorThresholds[sensorIdx].isCustomSet = false;
    }
    else
    {
        sensorThresholds[sensorIdx].isCustomSet = true;
    }
}

void SensorController::saveSensorThresholds(int sensorIdx)
{
    saveSensorThresholds(sensorIdx, true); // 기본적으로 출력 활성화
}

void SensorController::saveSensorThresholds(int sensorIdx, bool verbose)
{
    if (sensorIdx < 0 || sensorIdx >= SENSOR_MAX_COUNT)
        return;

    int addr = getEEPROMAddress(sensorIdx);

    // 값이 변경된 경우에만 EEPROM 쓰기 (수명 연장)
    float currentUpper, currentLower;
    EEPROM.get(addr, currentUpper);
    EEPROM.get(addr + 4, currentLower);

    if (currentUpper != sensorThresholds[sensorIdx].upperThreshold)
    {
        EEPROM.put(addr, sensorThresholds[sensorIdx].upperThreshold);
    }

    if (currentLower != sensorThresholds[sensorIdx].lowerThreshold)
    {
        EEPROM.put(addr + 4, sensorThresholds[sensorIdx].lowerThreshold);
    }

    if (verbose)
    {
        Serial.print("💾 EEPROM 저장 - 센서 ");
        Serial.print(sensorIdx + 1);
        Serial.print(": TH=");
        Serial.print(sensorThresholds[sensorIdx].upperThreshold, 1);
        Serial.print("°C, TL=");
        Serial.print(sensorThresholds[sensorIdx].lowerThreshold, 1);
        Serial.println("°C");
    }
}

int SensorController::getEEPROMAddress(int sensorIdx)
{
    return EEPROM_BASE_ADDR + (sensorIdx * EEPROM_SIZE_PER_SENSOR);
}

float SensorController::getUpperThreshold(int sensorIdx)
{
    if (sensorIdx < 0 || sensorIdx >= SENSOR_MAX_COUNT)
    {
        return DEFAULT_UPPER_THRESHOLD;
    }
    return sensorThresholds[sensorIdx].upperThreshold;
}

float SensorController::getLowerThreshold(int sensorIdx)
{
    if (sensorIdx < 0 || sensorIdx >= SENSOR_MAX_COUNT)
    {
        return DEFAULT_LOWER_THRESHOLD;
    }
    return sensorThresholds[sensorIdx].lowerThreshold;
}

void SensorController::setThresholds(int sensorIdx, float upperTemp, float lowerTemp)
{
    // sensorIdx는 표시 행 번호 기반 인덱스 (0-7)
    // 센서 논리 ID와는 무관하게 표시되는 위치로 임계값을 관리
    if (sensorIdx < 0 || sensorIdx >= SENSOR_MAX_COUNT)
    {
        Serial.println("❌ 오류: 잘못된 센서 인덱스");
        return;
    }

    // 입력 검증
    if (!isValidTemperature(upperTemp) || !isValidTemperature(lowerTemp))
    {
        Serial.println("❌ 오류: 온도 범위를 벗어났습니다 (-55~125°C)");
        return;
    }

    if (upperTemp <= lowerTemp)
    {
        Serial.println("❌ 오류: 상한값은 하한값보다 커야 합니다");
        return;
    }

    // 임계값 설정
    sensorThresholds[sensorIdx].upperThreshold = upperTemp;
    sensorThresholds[sensorIdx].lowerThreshold = lowerTemp;
    sensorThresholds[sensorIdx].isCustomSet = true;

    // EEPROM에 저장
    saveSensorThresholds(sensorIdx);

    Serial.print("✅ 센서 ");
    Serial.print(sensorIdx + 1);
    Serial.print(" 임계값 설정 완료: TH=");
    Serial.print(upperTemp, 1);
    Serial.print("°C, TL=");
    Serial.print(lowerTemp, 1);
    Serial.println("°C");
}

bool SensorController::isValidTemperature(float temp)
{
    return (temp >= DS18B20_MIN_TEMP && temp <= DS18B20_MAX_TEMP);
}

void SensorController::resetSensorThresholds(int sensorIdx)
{
    if (sensorIdx < 0 || sensorIdx >= SENSOR_MAX_COUNT)
        return;

    sensorThresholds[sensorIdx].upperThreshold = DEFAULT_UPPER_THRESHOLD;
    sensorThresholds[sensorIdx].lowerThreshold = DEFAULT_LOWER_THRESHOLD;
    sensorThresholds[sensorIdx].isCustomSet = false;

    saveSensorThresholds(sensorIdx);

    Serial.print("🔄 센서 ");
    Serial.print(sensorIdx + 1);
    Serial.println(" 임계값이 기본값으로 초기화되었습니다");
}

void SensorController::resetAllThresholds()
{
    Serial.println();
    Serial.println("=== 전체 센서 임계값 초기화 시작 ===");

    for (int i = 0; i < SENSOR_MAX_COUNT; i++)
    {
        resetSensorThresholds(i);
    }

    Serial.println("=== 전체 센서 임계값 초기화 완료 ===");
    Serial.println();
}

// 센서별 임계값을 사용한 상태 확인 메서드들
const char *SensorController::getUpperState(int sensorIdx, float temp)
{
    if (temp == DEVICE_DISCONNECTED_C)
        return "-";

    float threshold = getUpperThreshold(sensorIdx);
    return (temp > threshold) ? "초과" : "정상";
}

const char *SensorController::getLowerState(int sensorIdx, float temp)
{
    if (temp == DEVICE_DISCONNECTED_C)
        return "-";

    float threshold = getLowerThreshold(sensorIdx);
    return (temp < threshold) ? "초과" : "정상";
}

const char *SensorController::getSensorStatus(int sensorIdx, float temp)
{
    if (temp == DEVICE_DISCONNECTED_C)
        return "오류";

    float upperThreshold = getUpperThreshold(sensorIdx);
    float lowerThreshold = getLowerThreshold(sensorIdx);

    if (temp > upperThreshold || temp < lowerThreshold)
    {
        return "경고";
    }

    return "정상";
}

// ========== 측정 주기 관리 메서드들 ==========

void SensorController::initializeMeasurementInterval()
{
    Serial.print("EEPROM 측정 주기 로드 중");

    // 기본값으로 초기화
    measurementInterval = DEFAULT_MEASUREMENT_INTERVAL;

    // EEPROM에서 로드 시도
    loadMeasurementInterval();

    Serial.print(".");
    Serial.println(" 완료");

    Serial.print("현재 측정 주기: ");
    Serial.println(formatInterval(measurementInterval));
}

void SensorController::loadMeasurementInterval()
{
    unsigned long storedInterval;
    EEPROM.get(EEPROM_INTERVAL_ADDR, storedInterval);

    // 유효성 검사
    if (isValidMeasurementInterval(storedInterval))
    {
        measurementInterval = storedInterval;
    }
    else
    {
        // 유효하지 않은 값이면 기본값 사용 및 저장
        measurementInterval = DEFAULT_MEASUREMENT_INTERVAL;
        saveMeasurementInterval();
    }
}

void SensorController::saveMeasurementInterval()
{
    // 값이 변경된 경우에만 EEPROM 쓰기 (수명 연장)
    unsigned long currentInterval;
    EEPROM.get(EEPROM_INTERVAL_ADDR, currentInterval);

    if (currentInterval != measurementInterval)
    {
        EEPROM.put(EEPROM_INTERVAL_ADDR, measurementInterval);
        Serial.print("💾 EEPROM 저장 - 측정 주기: ");
        Serial.println(formatInterval(measurementInterval));
    }
}

unsigned long SensorController::getMeasurementInterval()
{
    return measurementInterval;
}

void SensorController::setMeasurementInterval(unsigned long intervalMs)
{
    if (!isValidMeasurementInterval(intervalMs))
    {
        Serial.println("❌ 오류: 측정 주기 범위를 벗어났습니다");
        return;
    }

    measurementInterval = intervalMs;
    saveMeasurementInterval();

    Serial.print("✅ 측정 주기 설정 완료: ");
    Serial.println(formatInterval(measurementInterval));
}

bool SensorController::isValidMeasurementInterval(unsigned long intervalMs)
{
    return (intervalMs >= MIN_MEASUREMENT_INTERVAL && intervalMs <= MAX_MEASUREMENT_INTERVAL);
}

String SensorController::formatInterval(unsigned long intervalMs)
{
    String result = "";

    // 밀리초를 초로 변환
    unsigned long totalSeconds = intervalMs / 1000;

    if (totalSeconds >= 86400)
    { // 1일 이상
        unsigned long days = totalSeconds / 86400;
        unsigned long remainingSeconds = totalSeconds % 86400;
        result += String(days) + "일";

        if (remainingSeconds >= 3600)
        { // 1시간 이상
            unsigned long hours = remainingSeconds / 3600;
            remainingSeconds %= 3600;
            result += " " + String(hours) + "시간";

            if (remainingSeconds >= 60)
            { // 1분 이상
                unsigned long minutes = remainingSeconds / 60;
                remainingSeconds %= 60;
                result += " " + String(minutes) + "분";

                if (remainingSeconds > 0)
                {
                    result += " " + String(remainingSeconds) + "초";
                }
            }
            else if (remainingSeconds > 0)
            {
                result += " " + String(remainingSeconds) + "초";
            }
        }
        else if (remainingSeconds >= 60)
        { // 1분 이상
            unsigned long minutes = remainingSeconds / 60;
            remainingSeconds %= 60;
            result += " " + String(minutes) + "분";

            if (remainingSeconds > 0)
            {
                result += " " + String(remainingSeconds) + "초";
            }
        }
        else if (remainingSeconds > 0)
        {
            result += " " + String(remainingSeconds) + "초";
        }
    }
    else if (totalSeconds >= 3600)
    { // 1시간 이상
        unsigned long hours = totalSeconds / 3600;
        unsigned long remainingSeconds = totalSeconds % 3600;
        result += String(hours) + "시간";

        if (remainingSeconds >= 60)
        { // 1분 이상
            unsigned long minutes = remainingSeconds / 60;
            remainingSeconds %= 60;
            result += " " + String(minutes) + "분";

            if (remainingSeconds > 0)
            {
                result += " " + String(remainingSeconds) + "초";
            }
        }
        else if (remainingSeconds > 0)
        {
            result += " " + String(remainingSeconds) + "초";
        }
    }
    else if (totalSeconds >= 60)
    { // 1분 이상
        unsigned long minutes = totalSeconds / 60;
        unsigned long remainingSeconds = totalSeconds % 60;
        result += String(minutes) + "분";

        if (remainingSeconds > 0)
        {
            result += " " + String(remainingSeconds) + "초";
        }
    }
    else
    { // 1분 미만
        result += String(totalSeconds) + "초";
    }

    return result;
}
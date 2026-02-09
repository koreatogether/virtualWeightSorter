# Arduino 온도 측정 및 BLE 전송 시스템 리팩토링 종합

## 1. 코드 구조 및 모듈화 개선

### 1.1. 모듈 분리
현재 코드를 기능별로 분리하여 관리하면 유지보수가 쉬워집니다:

```cpp
// SensorManager.h - 온도 센서 관리
class SensorManager {
private:
    OneWire* oneWire;
    DallasTemperature* sensors;
    int busPin;
    // ...기타 센서 관련 변수들
    
public:
    SensorManager(int pin);
    void begin();
    void readTemperatures();
    // ...기타 메소드들
};

// DisplayManager.h - LCD 디스플레이 관리
class DisplayManager {
    // LCD 관련 기능 캡슐화
};

// BLEManager.h - 블루투스 통신 관리
class BLEManager {
    // BLE 관련 기능 캡슐화
};
```

### 1.2. 설정값 분리
모든 상수와 설정값을 한 곳에서 관리:

```cpp
// config.h
// 핀 설정
#define ONE_WIRE_BUS_PIN 2
#define BUTTON_PIN 3

// LCD 설정
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_TIMEOUT 20000  // 20초

// 센서 설정
#define REQUIRED_SENSOR_COUNT 4
#define SENSOR_MIN_TEMP -55
#define SENSOR_MAX_TEMP 125

// BLE 설정
#define BLE_CHAR_BUFFER_SIZE 8
```

## 2. 오류 처리 개선

### 2.1. 에러 코드 및 메시지 체계화
```cpp
enum ErrorCode {
    NO_ERROR = 0,
    SENSOR_NOT_FOUND,
    SENSOR_DISCONNECTED,
    TEMPERATURE_OUT_OF_RANGE,
    DUPLICATE_SENSOR_ID
};

const char* getErrorMessage(ErrorCode error) {
    switch (error) {
        case SENSOR_NOT_FOUND: return "Sensor Not Found";
        case SENSOR_DISCONNECTED: return "Sensor Disconnected";
        case TEMPERATURE_OUT_OF_RANGE: return "Temp Out of Range";
        case DUPLICATE_SENSOR_ID: return "Duplicate Sensor ID";
        default: return "Unknown Error";
    }
}

struct SensorError {
    ErrorCode code;
    const char* location;    // 에러 발생 위치
    String details;          // 추가 정보
};
```

### 2.2. 문제 해결 가이드 추가
```cpp
void troubleshootSensor(ErrorCode error) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(getErrorMessage(error));
    lcd.setCursor(0, 1);
    
    switch (error) {
        case SENSOR_NOT_FOUND:
            lcd.print("Check wiring & power");
            Serial.println("해결방법: 센서 연결 상태와 전원을 확인하세요");
            break;
        case SENSOR_DISCONNECTED:
            lcd.print("Check connections");
            Serial.println("해결방법: 연결이 끊어진 센서를 확인하세요");
            break;
        // ...기타 에러 케이스
    }
}
```

## 3. 코드 최적화

### 3.1. 온도 측정 최적화
- 여러 함수에서 중복 호출되는 `requestTemperatures()` 통합
```cpp
void updateAllOutputs() {
    // 한 번만 온도 요청
    ds18b20.requestTemperatures();
    
    // 각 출력 업데이트
    updateLCDDisplay();
    updateBLEData();
    if (DEBUG) updateSerial();
}
```

### 3.2. LCD 업데이트 최적화
- 변경된 부분만 업데이트하여 깜빡임 방지
```cpp
void updateLCDDisplay() {
    // 이전 값과 비교하여 변경된 경우만 업데이트
    for (int i = 0; i < REQUIRED_SENSOR_COUNT; i++) {
        float newTemp = getSensorTemperature(i);
        if (newTemp != lastTemperatures[i]) {
            updateLCDLine(i, newTemp);
            lastTemperatures[i] = newTemp;
        }
    }
}
```

## 4. 유지보수성 개선

### 4.1. 센서 데이터 구조화
```cpp
struct SensorData {
    DeviceAddress address;
    int id;
    bool isValid;
    float lastTemperature;
    ErrorCode lastError;
    unsigned long lastReadTime;
};

SensorData sensors[REQUIRED_SENSOR_COUNT];
```

### 4.2. 상태 관리 개선
- 센서 상태를 열거형으로 명확하게 표현
```cpp
enum SensorStatus {
    SENSOR_OK,
    SENSOR_ERROR,
    SENSOR_DISCONNECTED,
    SENSOR_OUT_OF_RANGE
};

SensorStatus getSensorStatus(int sensorIndex) {
    if (!sensors[sensorIndex].isValid) return SENSOR_ERROR;
    
    float temp = ds18b20.getTempC(sensors[sensorIndex].address);
    if (temp == DEVICE_DISCONNECTED_C) return SENSOR_DISCONNECTED;
    if (temp < SENSOR_MIN_TEMP || temp > SENSOR_MAX_TEMP) return SENSOR_OUT_OF_RANGE;
    
    return SENSOR_OK;
}
```

## 5. 디버깅 기능 강화

### 5.1. 디버그 모드 추가
```cpp
#define DEBUG true  // 디버그 모드 활성화 여부

#ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif
```

### 5.2. 로그 기능 개선
```cpp
// 로그 레벨 정의
enum LogLevel {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
};

void log(LogLevel level, const char* message, const char* location = "") {
    if (!DEBUG && level == LOG_DEBUG) return;
    
    String prefix;
    switch (level) {
        case LOG_ERROR: prefix = "ERROR"; break;
        case LOG_WARNING: prefix = "WARN"; break;
        case LOG_INFO: prefix = "INFO"; break;
        case LOG_DEBUG: prefix = "DEBUG"; break;
    }
    
    String timestamp = String(millis());
    Serial.print("[");
    Serial.print(timestamp);
    Serial.print("][");
    Serial.print(prefix);
    Serial.print("] ");
    if (location && strlen(location) > 0) {
        Serial.print(location);
        Serial.print(": ");
    }
    Serial.println(message);
}
```

## 결론

이상의 리팩토링을 통해 다음과 같은 이점을 얻을 수 있습니다:

1. **모듈화된 구조**: 기능별로 코드가 분리되어 유지보수가 용이
2. **향상된 에러 처리**: 구체적인 에러 메시지와 해결 방법 제공
3. **최적화된 성능**: 중복 작업 제거 및 효율적인 업데이트 처리
4. **개선된 디버깅**: 체계적인 로그 시스템으로 문제 해결 용이
5. **확장성**: 새로운 기능 추가가 쉬운 구조

리팩토링 시 주의사항:
- 한 번에 모든 변경을 적용하지 말고 단계적으로 진행
- 각 변경 후 반드시 기능 테스트 수행
- 메모리 사용량을 고려하여 적절한 수준의 모듈화 적용
- 실제 하드웨어에서의 테스트를 통해 성능 영향 확인
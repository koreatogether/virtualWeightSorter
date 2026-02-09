/**
 * @file 6_rev1_중간리팩토링.ino
 * @brief DS18B20 온도 센서를 사용한 다중 온도 모니터링 시스템
 * @details 여러 개의 DS18B20 온도 센서를 사용하여 온도를 측정하고
 *          LCD 디스플레이에 표시하며 예외 상황을 처리하는 시스템입니다.
 */

/*
 * 목적:
 * 센서의 ID, 주소, 온도를 출력하면서 다양한 예외상황 처리
 * 원래 코드 위치 : sensors/온도센서/DS18B20/실제사용모양/06_예외처리추가.ino
 * Wokwi (온도센서 연결안함) 주소 : https://wokwi.com/projects/422825706774615041  ( id 검증부분 비활성화시킴)
 * Wokwi ( 온도센서 정상) 주소 : https://wokwi.com/projects/422970904159711233
 * Wokwi #6 주소 : https://wokwi.com/projects/423006796521012225  , https://wokwi.com/projects/423006796521012225
 * 실제 적용 결과 , 가상 결과와 같음
 * 부팅 후 최종화면 +  20 초간 LCD 화면을 보여주고 별도의 버튼을 눌렀을때 + 20초간 보여주는 로직을 추가 해보자
 */

/*---------- 배선 ---------------*/
/*
아두이노  <---> DS18B20
 2       <---->  DQ
 5V      <---->  VCC
 GND     <---->  GND
-----------------------------
아두이노 <---> I2C LCD
 A4      <---->  SDA
 A5      <---->  SCL
 5V      <---->  VCC
 GND     <---->  GND

아두이노 <---->  버튼
3       <---->  버튼의 다릿발중 한쪽
GND     <---->  버튼의 다릿발중 다른 한쪽

*/

/*
 * 목적:
 * - LCD 켜기/끄기 로직 개선
 * - 버튼 처리 로직 통합
 * - 비차단 방식으로 변경
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>
#include <ezButton.h>

/**
 * @brief 시스템 에러 코드를 정의하는 열거형
 */
enum class ErrorCode
{
    NONE = 0,
    /** @brief 센서를 찾을 수 없음 */
    SENSOR_NOT_FOUND = 100,
    /** @brief 센서 수가 예상과 다름 */
    SENSOR_COUNT_MISMATCH = 101,
    /** @brief 센서 주소 읽기 실패 */
    SENSOR_ADDRESS_READ_ERROR = 102,
    /** @brief 중복된 센서 ID 발견 */
    SENSOR_DUPLICATE_ID = 103,
    /** @brief 센서 통신 오류 */
    SENSOR_COMM_ERROR = 104,
    /** @brief 온도 범위 초과 */
    SENSOR_TEMP_RANGE_ERROR = 105,

    /** @brief LCD 초기화 오류 */
    LCD_INIT_ERROR = 200,
    /** @brief LCD 통신 오류 */
    LCD_COMM_ERROR = 201,

    /** @brief 시스템 초기화 오류 */
    SYSTEM_INIT_ERROR = 300,
    /** @brief 메모리 오류 */
    MEMORY_ERROR = 301
};

/**
 * @brief 로그 레벨을 정의하는 열거형
 */
enum class LogLevel
{
    DEBUG,   ///< 상세한 디버그 정보
    INFO,    ///< 일반적인 정보
    WARNING, ///< 경고
    ERROR    ///< 오류
};

/**
 * @brief 로깅 시스템을 관리하는 클래스
 */
class Logger
{
private:
    static LogLevel currentLevel;
    static char msgBuffer[128]; // 메시지 포맷팅을 위한 버퍼

    static void printLogPrefix(LogLevel level)
    {
        unsigned long timestamp = millis();
        switch (level)
        {
        case LogLevel::DEBUG:
            Serial.print(F("[DEBUG  ]"));
            break;
        case LogLevel::INFO:
            Serial.print(F("[INFO   ]"));
            break;
        case LogLevel::WARNING:
            Serial.print(F("[WARNING]"));
            break;
        case LogLevel::ERROR:
            Serial.print(F("[ERROR  ]"));
            break;
        }
        Serial.print(F("["));
        Serial.print(timestamp);
        Serial.print(F("ms] "));
    }

public:
    static void setLogLevel(LogLevel level)
    {
        currentLevel = level;
    }

    static void log(LogLevel level, const __FlashStringHelper *message, ErrorCode code = ErrorCode::NONE)
    {
        if (level >= currentLevel)
        {
            printLogPrefix(level);
            Serial.print(message);
            if (code != ErrorCode::NONE)
            {
                Serial.print(F(" (Error code: "));
                Serial.print(static_cast<int>(code));
                Serial.print(F(")"));
            }
            Serial.println();
        }
    }

    static void logf(LogLevel level, const __FlashStringHelper *format, ...)
    {
        if (level >= currentLevel)
        {
            va_list args;
            va_start(args, format);
            vsnprintf_P(msgBuffer, sizeof(msgBuffer), (const char *)format, args);
            va_end(args);

            printLogPrefix(level);
            Serial.println(msgBuffer);
        }
    }
};

LogLevel Logger::currentLevel = LogLevel::INFO;
char Logger::msgBuffer[128];

/**
 * @brief 함수 실행 결과를 나타내는 구조체
 */
struct Result
{
    bool success;   ///< 작업 성공 여부
    ErrorCode code; ///< 에러 코드
    String message; ///< 결과 메시지

    Result(bool s = true, ErrorCode c = ErrorCode::NONE, String msg = "")
        : success(s), code(c), message(msg) {}

    static Result ok()
    {
        return Result(true, ErrorCode::NONE, "");
    }

    static Result error(ErrorCode code, String message)
    {
        return Result(false, code, message);
    }
};

// InitResult를 Result로 변경
struct InitResult : public Result
{
    InitResult(bool s = true, ErrorCode c = ErrorCode::NONE, String msg = "")
        : Result(s, c, msg) {}
};

/**
 * @brief 센서의 현재 상태를 나타내는 열거형
 */
enum class SensorStatus
{
    OK,         ///< 정상
    ERROR,      ///< 센서 오류
    COMM_ERROR, ///< 통신 오류
    OVER_RANGE  ///< 온도 범위 초과
};

/**
 * @brief 센서의 상태 정보를 저장하는 구조체
 */
struct SensorState
{
    bool isValid;          ///< 센서 유효성
    int id;                ///< 센서 ID
    float temperature;     ///< 현재 온도
    SensorStatus status;   ///< 현재 상태
    DeviceAddress address; ///< 센서 주소

    String getStatusString()
    {
        switch (status)
        {
        case SensorStatus::OK:
            return "OK ";
        case SensorStatus::ERROR:
            return "ERR";
        case SensorStatus::COMM_ERROR:
            return "COM";
        case SensorStatus::OVER_RANGE:
            return "OVR";
        default:
            return "ERR";
        }
    }

    String getTemperatureString()
    {
        if (!isValid)
            return "--.-";
        if (status == SensorStatus::COMM_ERROR)
            return "ERR ";
        if (status == SensorStatus::OVER_RANGE)
            return "OVER";
        char tempStr[6];
        dtostrf(temperature, 4, 1, tempStr);
        return String(tempStr);
    }
};

/*------------  LCD 관련 상수 및 객체 -------------*/
const int I2C_ADDR = 0x27;
const int LCD_COLUNMS = 20;
const int LCD_ROWS = 4;
const int LCD_TIMEOUT = 20000; // 20초 타임아웃
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

/*------------  온도 센서 관련 상수 및 객체 -------------*/
const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int MIN_TEMP = -55;
const int MAX_TEMP = 125;

/*------------- 버튼 관련 상수 및 객체 -------------*/
const int BUTTON_PIN = 3;
ezButton button(BUTTON_PIN);

/*------------  LCD 상태 관리 상수 및 변수 -------------*/
unsigned long lcdStartTime = 0;
bool isLcdOn = true;

const int TIMEOUT = 20000; // 20초
const int LCD_LIGHT_ON = 255;
const int LCD_LIGHT_OFF = 0;

/*------------  센서 관련 변수 -------------*/
const int arraySizeByUser = 4;
SensorState sensors_state[arraySizeByUser];

/*------------ 시리얼 출력 타이머 변수 -------------*/
unsigned long lastSerialPrintTime = 0;
const unsigned long SERIAL_PRINT_INTERVAL = 2000; // 2초

/**
 * @brief LCD 디스플레이 관리 클래스
 */
class LCDDisplay
{
private:
    LiquidCrystal_PCF8574 &lcd;
    const int columns;
    const int rows;
    bool isOn;

public:
    LCDDisplay(LiquidCrystal_PCF8574 &lcd, int cols, int rows)
        : lcd(lcd), columns(cols), rows(rows), isOn(false)
    {
    }

    void begin()
    {
        lcd.begin(columns, rows);
        setBacklight(false);
        Logger::log(LogLevel::INFO, F("LCD initialized"));
    }

    void setBacklight(bool on)
    {
        isOn = on;
        lcd.setBacklight(on ? LCD_LIGHT_ON : LCD_LIGHT_OFF);
        Logger::logf(LogLevel::DEBUG, F("LCD backlight %s"), on ? "ON" : "OFF");
    }

    void clear()
    {
        lcd.clear();
    }

    // 템플릿 기반 센서 라인 포맷터
    template <typename T>
    void formatSensorLine(int row, const T &sensor)
    {
        if (!isOn)
            return;

        lcd.setCursor(0, row);

        // ID 출력
        lcd.print("ID:");
        lcd.print(sensor.isValid ? String(sensor.id) : "-");
        lcd.print(" ");

        // 온도 출력
        lcd.print("T:");
        lcd.print(sensor.getTemperatureString());
        lcd.print(" ");

        // 상태 출력
        lcd.setCursor(15, row);
        lcd.print(sensor.getStatusString());
    }

    // 초기화 메시지 출력
    void showInitMessage(const char *message)
    {
        clear();
        lcd.setCursor(0, 0);
        lcd.print(message);
    }

    // 에러 메시지 출력
    void showErrorMessage(const char *line1, const char *line2 = nullptr)
    {
        clear();
        lcd.setCursor(0, 0);
        lcd.print(line1);
        if (line2)
        {
            lcd.setCursor(0, 1);
            lcd.print(line2);
        }
        Logger::logf(LogLevel::ERROR, F("LCD Error message: %s"), line1);
    }
};

// LCD 디스플레이 객체 생성
LCDDisplay display(lcd, LCD_COLUNMS, LCD_ROWS);

/**
 * @brief LCD 상태를 업데이트하는 함수
 * @details 버튼 입력과 타이머에 따라 LCD의 백라이트를 제어합니다.
 */
void updateLCDState()
{
    static bool prevLcdState = true;
    bool shouldLcdBeOn = false;
    unsigned long currentTime = millis();

    // 버튼이 눌렸는지 확인
    if (button.isPressed())
    {
        Logger::log(LogLevel::DEBUG, F("Button pressed, LCD timer reset"));
        lcdStartTime = currentTime;
        shouldLcdBeOn = true;
    }

    // 타이머 확인
    if (currentTime - lcdStartTime < TIMEOUT)
    {
        shouldLcdBeOn = true;
    }

    // LCD 상태가 변경되었을 때만 업데이트
    if (shouldLcdBeOn != prevLcdState)
    {
        display.setBacklight(shouldLcdBeOn);
        if (shouldLcdBeOn)
        {
            Logger::log(LogLevel::DEBUG, F("LCD display turned on"));
            extern bool firstRun; // printSensorInforToLCD의 정적 변수 참조
            firstRun = true;      // LCD가 켜질 때 firstRun 플래그를 리셋
            printSensorInforToLCD();
        }
        else
        {
            Logger::log(LogLevel::DEBUG, F("LCD display turned off"));
        }
        prevLcdState = shouldLcdBeOn;
    }

    // LCD가 켜져있을 때만 정보 업데이트
    if (shouldLcdBeOn)
    {
        printSensorInforToLCD();
    }
}

/*------------ 센서 초기화 및 설정 함수 ----------------*/
bool initializeSensors()
{
    sensors.begin();
    uint8_t actualDeviceCount = sensors.getDeviceCount();

    if (actualDeviceCount == 0)
    {
        lcd.setCursor(0, 0);
        lcd.print("No sensors detected!");
        Serial.println(F("No sensors detected!"));
        return false;
    }

    if (actualDeviceCount < arraySizeByUser)
    {
        lcd.setCursor(0, 0);
        lcd.print("Warning: Found only");
        lcd.setCursor(0, 1);
        lcd.print(actualDeviceCount);
        lcd.print(" of ");
        lcd.print(arraySizeByUser);
        lcd.print(" sensors");
        return false;
    }

    return true;
}

void readAddress()
{
    for (int i = 0; i < arraySizeByUser; i++)
    {
        sensors_state[i].isValid = sensors.getAddress(sensors_state[i].address, i);
        if (!sensors_state[i].isValid)
        {
            Serial.print(F("Error: Failed to read address for sensor "));
            Serial.println(i);
        }
    }
}

void readIdFromDS18B20()
{
    bool foundDuplicate = false;
    for (int i = 0; i < arraySizeByUser; i++)
    {
        if (sensors_state[i].isValid)
        {
            sensors_state[i].id = sensors.getUserData(sensors_state[i].address);
            for (int j = 0; j < i; j++)
            {
                if (sensors_state[j].isValid && sensors_state[i].id == sensors_state[j].id)
                {
                    Serial.print(F("Error: Duplicate ID: "));
                    Serial.println(sensors_state[i].id);
                    foundDuplicate = true;
                }
            }
        }
    }
}

void sortIdByBubbleSort()
{
    SensorState temp;
    for (int i = 0; i < arraySizeByUser - 1; i++)
    {
        for (int j = 0; j < arraySizeByUser - i - 1; j++)
        {
            if (sensors_state[j].id > sensors_state[j + 1].id)
            {
                temp = sensors_state[j];
                sensors_state[j] = sensors_state[j + 1];
                sensors_state[j + 1] = temp;
            }
        }
    }
}

/**
 * @brief 센서 정보를 업데이트하는 함수
 * @param index 업데이트할 센서의 인덱스
 */
void updateSensorState(int index)
{
    if (!sensors_state[index].isValid)
    {
        sensors_state[index].status = SensorStatus::ERROR;
        Logger::logf(LogLevel::WARNING, F("Sensor %d is invalid"), index);
        return;
    }

    float tempC = sensors.getTempC(sensors_state[index].address);
    sensors_state[index].temperature = tempC;

    if (tempC == DEVICE_DISCONNECTED_C)
    {
        sensors_state[index].status = SensorStatus::COMM_ERROR;
        Logger::logf(LogLevel::ERROR, F("Communication error with sensor %d"), index,
                     static_cast<int>(ErrorCode::SENSOR_COMM_ERROR));
    }
    else if (tempC < MIN_TEMP || tempC > MAX_TEMP)
    {
        sensors_state[index].status = SensorStatus::OVER_RANGE;
        Logger::logf(LogLevel::WARNING, F("Temperature out of range for sensor %d: %.1f°C"),
                     index, tempC);
    }
    else
    {
        sensors_state[index].status = SensorStatus::OK;
        Logger::logf(LogLevel::DEBUG, F("Sensor %d temperature: %.1f°C"), index, tempC);
    }
}

void printSensorInfo()
{
    unsigned long currentTime = millis();

    // 2초가 지났는지 확인
    if (currentTime - lastSerialPrintTime < SERIAL_PRINT_INTERVAL)
    {
        return; // 2초가 지나지 않았으면 함수 종료
    }

    // 마지막 출력 시간 업데이트
    lastSerialPrintTime = currentTime;

    // 온도 측정 요청
    sensors.requestTemperatures();

    Serial.println(F("\n=== 센서 정보 ==="));
    Serial.println(F("순서\tID\t온도\t\t주소\t\t상태"));
    Serial.println(F("----------------------------------------------------"));

    for (int i = 0; i < arraySizeByUser; i++)
    {
        updateSensorState(i);

        Serial.print(i + 1);
        Serial.print(F("\t"));

        if (!sensors_state[i].isValid)
        {
            Serial.println(F("센서 오류 - 확인 필요"));
            continue;
        }

        Serial.print(sensors_state[i].id);
        Serial.print(F("\t"));

        float tempC = sensors.getTempC(sensors_state[i].address);
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            Serial.print(F("통신오류"));
        }
        else if (tempC < MIN_TEMP || tempC > MAX_TEMP)
        {
            Serial.print(F("온도범위초과"));
        }
        else
        {
            Serial.print(tempC, 1);
            Serial.print(F("°C"));
        }
        Serial.print(F("\t"));

        for (uint8_t j = 0; j < 8; j++)
        {
            if (sensors_state[i].address[j] < 16)
                Serial.print("0");
            Serial.print(sensors_state[i].address[j], HEX);
        }

        Serial.print(F("\t"));
        switch (sensors_state[i].status)
        {
        case SensorStatus::OK:
            Serial.print(F("정상"));
            break;
        case SensorStatus::COMM_ERROR:
            Serial.print(F("통신오류"));
            break;
        case SensorStatus::OVER_RANGE:
            Serial.print(F("온도범위초과"));
            break;
        default:
            Serial.print(F("센서오류"));
            break;
        }
        Serial.println();
    }
    Serial.println(F("----------------------------------------------------"));
}

void printSensorInforToLCD()
{
    static bool firstRun = true; // 첫 실행 여부를 추적하는 정적 변수
    sensors.requestTemperatures();

    if (firstRun)
    {
        display.clear();
        firstRun = false;
    }

    for (int i = 0; i < arraySizeByUser; i++)
    {
        updateSensorState(i);
        display.formatSensorLine(i, sensors_state[i]);
    }
}

// firstRun 변수를 전역 범위에서 정의
bool firstRun = true; // 전역 변수로 선언

// LCD 상태가 꺼질 때 firstRun 플래그를 리셋하는 함수 수정
void resetLCDFirstRun()
{
    static bool prevLcdState = true;
    bool shouldLcdBeOn = false;
    unsigned long currentTime = millis();

    if (currentTime - lcdStartTime >= TIMEOUT)
    {
        shouldLcdBeOn = false;
    }
    else
    {
        shouldLcdBeOn = true;
    }

    // LCD가 꺼질 때 firstRun 플래그를 리셋
    if (!shouldLcdBeOn && prevLcdState)
    {
        firstRun = true; // 전역 변수 직접 참조
    }

    prevLcdState = shouldLcdBeOn;
}

// 초기화 결과를 저장할 구조체
struct InitResult
{
    bool success;
    String message;

    InitResult(bool s = true, String msg = "") : success(s), message(msg) {}
};

/**
 * @brief 초기화 상태 인터페이스
 */
class InitializationState
{
public:
    /**
     * @brief 초기화 상태를 실행
     * @return 초기화 결과
     */
    virtual InitResult execute() = 0;

    /**
     * @brief 상태 이름을 반환
     * @return 상태 이름
     */
    virtual String getName() = 0;

    virtual ~InitializationState() {}
};

/**
 * @brief 센서 검색 상태 클래스
 */
class SensorDetectionState : public InitializationState
{
private:
    DallasTemperature &sensors;
    const int expectedCount;

public:
    SensorDetectionState(DallasTemperature &s, int count)
        : sensors(s), expectedCount(count) {}

    InitResult execute() override
    {
        sensors.begin();
        uint8_t actualCount = sensors.getDeviceCount();

        if (actualCount == 0)
        {
            Logger::log(LogLevel::ERROR, F("No sensors detected"), ErrorCode::SENSOR_NOT_FOUND);
            return InitResult(false, ErrorCode::SENSOR_NOT_FOUND, F("No sensors detected!"));
        }
        if (actualCount < expectedCount)
        {
            Logger::logf(LogLevel::WARNING, F("Found only %d of %d sensors"), actualCount, expectedCount);
            return InitResult(false, ErrorCode::SENSOR_COUNT_MISMATCH,
                              "Found only " + String(actualCount) + " of " + String(expectedCount) + " sensors");
        }
        Logger::logf(LogLevel::INFO, F("Detected %d sensors"), actualCount);
        return InitResult(true);
    }

    String getName() override
    {
        return "Sensor Detection";
    }
};

/**
 * @brief 주소 읽기 상태 클래스
 */
class AddressReadState : public InitializationState
{
private:
    DallasTemperature &sensors;
    SensorState *sensorStates;
    const int count;

public:
    AddressReadState(DallasTemperature &s, SensorState *states, int c)
        : sensors(s), sensorStates(states), count(c) {}

    InitResult execute() override
    {
        bool allSuccess = true;
        String errorMsg;

        for (int i = 0; i < count; i++)
        {
            sensorStates[i].isValid = sensors.getAddress(sensorStates[i].address, i);
            if (!sensorStates[i].isValid)
            {
                Logger::logf(LogLevel::ERROR, F("Failed to read address for sensor %d"), i);
                allSuccess = false;
                errorMsg += "Failed to read address for sensor " + String(i) + "\n";
            }
        }

        if (!allSuccess)
        {
            return InitResult(false, ErrorCode::SENSOR_ADDRESS_READ_ERROR, errorMsg);
        }
        Logger::log(LogLevel::INFO, F("All sensor addresses read successfully"));
        return InitResult(true);
    }

    String getName() override
    {
        return "Address Reading";
    }
};

/**
 * @brief ID 읽기 상태 클래스
 */
class IdReadState : public InitializationState
{
private:
    DallasTemperature &sensors;
    SensorState *sensorStates;
    const int count;

public:
    IdReadState(DallasTemperature &s, SensorState *states, int c)
        : sensors(s), sensorStates(states), count(c) {}

    InitResult execute() override
    {
        bool allSuccess = true;
        String errorMsg;

        for (int i = 0; i < count; i++)
        {
            if (sensorStates[i].isValid)
            {
                sensorStates[i].id = sensors.getUserData(sensorStates[i].address);
                Logger::logf(LogLevel::DEBUG, F("Read ID %d for sensor %d"), sensorStates[i].id, i);

                for (int j = 0; j < i; j++)
                {
                    if (sensorStates[j].isValid && sensorStates[i].id == sensorStates[j].id)
                    {
                        Logger::logf(LogLevel::ERROR, F("Duplicate ID found: %d"), sensorStates[i].id);
                        allSuccess = false;
                        errorMsg += "Duplicate ID found: " + String(sensorStates[i].id) + "\n";
                    }
                }
            }
        }

        if (!allSuccess)
        {
            return InitResult(false, ErrorCode::SENSOR_DUPLICATE_ID, errorMsg);
        }
        Logger::log(LogLevel::INFO, F("All sensor IDs read successfully"));
        return InitResult(true);
    }

    String getName() override
    {
        return "ID Reading";
    }
};

/**
 * @brief ID 정렬 상태 클래스
 */
class IdSortState : public InitializationState
{
private:
    SensorState *sensorStates;
    const int count;

public:
    IdSortState(SensorState *states, int c) : sensorStates(states), count(c) {}

    InitResult execute() override
    {
        SensorState temp;
        for (int i = 0; i < count - 1; i++)
        {
            for (int j = 0; j < count - i - 1; j++)
            {
                if (sensorStates[j].id > sensorStates[j + 1].id)
                {
                    temp = sensorStates[j];
                    sensorStates[j] = sensorStates[j + 1];
                    sensorStates[j + 1] = temp;
                }
            }
        }
        return InitResult(true, "Sensors sorted by ID");
    }

    String getName() override
    {
        return "ID Sorting";
    }
};

/**
 * @brief 초기화 관리자 클래스
 * @details 시스템의 초기화 과정을 단계별로 관리하고 실행합니다.
 */
class InitializationManager
{
private:
    LCDDisplay &display;
    std::vector<InitializationState *> states;
    unsigned long stateStartTime;
    const unsigned long STATE_DISPLAY_TIME = 3000;
    int currentStateIndex;

public:
    InitializationManager(LCDDisplay &disp)
        : display(disp), currentStateIndex(0), stateStartTime(0) {}

    void addState(InitializationState *state)
    {
        states.push_back(state);
    }

    bool update()
    {
        if (currentStateIndex >= states.size())
        {
            return true; // 초기화 완료
        }

        unsigned long currentTime = millis();
        InitializationState *currentState = states[currentStateIndex];

        // 새로운 상태 시작
        if (stateStartTime == 0)
        {
            display.showInitMessage(("Init: " + currentState->getName()).c_str());
            stateStartTime = currentTime;
            return false;
        }

        // 상태 표시 시간 대기
        if (currentTime - stateStartTime < STATE_DISPLAY_TIME)
        {
            return false;
        }

        // 상태 실행
        InitResult result = currentState->execute();
        if (!result.success)
        {
            display.showErrorMessage(result.message.c_str());
            Serial.println(result.message);
            return true; // 에러로 인한 초기화 중단
        }

        // 다음 상태로 이동
        currentStateIndex++;
        stateStartTime = 0;
        return false;
    }

    ~InitializationManager()
    {
        for (auto state : states)
        {
            delete state;
        }
    }
}

/**
 * @brief 아두이노 초기 설정 함수
 * @details 시스템 초기화, 센서 검색, 주소 읽기 등을 수행합니다.
 */
void
setup()
{
    Serial.begin(9600);

#ifdef DEBUG_MODE
    Logger::setLogLevel(LogLevel::DEBUG);
#else
    Logger::setLogLevel(LogLevel::INFO);
#endif

    Logger::log(LogLevel::INFO, F("System initialization started"));

    display.begin();
    display.setBacklight(true);
    button.setDebounceTime(50);

    // 초기화 관리자 생성 및 상태 추가
    InitializationManager initManager(display);
    initManager.addState(new SensorDetectionState(sensors, arraySizeByUser));
    initManager.addState(new AddressReadState(sensors, sensors_state, arraySizeByUser));
    initManager.addState(new IdReadState(sensors, sensors_state, arraySizeByUser));
    initManager.addState(new IdSortState(sensors_state, arraySizeByUser));

    // 초기화 프로세스 실행
    bool initComplete = false;
    while (!initComplete)
    {
        initComplete = initManager.update();
    }

    // 초기화 후 설정
    for (int i = 0; i < arraySizeByUser; i++)
    {
        if (sensors_state[i].isValid)
        {
            sensors.setResolution(sensors_state[i].address, 12);
            Logger::logf(LogLevel::DEBUG, F("Set resolution for sensor %d"), i);
        }
    }

    Logger::log(LogLevel::INFO, F("System initialization completed"));
    lcdStartTime = millis();
}

/**
 * @brief 아두이노 메인 루프 함수
 * @details 버튼 상태 확인, LCD 업데이트, 센서 정보 출력을 반복 수행합니다.
 */
void loop()
{
    button.loop();
    updateLCDState();

#ifdef DEBUG_MODE
    printSensorInfo(); // Debug 모드에서만 시리얼 출력
#endif
}

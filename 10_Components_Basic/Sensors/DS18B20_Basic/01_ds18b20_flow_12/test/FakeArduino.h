#ifndef FAKE_ARDUINO_H
#define FAKE_ARDUINO_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h> // for va_list
#include <vector>
#include <string>
#include <algorithm> // for std::transform
#include <cctype>    // for tolower
#include <queue>     // for std::queue

// Arduino의 __FlashStringHelper 정의
class __FlashStringHelper;

// Arduino의 PROGMEM 관련 매크로 및 함수
#define PROGMEM
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))

// snprintf_P 함수 (PROGMEM 버전)
inline int snprintf_P(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);
    return result;
}

// Arduino의 String 클래스를 흉내 내는 가짜 클래스
class String
{
public:
    String() : _str("") {}
    String(const char *s) : _str(s ? s : "") {}
    String(const String &s) : _str(s._str) {}
    String(int val) : _str(std::to_string(val)) {}
    String(long val) : _str(std::to_string(val)) {}
    String(float val) : _str(std::to_string(val)) {}
    String(const std::string &s) : _str(s) {}                                             // std::string을 인자로 받는 생성자 추가
    String(const __FlashStringHelper *str) : _str(reinterpret_cast<const char *>(str)) {} // __FlashStringHelper* 지원 추가

    String &operator=(const String &other)
    {
        _str = other._str;
        return *this;
    }
    String &operator=(const char *other)
    {
        _str = other ? other : "";
        return *this;
    }

    size_t length() const { return _str.length(); }
    const char *c_str() const { return _str.c_str(); }
    bool operator==(const String &other) const { return _str == other._str; }
    bool operator==(const char *other) const { return _str == (other ? other : ""); }
    bool operator!=(const String &other) const { return _str != other._str; }
    bool operator!=(const char *other) const { return _str != (other ? other : ""); }
    bool operator<(const String &other) const { return _str < other._str; }
    bool operator>(const String &other) const { return _str > other._str; }
    String operator+(const String &other) const { return String(_str + other._str); }
    String operator+(const char *other) const { return String(_str + (other ? other : "")); }
    String operator+(int val) const { return String(_str + std::to_string(val)); }
    String &operator+=(const String &other)
    {
        _str += other._str;
        return *this;
    }
    String &operator+=(const char *other)
    {
        _str += (other ? other : "");
        return *this;
    }
    String &operator+=(char c)
    {
        _str += c;
        return *this;
    }

    // operator[] 추가
    char operator[](size_t index) const { return _str.at(index); }

    char charAt(size_t index) const { return _str.at(index); }
    int toInt() const { return std::stoi(_str); }
    float toFloat() const { return std::stof(_str); }
    void trim()
    {
        size_t first = _str.find_first_not_of(" \t\n\r");
        if (std::string::npos == first)
        {
            _str = "";
            return;
        }
        size_t last = _str.find_last_not_of(" \t\n\r");
        _str = _str.substr(first, (last - first + 1));
    }
    String toLowerCase() const
    {
        String result = *this;
        std::transform(result._str.begin(), result._str.end(), result._str.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return result;
    }
    bool equalsIgnoreCase(const String &other) const
    {
        return _str.length() == other._str.length() &&
               std::equal(_str.begin(), _str.end(), other._str.begin(),
                          [](char a, char b)
                          { return std::tolower(a) == std::tolower(b); });
    }
    int indexOf(char c) const { return _str.find(c); }
    int indexOf(const char *str) const
    {
        size_t pos = _str.find(str);
        return (pos != std::string::npos) ? static_cast<int>(pos) : -1;
    }
    void replace(char find, char replace)
    {
        for (char &c : _str)
        {
            if (c == find)
                c = replace;
        }
    }

    // _str 멤버에 접근할 수 있는 public 메서드 추가
    std::string getString() const { return _str; }

private:
    std::string _str;
};

// const char* + String 연산을 위한 전역 operator+ 오버로드
inline String operator+(const char *lhs, const String &rhs)
{
    return String(lhs) + rhs;
}

// Arduino의 Serial 객체를 흉내 내는 가짜 클래스
class FakeSerial
{
public:
    // begin() 함수 추가
    void begin(long baud)
    {
        // 테스트에서는 실제 시리얼 통신 초기화가 필요 없으므로 비워둡니다.
    }

    // operator bool() 추가 (while (!Serial) 구문 처리용)
    operator bool() const
    {
        return true; // 테스트에서는 항상 시리얼이 사용 가능하다고 가정
    }

    // available() 함수 추가
    int available()
    {
        // 테스트를 위해 항상 0을 반환하거나, 필요에 따라 특정 값을 반환하도록 Mocking 가능
        return 0;
    }

    // read() 함수 추가
    int read()
    {
        // 테스트를 위해 항상 -1을 반환하거나, 필요에 따라 특정 문자를 반환하도록 Mocking 가능
        return -1;
    }

    // readStringUntil() 함수 추가
    String readStringUntil(char terminator)
    {
        // 테스트를 위해 빈 문자열을 반환하거나, 필요에 따라 특정 문자열을 반환하도록 Mocking 가능
        return "";
    }

    void print(const char *msg) { printf("%s", msg); }
    void println(const char *msg) { printf("%s\n", msg); }
    void print(String msg) { printf("%s", msg.c_str()); }
    void println(String msg) { printf("%s\n", msg.c_str()); }
    void println() { printf("\n"); }
};

static FakeSerial Serial; // 전역 가짜 Serial 객체

// DeviceAddress 타입을 테스트 코드에서도 사용할 수 있도록 정의
typedef uint8_t DeviceAddress[8];

// millis() 함수를 흉내 내는 가짜 함수 선언
unsigned long millis();
void setFakeMillis(unsigned long val);

// delay() 함수를 흉내 내는 가짜 함수 선언
void delay(unsigned long ms);

// isDigit 함수 (Arduino의 isDigit과 유사하게 동작)
inline bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// __FlashStringHelper 클래스 (Arduino PROGMEM 지원용)
class __FlashStringHelper
{
public:
    __FlashStringHelper() {}
    // 실제 Arduino에서는 PROGMEM 데이터를 처리하지만,
    // 테스트 환경에서는 일반 문자열로 처리
};

// PROGMEM 매크로 (테스트 환경에서는 빈 정의)
#define PROGMEM

// F() 매크로 (테스트 환경에서는 일반 문자열로 처리)
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))

#endif // FAKE_ARDUINO_H

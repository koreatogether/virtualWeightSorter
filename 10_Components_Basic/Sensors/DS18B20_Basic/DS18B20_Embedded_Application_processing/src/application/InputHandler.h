#pragma once
#include <Arduino.h>

// 안전한 Serial 입력 처리를 위한 클래스
class InputHandler
{
public:
    InputHandler();
    
    // 안전한 입력 처리 메서드
    bool processSerialInput(String& outputBuffer);
    void clearInputBuffer();
    
    // 입력 검증 메서드
    static bool isValidMenuChar(char c);
    static bool isValidSensorIndex(char c);
    
private:
    // 안전한 Serial 읽기를 위한 헬퍼 함수
    static bool safeSerialRead(char& outChar, int& attempts);
    
    // 보안 설정
    static const int MAX_CHARS_PER_CALL = 16;
    static const int MAX_INPUT_LENGTH = 8;
    static const unsigned long MAX_PROCESSING_TIME_MS = 5;
    static const int MAX_CONSECUTIVE_FAILURES = 5;
};
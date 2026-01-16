#include "SimpleTest.h"
#include <Arduino.h>
// 테스트할 함수들을 위한 분리된 로직
bool isValidPasswordFormat(String input)
{
    return input.length() == 8 &&
           input.startsWith("*") &&
           (input.charAt(7) == '#' || input.charAt(7) == '*');
}

String extractPassword(String input)
{
    if (!isValidPasswordFormat(input))
    {
        return "";
    }
    return input.substring(1, 7);
}

bool isPasswordCorrect(String password, String correctPassword)
{
    return password == correctPassword;
}

bool isDigitOnly(char c)
{
    return c >= '0' && c <= '9';
}

bool isValidPasswordChar(char c, int position)
{
    if (position == 0)
        return c == '*';
    if (position >= 1 && position <= 6)
        return isDigitOnly(c);
    if (position == 7)
        return c == '#' || c == '*';
    return false;
}

// 테스트 케이스들
void test_password_format_validation()
{
    // 올바른 형식
    SimpleTest::assertTrue(isValidPasswordFormat("*123456#"), "Valid format with #");
    SimpleTest::assertTrue(isValidPasswordFormat("*123456*"), "Valid format with *");
    SimpleTest::assertTrue(isValidPasswordFormat("*000000#"), "Valid format all zeros");
    SimpleTest::assertTrue(isValidPasswordFormat("*999999*"), "Valid format all nines");

    // 잘못된 형식
    SimpleTest::assertFalse(isValidPasswordFormat("123456#"), "Missing start *");
    SimpleTest::assertFalse(isValidPasswordFormat("*12345#"), "Too short");
    SimpleTest::assertFalse(isValidPasswordFormat("*1234567#"), "Too long");
    SimpleTest::assertFalse(isValidPasswordFormat("*123456"), "Missing end char");
    SimpleTest::assertFalse(isValidPasswordFormat("*123456A"), "Wrong end char");
    SimpleTest::assertFalse(isValidPasswordFormat(""), "Empty string");
}

void test_password_extraction()
{
    SimpleTest::assertEqual(extractPassword("*123456#"), "123456", "Extract valid password #");
    SimpleTest::assertEqual(extractPassword("*000000*"), "000000", "Extract valid password *");
    SimpleTest::assertEqual(extractPassword("*999999#"), "999999", "Extract all nines");

    // 잘못된 형식의 경우 빈 문자열 반환
    SimpleTest::assertEqual(extractPassword("123456#"), "", "Invalid format returns empty");
    SimpleTest::assertEqual(extractPassword("*12345#"), "", "Too short returns empty");
    SimpleTest::assertEqual(extractPassword(""), "", "Empty input returns empty");
}

void test_password_validation()
{
    String correctPassword = "123456";

    // 올바른 비밀번호
    SimpleTest::assertTrue(isPasswordCorrect("123456", correctPassword), "Correct password matches");

    // 잘못된 비밀번호
    SimpleTest::assertFalse(isPasswordCorrect("123457", correctPassword), "Wrong password fails");
    SimpleTest::assertFalse(isPasswordCorrect("654321", correctPassword), "Reversed password fails");
    SimpleTest::assertFalse(isPasswordCorrect("", correctPassword), "Empty password fails");
    SimpleTest::assertFalse(isPasswordCorrect("1234567", correctPassword), "Too long password fails");
}

void test_digit_validation()
{
    // 숫자 문자 검증
    SimpleTest::assertTrue(isDigitOnly('0'), "Digit 0 is valid");
    SimpleTest::assertTrue(isDigitOnly('5'), "Digit 5 is valid");
    SimpleTest::assertTrue(isDigitOnly('9'), "Digit 9 is valid");

    // 숫자가 아닌 문자
    SimpleTest::assertFalse(isDigitOnly('a'), "Letter a is not digit");
    SimpleTest::assertFalse(isDigitOnly('A'), "Letter A is not digit");
    SimpleTest::assertFalse(isDigitOnly('*'), "Star is not digit");
    SimpleTest::assertFalse(isDigitOnly('#'), "Hash is not digit");
}

void test_character_position_validation()
{
    // 위치별 문자 검증
    SimpleTest::assertTrue(isValidPasswordChar('*', 0), "Star at start position");
    SimpleTest::assertTrue(isValidPasswordChar('1', 1), "Digit at first position");
    SimpleTest::assertTrue(isValidPasswordChar('9', 6), "Digit at last position");
    SimpleTest::assertTrue(isValidPasswordChar('#', 7), "Hash at end position");
    SimpleTest::assertTrue(isValidPasswordChar('*', 7), "Star at end position");

    // 잘못된 위치-문자 조합
    SimpleTest::assertFalse(isValidPasswordChar('1', 0), "Digit at start position");
    SimpleTest::assertFalse(isValidPasswordChar('*', 1), "Star at digit position");
    SimpleTest::assertFalse(isValidPasswordChar('A', 7), "Letter at end position");
}

void test_edge_cases()
{
    // 경계값 테스트
    SimpleTest::assertTrue(isValidPasswordFormat("*000000#"), "All zeros format");
    SimpleTest::assertTrue(isValidPasswordFormat("*999999*"), "All nines format");

    // 특수 케이스
    SimpleTest::assertFalse(isValidPasswordFormat("**123456#"), "Double start character");
    SimpleTest::assertFalse(isValidPasswordFormat("*123456##"), "Double end character");
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
    }

    SimpleTest::begin("Password Logic Unit Tests");

    SimpleTest::runTest(test_password_format_validation, "Password Format Validation");
    SimpleTest::runTest(test_password_extraction, "Password Extraction");
    SimpleTest::runTest(test_password_validation, "Password Validation");
    SimpleTest::runTest(test_digit_validation, "Digit Validation");
    SimpleTest::runTest(test_character_position_validation, "Character Position Validation");
    SimpleTest::runTest(test_edge_cases, "Edge Cases");

    SimpleTest::summary();
}

void loop()
{
    // 테스트 완료
}
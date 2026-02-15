#include "unity_custom_output.h"
#include "unity/unity.h"
#include <stdio.h>
#include <string.h>

/* Global output file pointer */
FILE *unity_output_file = NULL;

/* Variables to store current test information */
static char current_test_name[256] = {0};
static char current_test_file[256] = {0};
static int current_test_line = 0;
static int current_suite_tests = 0;
static int current_suite_failures = 0;
static int current_suite_ignored = 0;

/* Test result constants */
#define TEST_RESULT_PASS 0
#define TEST_RESULT_FAIL 1
#define TEST_RESULT_IGNORE 2

/* Output file initialization function */
void unity_custom_init_file_output(const char *filename)
{
    if (unity_output_file == NULL)
    {
        unity_output_file = fopen(filename, "w");
        if (unity_output_file == NULL)
        {
            printf("Error opening unity output file: %s\n", filename);
        }
    }

    // XML 출력 파일도 초기화
    unity_xml_init("test_result/unity_test_output.xml");
}

/* Output file close function */
void unity_custom_close_file_output(void)
{
    if (unity_output_file != NULL)
    {
        fclose(unity_output_file);
        unity_output_file = NULL;
    }

    // XML 출력 파일도 닫기
    unity_xml_close();
}

/* Unity output character handler */
void unity_custom_output_char(int character)
{
    // 표준 출력에 출력
    putchar(character);

    // 파일에도 출력 (가능한 경우)
    if (unity_output_file != NULL)
    {
        fputc(character, unity_output_file);
        // 개행 문자일 경우 버퍼 플러시
        if (character == '\n')
        {
            fflush(unity_output_file);
        }
    }
}

/* Unity 출력 문자열 핸들러 */
void unity_custom_output_string(const char *string)
{
    if (string == NULL)
    {
        return;
    }

    while (*string)
    {
        unity_custom_output_char(*string);
        string++;
    }
}

/* Unity 출력 플러시 핸들러 */
void unity_custom_output_flush(void)
{
    fflush(stdout);
    if (unity_output_file != NULL)
    {
        fflush(unity_output_file);
    }
}

/* 메시지 출력 함수 */
void unity_custom_print_message(const char *message)
{
    unity_custom_output_string(message);
    unity_custom_output_char('\n');
    unity_custom_output_flush();
}

/* Unity 시작 콜백 */
int unity_custom_begin(void)
{
    // 테스트 시작 시 XML 테스트 스위트 시작 태그 작성
    unity_xml_begin_suite("Unity Test Suite");

    // 카운터 초기화
    current_suite_tests = 0;
    current_suite_failures = 0;
    current_suite_ignored = 0;

    // Unity 기본 시작 함수 호출
    UnityBegin(__FILE__);
    return 0;
}

/* Unity 종료 콜백 */
int unity_custom_end(void)
{
    // 테스트 종료 시 XML 테스트 스위트 종료 태그 작성
    unity_xml_end_suite(current_suite_tests, current_suite_failures, current_suite_ignored);

    // Unity 기본 종료 함수 호출
    return UnityEnd();
}

/* 현재 테스트 상세 정보 설정 */
void unity_custom_set_current_test(const char *test_name, const char *file, int line)
{
    if (test_name)
    {
        strncpy(current_test_name, test_name, sizeof(current_test_name) - 1);
        current_test_name[sizeof(current_test_name) - 1] = '\0';
    }

    if (file)
    {
        strncpy(current_test_file, file, sizeof(current_test_file) - 1);
        current_test_file[sizeof(current_test_file) - 1] = '\0';
    }

    current_test_line = line;

    // XML 출력에도 테스트 시작을 기록
    unity_xml_begin_test(test_name, file, line);

    // 테스트 카운터 증가
    current_suite_tests++;
}

/* 테스트 결과 기록 함수 */
void unity_custom_record_test_result(int result, const char *message, const char *file, int line)
{
    switch (result)
    {
    case TEST_RESULT_PASS:
        // 테스트 성공
        unity_xml_end_test_pass();
        break;
    case TEST_RESULT_FAIL:
        // 테스트 실패
        unity_xml_end_test_fail(message ? message : "Test failed",
                                file ? file : current_test_file,
                                line ? line : current_test_line);
        current_suite_failures++;
        break;
    case TEST_RESULT_IGNORE:
        // 테스트 무시
        unity_xml_end_test_ignore(message ? message : "Test ignored");
        current_suite_ignored++;
        break;
    }
}

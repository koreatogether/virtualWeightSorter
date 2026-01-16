#include "unity_xml_output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 글로벌 파일 포인터
static FILE *xml_output_file = NULL;
static int current_test_failed = 0;
static int current_suite_tests = 0;
static int current_suite_failures = 0;
static int current_suite_ignored = 0;
static char current_test_name[256] = {0};
static time_t test_start_time;

// XML 파일 초기화
void unity_xml_init(const char *filename)
{
    if (xml_output_file != NULL)
    {
        fclose(xml_output_file);
    }

    // 필요한 디렉토리 확인 및 생성
    char dir_path[256] = {0};
    char *last_slash = strrchr(filename, '/');
    if (last_slash != NULL)
    {
        strncpy(dir_path, filename, last_slash - filename);
        dir_path[last_slash - filename] = '\0';

// 간단한 디렉토리 생성 (Windows와 Linux 모두 작동)
#ifdef _WIN32
        char cmd[512];
        sprintf(cmd, "mkdir -p \"%s\"", dir_path);
        system(cmd);
#else
        char cmd[512];
        sprintf(cmd, "mkdir -p \"%s\"", dir_path);
        system(cmd);
#endif
    }

    xml_output_file = fopen(filename, "w");
    if (xml_output_file == NULL)
    {
        printf("Error: Could not open XML output file: %s\n", filename);
        return;
    }

    // XML 헤더 작성
    fprintf(xml_output_file, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(xml_output_file, "<testsuites>\n");
    fflush(xml_output_file);
}

// 테스트 스위트 시작
void unity_xml_begin_suite(const char *suite_name)
{
    if (xml_output_file == NULL)
        return;

    // 테스트 스위트 태그 시작
    fprintf(xml_output_file, "  <testsuite name=\"%s\">\n", suite_name);
    fflush(xml_output_file);

    // 스위트 카운터 초기화
    current_suite_tests = 0;
    current_suite_failures = 0;
    current_suite_ignored = 0;
}

// 테스트 케이스 시작
void unity_xml_begin_test(const char *test_name, const char *filename, int line)
{
    if (xml_output_file == NULL)
        return;

    // 현재 테스트 이름 저장
    strncpy(current_test_name, test_name, sizeof(current_test_name) - 1);
    current_test_name[sizeof(current_test_name) - 1] = '\0';

    // 테스트 케이스 시작 시간 기록
    time(&test_start_time);

    // 테스트 카운터 증가
    current_suite_tests++;
    current_test_failed = 0;
}

// 테스트 케이스 종료 (성공)
void unity_xml_end_test_pass()
{
    if (xml_output_file == NULL)
        return;

    // 테스트 경과 시간 계산 (초 단위)
    time_t test_end_time;
    time(&test_end_time);
    double elapsed_time = difftime(test_end_time, test_start_time);

    // 성공 테스트 케이스 출력
    fprintf(xml_output_file, "    <testcase name=\"%s\" time=\"%.3f\" />\n",
            current_test_name, elapsed_time);
    fflush(xml_output_file);
}

// 테스트 케이스 종료 (실패)
void unity_xml_end_test_fail(const char *message, const char *filename, int line)
{
    if (xml_output_file == NULL)
        return;

    // 테스트 경과 시간 계산 (초 단위)
    time_t test_end_time;
    time(&test_end_time);
    double elapsed_time = difftime(test_end_time, test_start_time);

    // 실패 테스트 케이스 출력
    fprintf(xml_output_file, "    <testcase name=\"%s\" time=\"%.3f\">\n",
            current_test_name, elapsed_time);
    fprintf(xml_output_file, "      <failure message=\"%s\">\n", message);
    fprintf(xml_output_file, "        <![CDATA[\n");
    fprintf(xml_output_file, "          File: %s\n", filename);
    fprintf(xml_output_file, "          Line: %d\n", line);
    fprintf(xml_output_file, "        ]]>\n");
    fprintf(xml_output_file, "      </failure>\n");
    fprintf(xml_output_file, "    </testcase>\n");
    fflush(xml_output_file);

    // 실패 카운터 증가
    current_suite_failures++;
    current_test_failed = 1;
}

// 테스트 케이스 종료 (무시됨)
void unity_xml_end_test_ignore(const char *message)
{
    if (xml_output_file == NULL)
        return;

    // 테스트 경과 시간 계산 (초 단위)
    time_t test_end_time;
    time(&test_end_time);
    double elapsed_time = difftime(test_end_time, test_start_time);

    // 무시된 테스트 케이스 출력
    fprintf(xml_output_file, "    <testcase name=\"%s\" time=\"%.3f\">\n",
            current_test_name, elapsed_time);
    fprintf(xml_output_file, "      <skipped message=\"%s\" />\n", message);
    fprintf(xml_output_file, "    </testcase>\n");
    fflush(xml_output_file);

    // 무시 카운터 증가
    current_suite_ignored++;
}

// 테스트 스위트 종료
void unity_xml_end_suite(int num_tests, int num_failures, int num_ignored)
{
    if (xml_output_file == NULL)
        return;

    // 테스트 스위트 태그 종료
    fprintf(xml_output_file, "  </testsuite>\n");
    fflush(xml_output_file);
}

// XML 출력 종료 및 파일 닫기
void unity_xml_close()
{
    if (xml_output_file == NULL)
        return;

    // XML 종료 태그 작성
    fprintf(xml_output_file, "</testsuites>\n");
    fflush(xml_output_file);

    // 파일 닫기
    fclose(xml_output_file);
    xml_output_file = NULL;
}

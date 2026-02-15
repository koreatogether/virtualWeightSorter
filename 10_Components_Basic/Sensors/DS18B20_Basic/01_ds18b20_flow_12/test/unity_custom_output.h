#ifndef UNITY_CUSTOM_OUTPUT_H
#define UNITY_CUSTOM_OUTPUT_H

#include <stdio.h>
#include "unity_xml_output.h"

/* 테스트 결과 상수 */
#define TEST_RESULT_PASS 0
#define TEST_RESULT_FAIL 1
#define TEST_RESULT_IGNORE 2

/* Unity 테스트 프레임워크의 출력을 직접 파일로 리디렉션하기 위한 설정 */
extern FILE *unity_output_file;

/* 출력 파일 초기화 함수 */
void unity_custom_init_file_output(const char *filename);

/* 출력 파일 닫기 함수 */
void unity_custom_close_file_output(void);

/* Unity 출력 문자 핸들러 */
void unity_custom_output_char(int character);

/* Unity 출력 문자열 핸들러 */
void unity_custom_output_string(const char *string);

/* Unity 출력 플러시 핸들러 */
void unity_custom_output_flush(void);

/* 메시지 출력 함수 */
void unity_custom_print_message(const char *message);

/* Unity 시작 및 종료 콜백 */
int unity_custom_begin(void);
int unity_custom_end(void);

/* 현재 테스트 상세 정보 설정 */
void unity_custom_set_current_test(const char *test_name, const char *file, int line);

/* 테스트 결과 기록 함수 */
void unity_custom_record_test_result(int result, const char *message, const char *file, int line);

#endif /* UNITY_CUSTOM_OUTPUT_H */

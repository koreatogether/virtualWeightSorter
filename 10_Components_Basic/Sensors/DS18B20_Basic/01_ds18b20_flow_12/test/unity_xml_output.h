#ifndef UNITY_XML_OUTPUT_H
#define UNITY_XML_OUTPUT_H

/**
 * @file unity_xml_output.h
 * @brief XML Output 핸들러를 Unity Test Framework에 제공합니다.
 */

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief XML 출력 초기화
     * @param filename 출력 파일 이름
     */
    void unity_xml_init(const char *filename);

    /**
     * @brief 테스트 스위트 시작 태그 작성
     * @param suite_name 테스트 스위트 이름
     */
    void unity_xml_begin_suite(const char *suite_name);

    /**
     * @brief 테스트 케이스 시작 태그 작성
     * @param test_name 테스트 이름
     * @param filename 테스트가 있는 파일 이름
     * @param line 테스트가 있는 라인 번호
     */
    void unity_xml_begin_test(const char *test_name, const char *filename, int line);

    /**
     * @brief 테스트 케이스 종료 태그 작성 (성공)
     */
    void unity_xml_end_test_pass();

    /**
     * @brief 테스트 케이스 종료 태그 작성 (실패)
     * @param message 실패 메시지
     * @param filename 실패가 발생한 파일
     * @param line 실패가 발생한 라인
     */
    void unity_xml_end_test_fail(const char *message, const char *filename, int line);

    /**
     * @brief 테스트 케이스 종료 태그 작성 (무시됨)
     * @param message 무시 이유
     */
    void unity_xml_end_test_ignore(const char *message);

    /**
     * @brief 테스트 스위트 종료 태그 작성
     * @param num_tests 총 테스트 수
     * @param num_failures 실패 테스트 수
     * @param num_ignored 무시된 테스트 수
     */
    void unity_xml_end_suite(int num_tests, int num_failures, int num_ignored);

    /**
     * @brief XML 출력 종료 및 파일 닫기
     */
    void unity_xml_close();

#ifdef __cplusplus
}
#endif

#endif /* UNITY_XML_OUTPUT_H */

#include "unity_config.h" // Unity configuration file - must be included first
#include "unity/unity.h"
#include <cstring>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h> // For mkdir on Windows
#endif
#include <ctime>
#include "TestGlobals.h"         // Include TestGlobals.h to bring in extern declarations
#include "unity_custom_output.h" // Custom output handlers
#include "unity_xml_output.h"    // XML output handlers

// 전역 변수 정의 및 초기화
MockDS18B20_Sensor *mockSensorManager = nullptr;
MockCommunicator *mockComm = nullptr;
MockDataProcessor *mockDataProcessor = nullptr;
App *app = nullptr;

// Declare test runner functions from other files
extern int run_app_menu_tests();
extern int run_scenario1_tests();
extern int run_utils_tests();
extern int run_scenario5_tests();
extern int run_scenario6_tests();
extern int run_scenario7_tests();
extern int run_scenario8_tests();
extern int run_scenario2_tests();
extern int run_scenario3_tests();
extern int run_scenario4_tests();
extern int run_scenario9_tests();
extern int run_scenario9_v2_tests();
extern void test_scenario11_menu_cancel();
extern void run_scenario_total_01_tests(); // Full regression test added

// Function to check if directory exists, create if not
void ensure_directory_exists(const char *path)
{
#ifdef _WIN32
    mkdir(path);
#else
    mkdir(path, 0777);
#endif
}

int main(int argc, char **argv)
{
    // test_result 디렉토리 생성
    ensure_directory_exists("test_result");

    // 결과 파일명 결정
    const char *result_file = "test_result/unity_test_output.txt";
    const char *direct_file = "test_result/direct_test_output.txt";
    if (argc >= 2)
    {
        if (strcmp(argv[1], "scenario1") == 0)
        {
            result_file = "test_result/test_result_scenario1.txt";
            direct_file = "test_result/direct_test_output_scenario1.txt";
        }
        else if (strcmp(argv[1], "scenario2") == 0)
        {
            result_file = "test_result/test_result_scenario2.txt";
            direct_file = "test_result/direct_test_output_scenario2.txt";
        }
        else if (strcmp(argv[1], "scenario3") == 0)
        {
            result_file = "test_result/test_result_scenario3.txt";
            direct_file = "test_result/direct_test_output_scenario3.txt";
        }
        else if (strcmp(argv[1], "scenario4") == 0)
        {
            result_file = "test_result/test_result_scenario4.txt";
            direct_file = "test_result/direct_test_output_scenario4.txt";
        }
        else if (strcmp(argv[1], "scenario9_v2") == 0)
        {
            result_file = "test_result/test_result_scenario9_v2.txt";
            direct_file = "test_result/direct_test_output_scenario9_v2.txt";
        }
        else if (strcmp(argv[1], "scenario11") == 0)
        {
            result_file = "test_result/test_result_scenario11.txt";
            direct_file = "test_result/direct_test_output_scenario11.txt";
        }
        else if (strcmp(argv[1], "scenario_total_01") == 0)
        {
            result_file = "test_result/test_result_scenario_total_01.txt";
            direct_file = "test_result/direct_test_output_scenario_total_01.txt";
        }
        else if (strcmp(argv[1], "app_menu") == 0)
        {
            result_file = "test_result/test_result_app_menu.txt";
            direct_file = "test_result/direct_test_output_app_menu.txt";
        }
        else if (strcmp(argv[1], "utils") == 0)
        {
            result_file = "test_result/test_result_utils.txt";
            direct_file = "test_result/direct_test_output_utils.txt";
        }
        else if (strcmp(argv[1], "all") == 0 || argc < 2)
        {
            result_file = "test_result/test_result_all.txt";
            direct_file = "test_result/direct_test_output_all.txt";
        }
    }

    // 유니티 테스트 출력 파일 초기화 (표준 출력 + 파일 출력 + XML 출력)
    unity_custom_init_file_output(result_file);
    // 시작 시간 기록 (result_file)
    FILE *rf = fopen(result_file, "a");
    if (rf)
    {
        time_t start_time = time(nullptr);
        char time_buf[64];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&start_time));
        fprintf(rf, "Test start time: %s\n", time_buf);
        fclose(rf);
    }

    // 직접 파일 출력도 유지 (기존 코드와의 호환성 및 추가 디버깅용)
    FILE *direct_output = fopen(direct_file, "w");
    if (direct_output)
    {
        fprintf(direct_output, "Test direct file output start\n");
        // Record start time
        time_t start_time = time(nullptr);
        char time_buf[64];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&start_time));
        fprintf(direct_output, "Test start time: %s\n", time_buf);
        fflush(direct_output);
    }

    unity_custom_print_message("===== TEST START =====");
    printf("Test Start\n");
    fflush(stdout);

    if (argc < 2 || strcmp(argv[1], "all") == 0)
    {
        int result = 0;
        // XML 출력 시작 - 전체 테스트 스위트
        unity_xml_begin_suite("AllTests");

        unity_custom_print_message("Running app_menu tests...");
        printf("Running app_menu tests...\n");
        if (direct_output)
        {
            fprintf(direct_output, "Running app_menu tests...\n");
            fflush(direct_output);
        }
        result |= run_app_menu_tests();

        unity_custom_print_message("Running scenario1 tests...");
        printf("Running scenario1 tests...\n");
        if (direct_output)
        {
            fprintf(direct_output, "Running scenario1 tests...\n");
            fflush(direct_output);
        }
        result |= run_scenario1_tests();

        unity_custom_print_message("Running scenario9 tests...");
        printf("Running scenario9 tests...\n");
        if (direct_output)
        {
            fprintf(direct_output, "Running scenario9 tests...\n");
            fflush(direct_output);
        }
        result |= run_scenario9_tests();

        unity_custom_print_message("Running scenario11 (menu cancel) tests...");
        printf("Running scenario11 (menu cancel) tests...\n");
        if (direct_output)
        {
            fprintf(direct_output, "Running scenario11 (menu cancel) tests...\n");
            fflush(direct_output);
        }
        test_scenario11_menu_cancel();

        unity_custom_print_message("Running scenario_total_01 (full regression) tests...");
        printf("Running scenario_total_01 (full regression) tests...\n");
        if (direct_output)
        {
            fprintf(direct_output, "Running scenario_total_01 (full regression) tests...\n");
            fflush(direct_output);
        }
        run_scenario_total_01_tests();

        unity_custom_print_message("Running utils tests...");
        printf("Running utils tests...\n");
        if (direct_output)
        {
            fprintf(direct_output, "Running utils tests...\n");
            fflush(direct_output);
        }
        result |= run_utils_tests();

        // XML 출력 종료 - 전체 테스트 스위트
        unity_xml_end_suite(0, 0, 0); // 총계는 각 개별 테스트 실행에서 계산됨

        unity_custom_print_message("===== 테스트 완료 =====");
        if (direct_output)
        {
            // 종료 시간 기록
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(direct_output, "테스트 종료 시간: %s\n", time_buf);
            fprintf(direct_output, "테스트 완료, 결과: %d\n", result);
            fclose(direct_output);
        }

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }
        // 유니티 출력 파일 닫기
        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "app_menu") == 0)
    {
        unity_custom_print_message("Running app_menu tests...");
        printf("Running app_menu tests...\n");

        // XML 출력 시작 - app_menu 테스트 스위트
        unity_xml_begin_suite("AppMenuTests");

        int result = run_app_menu_tests();

        // XML 출력 종료 - app_menu 테스트 스위트
        unity_xml_end_suite(0, 0, 0); // 총계는 테스트 실행에서 계산됨

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }
        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario1") == 0)
    {
        unity_custom_print_message("Running scenario1 tests...");
        printf("Running scenario1 tests...\n");

        // XML 출력 시작 - scenario1 테스트 스위트
        unity_xml_begin_suite("Scenario1Tests");

        int result = run_scenario1_tests();

        // XML 출력 종료 - scenario1 테스트 스위트
        unity_xml_end_suite(0, 0, 0); // 총계는 테스트 실행에서 계산됨

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario5") == 0)
    {
        unity_custom_print_message("Running scenario5 tests...");
        printf("Running scenario5 tests...\n");
        unity_xml_begin_suite("Scenario5Tests");
        int result = run_scenario5_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario6") == 0)
    {
        unity_custom_print_message("Running scenario6 tests...");
        printf("Running scenario6 tests...\n");
        unity_xml_begin_suite("Scenario6Tests");
        int result = run_scenario6_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario7") == 0)
    {
        unity_custom_print_message("Running scenario7 tests...");
        printf("Running scenario7 tests...\n");
        unity_xml_begin_suite("Scenario7Tests");
        int result = run_scenario7_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario8") == 0)
    {
        unity_custom_print_message("Running scenario8 tests...");
        printf("Running scenario8 tests...\n");
        unity_xml_begin_suite("Scenario8Tests");
        int result = run_scenario8_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario2") == 0)
    {
        unity_custom_print_message("Running scenario2 tests...");
        printf("Running scenario2 tests...\n");

        // XML 출력 시작 - scenario2 테스트 스위트
        unity_xml_begin_suite("Scenario2Tests");

        int result = run_scenario2_tests();

        // XML 출력 종료 - scenario2 테스트 스위트
        unity_xml_end_suite(0, 0, 0); // 총계는 테스트 실행에서 계산됨

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario3") == 0)
    {
        unity_custom_print_message("Running scenario3 tests...");
        printf("Running scenario3 tests...\n");

        // XML 출력 시작 - scenario3 테스트 스위트
        unity_xml_begin_suite("Scenario3Tests");

        int result = run_scenario3_tests();

        // XML 출력 종료 - scenario3 테스트 스위트
        unity_xml_end_suite(0, 0, 0); // 총계는 테스트 실행에서 계산됨

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario4") == 0)
    {
        unity_custom_print_message("Running scenario4 tests...");
        printf("Running scenario4 tests...\n");
        unity_xml_begin_suite("Scenario4Tests");
        int result = run_scenario4_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario9") == 0)
    {
        unity_custom_print_message("Running scenario9 tests...");
        printf("Running scenario9 tests...\n");
        unity_xml_begin_suite("Scenario9Tests");
        int result = run_scenario9_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario9_v2") == 0)
    {
        unity_custom_print_message("Running scenario9_v2 tests...");
        printf("Running scenario9_v2 tests...\n");
        unity_xml_begin_suite("Scenario9_v2Tests");
        int result = run_scenario9_v2_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else if (strcmp(argv[1], "scenario11") == 0)
    {
        unity_custom_print_message("Running scenario11 (menu cancel) tests...");
        printf("Running scenario11 (menu cancel) tests...\n");
        unity_xml_begin_suite("Scenario11Tests");
        test_scenario11_menu_cancel();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return 0;
    }
    else if (strcmp(argv[1], "scenario_total_01") == 0)
    {
        unity_custom_print_message("Running scenario_total_01 (full regression) tests...");
        printf("Running scenario_total_01 (full regression) tests...\n");
        unity_xml_begin_suite("ScenarioTotal01Tests");
        run_scenario_total_01_tests();
        unity_xml_end_suite(0, 0, 0);

        // 종료 시간 기록
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return 0;
    }
    else if (strcmp(argv[1], "utils") == 0)
    {
        unity_custom_print_message("Running utils tests...");
        printf("Running utils tests...\n");

        // XML 출력 시작 - utils 테스트 스위트
        unity_xml_begin_suite("UtilsTests");

        int result = run_utils_tests();

        // XML 출력 종료 - utils 테스트 스위트
        unity_xml_end_suite(0, 0, 0); // 총계는 테스트 실행에서 계산됨

        // 종료 시간 기록 (result_file)
        FILE *rf = fopen(result_file, "a");
        if (rf)
        {
            time_t end_time = time(nullptr);
            char time_buf[64];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            fprintf(rf, "테스트 종료 시간: %s\n", time_buf);
            fclose(rf);
        }

        unity_custom_close_file_output();
        return result;
    }
    else
    {
        unity_custom_print_message("Invalid command line arguments");
        printf("Usage: %s [all|app_menu|scenario1|scenario9|scenario11|scenario_total_01|utils]\n", argv[0]);
        unity_custom_close_file_output();
        return 1;
    }
}

// Unity 테스트 케이스 시작 전 호출되는 함수
void setUp(void)
{
    // 현재 실행 중인 테스트 정보 가져오기
    const char *test_name = Unity.CurrentTestName;
    const char *test_file = __FILE__; // Use current file as we can't access Unity.CurrentTestFileName
    int test_line = __LINE__;         // Use current line as we can't access Unity.CurrentTestLineNumber

    // 현재 테스트 정보 설정 및 XML 테스트 케이스 시작 기록
    unity_custom_set_current_test(test_name, test_file, test_line);

    // Output test start message
    unity_custom_print_message("Test case start");
    if (test_name)
    {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "Running test: %s", test_name);
        unity_custom_print_message(buffer);
    }
}

// Unity 테스트 케이스 종료 후 호출되는 함수
void tearDown(void)
{
    // 테스트 결과 기록
    int result = Unity.CurrentTestFailed ? TEST_RESULT_FAIL : TEST_RESULT_PASS;

    // 테스트 결과 상태 문자열로 변환
    const char *result_str = result == TEST_RESULT_PASS ? "PASS" : "FAIL";

    // 실패 메시지 처리 (Unity에서 실패 메시지는 직접 접근할 수 없음)
    const char *fail_msg = Unity.CurrentTestFailed ? "Test failed" : "Test passed";

    // Improved test result output
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Test case end: %s (%s)",
             Unity.CurrentTestName, result_str);
    unity_custom_print_message(buffer);

    if (Unity.CurrentTestFailed)
    {
        snprintf(buffer, sizeof(buffer), "Failure reason: %s",
                 "See test output for detailed failure information");
        unity_custom_print_message(buffer);
    }

    // Record the result
    unity_custom_record_test_result(result, fail_msg, NULL, 0);

    // Also record the result to standard output
    printf("Test '%s' result: %s\n", Unity.CurrentTestName, result_str);
    fflush(stdout);
}

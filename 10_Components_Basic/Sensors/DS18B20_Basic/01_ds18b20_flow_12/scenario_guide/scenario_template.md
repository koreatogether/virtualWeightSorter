# 시나리오 테스트 템플릿

## test_scenarioX.cpp 템플릿

```cpp
/**
 * @file test_scenarioX.cpp
 * @brief Scenario X 테스트 구현
 * @description [시나리오 설명을 여기에 작성]
 */

#include "unity.h"
#include "TestGlobals.h"
#include "MockDS18B20_Sensor.h"
#include "MockCommunicator.h" 
#include "MockDataProcessor.h"
#include "App.h"

/**
 * @brief 시나리오 X 메인 테스트 함수
 * @description [테스트 내용 설명]
 */
void test_scenarioX_main_functionality(void)
{
    // 테스트 시작 로그
    unity_custom_print_message("===== 테스트 시작 =====");
    printf("[SCENARIOX] 테스트 시작\n");
    
    // Mock 객체 초기화 (필요한 경우)
    // MockDS18B20_Sensor mockSensor;
    // MockCommunicator mockComm;
    // MockDataProcessor mockProcessor;
    
    // 테스트 로직 구현
    // 예시:
    // App app;
    // app.initialize();
    // 실제 테스트 수행...
    
    // 간단한 성공 확인 (실제 구현 시 적절한 테스트로 변경)
    TEST_ASSERT(1);
    
    // 테스트 완료 로그
    printf("[SCENARIOX] 테스트 성공\n");
    unity_custom_print_message("===== 테스트 종료 =====");
}

/**
 * @brief 시나리오 X 테스트 실행 함수
 * @return 테스트 결과 (0: 성공, 1: 실패)
 */
int run_scenarioX_tests(void)
{
    unity_custom_print_message("Running scenarioX tests...");
    printf("Running scenarioX tests...\n");
    
    // XML 출력 시작 - scenarioX 테스트 스위트
    unity_xml_begin_suite("ScenarioXTests");
    
    UNITY_BEGIN();
    RUN_TEST(test_scenarioX_main_functionality);
    int result = UNITY_END();
    
    // XML 출력 종료 - scenarioX 테스트 스위트  
    unity_xml_end_suite(0, 0, 0); // 총계는 테스트 실행에서 계산됨
    
    return result;
}
```

## test_main.cpp 추가 코드 템플릿

### 1. extern 선언 (파일 상단에 추가)
```cpp
extern int run_scenarioX_tests();
```

### 2. 결과 파일명 분기 (main 함수 내)
```cpp
else if (strcmp(argv[1], "scenarioX") == 0)
{
    result_file = "test_result/test_result_scenarioX.txt";
    direct_file = "test_result/direct_test_output_scenarioX.txt";
}
```

### 3. 실행 분기 (main 함수 내)
```cpp
else if (strcmp(argv[1], "scenarioX") == 0)
{
    unity_custom_print_message("Running scenarioX tests...");
    printf("Running scenarioX tests...\n");

    // XML 출력 시작 - scenarioX 테스트 스위트
    unity_xml_begin_suite("ScenarioXTests");

    int result = run_scenarioX_tests();

    // XML 출력 종료 - scenarioX 테스트 스위트
    unity_xml_end_suite(0, 0, 0); // 총계는 테스트 실행에서 계산됨

    unity_custom_close_file_output();
    return result;
}
```

## run_test.bat 추가 코드 템플릿

### 1. 컴파일 명령 (컴파일 섹션에 추가)
```batch
g++ -c -DTEST ..\test\test_scenarioX.cpp -o test_result\test_scenarioX.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenarioX.cpp. && pause && exit /b %errorlevel%)
```

### 2. 링킹 명령 (기존 링킹 명령에 추가)
```batch
# 기존: test_result\test_scenario3.o 다음에 추가
test_result\test_scenarioX.o
```

## 빠른 구현 가이드

### 1단계: 시나리오 문서 확인
```bash
# 시나리오 문서 위치
scenario/scenario_0X_[name].md
```

### 2단계: 테스트 파일 생성
1. 위 템플릿을 복사하여 `test/test_scenarioX.cpp` 생성
2. `X`를 실제 시나리오 번호로 변경
3. 테스트 함수명과 내용을 시나리오에 맞게 수정

### 3단계: test_main.cpp 수정
1. extern 선언 추가
2. 결과 파일명 분기 추가  
3. 실행 분기 추가

### 4단계: run_test.bat 수정
1. 컴파일 명령 추가
2. 링킹 명령에 .o 파일 추가

### 5단계: 테스트 실행
```batch
cd run_test
run_test.bat scenarioX
```

### 6단계: 결과 확인
```
test_result/test_result_scenarioX.txt
```

## 검증 체크리스트

- [ ] 컴파일 성공 (에러 없음)
- [ ] 링킹 성공 (undefined reference 없음)  
- [ ] 테스트 실행 성공
- [ ] 결과 파일 생성 확인
- [ ] 테스트 통과 확인 ("X Tests 0 Failures 0 Ignored OK")

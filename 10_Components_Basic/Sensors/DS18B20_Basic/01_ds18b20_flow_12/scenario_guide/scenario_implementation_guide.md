# 시나리오 테스트 구현 가이드 (Scenario 4-8)

## 📋 개요
시나리오 1, 2, 3 구현 과정에서 발견된 공통 패턴과 수정사항을 정리하여 시나리오 4~8 구현 시 참고할 수 있도록 작성된 가이드입니다.

## 🔄 공통 구현 패턴

### 1. 테스트 파일 생성 패턴
```cpp
// test_scenarioX.cpp 파일 구조
#include "unity.h"
#include "TestGlobals.h"
#include "MockDS18B20_Sensor.h"
#include "MockCommunicator.h"
#include "MockDataProcessor.h"
#include "App.h"

void test_specific_functionality(void)
{
    TEST_ASSERT(1); // 또는 실제 테스트 로직
}

int run_scenarioX_tests(void)
{
    unity_custom_print_message("Running scenarioX tests...");
    printf("Running scenarioX tests...\n");
    
    // XML 출력 시작
    unity_xml_begin_suite("ScenarioXTests");
    
    UNITY_BEGIN();
    RUN_TEST(test_specific_functionality);
    int result = UNITY_END();
    
    // XML 출력 종료
    unity_xml_end_suite(0, 0, 0);
    
    return result;
}
```

### 2. test_main.cpp 수정 사항

#### A. extern 선언 추가 (파일 상단)
```cpp
extern int run_scenario1_tests();
extern int run_scenario2_tests();
extern int run_scenario3_tests();
extern int run_scenario4_tests();  // 추가 필요
extern int run_scenario5_tests();  // 추가 필요
extern int run_scenario6_tests();  // 추가 필요
extern int run_scenario7_tests();  // 추가 필요
extern int run_scenario8_tests();  // 추가 필요
```

#### B. 결과 파일명 분기 추가
```cpp
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
// ... 기존 코드 ...
else if (strcmp(argv[1], "scenario4") == 0)  // 추가 필요
{
    result_file = "test_result/test_result_scenario4.txt";
    direct_file = "test_result/direct_test_output_scenario4.txt";
}
// scenario5~8 동일 패턴으로 추가
```

#### C. 실행 분기 추가
```cpp
else if (strcmp(argv[1], "scenario4") == 0)  // 추가 필요
{
    unity_custom_print_message("Running scenario4 tests...");
    printf("Running scenario4 tests...\n");

    // XML 출력 시작 - scenario4 테스트 스위트
    unity_xml_begin_suite("Scenario4Tests");

    int result = run_scenario4_tests();

    // XML 출력 종료 - scenario4 테스트 스위트
    unity_xml_end_suite(0, 0, 0);

    unity_custom_close_file_output();
    return result;
}
// scenario5~8 동일 패턴으로 추가
```

### 3. run_test.bat 수정 사항

#### A. 컴파일 명령 추가
```batch
g++ -c -DTEST ..\test\test_scenario4.cpp -o test_result\test_scenario4.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario4.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST ..\test\test_scenario5.cpp -o test_result\test_scenario5.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario5.cpp. && pause && exit /b %errorlevel%)

// scenario6~8 동일 패턴으로 추가
```

#### B. 링킹 명령에 오브젝트 파일 추가
```batch
g++ -o test_result\test_runner test_result\test_main.o test_result\test_utils.o test_result\test_app_menu.o test_result\test_scenario1.o test_result\test_scenario2.o test_result\test_scenario3.o test_result\test_scenario4.o test_result\test_scenario5.o test_result\test_scenario6.o test_result\test_scenario7.o test_result\test_scenario8.o test_result\App.o test_result\Communicator.o test_result\DataProcessor.o test_result\DS18B20_Sensor.o test_result\Utils.o test_result\unity.o test_result\FakeArduino.o test_result\unity_custom_output.o test_result\unity_xml_output.o
```

## ⚠️ 주요 문제점과 해결책

### 1. 링킹 에러: undefined reference
**문제**: `undefined reference to 'run_scenarioX_tests()'`
**원인**: 
- test_scenarioX.cpp 컴파일 누락
- 링킹 시 test_scenarioX.o 파일 누락
- test_main.cpp에 extern 선언 누락

**해결책**:
1. run_test.bat에 컴파일 명령 추가
2. 링킹 명령에 .o 파일 추가
3. test_main.cpp에 extern 선언 추가

### 2. 결과 파일 중복/덮어쓰기
**문제**: 모든 시나리오가 같은 결과 파일 사용
**해결책**: 시나리오별 고유 파일명 사용
- `test_result_scenario1.txt`, `test_result_scenario2.txt` 등

### 3. 컴파일 순서 중요
**문제**: 의존성 있는 파일들의 컴파일 순서
**해결책**: 
1. Unity 관련 파일 먼저 컴파일
2. 테스트 파일들 컴파일
3. 메인 소스 파일들 컴파일
4. 링킹 단계

## 🚀 시나리오 4-8 구현 체크리스트

### 각 시나리오별 체크리스트

#### ✅ Scenario 4 (selective_id_change)
- [ ] 시나리오 문서 확인: `scenario/scenario_04_selective_id_change.md`
- [ ] 테스트 파일 생성: `test/test_scenario4.cpp`
- [ ] test_main.cpp 수정:
  - [ ] extern 선언 추가
  - [ ] 결과 파일명 분기 추가
  - [ ] 실행 분기 추가
- [ ] run_test.bat 수정:
  - [ ] 컴파일 명령 추가
  - [ ] 링킹에 .o 파일 추가
- [ ] 테스트 실행: `run_test.bat scenario4`
- [ ] 결과 확인: `test_result/test_result_scenario4.txt`

#### ✅ Scenario 5 (auto_assign)
- [ ] 시나리오 문서 확인: `scenario/scenario_05_auto_assign.md`
- [ ] 테스트 파일 생성: `test/test_scenario5.cpp`
- [ ] test_main.cpp 수정 (동일 패턴)
- [ ] run_test.bat 수정 (동일 패턴)
- [ ] 테스트 실행 및 결과 확인

#### ✅ Scenario 6 (reset_all_ids)
- [ ] 시나리오 문서 확인: `scenario/scenario_06_reset_all_ids.md`
- [ ] 테스트 파일 생성: `test/test_scenario6.cpp`
- [ ] test_main.cpp 수정 (동일 패턴)
- [ ] run_test.bat 수정 (동일 패턴)
- [ ] 테스트 실행 및 결과 확인

#### ✅ Scenario 7 (invalid_input)
- [ ] 시나리오 문서 확인: `scenario/scenario_07_invalid_input.md`
- [ ] 테스트 파일 생성: `test/test_scenario7.cpp`
- [ ] test_main.cpp 수정 (동일 패턴)
- [ ] run_test.bat 수정 (동일 패턴)
- [ ] 테스트 실행 및 결과 확인

#### ✅ Scenario 8 (sensor_error)
- [ ] 시나리오 문서 확인: `scenario/scenario_08_sensor_error.md`
- [ ] 테스트 파일 생성: `test/test_scenario8.cpp`
- [ ] test_main.cpp 수정 (동일 패턴)
- [ ] run_test.bat 수정 (동일 패턴)
- [ ] 테스트 실행 및 결과 확인

## 📝 실행 명령어

```batch
# 개별 시나리오 실행
run_test.bat scenario4
run_test.bat scenario5
run_test.bat scenario6
run_test.bat scenario7
run_test.bat scenario8

# 전체 테스트 실행
run_test.bat all
```

## 📁 결과 파일 구조
```
run_test/test_result/
├── test_result_scenario1.txt
├── test_result_scenario2.txt
├── test_result_scenario3.txt
├── test_result_scenario4.txt
├── test_result_scenario5.txt
├── test_result_scenario6.txt
├── test_result_scenario7.txt
├── test_result_scenario8.txt
├── direct_test_output_scenario1.txt
├── direct_test_output_scenario2.txt
├── direct_test_output_scenario3.txt
├── direct_test_output_scenario4.txt
├── direct_test_output_scenario5.txt
├── direct_test_output_scenario6.txt
├── direct_test_output_scenario7.txt
├── direct_test_output_scenario8.txt
├── unity_test_output.txt
└── unity_test_output.xml
```

## 🔧 디버깅 팁

1. **컴파일 에러 시**: 
   - 헤더 파일 경로 확인 (`-I.. -I..\test -I..\test\unity`)
   - 매크로 정의 확인 (`-DTEST`)

2. **링킹 에러 시**:
   - .o 파일이 올바르게 생성되었는지 확인
   - 링킹 명령에 모든 필요한 .o 파일이 포함되었는지 확인

3. **실행 에러 시**:
   - test_runner.exe가 올바르게 생성되었는지 확인
   - 결과 파일 경로가 올바른지 확인

## 📚 참고 문서
- 시나리오 문서: `scenario/` 폴더
- 기존 테스트 파일: `test/test_scenario1.cpp`, `test/test_scenario2.cpp`, `test/test_scenario3.cpp`
- 메인 테스트 실행기: `test/test_main.cpp`
- 빌드 스크립트: `run_test/run_test.bat`

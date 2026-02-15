# 시나리오 구현 빠른 체크리스트

## 🚀 빠른 시작 가이드

### 새 시나리오 구현할 때 이 체크리스트를 사용하세요!

---

## ✅ Scenario X 구현 체크리스트

### ⚙️ 사전 준비
- [ ] 시나리오 문서 읽기: `scenario/scenario_0X_[name].md`
- [ ] 기존 테스트 파일 참고: `test/test_scenario1.cpp`, `test/test_scenario2.cpp`, `test/test_scenario3.cpp`

### 📝 1단계: 테스트 파일 생성
- [ ] `test/test_scenarioX.cpp` 파일 생성
- [ ] 템플릿 복사 (`scenario_guide/scenario_template.md` 참고)
- [ ] 함수명 변경: `test_scenarioX_main_functionality`
- [ ] 테스트 로직 구현 (또는 `TEST_ASSERT(1)`로 우선 구현)

### 🔧 2단계: test_main.cpp 수정
- [ ] **extern 선언 추가** (파일 상단, 다른 extern 선언들 다음에):
  ```cpp
  extern int run_scenarioX_tests();
  ```

- [ ] **결과 파일명 분기 추가** (main 함수 내, 기존 분기들 다음에):
  ```cpp
  else if (strcmp(argv[1], "scenarioX") == 0)
  {
      result_file = "test_result/test_result_scenarioX.txt";
      direct_file = "test_result/direct_test_output_scenarioX.txt";
  }
  ```

- [ ] **실행 분기 추가** (main 함수 내, 기존 실행 분기들 다음에):
  ```cpp
  else if (strcmp(argv[1], "scenarioX") == 0)
  {
      unity_custom_print_message("Running scenarioX tests...");
      printf("Running scenarioX tests...\n");

      unity_xml_begin_suite("ScenarioXTests");
      int result = run_scenarioX_tests();
      unity_xml_end_suite(0, 0, 0);

      unity_custom_close_file_output();
      return result;
  }
  ```

### 🔨 3단계: run_test.bat 수정
- [ ] **컴파일 명령 추가** (기존 시나리오 컴파일 명령들 다음에):
  ```batch
  g++ -c -DTEST ..\test\test_scenarioX.cpp -o test_result\test_scenarioX.o -I.. -I..\test -I..\test\unity
  if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenarioX.cpp. && pause && exit /b %errorlevel%)
  ```

- [ ] **링킹 명령 수정** (기존 링킹 명령에 추가):
  ```batch
  # 기존 링킹 명령에서 test_result\test_scenario3.o 다음에 추가:
  test_result\test_scenarioX.o
  ```

### 🧪 4단계: 테스트 실행
- [ ] 터미널에서 실행:
  ```batch
  cd run_test
  run_test.bat scenarioX
  ```

- [ ] 컴파일 오류 확인 (있다면 2단계, 3단계 재검토)
- [ ] 링킹 오류 확인 (주로 .o 파일 누락)

### 📊 5단계: 결과 확인
- [ ] 결과 파일 생성 확인: `run_test/test_result/test_result_scenarioX.txt`
- [ ] 테스트 통과 확인: `"1 Tests 0 Failures 0 Ignored OK"` 메시지
- [ ] XML 파일 확인: `run_test/test_result/unity_test_output.xml`

---

## 🐛 문제 해결 빠른 가이드

### 컴파일 에러
```
> Compilation FAILED for test_scenarioX.cpp
```
**해결책**: 
- [ ] 헤더 파일 경로 확인
- [ ] 함수명 오타 확인
- [ ] 템플릿 코드 완전성 확인

### 링킹 에러
```
undefined reference to 'run_scenarioX_tests()'
```
**해결책**: 
- [ ] test_scenarioX.cpp 컴파일 명령 확인
- [ ] run_test.bat 링킹 명령에 test_scenarioX.o 추가 확인
- [ ] test_main.cpp extern 선언 확인

### 실행 시 결과 파일 미생성
**해결책**:
- [ ] test_main.cpp 결과 파일명 분기 확인
- [ ] test_main.cpp 실행 분기 확인
- [ ] 테스트 함수명 일치 확인

---

## 📋 완료된 시나리오 추적

### ✅ 완료
- [x] Scenario 1: 센서 테이블 표시
- [x] Scenario 2: 메뉴 진입 및 취소
- [x] Scenario 3: 개별 센서 ID 변경

### 🔄 진행 예정 (권장 순서)
- [ ] Scenario 5: 센서 ID 자동 할당 (1순위 - 쉬움)
- [ ] Scenario 6: 센서 ID 전체 초기화 (2순위 - 쉬움)
- [ ] Scenario 4: 여러 센서 ID 선택적 변경 (3순위 - 중간)
- [ ] Scenario 7: 잘못된 입력 처리 (4순위 - 어려움)
- [ ] Scenario 8: 센서 오류 처리 (5순위 - 가장 어려움)

---

## 🎯 한 번에 여러 시나리오 구현 시

### 일괄 수정 팁:
1. **모든 테스트 파일 먼저 생성** (scenario_template.md 활용)
2. **test_main.cpp 한 번에 수정** (모든 extern, 분기 한번에 추가)
3. **run_test.bat 한 번에 수정** (모든 컴파일 명령, 링킹 한번에 추가)
4. **순서대로 테스트** (scenario4 → scenario5 → ... → scenario8)

### 실행 명령어:
```batch
# 개별 테스트
run_test.bat scenario4
run_test.bat scenario5
run_test.bat scenario6
run_test.bat scenario7
run_test.bat scenario8

# 전체 테스트 (모든 시나리오 포함)
run_test.bat all
```

---

## 📁 최종 파일 구조 (모든 시나리오 완료 시)

```
test/
├── test_scenario1.cpp    ✅
├── test_scenario2.cpp    ✅  
├── test_scenario3.cpp    ✅
├── test_scenario4.cpp    📝 구현 대상
├── test_scenario5.cpp    📝 구현 대상
├── test_scenario6.cpp    📝 구현 대상
├── test_scenario7.cpp    📝 구현 대상
├── test_scenario8.cpp    📝 구현 대상
└── test_main.cpp         🔧 수정 대상

run_test/
├── run_test.bat          🔧 수정 대상
└── test_result/
    ├── test_result_scenario1.txt    ✅
    ├── test_result_scenario2.txt    ✅
    ├── test_result_scenario3.txt    ✅
    ├── test_result_scenario4.txt    🎯 목표
    ├── test_result_scenario5.txt    🎯 목표
    ├── test_result_scenario6.txt    🎯 목표
    ├── test_result_scenario7.txt    🎯 목표
    └── test_result_scenario8.txt    🎯 목표
```

**이 체크리스트를 출력해서 옆에 두고 시나리오를 하나씩 구현하세요!** 📋✅

import os
import re

PROJECT_ROOT = "E:/Project_DS18b20/03_펌웨어/arduino_v2/01_ds18b20_flow_07"
TEST_DIR = os.path.join(PROJECT_ROOT, "test")

def generate_test_runner_for_file(filepath):
    """Generates or updates the run_xxx_tests() function in a given test file."""
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Extract test function names (void test_*)
    test_functions = re.findall(r'void\s+(test_[a-zA-Z0-9_]+)\s*\(', content)

    # Determine the base name (e.g., 'utils' from 'test_utils.cpp')
    base_name = os.path.basename(filepath).replace('test_', '').replace('.cpp', '')
    run_function_name = f"run_{base_name}_tests"

    # Construct the new run_xxx_tests() function content
    run_function_body = "    UNITY_BEGIN();\n"
    for func in test_functions:
        run_function_body += f"    RUN_TEST({func});\n"
    run_function_body += "    return UNITY_END();\n"

    new_run_function = f"int {run_function_name}() {{\n{run_function_body}}}"

    # Check if run_xxx_tests() already exists and replace it, or append it
    run_function_pattern = re.compile(r'int\s+' + re.escape(run_function_name) + r'\s*\(\)\s*\{.*?\}', re.DOTALL)
    if run_function_pattern.search(content):
        # Replace existing function
        content = run_function_pattern.sub(new_run_function, content)
    else:
        # Append new function to the end of the file
        content += f"\n{new_run_function}\n"

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"Updated {filepath} with {run_function_name}.")
    return run_function_name

def generate_main_test_runner(all_run_functions):
    """Generates the main test_main.cpp file."""
    main_content = """#include \"unity/unity.h\"
#include <cstring>
#include \"TestGlobals.h\" // TestGlobals.h를 포함하여 extern 선언을 가져옵니다.

// 전역 변수 정의 및 초기화
MockDS18B20_Sensor* mockSensorManager = nullptr;
MockCommunicator* mockComm = nullptr;
MockDataProcessor* mockDataProcessor = nullptr;
App* app = nullptr;

// Declare test runner functions from other files
"""
    for func in all_run_functions:
        main_content += f"extern int {func}();\n"

    main_content += """\nint main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], \"all\") == 0) {
        int result = 0;
"""
    for func in all_run_functions:
        # Extract the base name from run_xxx_tests (e.g., 'utils' from 'run_utils_tests')
        base_name = func.replace('run_', '').replace('_tests', '')
        main_content += f"        printf("\nRunning {base_name} tests...\n");\n"
        main_content += f"        result |= {func}();\n"

    main_content += """        return result;
    }
"""
    # Build the usage options string
    usage_options = ""
    for func in all_run_functions:
        base_name = func.replace('run_', '').replace('_tests', '')
        usage_options += f"|{base_name}"

    # Add specific test execution blocks
    for func in all_run_functions:
        base_name = func.replace('run_', '').replace('_tests', '')
        main_content += f"""    else if (strcmp(argv[1], \"{base_name}\") == 0) {{
        printf("\nRunning {base_name} tests...\n");
        return {func}();
    }}
"""
    main_content += f"""    else {{
        printf(\"Usage: %s [all{usage_options}]\n\", argv[0]);
        return 1;
    }}
}}

void setUp(void) {{
    // 테스트 시작 전 초기화 코드 (필요시 추가)
}}

void tearDown(void) {{
    // 테스트 종료 후 정리 코드 (필요시 추가)
}}
"""
    main_filepath = os.path.join(TEST_DIR, "test_main.cpp")
    with open(main_filepath, 'w', encoding='utf-8') as f:
        f.write(main_content)
    print(f"Generated {main_filepath}.")

def generate_run_test_bat():
    """Generates the run_test.bat file."""
    bat_content = f"""@echo off
chcp 65001 > nul
REM --- Test Execution Batch Script (Automated) ---

REM Temporarily add the MinGW compiler to the PATH for this script only.
set \"PATH=E:\\mingw64\\bin;%PATH%\"

echo Compiling test files to object files...
echo Current PATH: %PATH%

REM Compile each .cpp file to an object file (.o)
g++ -c -DTEST {TEST_DIR}/test_main.cpp -o {TEST_DIR}/test_main.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_main.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST {TEST_DIR}/test_utils.cpp -o {TEST_DIR}/test_utils.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_utils.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST {TEST_DIR}/test_app_menu.cpp -o {TEST_DIR}/test_app_menu.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_app_menu.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST {TEST_DIR}/test_scenario1.cpp -o {TEST_DIR}/test_scenario1.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario1.cpp. && pause && exit /b %errorlevel%)

g++ -c App.cpp -o App.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for App.cpp. && pause && exit /b %errorlevel%)

g++ -c Communicator.cpp -o Communicator.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for Communicator.cpp. && pause && exit /b %errorlevel%)

g++ -c DataProcessor.cpp -o DataProcessor.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for DataProcessor.cpp. && pause && exit /b %errorlevel%)

g++ -c DS18B20_Sensor.cpp -o DS18B20_Sensor.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for DS18B20_Sensor.cpp. && pause && exit /b %errorlevel%)

g++ -c Utils.cpp -o Utils.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for Utils.cpp. && pause && exit /b %errorlevel%)

g++ -c {TEST_DIR}/unity/unity.c -o {TEST_DIR}/unity/unity.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for unity.c. && pause && exit /b %errorlevel%)

g++ -c {TEST_DIR}/FakeArduino.cpp -o {TEST_DIR}/FakeArduino.o -I. -I{TEST_DIR} -I{TEST_DIR}/unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for FakeArduino.cpp. && pause && exit /b %errorlevel%)


echo Linking object files to create executable...
g++ -o test_runner {TEST_DIR}/test_main.o {TEST_DIR}/test_utils.o {TEST_DIR}/test_app_menu.o {TEST_DIR}/test_scenario1.o App.o Communicator.o DataProcessor.o DS18B20_Sensor.o Utils.o {TEST_DIR}/unity/unity.o {TEST_DIR}/FakeArduino.o
if %errorlevel% neq 0 (
    echo.
    echo  ^> Linking FAILED.
    pause
    exit /b %errorlevel%)

echo.
echo  ^> Compilation SUCCEEDED.
echo.
echo --------------------------------------
echo         Running Unit Tests
echo --------------------------------------

REM Run the compiled test executable with the provided argument.
if \"%1\"==\"\" (
    echo Running all tests...
    .\test_runner.exe all
) else (
    echo Running tests for scenario: %1
    .\test_runner.exe %1
)

echo --------------------------------------
echo.

REM Clean up the created executable file and object files.
del test_runner.exe
del *.o
del {TEST_DIR}\\*.o
del {TEST_DIR}\\unity\\*.o

echo Test run finished.
pause
"""
    bat_filepath = os.path.join(PROJECT_ROOT, "run_test.bat")
    with open(bat_filepath, 'w', encoding='utf-8') as f:
        f.write(bat_content)
    print(f"Generated {bat_filepath}.")

def main():
    test_files = [f for f in os.listdir(TEST_DIR) if f.startswith('test_') and f.endswith('.cpp') and f != 'test_main.cpp']
    all_run_functions = []

    for filename in test_files:
        filepath = os.path.join(TEST_DIR, filename)
        run_func_name = generate_test_runner_for_file(filepath)
        all_run_functions.append(run_func_name)

    generate_main_test_runner(all_run_functions)
    generate_run_test_bat() # Generate run_test.bat here

if __name__ == "__main__":
    main()
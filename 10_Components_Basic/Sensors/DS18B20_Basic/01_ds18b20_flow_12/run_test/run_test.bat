g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario9_v2.cpp -o test_result\test_scenario9_v2.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario9_v2.cpp. && pause && exit /b %errorlevel%)
chcp 65001 > nul
REM --- Test Execution Batch Script (Automated) ---

REM Create output folders if not exist
if not exist test_result mkdir test_result

REM Temporarily add the MinGW compiler to the PATH for this script only.
set "PATH=E:\mingw64\bin;%PATH%"

echo Compiling test files to object files...
echo Current PATH: %PATH%

REM Compile each .cpp file to an object file (.o) in test_result

g++ -c -DTEST -DUNIT_TEST ..\test\unity_custom_output.cpp -o test_result\unity_custom_output.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for unity_custom_output.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\unity_xml_output.cpp -o test_result\unity_xml_output.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for unity_xml_output.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_main.cpp -o test_result\test_main.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_main.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_utils.cpp -o test_result\test_utils.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_utils.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_app_menu.cpp -o test_result\test_app_menu.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_app_menu.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario1.cpp -o test_result\test_scenario1.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario1.cpp. && pause && exit /b %errorlevel%)
g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario2.cpp -o test_result\test_scenario2.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario2.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario3.cpp -o test_result\test_scenario3.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario3.cpp. && pause && exit /b %errorlevel%)
g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario4.cpp -o test_result\test_scenario4.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario4.cpp. && pause && exit /b %errorlevel%)

g++ -c -DUNIT_TEST ..\App.cpp -o test_result\App.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for App.cpp. && pause && exit /b %errorlevel%)
g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario5.cpp -o test_result\test_scenario5.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario5.cpp. && pause && exit /b %errorlevel%)
g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario6.cpp -o test_result\test_scenario6.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario6.cpp. && pause && exit /b %errorlevel%)
g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario7.cpp -o test_result\test_scenario7.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario7.cpp. && pause && exit /b %errorlevel%)
g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario8.cpp -o test_result\test_scenario8.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario8.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario9_simple.cpp -o test_result\test_scenario9_simple.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario9_simple.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario11.cpp -o test_result\test_scenario11.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario11.cpp. && pause && exit /b %errorlevel%)

g++ -c -DTEST -DUNIT_TEST ..\test\test_scenario_total_01.cpp -o test_result\test_scenario_total_01.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for test_scenario_total_01.cpp. && pause && exit /b %errorlevel%)

g++ -c -DUNIT_TEST ..\Communicator.cpp -o test_result\Communicator.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for Communicator.cpp. && pause && exit /b %errorlevel%)

g++ -c -DUNIT_TEST ..\DataProcessor.cpp -o test_result\DataProcessor.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for DataProcessor.cpp. && pause && exit /b %errorlevel%)

g++ -c -DUNIT_TEST ..\DS18B20_Sensor.cpp -o test_result\DS18B20_Sensor.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for DS18B20_Sensor.cpp. && pause && exit /b %errorlevel%)

g++ -c -DUNIT_TEST ..\Utils.cpp -o test_result\Utils.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for Utils.cpp. && pause && exit /b %errorlevel%)

g++ -c ..\test\unity\unity.c -o test_result\unity.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for unity.c. && pause && exit /b %errorlevel%)

g++ -c ..\test\FakeArduino.cpp -o test_result\FakeArduino.o -I.. -I..\test -I..\test\unity
if %errorlevel% neq 0 (echo ^> Compilation FAILED for FakeArduino.cpp. && pause && exit /b %errorlevel%)


echo Linking object files to create executable...
g++ -o test_result\test_runner test_result\test_main.o test_result\test_utils.o test_result\test_app_menu.o test_result\test_scenario1.o test_result\test_scenario2.o test_result\test_scenario3.o test_result\test_scenario4.o test_result\test_scenario5.o test_result\test_scenario6.o test_result\test_scenario7.o test_result\test_scenario8.o test_result\test_scenario9_simple.o test_result\test_scenario9_v2.o test_result\test_scenario11.o test_result\test_scenario_total_01.o test_result\App.o test_result\Communicator.o test_result\DataProcessor.o test_result\DS18B20_Sensor.o test_result\Utils.o test_result\unity.o test_result\FakeArduino.o test_result\unity_custom_output.o test_result\unity_xml_output.o
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

REM Run the compiled test executable with the provided argument and save output
if "%1"=="" (
    echo Running all tests...
    REM Run the test runner and tee the output both to the console and to the file
    test_result\test_runner.exe all > test_result\test_result_all.txt 2>&1
    echo Test results:
    type test_result\test_result_all.txt
) else (
    echo Running tests for scenario: %1
    REM Run the test runner and tee the output both to the console and to the file
    test_result\test_runner.exe %1 > test_result\test_result_%1.txt 2>&1
    echo Test results:
    type test_result\test_result_%1.txt
)

echo --------------------------------------
echo.

REM Clean up only object files, keep test_runner.exe for debugging.
del test_result\*.o

echo Test run finished. 
echo.
echo Results are stored in the following files:
echo - Text results: test_result\test_result_%1.txt (or test_result_all.txt for all tests)
echo - Unity test output: test_result\unity_test_output.txt 
echo - XML test results: test_result\unity_test_output.xml

echo.
echo To check scenario1 test results, run: run_test.bat scenario1
echo To check scenario2 test results, run: run_test.bat scenario2
echo To check all test results, run: run_test.bat all
echo.
pause

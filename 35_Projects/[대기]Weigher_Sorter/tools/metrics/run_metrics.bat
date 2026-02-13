@echo off
echo ========================================
echo DHT22 Project Metrics and Coverage Analysis
echo ========================================

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%\..\..

cd /d "%PROJECT_ROOT%"

if "%1"=="python" goto python_only
if "%1"=="arduino" goto arduino_only
if "%1"=="integrated" goto integrated_only
if "%1"=="help" goto show_help

:all_metrics
echo.
echo Running complete metrics analysis...
python "%SCRIPT_DIR%\integrated_metrics.py"
goto end

:python_only
echo.
echo Running Python metrics analysis...
python "%SCRIPT_DIR%\python_coverage.py"
goto end

:arduino_only
echo.
echo Running Arduino metrics analysis...
python "%SCRIPT_DIR%\arduino_metrics.py"
goto end

:integrated_only
echo.
echo Generating integrated dashboard...
python "%SCRIPT_DIR%\integrated_metrics.py"
goto end

:show_help
echo.
echo Usage:
echo   run_metrics.bat [option]
echo.
echo Options:
echo   (none)     - Run complete metrics analysis
echo   python     - Analyze Python code only
echo   arduino    - Analyze Arduino code only
echo   integrated - Generate integrated dashboard only
echo   help       - Show this help message
echo.
echo Examples:
echo   run_metrics.bat
echo   run_metrics.bat python
echo   run_metrics.bat arduino
goto end

:end
echo.
echo Metrics analysis completed!
echo Results can be found in tools\metrics\reports\ folder.
pause
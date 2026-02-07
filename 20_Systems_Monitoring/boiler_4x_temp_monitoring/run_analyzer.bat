@echo off
title Boiler Integrated Analyzer v2.0
set SCRIPT_DIR=%~dp0tools\analyzer_integrated

echo ========================================================
echo   Boiler Integrated Analyzer v2.0
echo   - Device: 192.168.0.150:80  (ESP32C3 Xiao)
echo   - Dashboard: http://localhost:8080
echo ========================================================
echo.

cd /d "%SCRIPT_DIR%"

REM Check Python
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Python not found. Please install Python 3.
    pause
    exit /b
)

REM Kill zombie python processes using port 8080
echo [INFO] Cleaning up port 8080...
for /f "tokens=5" %%p in ('netstat -aon ^| findstr :8080 ^| findstr LISTENING 2^>nul') do (
    echo [INFO] Killing process PID %%p on port 8080
    taskkill /F /PID %%p >nul 2>&1
)
timeout /t 1 /nobreak >nul

echo [INFO] Starting server...
echo.
python analyzer_server.py

pause

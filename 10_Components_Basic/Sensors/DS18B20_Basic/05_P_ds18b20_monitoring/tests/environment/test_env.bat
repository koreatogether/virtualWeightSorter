@echo off
chcp 65001 >nul
REM Windows CMD/PowerShell용 환경 테스트 스크립트

echo ========================================
echo DS18B20 모니터링 시스템 환경 테스트
echo ========================================
echo.

REM uv PATH 설정 시도
set "UV_PATH=%USERPROFILE%\.local\bin"
if exist "%UV_PATH%\uv.exe" (
    set "PATH=%UV_PATH%;%PATH%"
    echo ✅ uv PATH 설정 완료
) else (
    echo ⚠️ uv를 기본 위치에서 찾을 수 없습니다.
)

echo [1] uv를 사용한 테스트 (권장)
uv --version >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo ✅ uv 사용 가능
    uv run python tools/test_environment/test_environment.py
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ✅ uv 환경 테스트 성공!
        goto :success
    ) else (
        echo.
        echo ❌ uv 환경 테스트 실패. 직접 실행을 시도합니다...
    )
) else (
    echo ❌ uv를 찾을 수 없습니다.
)

echo.
echo [2] 직접 Python 실행 테스트
python --version >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo ✅ Python 사용 가능
    python tools/test_environment/test_environment.py
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ✅ 직접 실행 테스트 성공!
        goto :success
    ) else (
        echo.
        echo ❌ 직접 실행도 실패했습니다.
        goto :failure
    )
) else (
    echo ❌ Python을 찾을 수 없습니다.
    goto :failure
)

:failure
echo.
echo ❌ 모든 테스트가 실패했습니다.
echo 💡 해결 방법:
echo    1. setup_env.bat를 먼저 실행하세요
echo    2. uv sync로 패키지를 재설치하세요
echo    3. uv add dash-bootstrap-components 실행하세요
echo.
goto :end

:success
echo.
echo 🎉 환경 테스트 성공! Phase 1 개발을 시작할 수 있습니다.
echo.

:end
echo 테스트 완료. 아무 키나 누르면 종료됩니다...
pause >nul
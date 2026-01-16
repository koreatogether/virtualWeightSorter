@echo off
REM DS18B20 모니터링 시스템 환경 설정 스크립트

echo ========================================
echo DS18B20 모니터링 시스템 환경 설정
echo ========================================
echo.

REM uv PATH 설정 시도
set "UV_PATH=%USERPROFILE%\.local\bin"
if exist "%UV_PATH%\uv.exe" (
    echo ✅ uv 발견: %UV_PATH%\uv.exe
    set "PATH=%UV_PATH%;%PATH%"
    echo ✅ uv를 PATH에 추가했습니다.
) else (
    echo ❌ uv를 찾을 수 없습니다.
    echo 💡 uv 설치 방법:
    echo    PowerShell에서: irm https://astral.sh/uv/install.ps1 ^| iex
    echo    또는 pip install uv
    echo.
)

REM Python 버전 확인
echo [Python 환경 확인]
python --version 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Python을 찾을 수 없습니다.
    echo 💡 Python 설치가 필요합니다.
    pause
    exit /b 1
)

REM uv 버전 확인
echo [uv 환경 확인]
uv --version 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ❌ uv를 실행할 수 없습니다.
    echo 💡 uv 설치 또는 PATH 설정이 필요합니다.
) else (
    echo ✅ uv가 정상적으로 작동합니다.
)

echo.
echo [가상환경 확인]
if exist ".venv" (
    echo ✅ uv 가상환경이 존재합니다.
) else (
    echo ❌ 가상환경이 없습니다.
    echo 💡 가상환경 생성: uv init --python 3.10
)

echo.
echo ========================================
echo 환경 설정 완료
echo ========================================
echo.
echo 다음 명령으로 테스트를 실행하세요:
echo   uv run python tools/test_environment/test_environment.py
echo.
echo 또는 직접 Python으로:
echo   python tools/test_environment/test_environment.py
echo.
@echo off
REM UV 가상환경 자동 활성화 스크립트
cd /d "%~dp0"
echo Activating UV virtual environment...

REM UV 가상환경 활성화
call .venv\Scripts\activate.bat

REM 프롬프트 변경
cmd /k "echo UV virtual environment activated! && echo Project root: %CD% && echo Python path: && python --version"
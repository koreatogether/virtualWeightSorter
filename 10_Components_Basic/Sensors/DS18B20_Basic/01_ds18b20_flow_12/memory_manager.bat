@echo off
REM 메모리 분석 기능 관리 배치 스크립트
REM 사용법: memory_manager.bat [enable|disable|compile|status|help]

setlocal

if "%1"=="" goto :help
if "%1"=="help" goto :help
if "%1"=="enable" goto :enable
if "%1"=="disable" goto :disable
if "%1"=="compile" goto :compile
if "%1"=="status" goto :status
goto :help

:enable
echo 메모리 분석 기능 활성화 중...
python memory_analysis_manager.py enable
goto :end

:disable
echo 메모리 분석 기능 비활성화 중...
python memory_analysis_manager.py disable
goto :end

:compile
echo 프로젝트 컴파일 중...
python memory_analysis_manager.py compile
goto :end

:status
echo 메모리 분석 기능 상태 확인 중...
python memory_analysis_manager.py status
goto :end

:help
echo 메모리 분석 기능 관리 배치 스크립트
echo 사용법: memory_manager.bat [명령]
echo.
echo 사용 가능한 명령:
echo   enable   - 메모리 분석 기능 활성화
echo   disable  - 메모리 분석 기능 비활성화
echo   compile  - 현재 설정으로 프로젝트 컴파일
echo   status   - 현재 메모리 분석 기능 상태 확인
echo   help     - 이 도움말 표시

:end
endlocal

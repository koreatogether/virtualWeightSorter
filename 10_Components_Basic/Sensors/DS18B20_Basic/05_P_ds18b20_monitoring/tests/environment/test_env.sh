#!/bin/bash
# Git Bash/WSL/Linux용 환경 테스트 스크립트

echo "========================================"
echo "DS18B20 모니터링 시스템 환경 테스트"
echo "========================================"
echo

echo "[1] uv를 사용한 테스트 (권장)"
if uv run python tools/test_environment/test_environment.py; then
    echo
    echo "✅ uv 환경 테스트 성공!"
else
    echo
    echo "❌ uv 환경 테스트 실패. 직접 실행을 시도합니다..."
    echo
    echo "[2] 직접 Python 실행 테스트"
    if python tools/test_environment/test_environment.py; then
        echo
        echo "✅ 직접 실행 테스트 성공!"
    elif python3 tools/test_environment/test_environment.py; then
        echo
        echo "✅ python3 실행 테스트 성공!"
    else
        echo
        echo "❌ 모든 실행 방법이 실패했습니다."
        echo "💡 해결 방법:"
        echo "   1. Python이 설치되어 있는지 확인"
        echo "   2. uv가 설치되어 있는지 확인"
        echo "   3. 가상환경이 올바르게 설정되어 있는지 확인"
        exit 1
    fi
fi

echo
echo "테스트 완료!"
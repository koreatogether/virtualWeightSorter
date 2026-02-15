#!/usr/bin/env python3
"""빠른 품질 검사 스크립트

코딩 후 즉시 실행하여 품질 문제를 빠르게 발견하고 수정합니다.
사용법: python quick_quality_check.py [파일경로]
"""

import platform
import subprocess
import sys
from pathlib import Path

# Windows에서 UTF-8 출력 설정
if platform.system() == "Windows":
    try:
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        import codecs

        sys.stdout = codecs.getwriter("utf-8")(sys.stdout.buffer, "strict")
        sys.stderr = codecs.getwriter("utf-8")(sys.stderr.buffer, "strict")


def run_command(cmd: list[str], description: str) -> tuple[bool, str]:
    """명령어 실행 및 결과 반환"""
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            cwd=Path.cwd(),
            timeout=30,
        )

        if result.returncode == 0:
            print(f"✅ {description}: PASSED")
            return True, result.stdout
        else:
            print(f"❌ {description}: FAILED")
            if result.stdout:
                print(f"   Output: {result.stdout[:200]}...")
            if result.stderr:
                print(f"   Error: {result.stderr[:200]}...")
            return False, result.stderr

    except subprocess.TimeoutExpired:
        print(f"⏰ {description}: TIMEOUT")
        return False, "Command timed out"
    except Exception as e:
        print(f"🚨 {description}: ERROR - {e}")
        return False, str(e)


def check_specific_file(file_path: str) -> None:
    """특정 파일에 대한 품질 검사"""
    path = Path(file_path)
    if not path.exists():
        print(f"❌ 파일을 찾을 수 없습니다: {file_path}")
        return

    print(f"\n🔍 파일별 검사: {file_path}")
    print("=" * 50)

    # 1. Ruff 린트 체크
    run_command(
        ["python", "-m", "ruff", "check", str(path)], f"Ruff 린트 ({path.name})"
    )

    # 2. Ruff 포맷 체크
    run_command(
        ["python", "-m", "ruff", "format", "--check", str(path)],
        f"Ruff 포맷 ({path.name})",
    )

    # 3. MyPy 타입 체크 (Python 파일인 경우)
    if path.suffix == ".py":
        run_command(["python", "-m", "mypy", str(path)], f"MyPy 타입 ({path.name})")


def check_all_project() -> None:
    """전체 프로젝트 품질 검사"""
    print("\n🌐 전체 프로젝트 검사")
    print("=" * 50)

    checks = [
        (["python", "-m", "ruff", "check", "src/", "tools/"], "전체 Ruff 린트"),
        (
            ["python", "-m", "ruff", "format", "--check", "src/", "tools/"],
            "전체 Ruff 포맷",
        ),
        (["python", "-m", "mypy", "src/python/"], "전체 MyPy 타입"),
    ]

    passed = 0
    total = len(checks)

    for cmd, desc in checks:
        success, _ = run_command(cmd, desc)
        if success:
            passed += 1

    print(f"\n📊 결과: {passed}/{total} 통과")

    if passed == total:
        print("🎉 모든 검사 통과!")
    else:
        print("🔧 수정이 필요한 항목이 있습니다.")


def auto_fix_project() -> None:
    """자동 수정 가능한 문제들 해결"""
    print("\n🛠️ 자동 수정 실행")
    print("=" * 50)

    # 1. Ruff 자동 수정
    success1, _ = run_command(
        ["python", "-m", "ruff", "check", "--fix", "src/", "tools/"], "Ruff 자동 수정"
    )

    # 2. Ruff 포맷팅
    success2, _ = run_command(
        ["python", "-m", "ruff", "format", "src/", "tools/"], "Ruff 자동 포맷팅"
    )

    if success1 and success2:
        print("✨ 자동 수정 완료!")
    else:
        print("⚠️ 일부 수정이 실패했습니다. 수동 확인이 필요합니다.")


def run_comprehensive_check() -> None:
    """포괄적인 품질 검사 실행"""
    print("\n🎯 포괄적 품질 검사")
    print("=" * 50)

    success, _ = run_command(
        ["python", "tools/quality/quality_check.py"], "통합 품질 검사"
    )

    if success:
        print("📈 상세 리포트는 tools/quality/reports/ 폴더를 확인하세요.")


def print_usage() -> None:
    """사용법 출력"""
    print(
        """
🚀 빠른 품질 검사 도구

사용법:
  python quick_quality_check.py                    # 전체 프로젝트 검사
  python quick_quality_check.py file.py           # 특정 파일 검사
  python quick_quality_check.py --fix             # 자동 수정 실행
  python quick_quality_check.py --full            # 포괄적 검사 실행
  python quick_quality_check.py --help            # 도움말 출력

예시:
  python quick_quality_check.py src/python/dashboard/app.py
  python quick_quality_check.py --fix
    """
    )


def main() -> None:
    """메인 함수"""
    args = sys.argv[1:]

    if not args or "--help" in args:
        print_usage()
        return

    if "--fix" in args:
        auto_fix_project()
        print("\n🔄 수정 후 재검사:")
        check_all_project()
        return

    if "--full" in args:
        auto_fix_project()
        check_all_project()
        run_comprehensive_check()
        return

    # 특정 파일 검사
    if len(args) == 1 and not args[0].startswith("--"):
        check_specific_file(args[0])
        return

    # 기본: 전체 프로젝트 검사
    check_all_project()


if __name__ == "__main__":
    print("🎯 빠른 품질 검사 시작")
    print("=" * 50)
    main()

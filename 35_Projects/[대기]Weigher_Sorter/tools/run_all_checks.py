#!/usr/bin/env python3
"""모든 품질 및 보안 검사를 실행하는 통합 스크립트

이 스크립트는 다음을 실행합니다:
1. Python 코드 품질 검사 (Black, Ruff, MyPy, pytest)
2. Arduino 코드 검사
3. 보안 스캔 (TruffleHog 기반)
4. 종합 리포트 생성
"""

import json
import platform
import subprocess
import sys
from datetime import datetime
from pathlib import Path

# Windows에서 UTF-8 출력 설정
if platform.system() == "Windows":
    try:
        # Windows 콘솔에서 UTF-8 출력 활성화
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        # Python 3.7 이하에서는 다른 방법 사용
        import codecs

        sys.stdout = codecs.getwriter("utf-8")(sys.stdout.buffer, "strict")
        sys.stderr = codecs.getwriter("utf-8")(sys.stderr.buffer, "strict")


class IntegratedChecker:
    def __init__(self):
        self.project_root = Path.cwd()
        self.results = {}

    def run_python_checks(self) -> bool:
        """Python 코드 품질 검사 실행"""
        print("🐍 Python 코드 품질 검사 실행 중...")
        try:
            result = subprocess.run(
                [sys.executable, "tools/quality/quality_check.py"],
                capture_output=True,
                text=True,
                cwd=self.project_root,
            )

            self.results["python_quality"] = {
                "passed": result.returncode == 0,
                "output": result.stdout,
                "errors": result.stderr,
            }

            if result.returncode == 0:
                print("✅ Python 품질 검사 통과")
            else:
                print("❌ Python 품질 검사 실패")

            return result.returncode == 0

        except Exception as e:
            print(f"💥 Python 품질 검사 오류: {e}")
            self.results["python_quality"] = {"passed": False, "error": str(e)}
            return False

    def run_arduino_checks(self) -> bool:
        """Arduino 코드 검사 실행"""
        print("🔧 Arduino 코드 검사 실행 중...")
        try:
            result = subprocess.run(
                [sys.executable, "tools/quality/arduino_check.py"],
                capture_output=True,
                text=True,
                cwd=self.project_root,
            )

            self.results["arduino_quality"] = {
                "passed": result.returncode == 0,
                "output": result.stdout,
                "errors": result.stderr,
            }

            if result.returncode == 0:
                print("✅ Arduino 코드 검사 통과")
            else:
                print("❌ Arduino 코드 검사 실패")

            return result.returncode == 0

        except Exception as e:
            print(f"💥 Arduino 코드 검사 오류: {e}")
            self.results["arduino_quality"] = {"passed": False, "error": str(e)}
            return False

    def run_security_checks(self) -> bool:
        """보안 검사 실행"""
        print("🛡️  보안 검사 실행 중...")
        try:
            result = subprocess.run(
                [sys.executable, "tools/security/trufflehog_check.py"],
                capture_output=True,
                text=True,
                cwd=self.project_root,
            )

            self.results["security_scan"] = {
                "passed": result.returncode == 0,
                "output": result.stdout,
                "errors": result.stderr,
            }

            if result.returncode == 0:
                print("✅ 보안 검사 통과")
            else:
                print("❌ 보안 검사에서 이슈 발견")

            return result.returncode == 0

        except Exception as e:
            print(f"💥 보안 검사 오류: {e}")
            self.results["security_scan"] = {"passed": False, "error": str(e)}
            return False

    def run_dependency_check(self) -> bool:
        """의존성 보안 검사"""
        print("📦 의존성 보안 검사 실행 중...")
        try:
            result = subprocess.run(
                ["uv", "run", "safety", "check", "--json"],
                capture_output=True,
                text=True,
                cwd=self.project_root,
            )

            # Safety는 취약점이 없으면 exit code 0, 있으면 1
            vulnerabilities_found = result.returncode != 0

            self.results["dependency_security"] = {
                "passed": not vulnerabilities_found,
                "output": result.stdout,
                "errors": result.stderr,
            }

            if not vulnerabilities_found:
                print("✅ 의존성 보안 검사 통과")
            else:
                print("⚠️  의존성에서 취약점 발견")

            return not vulnerabilities_found

        except Exception as e:
            print(f"💥 의존성 검사 오류: {e}")
            self.results["dependency_security"] = {"passed": False, "error": str(e)}
            return False

    def generate_summary_report(self) -> None:
        """종합 리포트 생성"""
        # 전체 결과 계산
        total_checks = len(self.results)
        passed_checks = sum(
            1 for result in self.results.values() if result.get("passed", False)
        )

        # 콘솔 요약
        print("\n" + "=" * 80)
        print("🎯 Weigher-Sorter 프로젝트 종합 품질 및 보안 검사 결과")
        print("=" * 80)

        print(f"📊 전체 결과: {passed_checks}/{total_checks} 통과")
        print(f"📅 검사 완료 시간: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

        print("\n📋 세부 결과:")
        check_names = {
            "python_quality": "🐍 Python 코드 품질",
            "arduino_quality": "🔧 Arduino 코드 품질",
            "security_scan": "🛡️  보안 스캔",
            "dependency_security": "📦 의존성 보안",
        }

        for check_key, result in self.results.items():
            check_name = check_names.get(check_key, check_key)
            status = "✅ 통과" if result.get("passed", False) else "❌ 실패"
            print(f"   {check_name:25} : {status}")

            if not result.get("passed", False) and "error" in result:
                print(f"      오류: {result['error']}")

        # JSON 리포트 저장 (로그 폴더에 타임스탬프 접두사로)
        log_dir = Path("tools/logs")
        log_dir.mkdir(exist_ok=True)

        timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        report_file = log_dir / f"{timestamp}_통합_검사_결과.json"

        report_data = {
            "timestamp": datetime.now().isoformat(),
            "summary": {
                "total_checks": total_checks,
                "passed_checks": passed_checks,
                "failed_checks": total_checks - passed_checks,
                "success_rate": (
                    round((passed_checks / total_checks) * 100, 2)
                    if total_checks > 0
                    else 0
                ),
            },
            "results": self.results,
        }

        with open(report_file, "w", encoding="utf-8") as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False)

        print(f"\n📄 종합 리포트: {report_file}")
        
        # 호환성을 위해 기존 위치에도 저장
        report_dir = Path("tools/reports")
        report_dir.mkdir(exist_ok=True)
        old_timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        old_report_file = report_dir / f"integrated_report_{old_timestamp}.json"
        
        with open(old_report_file, "w", encoding="utf-8") as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False)

        # 결과에 따른 메시지
        if passed_checks == total_checks:
            print("\n🎉 모든 검사를 통과했습니다! 커밋할 준비가 되었습니다.")
            return True
        else:
            print(
                f"\n⚠️  {total_checks - passed_checks}개의 검사가 실패했습니다. 문제를 해결한 후 다시 시도하세요."
            )
            return False


def main():
    """메인 함수"""
    print("🚀 Weigher-Sorter 프로젝트 통합 품질 및 보안 검사 시작")
    print(f"📅 시작 시간: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    # 프로젝트 루트에서 실행되고 있는지 확인
    if not Path("pyproject.toml").exists():
        print("❌ pyproject.toml을 찾을 수 없습니다. 프로젝트 루트에서 실행해주세요.")
        sys.exit(1)

    checker = IntegratedChecker()

    # 각 검사 실행

    # Python 품질 검사
    if not checker.run_python_checks():
        pass

    # Arduino 검사
    if not checker.run_arduino_checks():
        pass

    # 보안 검사
    if not checker.run_security_checks():
        pass

    # 의존성 보안 검사
    if not checker.run_dependency_check():
        pass

    # 종합 리포트 생성
    final_success = checker.generate_summary_report()

    # 전체 결과에 따른 종료 코드
    if not final_success:
        sys.exit(1)


if __name__ == "__main__":
    main()

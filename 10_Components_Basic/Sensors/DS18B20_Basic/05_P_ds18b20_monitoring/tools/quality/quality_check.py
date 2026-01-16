#!/usr/bin/env python3
"""Integrated Code Quality Check Tool

This script performs the following checks:
- Black: Code formatting check
- Ruff: Linting and code style check
- MyPy: Type hint check
- Pytest: Unit test execution
"""

import contextlib
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


def run_command(cmd: list[str], description: str) -> tuple[bool, str]:
    """Execute command and return results."""
    print(f"Checking {description}...")
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            cwd=Path.cwd(),
            timeout=300,  # 5 minute timeout
            encoding="utf-8",
            errors="replace",  # Use replacement character for decode errors
        )

        if result.returncode == 0:
            print(f"{description} passed")
            return True, result.stdout
        else:
            print(f"{description} failed")
            print(f"STDOUT: {result.stdout}")
            print(f"STDERR: {result.stderr}")
            return False, result.stderr

    except subprocess.TimeoutExpired:
        print(f"{description} timed out")
        return False, "Command timed out"
    except Exception as e:
        print(f"{description} error: {e!s}")
        return False, str(e)


def check_ruff() -> tuple[bool, str]:
    """Ruff linting check"""
    return run_command(["uv", "run", "ruff", "check", "src/", "tools/"], "Ruff linting")


def fix_ruff() -> tuple[bool, str]:
    """Ruff auto-fix"""
    return run_command(
        [
            "uv",
            "run",
            "ruff",
            "check",
            "--fix",
            "--unsafe-fixes",
            "src/",
            "tools/",
        ],
        "Ruff auto-fix",
    )


def check_ruff_format() -> tuple[bool, str]:
    """Ruff formatting check"""
    return run_command(
        ["uv", "run", "ruff", "format", "--check", "src/", "tools/"],
        "Ruff formatting",
    )


def check_mypy() -> tuple[bool, str]:
    """MyPy type check"""
    return run_command(["uv", "run", "mypy", "src/"], "MyPy type check")


def run_tests() -> tuple[bool, str]:
    """Pytest unit test execution"""
    tests_dir = Path("tests")
    if not tests_dir.exists():
        print("No tests directory found. Skipping tests.")
        return True, "No tests directory found"

    return run_command(["uv", "run", "pytest", "--tb=short"], "pytest unit tests")


def check_imports() -> tuple[bool, str]:
    """Python import check (Option A: core modules only, dashboard optional)

    Rationale:
    - Dashboard depends on real third-party packages (dash, plotly, etc.).
    - To keep CI/import check reliable without heavy UI deps we only require
      backend + simulator core modules to import.
    - We attempt an optional dashboard import; failures are recorded in the
      message but DO NOT fail the overall import check.
    """
    inserted_dummies: list[str] = []
    try:
        sys.path.insert(0, str(Path("src/python").absolute()))

        import types

        optional_modules = [
            "dash",
            "dash_bootstrap_components",
            "plotly",
            "plotly.graph_objects",
            "serial",
        ]

        for mod_name in optional_modules:
            if mod_name not in sys.modules:
                sys.modules[mod_name] = types.ModuleType(mod_name)
                inserted_dummies.append(mod_name)

        core_import_errors: list[str] = []
        # Core required modules (keep small & fast)
        # Core modules purposely exclude heavy web/dashboard deps so CI stays light.
        required_modules = [
            ("simulator.protocol", "simulator/protocol.py"),
            ("simulator.ds18b20_simulator", "simulator/ds18b20_simulator.py"),
            ("simulator.simulator_manager", "simulator/simulator_manager.py"),
        ]

        import importlib.util

        def import_from_path(dotted: str, rel_path: str):
            p = Path("src/python") / rel_path
            spec = importlib.util.spec_from_file_location(dotted, p)
            if spec and spec.loader:
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)  # type: ignore[attr-defined]
                return module
            raise ImportError(f"Cannot import {dotted} from {p}")

        for dotted, rel in required_modules:
            try:
                import_from_path(dotted, rel)
            except Exception as e:
                core_import_errors.append(f"{dotted}: {e}")

        dashboard_note = ""
        # Optional dashboard attempt (non-fatal)
        try:
            import_from_path("dashboard.app", "dashboard/app.py")
        except Exception as e:
            dashboard_note = f"(dashboard optional import skipped: {e})"

        if core_import_errors:
            msg = "; ".join(core_import_errors)
            print(f"Core import failures: {msg}")
            return False, f"Core imports failed: {msg} {dashboard_note}".strip()

        success_msg = "Core imports successful" + (
            f" {dashboard_note}" if dashboard_note else ""
        )
        print(success_msg)
        return True, success_msg
    except Exception as e:
        print(f"Import check error: {e}")
        return False, str(e)
    finally:
        if str(Path("src/python").absolute()) in sys.path:
            with contextlib.suppress(ValueError):
                sys.path.remove(str(Path("src/python").absolute()))
        for m in inserted_dummies:
            try:
                if m in sys.modules:
                    del sys.modules[m]
            except Exception:
                pass


def generate_report(results: dict[str, tuple[bool, str]]) -> None:
    """Generate check results report"""
    # Console summary
    print("\n" + "=" * 60)
    print("Code Quality Check Results Summary")
    print("=" * 60)

    passed = 0
    total = len(results)

    for check_name, (success, _output) in results.items():
        status = "PASSED" if success else "FAILED"
        print(f"{check_name:20} : {status}")
        if success:
            passed += 1

    print(f"\nOverall results: {passed}/{total} passed")

    # Generate JSON report
    report_dir = Path("tools/quality/reports")
    report_dir.mkdir(exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    report_file = report_dir / f"quality_report_{timestamp}.json"

    report_data = {
        "timestamp": datetime.now().isoformat(),
        "summary": {
            "total_checks": total,
            "passed": passed,
            "failed": total - passed,
            "success_rate": round((passed / total) * 100, 2) if total > 0 else 0,
        },
        "results": {
            name: {"passed": success, "output": output[:1000]}  # Limit output length
            for name, (success, output) in results.items()
        },
    }

    with open(report_file, "w", encoding="utf-8") as f:
        json.dump(report_data, f, indent=2, ensure_ascii=False)

    print(f"Detailed report: {report_file}")

    # Exit with code 1 if any checks failed
    if passed < total:
        print(f"\n{total - passed} checks failed. Please review the errors above.")
        sys.exit(1)
    else:
        print("\nAll quality checks passed!")


def main():
    """Main function"""
    # Set Windows console encoding
    import codecs

    if sys.platform.startswith("win"):
        try:
            sys.stdout = codecs.getwriter("utf-8")(sys.stdout.buffer)
            sys.stderr = codecs.getwriter("utf-8")(sys.stderr.buffer)
        except AttributeError:
            pass

    print("DHT22 Project Code Quality Check Starting")
    print(f"Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    # Check if running from project root
    if not Path("pyproject.toml").exists():
        print("pyproject.toml not found. Please run from project root.")
        sys.exit(1)

    # Run each check
    checks = {
        "Import Check": check_imports(),
        "Ruff Linting": check_ruff(),
        "Ruff Formatting": check_ruff_format(),
        "MyPy Type Check": check_mypy(),
        "Unit Tests": run_tests(),
    }

    # Generate results report
    generate_report(checks)


if __name__ == "__main__":
    main()

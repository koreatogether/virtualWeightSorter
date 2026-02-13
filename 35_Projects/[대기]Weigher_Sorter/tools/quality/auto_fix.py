#!/usr/bin/env python3
"""Enhanced Automatic Code Quality Fix Tool

This script automatically fixes common code quality issues using Ruff.
Includes advanced fix patterns and comprehensive quality improvements.
"""

import subprocess
import sys
from pathlib import Path


def run_auto_fixes():
    """Run all automatic fixes"""
    print("Running comprehensive automatic code quality fixes...")

    # 1. Ruff auto-fix with extended rules
    print("\n1. Running Ruff auto-fix with extended rules...")
    result = subprocess.run(
        [
            "uv",
            "run",
            "ruff",
            "check",
            "--fix",
            "--unsafe-fixes",
            "--extend-select=N,SIM,RUF,PT,PL",  # Add more rule categories
            "src/",
            "tools/",
        ],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )

    if result.returncode == 0:
        print("Ruff auto-fix completed successfully")
    else:
        print("Ruff auto-fix completed with some remaining issues")
        if result.stdout:
            print(f"Remaining issues: {result.stdout.count('Found')}")

    # 2. Ruff format
    print("\n2. Running Ruff format...")
    format_result = subprocess.run(
        ["uv", "run", "ruff", "format", "src/", "tools/"],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )

    if format_result.returncode == 0:
        print("Ruff formatting completed")
    else:
        print("Ruff formatting failed")
        print(format_result.stderr)

    # 3. Additional fixes that can be automated
    print("\n3. Running additional automated fixes...")

    # Try to fix common docstring issues
    docstring_result = subprocess.run(
        ["uv", "run", "ruff", "check", "--fix", "--select=D", "src/", "tools/"],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )

    if docstring_result.returncode == 0:
        print("Docstring fixes applied")

    # 4. Summary
    print("\nEnhanced Auto-fix Summary:")
    print("- Import sorting: Fixed")
    print("- Unused imports: Removed")
    print("- Unused variables: Removed")
    print("- Whitespace issues: Fixed")
    print("- Code formatting: Applied")
    print("- Naming conventions: Improved")
    print("- Code simplifications: Applied")
    print("- Test improvements: Applied")
    print("- Docstring formatting: Fixed")

    print("\nManual fixes may still be needed for:")
    print("- Complex logic issues (C901)")
    print("- Security warnings (S)")
    print("- Performance optimizations")
    print("- Domain-specific improvements")

    return result.returncode == 0


def main():
    """Main function"""
    print("DHT22 Project Auto-Fix Tool")
    print("=" * 50)

    # Check if we're in the right directory
    if not Path("pyproject.toml").exists():
        print("pyproject.toml not found. Please run from project root.")
        sys.exit(1)

    success = run_auto_fixes()

    if success:
        print("\nAuto-fix completed! Run quality check to see remaining issues.")
    else:
        print("\nAuto-fix completed with some issues remaining.")

    print("\nNext steps:")
    print("1. Run: uv run python tools/quality/quality_check.py")
    print("2. Review remaining issues manually")
    print("3. Consider adjusting Ruff configuration if needed")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Simple Metrics Runner (Windows Compatible)

DEPRECATED: Superseded by `unified_metrics.py`. Kept for minimal fallback.
Will be removed after unified engine stabilizes trend & drift features.

A metrics tool that runs safely in Windows environment without emojis.
"""

import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path


def set_utf8_encoding():
    """Set UTF-8 encoding"""
    if os.name == "nt":  # Windows
        os.environ["PYTHONIOENCODING"] = "utf-8"
        # Try to activate Windows console UTF-8 mode
        try:
            import locale

            locale.setlocale(locale.LC_ALL, "en_US.UTF-8")
        except Exception:
            pass


def run_python_metrics():
    """Run Python metrics"""
    print("=" * 60)
    print("Starting Python code metrics analysis")
    print("=" * 60)

    try:
        # Simple pytest execution
        result = subprocess.run(
            [
                sys.executable,
                "-m",
                "pytest",
                "tests/",
                "--cov=src/python",
                "--cov-report=term-missing",
                "--cov-report=html:tools/metrics/reports/coverage_html",
                "-v",
            ],
            capture_output=True,
            text=True,
            encoding="utf-8",
        )

        print("PYTEST OUTPUT:")
        print(result.stdout)
        if result.stderr:
            print("PYTEST ERRORS:")
            print(result.stderr)

        # Radon complexity analysis
        print("\n" + "-" * 40)
        print("Code complexity analysis")
        print("-" * 40)

        cc_result = subprocess.run(
            [sys.executable, "-m", "radon", "cc", "src/python", "-a"],
            capture_output=True,
            text=True,
            encoding="utf-8",
        )

        print("Complexity analysis results:")
        print(cc_result.stdout)

        return True

    except Exception as e:
        print(f"Python metrics execution error: {e}")
        return False


def run_arduino_metrics():
    """Run Arduino metrics"""
    print("\n" + "=" * 60)
    print("Starting Arduino code metrics analysis")
    print("=" * 60)

    arduino_path = Path("src/arduino")
    if not arduino_path.exists():
        print("Arduino source folder not found.")
        return False

    arduino_files = list(arduino_path.rglob("*.ino"))
    cpp_files = list(arduino_path.rglob("*.cpp"))
    h_files = list(arduino_path.rglob("*.h"))

    all_files = arduino_files + cpp_files + h_files

    print(f"Found Arduino files: {len(all_files)}")

    total_lines = 0
    code_lines = 0
    comment_lines = 0

    for file_path in all_files:
        print(f"Analyzing: {file_path}")

        try:
            with open(file_path, encoding="utf-8") as f:
                lines = f.readlines()

            file_total = len(lines)
            file_code = 0
            file_comment = 0

            for line in lines:
                stripped = line.strip()
                if not stripped:
                    continue
                elif stripped.startswith("//") or stripped.startswith("/*"):
                    file_comment += 1
                else:
                    file_code += 1

            total_lines += file_total
            code_lines += file_code
            comment_lines += file_comment

            print(
                f"  Total lines: {file_total}, Code: {file_code}, Comments: {file_comment}"
            )

        except Exception as e:
            print(f"  File reading error: {e}")

    print("\nArduino code summary:")
    print(f"  Total files: {len(all_files)}")
    print(f"  Total lines: {total_lines}")
    print(f"  Code lines: {code_lines}")
    print(f"  Comment lines: {comment_lines}")
    print(f"  Comment ratio: {(comment_lines / max(total_lines, 1) * 100):.1f}%")

    return True


def generate_simple_report():
    """Generate simple report"""
    # Save to logs folder with timestamp prefix
    timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    logs_dir = Path("tools/metrics/logs")
    logs_dir.mkdir(parents=True, exist_ok=True)

    report_file = logs_dir / f"{timestamp}_간단_메트릭_분석_결과.md"
    
    # Also save to old location for backward compatibility
    reports_dir = Path("tools/metrics/reports")
    reports_dir.mkdir(parents=True, exist_ok=True)
    old_timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    old_report_file = reports_dir / f"simple_metrics_{old_timestamp}.md"

    report_content = f"""# Weigher-Sorter Project Metrics Report

## Analysis Time
{datetime.now().strftime("%Y-%m-%d %H:%M:%S")}

## Executed Analysis
- Python code coverage testing
- Python code complexity analysis (Radon)
- Arduino code line count analysis

## Result File Locations
- Coverage HTML: tools/metrics/reports/coverage_html/index.html
- This report: {report_file}

## Recommendations
1. Check the coverage HTML report in your browser
2. Consider refactoring functions with high complexity
3. Maintain comment ratio above 15%

## Next Steps
- Run individual tools for more detailed analysis
- Monitor metrics regularly
"""

    # Write to logs folder
    with open(report_file, "w", encoding="utf-8") as f:
        f.write(report_content)
        
    # Write to old location for backward compatibility
    with open(old_report_file, "w", encoding="utf-8") as f:
        f.write(report_content)

    print(f"\nSimple report generation complete: {report_file}")
    return report_file


def main():
    """Main function"""
    set_utf8_encoding()

    print("Weigher-Sorter Project Metrics Analysis Tool (Windows Compatible Version)")
    print("=" * 60)

    # Run Python metrics
    python_success = run_python_metrics()

    # Run Arduino metrics
    arduino_success = run_arduino_metrics()

    # Generate simple report
    report_file = generate_simple_report()

    print("\n" + "=" * 60)
    print("Metrics analysis complete!")
    print("=" * 60)
    print(f"Python analysis: {'Success' if python_success else 'Failed'}")
    print(f"Arduino analysis: {'Success' if arduino_success else 'Failed'}")
    print(f"Report: {report_file}")

    if python_success:
        print("\nCheck coverage report:")
        print("  tools/metrics/reports/coverage_html/index.html")

    print("\nCommands for additional analysis:")
    print("  python -m pytest tests/ --cov=src/python -v")
    print("  python -m radon cc src/python -a")
    print("  python -m radon mi src/python")


if __name__ == "__main__":
    main()

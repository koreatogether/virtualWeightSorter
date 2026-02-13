#!/usr/bin/env python3
"""Python Code Coverage and Metrics Analysis Tool

Provides coverage, complexity, quality, and line statistics with a retry
guard for intermittently empty coverage reports.
"""

import json
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Any


class PythonMetricsAnalyzer:
    def __init__(self):
        self.project_root = Path.cwd()
        self.src_path = self.project_root / "src" / "python"
        self.tools_path = self.project_root / "tools"
        self.tests_path = self.project_root / "tests"
        self.reports_dir = self.project_root / "tools" / "metrics" / "reports"
        self.reports_dir.mkdir(parents=True, exist_ok=True)

        self.python_files = list(self.src_path.rglob("*.py"))
        self.tool_files = list(self.tools_path.rglob("*.py"))

    def check_dependencies(self) -> dict[str, bool]:
        deps = {
            "pytest": False,
            "pytest-cov": False,
            "radon": False,
            "pylint": False,
            "cloc": False,
        }
        print("Checking dependencies...")
        for package in ["pytest", "pytest-cov", "radon", "pylint"]:
            try:
                result = subprocess.run(
                    [sys.executable, "-c", f"import {package.replace('-', '_')}"],
                    capture_output=True,
                    text=True,
                )
                deps[package] = result.returncode == 0
                print(f"  {'OK' if deps[package] else 'MISSING'} {package}")
            except Exception:
                print(f"  MISSING {package}")
        try:
            result = subprocess.run(
                ["cloc", "--version"], capture_output=True, text=True
            )
            deps["cloc"] = result.returncode == 0
            print(f"  {'OK' if deps['cloc'] else 'MISSING'} cloc")
        except Exception:
            print("  MISSING cloc")
        return deps

    def install_missing_dependencies(self, deps: dict[str, bool]) -> None:
        missing = [pkg for pkg, ok in deps.items() if not ok and pkg != "cloc"]
        if missing:
            print(f"\nInstalling missing packages: {', '.join(missing)}")
            try:
                subprocess.run(
                    [sys.executable, "-m", "pip", "install", *missing], check=True
                )
                print("Package installation complete")
            except subprocess.CalledProcessError as e:
                print(f"Package installation failed: {e}")
        if not deps["cloc"]:
            print(
                "\ncloc not installed (optional). Windows: choco install cloc | Linux: apt-get install cloc | macOS: brew install cloc"
            )

    def run_coverage_analysis(self) -> dict[str, Any]:
        """Run code coverage analysis"""
        print("\nRunning code coverage analysis...")

        coverage_data = {
            "timestamp": datetime.now().isoformat(),
            "total_coverage": 0,
            "files": {},
            "missing_lines": {},
            "summary": {},
        }

        try:
            # Measure coverage with pytest-cov
            # NOTE: Removed duplicate generic '--cov-report=json' which occasionally led
            # to empty coverage JSON artifacts in some environments.
            cmd = [
                sys.executable,
                "-m",
                "pytest",
                "--cov=src/python",
                "--cov-report=json:tools/reports/coverage.json",
                "--cov-report=html:" + str(self.reports_dir / "coverage_html"),
                "--cov-report=term-missing",
                "tests/",
                "-v",
            ]

            attempts = 0
            max_attempts = 2
            last_result: subprocess.CompletedProcess[str] | None = None
            while attempts < max_attempts:
                attempts += 1
                last_result = subprocess.run(
                    cmd, capture_output=True, text=True, cwd=self.project_root
                )
                coverage_json = Path("tools/reports/coverage.json")
                empty_flag = False
                if last_result.returncode == 0 and coverage_json.exists():
                    try:
                        with coverage_json.open(encoding="utf-8") as f:
                            coverage_raw = json.load(f)
                    except json.JSONDecodeError:
                        coverage_raw = {}
                    coverage_data["total_coverage"] = coverage_raw.get(
                        "totals", {}
                    ).get("percent_covered", 0)
                    coverage_data["files"] = coverage_raw.get("files", {})
                    coverage_data["summary"] = coverage_raw.get("totals", {})
                    if not coverage_data["files"]:
                        empty_flag = True
                        print(f"WARNING: Empty coverage report (attempt {attempts})")
                    else:
                        print(
                            f"Total coverage: {coverage_data['total_coverage']:.1f}% across {len(coverage_data['files'])} files"
                        )
                        break
                else:
                    break
                if empty_flag and attempts < max_attempts:
                    print("Retrying coverage collection due to empty report...")
            if last_result and last_result.returncode != 0:
                print(f"Coverage analysis failed: {last_result.stderr}")
                coverage_data["error"] = last_result.stderr or "pytest run failed"
                if last_result.stderr:
                    coverage_data["stderr_tail"] = last_result.stderr[-1200:]
            elif not coverage_data["files"]:
                coverage_data["error"] = "empty coverage report"
                if last_result and last_result.stderr:
                    coverage_data["stderr_tail"] = last_result.stderr[-1200:]
            elif coverage_data["total_coverage"] == 0 and coverage_data["files"]:
                print("WARNING: Coverage percent reported as 0.0 despite file entries")

        except Exception as e:
            print(f"Coverage analysis error: {e}")
            coverage_data["error"] = str(e)

        return coverage_data

    def run_complexity_analysis(self) -> dict[str, Any]:
        """Code complexity analysis (Radon)"""
        print("\nAnalyzing code complexity...")

        complexity_data = {
            "timestamp": datetime.now().isoformat(),
            "cyclomatic_complexity": {},
            "maintainability_index": {},
            "halstead_metrics": {},
        }

        try:
            # Cyclomatic Complexity
            cc_cmd = [sys.executable, "-m", "radon", "cc", str(self.src_path), "-j"]
            cc_result = subprocess.run(cc_cmd, capture_output=True, text=True)

            if cc_result.returncode == 0:
                complexity_data["cyclomatic_complexity"] = json.loads(cc_result.stdout)
                print("Cyclomatic complexity analysis completed")

            # Maintainability Index
            mi_cmd = [sys.executable, "-m", "radon", "mi", str(self.src_path), "-j"]
            mi_result = subprocess.run(mi_cmd, capture_output=True, text=True)

            if mi_result.returncode == 0:
                complexity_data["maintainability_index"] = json.loads(mi_result.stdout)
                print("Maintainability index analysis completed")

            # Halstead Metrics
            hal_cmd = [sys.executable, "-m", "radon", "hal", str(self.src_path), "-j"]
            hal_result = subprocess.run(hal_cmd, capture_output=True, text=True)

            if hal_result.returncode == 0:
                complexity_data["halstead_metrics"] = json.loads(hal_result.stdout)
                print("Halstead metrics analysis completed")

        except Exception as e:
            print(f"Complexity analysis error: {e}")
            complexity_data["error"] = str(e)

        return complexity_data

    def run_quality_analysis(self) -> dict[str, Any]:
        """Code quality analysis (Pylint)"""
        print("\nAnalyzing code quality...")

        quality_data = {
            "timestamp": datetime.now().isoformat(),
            "overall_score": 0,
            "files": {},
            "issues": [],
        }

        try:
            for py_file in self.python_files:
                if "__pycache__" in str(py_file):
                    continue

                cmd = [
                    sys.executable,
                    "-m",
                    "pylint",
                    str(py_file),
                    "--output-format=json",
                    "--disable=C0114,C0115,C0116",  # 문서화 관련 경고 비활성화
                ]

                result = subprocess.run(cmd, capture_output=True, text=True)

                # Pylint는 점수가 낮아도 0이 아닌 exit code를 반환할 수 있음
                if result.stdout:
                    try:
                        issues = json.loads(result.stdout)
                        quality_data["files"][str(py_file)] = {
                            "issues": issues,
                            "issue_count": len(issues),
                        }
                        quality_data["issues"].extend(issues)
                    except json.JSONDecodeError:
                        # JSON이 아닌 경우 텍스트로 처리
                        pass

            print(
                f"Quality analysis completed - {len(quality_data['issues'])} issues found"
            )

        except Exception as e:
            print(f"Quality analysis error: {e}")
            quality_data["error"] = str(e)

        return quality_data

    def run_line_count_analysis(self) -> dict[str, Any]:
        """Line count statistics analysis (cloc)"""
        print("\nAnalyzing line count statistics...")

        line_data = {
            "timestamp": datetime.now().isoformat(),
            "summary": {},
            "by_language": {},
            "by_file": {},
        }

        try:
            cmd = ["cloc", str(self.src_path), "--json"]
            result = subprocess.run(cmd, capture_output=True, text=True)

            if result.returncode == 0:
                cloc_data = json.loads(result.stdout)
                line_data["summary"] = cloc_data.get("SUM", {})
                line_data["by_language"] = {
                    k: v for k, v in cloc_data.items() if k not in ["header", "SUM"]
                }
                print("✅ 라인 수 통계 완료")

                # 요약 출력
                summary = line_data["summary"]
                if summary:
                    print(f"Total files: {summary.get('nFiles', 0)}")
                    print(f"Total lines: {summary.get('nLines', 0)}")
                    print(f"Code lines: {summary.get('nCode', 0)}")
                    print(f"Comment lines: {summary.get('nComment', 0)}")
                    print(f"Blank lines: {summary.get('nBlank', 0)}")
            else:
                print(
                    "Warning: cloc is not available. Attempting manual line calculation."
                )
                line_data = self._manual_line_count()

        except Exception as e:
            print(f"Line count analysis error: {e}")
            line_data["error"] = str(e)
            line_data = self._manual_line_count()

        return line_data

    def _manual_line_count(self) -> dict[str, Any]:
        """수동 라인 수 계산"""
        line_data = {
            "timestamp": datetime.now().isoformat(),
            "summary": {
                "nFiles": 0,
                "nLines": 0,
                "nCode": 0,
                "nComment": 0,
                "nBlank": 0,
            },
            "by_file": {},
        }

        for py_file in self.python_files:
            if "__pycache__" in str(py_file):
                continue

            try:
                with open(py_file, encoding="utf-8") as f:
                    lines = f.readlines()

                total_lines = len(lines)
                code_lines = 0
                comment_lines = 0
                blank_lines = 0

                for line in lines:
                    stripped = line.strip()
                    if not stripped:
                        blank_lines += 1
                    elif stripped.startswith("#"):
                        comment_lines += 1
                    else:
                        code_lines += 1

                file_data = {
                    "nLines": total_lines,
                    "nCode": code_lines,
                    "nComment": comment_lines,
                    "nBlank": blank_lines,
                }

                line_data["by_file"][str(py_file)] = file_data
                line_data["summary"]["nFiles"] += 1
                line_data["summary"]["nLines"] += total_lines
                line_data["summary"]["nCode"] += code_lines
                line_data["summary"]["nComment"] += comment_lines
                line_data["summary"]["nBlank"] += blank_lines

            except Exception as e:
                print(f"⚠️  파일 읽기 오류 {py_file}: {e}")

        return line_data

    def generate_comprehensive_report(
        self,
        coverage_data: dict,
        complexity_data: dict,
        quality_data: dict,
        line_data: dict,
    ) -> None:
        """Generate comprehensive report"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

        # JSON 리포트
        comprehensive_data = {
            "timestamp": datetime.now().isoformat(),
            "project": "DHT22 Environmental Monitoring",
            "coverage": coverage_data,
            "complexity": complexity_data,
            "quality": quality_data,
            "lines": line_data,
            "summary": {
                "total_coverage": coverage_data.get("total_coverage", 0),
                "total_files": line_data.get("summary", {}).get("nFiles", 0),
                "total_lines": line_data.get("summary", {}).get("nLines", 0),
                "code_lines": line_data.get("summary", {}).get("nCode", 0),
                "quality_issues": len(quality_data.get("issues", [])),
            },
        }

        json_report = self.reports_dir / f"python_metrics_{timestamp}.json"
        with open(json_report, "w", encoding="utf-8") as f:
            json.dump(comprehensive_data, f, indent=2, ensure_ascii=False)

        # Markdown 리포트
        md_report = self.reports_dir / f"python_metrics_{timestamp}.md"
        self._generate_markdown_report(md_report, comprehensive_data)

        print("\nComprehensive report generation completed:")
        print(f"   JSON: {json_report}")
        print(f"   Markdown: {md_report}")
        if coverage_data.get("total_coverage", 0) > 0:
            print(f"   HTML Coverage: {self.reports_dir}/coverage_html/index.html")

    def _generate_markdown_report(self, report_path: Path, data: dict) -> None:
        """Markdown 리포트 생성"""
        with open(report_path, "w", encoding="utf-8") as f:
            f.write(
                f"""# Python Code Metrics Report

## Project Overview
- **Project**: {data["project"]}
- **Analysis Time**: {data["timestamp"]}
- **Total Files**: {data["summary"]["total_files"]} files
- **Total Lines**: {data["summary"]["total_lines"]} lines

## Coverage Analysis
- **Total Coverage**: {data["summary"]["total_coverage"]:.1f}%
- **Coverage Status**: {"Excellent" if data["summary"]["total_coverage"] >= 80 else "Good" if data["summary"]["total_coverage"] >= 60 else "Needs Improvement"}

## Code Complexity
- **Analysis Complete**: {"Yes" if "error" not in data["complexity"] else "No"}

## Code Quality
- **품질 이슈**: {data["summary"]["quality_issues"]}개
- **품질 상태**: {"🟢 우수" if data["summary"]["quality_issues"] <= 10 else "🟡 보통" if data["summary"]["quality_issues"] <= 50 else "🔴 개선 필요"}

## 📏 라인 수 통계
- **코드 라인**: {data["summary"]["code_lines"]}줄
- **주석 비율**: {(data["lines"]["summary"].get("nComment", 0) / max(data["summary"]["total_lines"], 1) * 100):.1f}%

## Recommendations
"""
            )

            # 권장사항 생성
            recommendations = []

            if data["summary"]["total_coverage"] < 80:
                recommendations.append("- Improve test coverage to 80% or higher")

            if data["summary"]["quality_issues"] > 20:
                recommendations.append("- 코드 품질 이슈를 20개 이하로 줄이세요")

            comment_ratio = (
                data["lines"]["summary"].get("nComment", 0)
                / max(data["summary"]["total_lines"], 1)
                * 100
            )
            if comment_ratio < 10:
                recommendations.append("- 주석 비율을 10% 이상으로 늘리세요")

            if not recommendations:
                recommendations.append("- Current code quality is excellent!")

            for rec in recommendations:
                f.write(f"{rec}\n")


def main():
    """Main function"""
    print("Python Code Metrics and Coverage Analysis Started")
    print("=" * 60)

    analyzer = PythonMetricsAnalyzer()

    # 의존성 확인 및 설치
    dependencies = analyzer.check_dependencies()
    analyzer.install_missing_dependencies(dependencies)

    # Run analysis
    coverage_data = analyzer.run_coverage_analysis()
    complexity_data = analyzer.run_complexity_analysis()
    quality_data = analyzer.run_quality_analysis()
    line_data = analyzer.run_line_count_analysis()

    # Generate comprehensive report
    analyzer.generate_comprehensive_report(
        coverage_data, complexity_data, quality_data, line_data
    )

    print("\nPython metrics analysis completed!")


if __name__ == "__main__":
    main()

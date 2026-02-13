#!/usr/bin/env python3
"""Integrated Metrics Management System

DEPRECATED: This legacy script is retained temporarily for reference.
Use `tools/metrics/unified_metrics.py --full` instead. Plan removal after Phase 5C.

Integrates and manages metrics for Python and Arduino code.
- Overall project metrics dashboard
- Time-based metrics trend analysis
- Quality target progress tracking
- Automated report generation
"""

import json
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Any

# Make matplotlib optional: the script can run without it and will skip chart generation
try:
    import matplotlib.pyplot as plt

    # Configure Korean font for matplotlib (best-effort)
    try:
        plt.rcParams["font.family"] = "Malgun Gothic"
        plt.rcParams["axes.unicode_minus"] = False
    except Exception:
        # If font configuration fails, continue without failing the whole script
        pass

    _MATPLOTLIB_AVAILABLE = True
except Exception:
    plt = None  # type: ignore
    _MATPLOTLIB_AVAILABLE = False


class IntegratedMetricsManager:
    def __init__(self):
        self.project_root = Path.cwd()
        self.metrics_dir = self.project_root / "tools" / "metrics"
        self.reports_dir = self.metrics_dir / "reports"
        self.reports_dir.mkdir(parents=True, exist_ok=True)

        # 품질 목표 설정
        self.quality_targets = {
            "python": {
                "coverage": 80.0,  # 80% 이상
                "quality_issues": 20,  # 20개 이하
                "comment_ratio": 10.0,  # 10% 이상
                "max_complexity": 10,  # 함수당 복잡도 10 이하
            },
            "arduino": {
                "comment_ratio": 15.0,  # 15% 이상
                "avg_complexity": 8.0,  # 평균 복잡도 8 이하
                "max_flash": 25000,  # 25KB 이하
                "max_ram": 1500,  # 1.5KB 이하
            },
        }

    def run_all_analyses(self) -> dict[str, Any]:
        """Run all metrics analysis"""
        print("Starting integrated metrics analysis")
        print("=" * 60)

        results = {
            "timestamp": datetime.now().isoformat(),
            "python": None,
            "arduino": None,
            "integration": {},
        }

        # Run Python metrics
        print("\nRunning Python metrics analysis...")
        try:
            python_result = subprocess.run(
                [sys.executable, str(self.metrics_dir / "python_coverage.py")],
                capture_output=True,
                text=True,
                cwd=self.project_root,
            )

            if python_result.returncode == 0:
                print("Python analysis complete")
                results["python"] = self._load_latest_python_report()
            else:
                print(f"Python analysis failed: {python_result.stderr}")
                results["python"] = {"error": python_result.stderr}
        except Exception as e:
            print(f"Python analysis error: {e}")
            results["python"] = {"error": str(e)}

        # Run Arduino metrics
        print("\nRunning Arduino metrics analysis...")
        try:
            arduino_result = subprocess.run(
                [sys.executable, str(self.metrics_dir / "arduino_metrics.py")],
                capture_output=True,
                text=True,
                cwd=self.project_root,
            )

            if arduino_result.returncode == 0:
                print("Arduino analysis complete")
                results["arduino"] = self._load_latest_arduino_report()
            else:
                print(f"Arduino analysis failed: {arduino_result.stderr}")
                results["arduino"] = {"error": arduino_result.stderr}
        except Exception as e:
            print(f"Arduino analysis error: {e}")
            results["arduino"] = {"error": str(e)}

        # Integration analysis
        results["integration"] = self._perform_integration_analysis(results)

        return results

    def _load_latest_python_report(self) -> dict | None:
        """최신 Python 리포트 로드"""
        python_reports = list(self.reports_dir.glob("python_metrics_*.json"))
        if not python_reports:
            return None

        latest_report = max(python_reports, key=lambda x: x.stat().st_mtime)
        try:
            with open(latest_report, encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            print(f"⚠️  Python 리포트 로드 실패: {e}")
            return None

    def _load_latest_arduino_report(self) -> dict | None:
        """최신 Arduino 리포트 로드"""
        arduino_reports = list(self.reports_dir.glob("arduino_metrics_*.json"))
        if not arduino_reports:
            return None

        latest_report = max(arduino_reports, key=lambda x: x.stat().st_mtime)
        try:
            with open(latest_report, encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            print(f"⚠️  Arduino 리포트 로드 실패: {e}")
            return None

    def _perform_integration_analysis(self, results: dict) -> dict[str, Any]:
        """Perform integration analysis"""
        integration = {
            "project_health": "unknown",
            "quality_score": 0,
            "recommendations": [],
            "achievements": [],
            "total_lines": 0,
            "total_files": 0,
            "language_distribution": {},
        }

        python_data = results.get("python")
        arduino_data = results.get("arduino")

        # 전체 통계 계산
        if python_data and "summary" in python_data:
            py_summary = python_data["summary"]
            integration["total_lines"] += py_summary.get("total_lines", 0)
            integration["total_files"] += py_summary.get("total_files", 0)
            integration["language_distribution"]["Python"] = {
                "lines": py_summary.get("code_lines", 0),
                "files": py_summary.get("total_files", 0),
            }

        if arduino_data and "summary" in arduino_data:
            ard_summary = arduino_data["summary"]
            integration["total_lines"] += ard_summary.get("total_lines", 0)
            integration["total_files"] += ard_summary.get("total_files", 0)
            integration["language_distribution"]["Arduino"] = {
                "lines": ard_summary.get("code_lines", 0),
                "files": ard_summary.get("total_files", 0),
            }

        # Calculate quality score
        quality_score = 0
        max_score = 0

        # Python 품질 평가
        if python_data and "summary" in python_data:
            py_summary = python_data["summary"]
            targets = self.quality_targets["python"]

            # Coverage score (25 points)
            coverage = py_summary.get("total_coverage", 0)
            if coverage >= targets["coverage"]:
                quality_score += 25
                integration["achievements"].append(
                    f"Python coverage target achieved: {coverage:.1f}%"
                )
            else:
                quality_score += (coverage / targets["coverage"]) * 25
                integration["recommendations"].append(
                    f"Improve Python coverage to {targets['coverage']}% (current: {coverage:.1f}%)"
                )
            max_score += 25

            # 품질 이슈 점수 (25점)
            issues = py_summary.get("quality_issues", 0)
            if issues <= targets["quality_issues"]:
                quality_score += 25
                integration["achievements"].append(
                    f"Python 품질 이슈 목표 달성: {issues}개"
                )
            else:
                quality_score += max(
                    0,
                    (targets["quality_issues"] - issues)
                    / targets["quality_issues"]
                    * 25,
                )
                integration["recommendations"].append(
                    f"Python 품질 이슈를 {targets['quality_issues']}개 이하로 줄이세요 (현재: {issues}개)"
                )
            max_score += 25

        # Arduino 품질 평가
        if arduino_data and "summary" in arduino_data:
            ard_summary = arduino_data["summary"]
            targets = self.quality_targets["arduino"]

            # 복잡도 점수 (25점)
            avg_complexity = ard_summary.get("avg_complexity", 0)
            if avg_complexity <= targets["avg_complexity"]:
                quality_score += 25
                integration["achievements"].append(
                    f"Arduino 복잡도 목표 달성: {avg_complexity:.1f}"
                )
            else:
                quality_score += max(
                    0,
                    (targets["avg_complexity"] - avg_complexity)
                    / targets["avg_complexity"]
                    * 25,
                )
                integration["recommendations"].append(
                    f"Arduino 평균 복잡도를 {targets['avg_complexity']} 이하로 줄이세요 (현재: {avg_complexity:.1f})"
                )
            max_score += 25

            # 메모리 사용량 점수 (25점)
            flash_usage = ard_summary.get("estimated_flash", 0)
            if flash_usage <= targets["max_flash"]:
                quality_score += 25
                integration["achievements"].append(
                    f"Arduino Flash 메모리 목표 달성: {flash_usage:,}B"
                )
            else:
                quality_score += max(
                    0, (targets["max_flash"] - flash_usage) / targets["max_flash"] * 25
                )
                integration["recommendations"].append(
                    f"Arduino Flash 사용량을 {targets['max_flash']:,}B 이하로 줄이세요 (현재: {flash_usage:,}B)"
                )
            max_score += 25

        # Calculate final quality score
        if max_score > 0:
            integration["quality_score"] = (quality_score / max_score) * 100

        # 프로젝트 건강도 평가
        if integration["quality_score"] >= 90:
            integration["project_health"] = "excellent"
        elif integration["quality_score"] >= 75:
            integration["project_health"] = "good"
        elif integration["quality_score"] >= 60:
            integration["project_health"] = "fair"
        else:
            integration["project_health"] = "needs_improvement"

        return integration

    def generate_dashboard_report(self, results: dict) -> None:
        """대시보드 리포트 생성"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

        # JSON 리포트
        json_report = self.reports_dir / f"integrated_metrics_{timestamp}.json"
        with open(json_report, "w", encoding="utf-8") as f:
            json.dump(results, f, indent=2, ensure_ascii=False)

        # Markdown 대시보드
        md_report = self.reports_dir / f"metrics_dashboard_{timestamp}.md"
        self._generate_dashboard_markdown(md_report, results)

        # 시각화 차트 생성 (matplotlib 사용 가능한 경우)
        try:
            self._generate_charts(results, timestamp)
        except ImportError:
            print(
                "⚠️  matplotlib를 설치하면 시각화 차트를 생성할 수 있습니다: pip install matplotlib"
            )
        except Exception as e:
            print(f"⚠️  차트 생성 실패: {e}")

        print("\nIntegrated dashboard generation completed:")
        print(f"   JSON: {json_report}")
        print(f"   Dashboard: {md_report}")

    def _generate_dashboard_markdown(self, report_path: Path, results: dict) -> None:
        """대시보드 Markdown 생성"""
        integration = results.get("integration", {})
        python_data = results.get("python")
        arduino_data = results.get("arduino")

        with open(report_path, "w", encoding="utf-8") as f:
            f.write(
                f"""# DHT22 Project Metrics Dashboard

## Project Health
- **Overall Quality Score**: {integration.get("quality_score", 0):.1f}/100
- **Project Status**: {integration.get("project_health", "unknown").replace("_", " ").title()}
- **Analysis Time**: {results.get("timestamp", "N/A")}

## Overall Statistics
- **Total Files**: {integration.get("total_files", 0)} files
- **Total Lines**: {integration.get("total_lines", 0):,} lines

### Language Distribution
"""
            )

            for lang, stats in integration.get("language_distribution", {}).items():
                f.write(
                    f"- **{lang}**: {stats['files']}개 파일, {stats['lines']:,}줄\n"
                )

            f.write(
                """
## Python Metrics
"""
            )

            if python_data and "summary" in python_data:
                py_summary = python_data["summary"]
                f.write(
                    f"""- **Coverage**: {py_summary.get("total_coverage", 0):.1f}%
- **Quality Issues**: {py_summary.get("quality_issues", 0)} issues
- **Code Lines**: {py_summary.get("code_lines", 0):,} lines
- **파일 수**: {py_summary.get("total_files", 0)}개
"""
                )
            else:
                f.write("- No Python analysis data available\n")

            f.write(
                """
## Arduino Metrics
"""
            )

            if arduino_data and "summary" in arduino_data:
                ard_summary = arduino_data["summary"]
                f.write(
                    f"""- **평균 복잡도**: {ard_summary.get("avg_complexity", 0):.1f}
- **Flash 메모리**: ~{ard_summary.get("estimated_flash", 0):,}바이트
- **RAM 사용량**: ~{ard_summary.get("estimated_ram", 0):,}바이트
- **함수 수**: {ard_summary.get("total_functions", 0)}개
- **라이브러리**: {ard_summary.get("library_count", 0)}개
"""
                )
            else:
                f.write("- No Arduino analysis data available\n")

            f.write(
                """
## Achievements
"""
            )

            achievements = integration.get("achievements", [])
            if achievements:
                for achievement in achievements:
                    f.write(f"- ✅ {achievement}\n")
            else:
                f.write("- 아직 달성한 목표가 없습니다.\n")

            f.write(
                """
## Improvement Recommendations
"""
            )

            recommendations = integration.get("recommendations", [])
            if recommendations:
                for rec in recommendations:
                    f.write(f"- {rec}\n")
            else:
                f.write("- All quality targets have been achieved!\n")

            f.write(
                f"""
## Quality Targets
### Python
- Coverage: {self.quality_targets["python"]["coverage"]}% or higher
- Quality Issues: {self.quality_targets["python"]["quality_issues"]} or fewer
- Comment Ratio: {self.quality_targets["python"]["comment_ratio"]}% or higher

### Arduino
- 평균 복잡도: {self.quality_targets["arduino"]["avg_complexity"]} 이하
- Flash 메모리: {self.quality_targets["arduino"]["max_flash"]:,}바이트 이하
- RAM 사용량: {self.quality_targets["arduino"]["max_ram"]:,}바이트 이하
- 주석 비율: {self.quality_targets["arduino"]["comment_ratio"]}% 이상
"""
            )

    def _generate_charts(self, results: dict, timestamp: str) -> None:
        """시각화 차트 생성"""
        # If matplotlib is not available, signal to caller so it can handle gracefully
        if not _MATPLOTLIB_AVAILABLE:
            raise ImportError("matplotlib is not available")

        integration = results.get("integration", {})

        # 언어별 분포 파이 차트
        lang_dist = integration.get("language_distribution", {})
        if lang_dist:
            plt.figure(figsize=(10, 6))

            # 라인 수 분포
            plt.subplot(1, 2, 1)
            languages = list(lang_dist.keys())
            lines = [lang_dist[lang]["lines"] for lang in languages]
            plt.pie(lines, labels=languages, autopct="%1.1f%%", startangle=90)
            plt.title("언어별 코드 라인 분포")

            # 파일 수 분포
            plt.subplot(1, 2, 2)
            files = [lang_dist[lang]["files"] for lang in languages]
            plt.pie(files, labels=languages, autopct="%1.1f%%", startangle=90)
            plt.title("언어별 파일 수 분포")

            plt.tight_layout()
            chart_path = self.reports_dir / f"language_distribution_{timestamp}.png"
            plt.savefig(chart_path, dpi=300, bbox_inches="tight")
            plt.close()

            print(f"   차트: {chart_path}")

        # Quality score gauge chart
        quality_score = integration.get("quality_score", 0)
        plt.figure(figsize=(8, 6))

        # Display quality score with simple bar chart
        categories = ["Quality Score"]
        scores = [quality_score]
        colors = [
            (
                "green"
                if quality_score >= 80
                else "orange"
                if quality_score >= 60
                else "red"
            )
        ]

        bars = plt.bar(categories, scores, color=colors, alpha=0.7)
        plt.ylim(0, 100)
        plt.ylabel("Score")
        plt.title(f"Project Quality Score: {quality_score:.1f}/100")

        # Add score text
        for bar, score in zip(bars, scores, strict=False):
            plt.text(
                bar.get_x() + bar.get_width() / 2,
                bar.get_height() + 1,
                f"{score:.1f}",
                ha="center",
                va="bottom",
                fontsize=12,
                fontweight="bold",
            )

        chart_path = self.reports_dir / f"quality_score_{timestamp}.png"
        plt.savefig(chart_path, dpi=300, bbox_inches="tight")
        plt.close()

        print(f"   차트: {chart_path}")

    def analyze_trends(self) -> dict[str, Any]:
        """Analyze metrics trends"""
        print("\nAnalyzing metrics trends...")

        # 최근 리포트들 수집
        all_reports = list(self.reports_dir.glob("integrated_metrics_*.json"))

        if len(all_reports) < 2:
            print("Warning: At least 2 reports are required for trend analysis.")
            return {"error": "insufficient_data"}

        # 시간순 정렬
        all_reports.sort(key=lambda x: x.stat().st_mtime)

        trends = {
            "timestamp": datetime.now().isoformat(),
            "report_count": len(all_reports),
            "time_range": {},
            "quality_trend": [],
            "coverage_trend": [],
            "complexity_trend": [],
        }

        for report_path in all_reports[-10:]:  # 최근 10개만
            try:
                with open(report_path, encoding="utf-8") as f:
                    data = json.load(f)

                timestamp = data.get("timestamp", "")
                integration = data.get("integration", {})
                python_data = data.get("python", {})

                trends["quality_trend"].append(
                    {
                        "timestamp": timestamp,
                        "quality_score": integration.get("quality_score", 0),
                    }
                )

                if python_data and "summary" in python_data:
                    trends["coverage_trend"].append(
                        {
                            "timestamp": timestamp,
                            "coverage": python_data["summary"].get("total_coverage", 0),
                        }
                    )

            except Exception as e:
                print(f"⚠️  리포트 읽기 실패 {report_path}: {e}")

        if trends["quality_trend"]:
            first_report = datetime.fromisoformat(
                trends["quality_trend"][0]["timestamp"].replace("Z", "+00:00")
            )
            last_report = datetime.fromisoformat(
                trends["quality_trend"][-1]["timestamp"].replace("Z", "+00:00")
            )
            trends["time_range"] = {
                "start": first_report.isoformat(),
                "end": last_report.isoformat(),
                "days": (last_report - first_report).days,
            }

        print(f"Trend analysis completed - {len(trends['quality_trend'])} data points")
        return trends


def main():
    """Main function"""
    print("DHT22 Integrated Metrics Management System")
    print("=" * 60)

    manager = IntegratedMetricsManager()

    # 전체 분석 실행
    results = manager.run_all_analyses()

    # 대시보드 리포트 생성
    manager.generate_dashboard_report(results)

    # Trend analysis (optional)
    trends = manager.analyze_trends()
    if "error" not in trends:
        print(
            f"\n📈 추이 분석: {trends['report_count']}개 리포트, {trends['time_range'].get('days', 0)}일간"
        )

    # 요약 출력
    integration = results.get("integration", {})
    print("\n🎯 최종 결과:")
    print(f"   품질 점수: {integration.get('quality_score', 0):.1f}/100")
    print(f"   프로젝트 상태: {integration.get('project_health', 'unknown')}")
    print(f"   총 파일: {integration.get('total_files', 0)}개")
    print(f"   총 라인: {integration.get('total_lines', 0):,}줄")
    print(f"   달성 사항: {len(integration.get('achievements', []))}개")
    print(f"   개선 권장: {len(integration.get('recommendations', []))}개")

    print("\n🎉 통합 메트릭스 분석 완료!")


if __name__ == "__main__":
    # Set console encoding to UTF-8 for Windows
    if os.name == "nt":
        try:
            sys.stdout.reconfigure(encoding="utf-8")
            sys.stderr.reconfigure(encoding="utf-8")
        except Exception:
            import codecs

            try:
                sys.stdout = codecs.getwriter("utf-8")(sys.stdout.detach())
                sys.stderr = codecs.getwriter("utf-8")(sys.stderr.detach())
            except Exception:
                pass

    main()

#!/usr/bin/env python3
"""Arduino Code Metrics Analysis Tool

This script performs the following:
- Arduino code line count analysis
- Function complexity analysis
- Memory usage estimation
- Library dependency analysis
- Code quality metrics
"""

import json
import re
from datetime import datetime
from pathlib import Path
from typing import Any


class ArduinoMetricsAnalyzer:
    def __init__(self):
        self.project_root = Path.cwd()
        self.arduino_path = self.project_root / "src" / "arduino"
        self.reports_dir = self.project_root / "tools" / "metrics" / "reports"
        self.reports_dir.mkdir(parents=True, exist_ok=True)

        # Arduino 파일들 찾기
        self.arduino_files = list(self.arduino_path.rglob("*.ino"))
        self.cpp_files = list(self.arduino_path.rglob("*.cpp"))
        self.h_files = list(self.arduino_path.rglob("*.h"))

        self.all_files = self.arduino_files + self.cpp_files + self.h_files

    def analyze_line_metrics(self) -> dict[str, Any]:
        """Analyze line count metrics"""
        print("Analyzing Arduino line metrics...")

        metrics = {
            "timestamp": datetime.now().isoformat(),
            "files": {},
            "summary": {
                "total_files": 0,
                "total_lines": 0,
                "code_lines": 0,
                "comment_lines": 0,
                "blank_lines": 0,
                "preprocessor_lines": 0,
            },
        }

        for file_path in self.all_files:
            file_metrics = self._analyze_single_file(file_path)
            metrics["files"][str(file_path)] = file_metrics

            # 요약에 추가
            metrics["summary"]["total_files"] += 1
            metrics["summary"]["total_lines"] += file_metrics["total_lines"]
            metrics["summary"]["code_lines"] += file_metrics["code_lines"]
            metrics["summary"]["comment_lines"] += file_metrics["comment_lines"]
            metrics["summary"]["blank_lines"] += file_metrics["blank_lines"]
            metrics["summary"]["preprocessor_lines"] += file_metrics[
                "preprocessor_lines"
            ]

        print(f"Line analysis complete - {metrics['summary']['total_files']} files")
        return metrics

    def _analyze_single_file(self, file_path: Path) -> dict[str, Any]:
        """Analyze single file"""
        try:
            with open(file_path, encoding="utf-8") as f:
                lines = f.readlines()
        except UnicodeDecodeError:
            # UTF-8이 안되면 다른 인코딩 시도
            try:
                with open(file_path, encoding="latin-1") as f:
                    lines = f.readlines()
            except Exception as e:
                print(f"File reading error {file_path}: {e}")
                return {"error": str(e)}

        metrics = {
            "total_lines": len(lines),
            "code_lines": 0,
            "comment_lines": 0,
            "blank_lines": 0,
            "preprocessor_lines": 0,
            "functions": [],
            "includes": [],
            "defines": [],
        }

        in_multiline_comment = False

        for i, line in enumerate(lines, 1):
            stripped = line.strip()

            # Empty line
            if not stripped:
                metrics["blank_lines"] += 1
                continue

            # Multiline comment handling
            if "/*" in stripped and "*/" in stripped:
                # Start and end on same line
                metrics["comment_lines"] += 1
                continue
            elif "/*" in stripped:
                in_multiline_comment = True
                metrics["comment_lines"] += 1
                continue
            elif "*/" in stripped:
                in_multiline_comment = False
                metrics["comment_lines"] += 1
                continue
            elif in_multiline_comment:
                metrics["comment_lines"] += 1
                continue

            # Single line comment
            if stripped.startswith("//"):
                metrics["comment_lines"] += 1
                continue

            # Preprocessor directive
            if stripped.startswith("#"):
                metrics["preprocessor_lines"] += 1

                # Include analysis
                if stripped.startswith("#include"):
                    include_match = re.search(r'#include\s*[<"]([^>"]+)[>"]', stripped)
                    if include_match:
                        metrics["includes"].append(include_match.group(1))

                # Define analysis
                elif stripped.startswith("#define"):
                    define_match = re.search(r"#define\s+(\w+)", stripped)
                    if define_match:
                        metrics["defines"].append(define_match.group(1))
                continue

            # Function definition search
            function_match = re.search(r"(\w+\s+)?(\w+)\s*\([^)]*\)\s*\{?", stripped)
            if (
                function_match
                and not stripped.startswith("if")
                and not stripped.startswith("while")
            ):
                func_name = function_match.group(2)
                if func_name not in ["if", "while", "for", "switch"]:
                    metrics["functions"].append(
                        {"name": func_name, "line": i, "signature": stripped}
                    )

            # Regular code line
            metrics["code_lines"] += 1

        return metrics

    def analyze_complexity(self) -> dict[str, Any]:
        """Code complexity analysis"""
        print("Analyzing Arduino complexity...")

        complexity_data = {
            "timestamp": datetime.now().isoformat(),
            "files": {},
            "summary": {
                "total_functions": 0,
                "avg_complexity": 0,
                "max_complexity": 0,
                "complex_functions": [],
            },
        }

        total_complexity = 0

        for file_path in self.all_files:
            file_complexity = self._analyze_file_complexity(file_path)
            complexity_data["files"][str(file_path)] = file_complexity

            for func in file_complexity.get("functions", []):
                complexity_data["summary"]["total_functions"] += 1
                func_complexity = func.get("complexity", 0)
                total_complexity += func_complexity

                if func_complexity > complexity_data["summary"]["max_complexity"]:
                    complexity_data["summary"]["max_complexity"] = func_complexity

                if func_complexity > 10:  # Complexity 10+ is complex function
                    complexity_data["summary"]["complex_functions"].append(
                        {
                            "file": str(file_path),
                            "function": func["name"],
                            "complexity": func_complexity,
                        }
                    )

        if complexity_data["summary"]["total_functions"] > 0:
            complexity_data["summary"]["avg_complexity"] = (
                total_complexity / complexity_data["summary"]["total_functions"]
            )

        print(
            f"Complexity analysis complete - Average complexity: {complexity_data['summary']['avg_complexity']:.1f}"
        )
        return complexity_data

    def _analyze_file_complexity(self, file_path: Path) -> dict[str, Any]:
        """File-level complexity analysis"""
        try:
            with open(file_path, encoding="utf-8") as f:
                content = f.read()
        except UnicodeDecodeError:
            try:
                with open(file_path, encoding="latin-1") as f:
                    content = f.read()
            except Exception as e:
                return {"error": str(e)}

        # Calculate complexity per function
        functions = []

        # 간단한 함수 추출 (정확하지 않지만 근사치)
        function_pattern = (
            r"(\w+\s+)?(\w+)\s*\([^)]*\)\s*\{([^}]*(?:\{[^}]*\}[^}]*)*)\}"
        )
        matches = re.finditer(function_pattern, content, re.DOTALL)

        for match in matches:
            func_name = match.group(2)
            func_body = match.group(3)

            if func_name in ["if", "while", "for", "switch"]:
                continue

            # Calculate cyclomatic complexity (simple version)
            complexity = 1  # Base complexity

            # Count conditional statements
            complexity += len(re.findall(r"\bif\b", func_body))
            complexity += len(re.findall(r"\belse\b", func_body))
            complexity += len(re.findall(r"\bwhile\b", func_body))
            complexity += len(re.findall(r"\bfor\b", func_body))
            complexity += len(re.findall(r"\bswitch\b", func_body))
            complexity += len(re.findall(r"\bcase\b", func_body))
            complexity += len(re.findall(r"\bcatch\b", func_body))
            complexity += len(re.findall(r"\b&&\b", func_body))
            complexity += len(re.findall(r"\b\|\|\b", func_body))

            functions.append(
                {
                    "name": func_name,
                    "complexity": complexity,
                    "lines": len(func_body.split("\n")),
                }
            )

        return {"functions": functions}

    def analyze_memory_usage(self) -> dict[str, Any]:
        """Memory usage estimation"""
        print("Estimating Arduino memory usage...")

        memory_data = {
            "timestamp": datetime.now().isoformat(),
            "estimated_flash": 0,
            "estimated_ram": 0,
            "libraries": [],
            "large_arrays": [],
            "string_literals": [],
        }

        for file_path in self.all_files:
            file_memory = self._estimate_file_memory(file_path)
            memory_data["estimated_flash"] += file_memory["flash"]
            memory_data["estimated_ram"] += file_memory["ram"]
            memory_data["libraries"].extend(file_memory["libraries"])
            memory_data["large_arrays"].extend(file_memory["large_arrays"])
            memory_data["string_literals"].extend(file_memory["string_literals"])

        # 중복 제거
        memory_data["libraries"] = list(set(memory_data["libraries"]))

        print(
            f"Memory estimation complete - Flash: ~{memory_data['estimated_flash']}B, RAM: ~{memory_data['estimated_ram']}B"
        )
        return memory_data

    def _estimate_file_memory(self, file_path: Path) -> dict[str, Any]:
        """File-level memory usage estimation"""
        try:
            with open(file_path, encoding="utf-8") as f:
                content = f.read()
        except UnicodeDecodeError:
            try:
                with open(file_path, encoding="latin-1") as f:
                    content = f.read()
            except Exception:
                return {
                    "flash": 0,
                    "ram": 0,
                    "libraries": [],
                    "large_arrays": [],
                    "string_literals": [],
                }

        memory_data = {
            "flash": 0,
            "ram": 0,
            "libraries": [],
            "large_arrays": [],
            "string_literals": [],
        }

        # 라이브러리 include 찾기
        includes = re.findall(r'#include\s*[<"]([^>"]+)[>"]', content)
        memory_data["libraries"] = includes

        # Approximate memory usage per library (estimated)
        library_flash_cost = {
            "DHT.h": 2000,
            "ArduinoJson.h": 8000,
            "WiFi.h": 15000,
            "Ethernet.h": 12000,
            "SoftwareSerial.h": 1500,
            "Wire.h": 1000,
            "SPI.h": 800,
        }

        for lib in includes:
            memory_data["flash"] += library_flash_cost.get(
                lib, 500
            )  # Default 500 bytes

        # 큰 배열 찾기
        array_pattern = r"(\w+)\s+(\w+)\s*\[\s*(\d+)\s*\]"
        arrays = re.findall(array_pattern, content)

        for array_type, array_name, array_size in arrays:
            size = int(array_size)
            if size > 50:  # 50개 이상의 요소를 가진 배열
                type_size = self._get_type_size(array_type)
                memory_usage = size * type_size
                memory_data["ram"] += memory_usage
                memory_data["large_arrays"].append(
                    {
                        "name": array_name,
                        "type": array_type,
                        "size": size,
                        "memory": memory_usage,
                    }
                )

        # 문자열 리터럴 찾기
        string_literals = re.findall(r'"([^"]*)"', content)
        for literal in string_literals:
            if len(literal) > 10:  # 10자 이상의 문자열
                memory_data["ram"] += len(literal) + 1  # null terminator
                memory_data["string_literals"].append(literal)

        # Basic code size estimation (very approximate)
        code_lines = len(
            [
                line
                for line in content.split("\n")
                if line.strip() and not line.strip().startswith("//")
            ]
        )
        memory_data["flash"] += code_lines * 4  # Average 4 bytes per line estimate

        return memory_data

    def _get_type_size(self, type_name: str) -> int:
        """데이터 타입별 크기 반환"""
        type_sizes = {
            "char": 1,
            "byte": 1,
            "int": 2,
            "unsigned int": 2,
            "long": 4,
            "unsigned long": 4,
            "float": 4,
            "double": 4,  # Arduino에서는 float와 같음
            "bool": 1,
            "boolean": 1,
        }
        return type_sizes.get(type_name.lower(), 4)  # Default 4 bytes

    def analyze_dependencies(self) -> dict[str, Any]:
        """Dependency analysis"""
        print("Analyzing Arduino dependencies...")

        deps_data = {
            "timestamp": datetime.now().isoformat(),
            "libraries": {},
            "custom_includes": [],
            "defines": [],
            "dependency_graph": {},
        }

        all_includes = set()
        all_defines = set()

        for file_path in self.all_files:
            file_deps = self._analyze_file_dependencies(file_path)

            # 라이브러리 정보 수집
            for lib in file_deps["libraries"]:
                if lib not in deps_data["libraries"]:
                    deps_data["libraries"][lib] = {
                        "used_in": [],
                        "type": (
                            "external"
                            if lib.endswith(".h") and not lib.startswith("Arduino")
                            else "system"
                        ),
                    }
                deps_data["libraries"][lib]["used_in"].append(str(file_path))
                all_includes.add(lib)

            deps_data["custom_includes"].extend(file_deps["custom_includes"])
            all_defines.update(file_deps["defines"])

            deps_data["dependency_graph"][str(file_path)] = file_deps

        deps_data["defines"] = list(all_defines)
        deps_data["custom_includes"] = list(set(deps_data["custom_includes"]))

        print(f"Dependency analysis complete - {len(all_includes)} libraries")
        return deps_data

    def _analyze_file_dependencies(self, file_path: Path) -> dict[str, Any]:
        """File-level dependency analysis"""
        try:
            with open(file_path, encoding="utf-8") as f:
                content = f.read()
        except UnicodeDecodeError:
            try:
                with open(file_path, encoding="latin-1") as f:
                    content = f.read()
            except Exception:
                return {"libraries": [], "custom_includes": [], "defines": []}

        # Include analysis
        includes = re.findall(r'#include\s*[<"]([^>"]+)[>"]', content)

        # Distinguish between system libraries and custom includes
        libraries = []
        custom_includes = []

        for include in includes:
            if include.endswith(".h") and (
                "/" not in include or include.startswith("Arduino")
            ):
                libraries.append(include)
            else:
                custom_includes.append(include)

        # Define analysis
        defines = re.findall(r"#define\s+(\w+)", content)

        return {
            "libraries": libraries,
            "custom_includes": custom_includes,
            "defines": defines,
        }

    def generate_comprehensive_report(
        self,
        line_metrics: dict,
        complexity_data: dict,
        memory_data: dict,
        deps_data: dict,
    ) -> None:
        """Generate comprehensive report"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

        # JSON report
        comprehensive_data = {
            "timestamp": datetime.now().isoformat(),
            "project": "DHT22 Arduino Environmental Sensor",
            "line_metrics": line_metrics,
            "complexity": complexity_data,
            "memory": memory_data,
            "dependencies": deps_data,
            "summary": {
                "total_files": line_metrics["summary"]["total_files"],
                "total_lines": line_metrics["summary"]["total_lines"],
                "code_lines": line_metrics["summary"]["code_lines"],
                "total_functions": complexity_data["summary"]["total_functions"],
                "avg_complexity": complexity_data["summary"]["avg_complexity"],
                "estimated_flash": memory_data["estimated_flash"],
                "estimated_ram": memory_data["estimated_ram"],
                "library_count": len(deps_data["libraries"]),
            },
        }

        json_report = self.reports_dir / f"arduino_metrics_{timestamp}.json"
        with open(json_report, "w", encoding="utf-8") as f:
            json.dump(comprehensive_data, f, indent=2, ensure_ascii=False)

        # Markdown report
        md_report = self.reports_dir / f"arduino_metrics_{timestamp}.md"
        self._generate_markdown_report(md_report, comprehensive_data)

        print("\nArduino comprehensive report generation complete:")
        print(f"   JSON: {json_report}")
        print(f"   Markdown: {md_report}")

    def _generate_markdown_report(self, report_path: Path, data: dict) -> None:
        """Generate Markdown report"""
        with open(report_path, "w", encoding="utf-8") as f:
            f.write(
                f"""# Arduino Code Metrics Report

## Project Overview
- **Project**: {data["project"]}
- **Analysis Time**: {data["timestamp"]}
- **Total Files**: {data["summary"]["total_files"]}
- **Total Lines**: {data["summary"]["total_lines"]}

## Line Count Statistics
- **Code Lines**: {data["summary"]["code_lines"]}
- **Comment Lines**: {data["line_metrics"]["summary"]["comment_lines"]}
- **Blank Lines**: {data["line_metrics"]["summary"]["blank_lines"]}
- **Preprocessor Lines**: {data["line_metrics"]["summary"]["preprocessor_lines"]}
- **Comment Ratio**: {(data["line_metrics"]["summary"]["comment_lines"] / max(data["summary"]["total_lines"], 1) * 100):.1f}%

## Complexity Analysis
- **Total Functions**: {data["summary"]["total_functions"]}
- **Average Complexity**: {data["summary"]["avg_complexity"]:.1f}
- **Maximum Complexity**: {data["complexity"]["summary"]["max_complexity"]}
- **Complex Functions**: {len(data["complexity"]["summary"]["complex_functions"])}

## Memory Usage Estimation
- **Flash Memory**: ~{data["summary"]["estimated_flash"]:,} bytes
- **RAM Usage**: ~{data["summary"]["estimated_ram"]:,} bytes
- **Used Libraries**: {data["summary"]["library_count"]}

### Main Libraries
"""
            )

            for lib, info in data["dependencies"]["libraries"].items():
                f.write(f"- **{lib}**: Used in {len(info['used_in'])} files\n")

            f.write(
                """
## Recommendations
"""
            )

            # Generate recommendations
            recommendations = []

            if data["summary"]["avg_complexity"] > 10:
                recommendations.append(
                    "- Average complexity is high. Consider splitting functions into smaller parts"
                )

            if len(data["complexity"]["summary"]["complex_functions"]) > 0:
                recommendations.append(
                    f"- {len(data['complexity']['summary']['complex_functions'])} complex functions found. Consider refactoring"
                )

            comment_ratio = (
                data["line_metrics"]["summary"]["comment_lines"]
                / max(data["summary"]["total_lines"], 1)
                * 100
            )
            if comment_ratio < 15:
                recommendations.append(
                    "- Recommend increasing comment ratio to 15% or higher"
                )

            if data["summary"]["estimated_flash"] > 20000:
                recommendations.append(
                    "- Flash memory usage is high. Consider code optimization"
                )

            if data["summary"]["estimated_ram"] > 1500:
                recommendations.append(
                    "- RAM usage is high. Optimize large arrays or strings"
                )

            if not recommendations:
                recommendations.append("- Current code quality is excellent!")

            for rec in recommendations:
                f.write(f"{rec}\n")


def main():
    """Main function"""
    print("Starting Arduino code metrics analysis")
    print("=" * 60)

    analyzer = ArduinoMetricsAnalyzer()

    if not analyzer.all_files:
        print("Arduino files not found.")
        print(f"   Check path: {analyzer.arduino_path}")
        return

    print(f"Files found: {len(analyzer.all_files)}")
    for file_path in analyzer.all_files:
        print(f"   - {file_path}")

    # Run analysis
    line_metrics = analyzer.analyze_line_metrics()
    complexity_data = analyzer.analyze_complexity()
    memory_data = analyzer.analyze_memory_usage()
    deps_data = analyzer.analyze_dependencies()

    # Generate comprehensive report
    analyzer.generate_comprehensive_report(
        line_metrics, complexity_data, memory_data, deps_data
    )

    print("\nArduino metrics analysis complete!")


if __name__ == "__main__":
    main()

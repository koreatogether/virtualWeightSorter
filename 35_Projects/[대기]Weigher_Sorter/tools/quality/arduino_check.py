#!/usr/bin/env python3
"""Arduino 코드 품질 검사 도구

이 스크립트는 Arduino .ino 파일들의 기본적인 문법과 구조를 검사합니다:
- 기본 함수 (setup, loop) 존재 확인
- 문법 오류 검사
- 코딩 스타일 검사
- 라이브러리 include 검사
"""

import json
import re
import sys
from datetime import datetime
from pathlib import Path


class ArduinoChecker:
    def __init__(self):
        self.project_root = Path.cwd()
        self.issues = []

    def check_arduino_files(self) -> tuple[bool, list[dict]]:
        """모든 Arduino 파일 검사"""
        arduino_files = list(self.project_root.rglob("*.ino"))

        if not arduino_files:
            print("📝 Arduino 파일이 없습니다.")
            return True, []

        print(f"🔍 {len(arduino_files)}개의 Arduino 파일 검사 중...")

        all_passed = True
        for arduino_file in arduino_files:
            file_passed = self.check_single_file(arduino_file)
            if not file_passed:
                all_passed = False

        return all_passed, self.issues

    def check_single_file(self, file_path: Path) -> bool:
        """단일 Arduino 파일 검사"""
        print(f"📁 검사 중: {file_path.relative_to(self.project_root)}")

        try:
            with open(file_path, encoding="utf-8") as f:
                content = f.read()
        except Exception as e:
            self.add_issue(
                file_path, 0, "파일 읽기 오류", f"파일을 읽을 수 없습니다: {e}", "HIGH"
            )
            return False

        file_passed = True

        # 기본 구조 검사
        if not self.check_basic_structure(file_path, content):
            file_passed = False

        # 문법 검사
        if not self.check_syntax(file_path, content):
            file_passed = False

        # 스타일 검사
        if not self.check_style(file_path, content):
            file_passed = False

        # 라이브러리 검사
        if not self.check_libraries(file_path, content):
            file_passed = False

        # 보안 검사
        if not self.check_security(file_path, content):
            file_passed = False

        return file_passed

    def check_basic_structure(self, file_path: Path, content: str) -> bool:
        """기본 Arduino 구조 검사"""
        passed = True

        # setup() 함수 존재 확인
        if not re.search(r"void\s+setup\s*\(\s*\)\s*\{", content):
            self.add_issue(file_path, 0, "구조 오류", "setup() 함수가 없습니다", "HIGH")
            passed = False

        # loop() 함수 존재 확인
        if not re.search(r"void\s+loop\s*\(\s*\)\s*\{", content):
            self.add_issue(file_path, 0, "구조 오류", "loop() 함수가 없습니다", "HIGH")
            passed = False

        return passed

    def check_syntax(self, file_path: Path, content: str) -> bool:
        """기본 문법 검사"""
        passed = True
        lines = content.splitlines()

        # 괄호 매칭 검사
        brace_count = 0
        paren_count = 0

        for line_num, line in enumerate(lines, 1):
            # 주석 제거
            line = re.sub(r"//.*$", "", line)
            line = re.sub(r"/\*.*?\*/", "", line)

            # 괄호 카운트
            brace_count += line.count("{") - line.count("}")
            paren_count += line.count("(") - line.count(")")

            # 세미콜론 누락 검사 (간단한 버전)
            stripped = line.strip()
            if (
                stripped
                and not stripped.endswith((";", "{", "}", ":", "#"))
                and not stripped.startswith(("#", "//", "/*"))
                and not any(
                    keyword in stripped
                    for keyword in [
                        "if",
                        "else",
                        "for",
                        "while",
                        "switch",
                        "case",
                        "default",
                    ]
                )
                and re.search(r"[a-zA-Z_]\w*\s*\(.*\)\s*$", stripped)
            ):
                self.add_issue(
                    file_path,
                    line_num,
                    "문법 오류",
                    f"세미콜론이 누락되었을 수 있습니다: {stripped}",
                    "MEDIUM",
                )
                passed = False

        # 괄호 불일치 검사
        if brace_count != 0:
            self.add_issue(
                file_path,
                0,
                "문법 오류",
                f"중괄호가 {abs(brace_count)}개 {'부족' if brace_count < 0 else '초과'}합니다",
                "HIGH",
            )
            passed = False

        if paren_count != 0:
            self.add_issue(
                file_path,
                0,
                "문법 오류",
                f"소괄호가 {abs(paren_count)}개 {'부족' if paren_count < 0 else '초과'}합니다",
                "HIGH",
            )
            passed = False

        return passed

    def check_style(self, file_path: Path, content: str) -> bool:
        """코딩 스타일 검사"""
        passed = True
        lines = content.splitlines()

        for line_num, line in enumerate(lines, 1):
            # 탭 사용 검사
            if "\t" in line:
                self.add_issue(
                    file_path,
                    line_num,
                    "스타일",
                    "탭 대신 스페이스 사용을 권장합니다",
                    "LOW",
                )

            # 줄 길이 검사
            if len(line) > 120:
                self.add_issue(
                    file_path,
                    line_num,
                    "스타일",
                    f"줄이 너무 깁니다 ({len(line)}자)",
                    "LOW",
                )

            # 후행 공백 검사
            if line.endswith(" ") or line.endswith("\t"):
                self.add_issue(
                    file_path,
                    line_num,
                    "스타일",
                    "줄 끝에 불필요한 공백이 있습니다",
                    "LOW",
                )

        return passed

    def check_libraries(self, file_path: Path, content: str) -> bool:
        """라이브러리 include 검사"""
        passed = True

        # 필요한 include 확인
        required_includes = {
            "DHT": ["#include <DHT.h>", '#include "DHT.h"'],
            "ArduinoJson": ["#include <ArduinoJson.h>", '#include "ArduinoJson.h"'],
        }

        # DHT 관련 코드가 있는지 확인
        if "DHT" in content or "dht" in content:
            has_dht_include = any(
                include in content
                for includes in required_includes["DHT"]
                for include in includes
            )
            if not has_dht_include:
                self.add_issue(
                    file_path,
                    0,
                    "라이브러리",
                    "DHT 라이브러리 include가 필요할 수 있습니다",
                    "MEDIUM",
                )

        # JSON 관련 코드가 있는지 확인
        if any(keyword in content for keyword in ["Json", "json", "JSON"]):
            has_json_include = any(
                include in content
                for includes in required_includes["ArduinoJson"]
                for include in includes
            )
            if not has_json_include:
                self.add_issue(
                    file_path,
                    0,
                    "라이브러리",
                    "ArduinoJson 라이브러리 include가 필요할 수 있습니다",
                    "MEDIUM",
                )

        return passed

    def check_security(self, file_path: Path, content: str) -> bool:
        """보안 관련 검사"""
        passed = True
        lines = content.splitlines()

        for line_num, line in enumerate(lines, 1):
            # 하드코딩된 비밀번호나 키 검사
            if re.search(
                r'(password|pwd|secret|key)\s*=\s*"[^"]*"', line, re.IGNORECASE
            ):
                self.add_issue(
                    file_path,
                    line_num,
                    "보안",
                    "하드코딩된 비밀번호나 키가 발견되었습니다",
                    "HIGH",
                )
                passed = False

            # 디버그 정보 노출 검사
            if "Serial.println" in line and any(
                keyword in line.lower() for keyword in ["password", "secret", "key"]
            ):
                self.add_issue(
                    file_path,
                    line_num,
                    "보안",
                    "민감한 정보가 시리얼로 출력될 수 있습니다",
                    "MEDIUM",
                )

        return passed

    def add_issue(
        self, file_path: Path, line_num: int, category: str, message: str, severity: str
    ):
        """이슈 추가"""
        self.issues.append(
            {
                "file": str(file_path.relative_to(self.project_root)),
                "line": line_num,
                "category": category,
                "message": message,
                "severity": severity,
                "timestamp": datetime.now().isoformat(),
            }
        )

    def generate_report(self, all_passed: bool) -> None:
        """리포트 생성"""
        # 심각도별 분류
        severity_counts = {"HIGH": 0, "MEDIUM": 0, "LOW": 0}
        for issue in self.issues:
            severity_counts[issue["severity"]] += 1

        # 콘솔 출력
        print("\n" + "=" * 60)
        print("🔧 Arduino 코드 검사 결과")
        print("=" * 60)

        if all_passed and not self.issues:
            print("✅ 모든 Arduino 코드가 품질 기준을 통과했습니다!")
        else:
            print(f"⚠️  총 {len(self.issues)}개의 이슈 발견")
            print(f"   🔴 HIGH: {severity_counts['HIGH']}개")
            print(f"   🟡 MEDIUM: {severity_counts['MEDIUM']}개")
            print(f"   🟢 LOW: {severity_counts['LOW']}개")

            # 이슈별 상세 출력
            for i, issue in enumerate(self.issues, 1):
                severity_icon = {"HIGH": "🔴", "MEDIUM": "🟡", "LOW": "🟢"}[
                    issue["severity"]
                ]
                print(f"\n{i}. {severity_icon} {issue['category']}")
                print(f"   📁 파일: {issue['file']}:{issue['line']}")
                print(f"   📝 내용: {issue['message']}")

        # JSON 리포트 저장
        report_dir = Path("tools/quality/reports")
        report_dir.mkdir(exist_ok=True)

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        report_file = report_dir / f"arduino_check_{timestamp}.json"

        report_data = {
            "timestamp": datetime.now().isoformat(),
            "summary": {
                "total_issues": len(self.issues),
                "severity_breakdown": severity_counts,
                "passed": all_passed,
            },
            "issues": self.issues,
        }

        with open(report_file, "w", encoding="utf-8") as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False)

        print(f"\n📄 상세 리포트: {report_file}")


def main():
    """메인 함수"""
    # Windows 콘솔 인코딩 설정
    import codecs

    if sys.platform.startswith("win"):
        try:
            sys.stdout = codecs.getwriter("utf-8")(sys.stdout.buffer)
            sys.stderr = codecs.getwriter("utf-8")(sys.stderr.buffer)
        except AttributeError:
            pass

    print("🔧 Arduino 코드 품질 검사 시작")
    print(f"📅 시작 시간: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    checker = ArduinoChecker()
    all_passed, issues = checker.check_arduino_files()
    checker.generate_report(all_passed)

    # HIGH 심각도 이슈가 있으면 실패
    high_issues = sum(1 for issue in issues if issue["severity"] == "HIGH")
    if high_issues > 0:
        print(f"\n🚨 {high_issues}개의 심각한 이슈가 발견되었습니다!")
        sys.exit(1)

    print("✅ Arduino 코드 검사 완료")


if __name__ == "__main__":
    main()

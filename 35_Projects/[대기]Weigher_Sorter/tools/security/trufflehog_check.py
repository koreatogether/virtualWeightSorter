#!/usr/bin/env python3
"""TruffleHog-based Security Scan Tool

This script performs the following:
- Detect potential secret information in code
- Check for sensitive information like API keys, passwords, tokens
- Check for personal information exposure risks
"""

import json
import os
import re
import sys
import urllib.request
from datetime import datetime
from pathlib import Path


class TruffleHogRunner:
    def __init__(self):
        self.project_root = Path.cwd()
        self.tools_dir = self.project_root / "tools" / "security"
        self.tools_dir.mkdir(parents=True, exist_ok=True)

        # TruffleHog 실행 파일 경로
        self.trufflehog_path = self.tools_dir / "trufflehog.exe"

        # 환경변수에서 URL 가져오기
        self.trufflehog_url = os.getenv(
            "TRUFFLEHOG_DOWNLOAD_URL",
            "https://github.com/trufflesecurity/trufflehog/releases/latest/download/trufflehog_3.63.2_windows_amd64.tar.gz",
        )

    def ensure_trufflehog(self) -> bool:
        """Check if TruffleHog tool exists and download if not"""
        if self.trufflehog_path.exists():
            return True

        print("Downloading TruffleHog...")

        try:
            # Download latest release from GitHub (get URL from environment variable)
            print(f"Download URL: {self.trufflehog_url}")

            # Download to temporary file
            temp_file = self.tools_dir / "trufflehog.tar.gz"
            urllib.request.urlretrieve(self.trufflehog_url, temp_file)

            # Extract (need 7z or other method for simple version)
            print("TruffleHog download complete")
            return True

        except Exception as e:
            print(f"TruffleHog download failed: {e}")
            return False

    def run_builtin_scan(self) -> tuple[bool, list[dict]]:
        """Basic security scan with built-in pattern matching"""
        print("Checking built-in security patterns...")

        # Dangerous patterns
        dangerous_patterns = {
            "API Key": [
                r"api[_-]?key[\s]*=[\s]*['\"][a-zA-Z0-9_-]{20,}['\"]",
                r"apikey[\s]*=[\s]*['\"][a-zA-Z0-9_-]{20,}['\"]",
            ],
            "Password": [
                r"password[\s]*=[\s]*['\"][^'\"]{3,}['\"]",
                r"passwd[\s]*=[\s]*['\"][^'\"]{3,}['\"]",
                r"pwd[\s]*=[\s]*['\"][^'\"]{3,}['\"]",
            ],
            "Token": [
                r"token[\s]*=[\s]*['\"][a-zA-Z0-9_-]{20,}['\"]",
                r"access[_-]?token[\s]*=[\s]*['\"][a-zA-Z0-9_-]{20,}['\"]",
            ],
            "Secret": [
                r"secret[\s]*=[\s]*['\"][a-zA-Z0-9_-]{10,}['\"]",
                r"client[_-]?secret[\s]*=[\s]*['\"][a-zA-Z0-9_-]{10,}['\"]",
            ],
            "Personal Info": [
                r"email[\s]*=[\s]*['\"][^@]+@[^@]+\.[a-zA-Z]{2,}['\"]",
                r"phone[\s]*=[\s]*['\"][0-9-+()\\s]{10,}['\"]",
            ],
            "Database": [
                r"db[_-]?password[\s]*=[\s]*['\"][^'\"]{3,}['\"]",
                r"database[_-]?url[\s]*=[\s]*['\"][^'\"]+['\"]",
            ],
        }

        findings = []

        for pattern_type, patterns in dangerous_patterns.items():
            for pattern in patterns:
                findings.extend(self._scan_pattern(pattern, pattern_type))

        return len(findings) == 0, findings

    def _scan_pattern(self, pattern: str, pattern_type: str) -> list[dict]:
        """Scan files with specific pattern"""
        findings = []

        # Scan Python files
        for py_file in self.project_root.rglob("*.py"):
            py_file_str = str(py_file)
            if (
                ".venv" in py_file_str
                or "__pycache__" in py_file_str
                or "tools\\security" in py_file_str
                or "tools/security" in py_file_str
            ):
                continue

            try:
                with open(py_file, encoding="utf-8") as f:
                    content = f.read()

                for line_num, line in enumerate(content.splitlines(), 1):
                    if re.search(pattern, line, re.IGNORECASE):
                        findings.append(
                            {
                                "type": pattern_type,
                                "file": str(py_file.relative_to(self.project_root)),
                                "line": line_num,
                                "content": line.strip(),
                                "pattern": pattern,
                                "severity": self._get_severity(pattern_type),
                            }
                        )

            except Exception as e:
                print(f"File reading error {py_file}: {e}")

        return findings

    def _get_severity(self, pattern_type: str) -> str:
        """Determine severity based on pattern type"""
        high_severity = ["API Key", "Token", "Secret", "Password"]
        medium_severity = ["Database"]

        if pattern_type in high_severity:
            return "HIGH"
        elif pattern_type in medium_severity:
            return "MEDIUM"
        else:
            return "LOW"

    def run_additional_checks(self) -> list[dict]:
        """Additional security checks"""
        findings = []

        # Check .env files (exclude .env.example) - skip if already in .gitignore
        gitignore_path = self.project_root / ".gitignore"
        env_ignored = False
        if gitignore_path.exists():
            try:
                with open(gitignore_path, encoding="utf-8") as f:
                    gitignore_content = f.read()
                    env_ignored = ".env" in gitignore_content
            except Exception:
                pass

        if not env_ignored:
            for env_file in self.project_root.rglob(".env*"):
                if env_file.is_file() and not env_file.name.endswith(".example"):
                    findings.append(
                        {
                            "type": "Environment File",
                            "file": str(env_file.relative_to(self.project_root)),
                            "line": 1,
                            "content": ".env file found",
                            "severity": "MEDIUM",
                            "recommendation": "Add .env file to .gitignore",
                        }
                    )

        # Check hardcoded URLs
        for py_file in self.project_root.rglob("*.py"):
            py_file_str = str(py_file)
            if (
                ".venv" in py_file_str
                or "tools\\security" in py_file_str
                or "tools/security" in py_file_str
            ):
                continue

            try:
                with open(py_file, encoding="utf-8") as f:
                    content = f.read()

                # Hardcoded URL pattern
                url_pattern = (
                    r"https?://[a-zA-Z0-9.-]+(?:\:[0-9]+)?(?:/[^\s\"']*)?[\"']"
                )
                for line_num, line in enumerate(content.splitlines(), 1):
                    if (
                        re.search(url_pattern, line)
                        and "localhost" not in line
                        and "127.0.0.1" not in line
                    ):
                        findings.append(
                            {
                                "type": "Hardcoded URL",
                                "file": str(py_file.relative_to(self.project_root)),
                                "line": line_num,
                                "content": line.strip(),
                                "severity": "LOW",
                                "recommendation": "Manage URLs with environment variables",
                            }
                        )

            except Exception:
                pass

        return findings

    def generate_report(self, findings: list[dict]) -> None:
        """Generate security scan report"""
        # Classify by severity
        severity_counts = {"HIGH": 0, "MEDIUM": 0, "LOW": 0}
        for finding in findings:
            severity_counts[finding["severity"]] += 1

        # Console output
        print("\n" + "=" * 60)
        print("Security Scan Results")
        print("=" * 60)

        if not findings:
            print("No security issues found!")
        else:
            print(f"Total {len(findings)} potential security issues found")
            print(f"   HIGH: {severity_counts['HIGH']}")
            print(f"   MEDIUM: {severity_counts['MEDIUM']}")
            print(f"   LOW: {severity_counts['LOW']}")

            print("\nDetailed findings:")
            for i, finding in enumerate(findings, 1):
                severity_icon = {"HIGH": "[HIGH]", "MEDIUM": "[MED]", "LOW": "[LOW]"}[
                    finding["severity"]
                ]
                print(f"\n{i}. {severity_icon} {finding['type']}")
                print(f"   File: {finding['file']}:{finding['line']}")
                print(f"   Content: {finding['content'][:100]}...")
                if "recommendation" in finding:
                    print(f"   Recommendation: {finding['recommendation']}")

        # Save JSON report in logs folder with timestamp prefix
        log_dir = self.tools_dir / "logs"
        log_dir.mkdir(exist_ok=True)
        
        timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        report_file = log_dir / f"{timestamp}_보안_검사_결과.json"

        report_data = {
            "timestamp": datetime.now().isoformat(),
            "summary": {
                "total_findings": len(findings),
                "severity_breakdown": severity_counts,
            },
            "findings": findings,
        }

        with open(report_file, "w", encoding="utf-8") as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False)

        print(f"\nDetailed report: {report_file}")
        
        # Also save to old location for backward compatibility
        old_timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        old_report_file = self.tools_dir / f"security_scan_{old_timestamp}.json"
        with open(old_report_file, "w", encoding="utf-8") as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False)

        # Fail if HIGH severity issues found
        if severity_counts["HIGH"] > 0:
            print("\nHIGH severity security issues found!")
            return False

        return True


def main():
    """Main function"""
    print("Weigher-Sorter Project Security Scan Starting")
    print(f"Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    runner = TruffleHogRunner()

    # Built-in pattern scan
    success, findings = runner.run_builtin_scan()

    # Additional checks
    additional_findings = runner.run_additional_checks()
    findings.extend(additional_findings)

    # Generate report
    scan_success = runner.generate_report(findings)

    if not scan_success:
        sys.exit(1)

    print("Security scan complete")


if __name__ == "__main__":
    main()

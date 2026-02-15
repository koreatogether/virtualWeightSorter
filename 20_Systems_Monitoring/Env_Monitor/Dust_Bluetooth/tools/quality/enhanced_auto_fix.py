#!/usr/bin/env python3
"""Enhanced Project-Specific Automatic Code Quality Fix Tool

This script automatically fixes DS18B20 project-specific issues that appear
repeatedly across development cycles, with special handling for:
- getUserData/setUserData naming with backward compatibility
- Import conflicts in multi-module projects
- Type annotation improvements
- Test environment fixes
"""

import json
import re
import subprocess
import sys
from pathlib import Path


class DS18B20ProjectAutoFixer:
    """DS18B20 프로젝트 전용 자동 수정 도구"""

    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.src_path = project_root / "src" / "python"
        self.fixed_issues: list[str] = []
        self.manual_fixes_needed: list[str] = []

    def run_all_fixes(self) -> bool:
        """모든 자동 수정 실행"""
        print("DS18B20 프로젝트 전용 자동 수정 시작...")
        print("=" * 60)

        success = True

        # 1. 표준 Ruff 자동 수정
        success &= self._run_standard_ruff_fixes()

        # 2. 프로젝트별 특화 수정
        success &= self._fix_naming_conventions_with_compatibility()
        success &= self._fix_import_conflicts()
        success &= self._fix_type_annotations()

        # 3. 테스트 환경 자동 설정
        success &= self._setup_test_environment()

        # 4. 결과 리포트
        self._generate_fix_report()

        return success

    def _run_standard_ruff_fixes(self) -> bool:
        """표준 Ruff 자동 수정 (확장 규칙 포함)"""
        print("\n1. 표준 Ruff 자동 수정 실행...")

        # 확장된 규칙으로 수정
        cmd = [
            "uv",
            "run",
            "ruff",
            "check",
            "--fix",
            "--unsafe-fixes",
            "--extend-select=N,SIM,RUF,PT,PL,UP,F,E,W,I",
            "src/",
            "tools/",
        ]

        result = subprocess.run(
            cmd, capture_output=True, text=True, encoding="utf-8", errors="replace"
        )

        if result.returncode == 0:
            self.fixed_issues.append("표준 Ruff 수정 완료")
            print("   표준 Ruff 수정 완료")
        else:
            remaining = result.stdout.count("Found") if result.stdout else 0
            print(f"   표준 Ruff 수정 완료 (남은 이슈: {remaining}개)")

        # 포맷팅도 함께 실행
        format_cmd = ["uv", "run", "ruff", "format", "src/", "tools/"]
        subprocess.run(format_cmd, capture_output=True)
        self.fixed_issues.append("✅ 코드 포맷팅 통일")

        return True

    def _fix_naming_conventions_with_compatibility(self) -> bool:
        """네이밍 컨벤션 수정 (하위 호환성 유지)"""
        print("\n2️⃣ getUserData/setUserData 네이밍 컨벤션 수정...")

        files_to_fix = [
            self.src_path / "simulator" / "ds18b20_simulator.py",
            self.src_path / "simulator" / "offline_simulator.py",
        ]

        fixed_count = 0
        for file_path in files_to_fix:
            if file_path.exists() and self._add_snake_case_methods(file_path):
                fixed_count += 1
                print(f"   ✅ {file_path.name} 수정 완료")

        if fixed_count > 0:
            self.fixed_issues.append(
                f"✅ {fixed_count}개 파일에 snake_case 메서드 추가"
            )
            return True
        else:
            self.manual_fixes_needed.append("❌ 네이밍 컨벤션 수정 필요")
            return False

    def _add_snake_case_methods(self, file_path: Path) -> bool:
        """파일에 snake_case 메서드와 호환성 래퍼 추가"""
        try:
            content = file_path.read_text(encoding="utf-8")

            # getUserData 함수 찾기
            get_user_pattern = r"(\s+)def getUserData\(self\) -> int:"
            if re.search(get_user_pattern, content):
                # snake_case 버전과 deprecation 래퍼 추가
                snake_case_methods = '''
    def get_user_data(self) -> int:
        """사용자 ID 조회 (권장 방법)

        Returns:
            1~8 범위의 사용자 ID
        """
        return self.eeprom_data["user_id"]

    def getUserData(self) -> int:
        """레거시 호환용 - deprecated

        Warning: 이 메서드는 향후 제거될 예정입니다.
        get_user_data()를 사용하세요.
        """
        import warnings
        warnings.warn(
            "getUserData is deprecated, use get_user_data instead",
            DeprecationWarning,
            stacklevel=2
        )
        return self.get_user_data()
'''

                # 기존 getUserData 메서드를 snake_case + wrapper로 교체
                content = re.sub(
                    r'(\s+)def getUserData\(self\) -> int:\s*\n\s*"""[^"]*"""\s*\n\s*return self\.eeprom_data\["user_id"\]',
                    snake_case_methods,
                    content,
                )

            # setUserData도 동일하게 처리
            set_user_pattern = r"(\s+)def setUserData\(self, user_id: int\) -> bool:"
            if re.search(set_user_pattern, content):
                snake_case_set_methods = '''
    def set_user_data(self, user_id: int) -> bool:
        """사용자 ID 설정 (권장 방법)

        Args:
            user_id: 1~8 범위의 사용자 ID

        Returns:
            설정 성공 여부
        """
        if not (1 <= user_id <= 8):
            return False

        if self.eeprom_data["user_id"] != user_id:
            self.eeprom_data["user_id"] = user_id
            self._save_eeprom_if_changed()
        return True

    def setUserData(self, user_id: int) -> bool:
        """레거시 호환용 - deprecated

        Warning: 이 메서드는 향후 제거될 예정입니다.
        set_user_data()를 사용하세요.
        """
        import warnings
        warnings.warn(
            "setUserData is deprecated, use set_user_data instead",
            DeprecationWarning,
            stacklevel=2
        )
        return self.set_user_data(user_id)
'''

                # 복잡한 setUserData 로직을 snake_case + wrapper로 교체
                content = re.sub(
                    r"(\s+)def setUserData\(self, user_id: int\) -> bool:.*?return True",
                    snake_case_set_methods,
                    content,
                    flags=re.DOTALL,
                )

            # 수정된 내용 저장
            file_path.write_text(content, encoding="utf-8")
            return True

        except Exception as e:
            print(f"   ❌ {file_path.name} 수정 실패: {e}")
            return False

    def _fix_import_conflicts(self) -> bool:
        """임포트 충돌 해결"""
        print("\n3️⃣ 임포트 충돌 해결...")

        serial_handler_path = self.src_path / "dashboard" / "serial_handler.py"
        if not serial_handler_path.exists():
            return False

        try:
            content = serial_handler_path.read_text(encoding="utf-8")

            # TYPE_CHECKING 패턴으로 변경
            new_import_pattern = """try:
    # Prefer absolute import when src/python is on sys.path (test runner)
    from simulator.protocol import ProtocolHandler
except ImportError:
    # Fallback to relative import when used as a package
    from ..simulator.protocol import ProtocolHandler"""

            type_checking_pattern = """from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from simulator.protocol import ProtocolHandler
else:
    try:
        from simulator.protocol import ProtocolHandler
    except ImportError:
        from ..simulator.protocol import ProtocolHandler"""

            # 기존 임포트 패턴 교체
            if (
                "try:" in content
                and "from simulator.protocol import ProtocolHandler" in content
            ):
                content = content.replace(new_import_pattern, type_checking_pattern)
                serial_handler_path.write_text(content, encoding="utf-8")

                self.fixed_issues.append("✅ serial_handler.py 임포트 충돌 해결")
                print("   ✅ serial_handler.py 임포트 충돌 해결")
                return True

        except Exception as e:
            print(f"   ❌ 임포트 충돌 해결 실패: {e}")

        self.manual_fixes_needed.append(
            "❌ serial_handler.py 임포트 충돌 수동 해결 필요"
        )
        return False

    def _fix_type_annotations(self) -> bool:
        """타입 애노테이션 개선"""
        print("\n4️⃣ 타입 애노테이션 자동 개선...")

        # simulator_manager.py의 다중 상속 문제 해결
        manager_path = self.src_path / "simulator" / "simulator_manager.py"
        if manager_path.exists():
            try:
                content = manager_path.read_text(encoding="utf-8")

                # 문제가 되는 클래스 구조 찾기 및 수정
                problematic_pattern = (
                    r"class\s+(\w+)\(list\[str\],\s*dict\[Any,\s*Any\]\):"
                )
                if re.search(problematic_pattern, content):
                    # 컴포지션 패턴으로 변경
                    composition_replacement = '''class \\1:
    """Composition-based class instead of multiple inheritance"""

    def __init__(self):
        self._list_data: list[str] = []
        self._dict_data: dict[Any, Any] = {}

    # List-like interface
    def append(self, item: str) -> None:
        self._list_data.append(item)

    def __iter__(self):
        return iter(self._list_data)

    # Dict-like interface
    def get(self, key: Any, default: Any = None) -> Any:
        return self._dict_data.get(key, default)

    def __getitem__(self, key: Any) -> Any:
        return self._dict_data[key]

    def __setitem__(self, key: Any, value: Any) -> None:
        self._dict_data[key] = value'''

                    content = re.sub(
                        problematic_pattern, composition_replacement, content
                    )
                    manager_path.write_text(content, encoding="utf-8")

                    self.fixed_issues.append(
                        "✅ simulator_manager.py 다중 상속 → 컴포지션 변경"
                    )
                    print("   ✅ 다중 상속 문제 해결")
                    return True

            except Exception as e:
                print(f"   ❌ 타입 애노테이션 수정 실패: {e}")

        return True

    def _setup_test_environment(self) -> bool:
        """테스트 환경 자동 설정"""
        print("\n5️⃣ 테스트 환경 자동 설정...")

        # conftest.py 개선
        tests_dir = self.project_root / "tests"
        conftest_path = tests_dir / "conftest.py"

        if conftest_path.exists():
            enhanced_conftest = '''"""Test configuration for DS18B20 monitoring project."""

import sys
from pathlib import Path

import pytest

# src/python을 sys.path에 추가하여 모듈 임포트 문제 해결
src_path = Path(__file__).parent.parent / "src" / "python"
if str(src_path) not in sys.path:
    sys.path.insert(0, str(src_path))

@pytest.fixture
def project_root():
    """프로젝트 루트 디렉토리 경로 제공"""
    return Path(__file__).parent.parent

@pytest.fixture
def sample_sensor_data():
    """테스트용 샘플 센서 데이터"""
    return {
        "type": "sensor_data",
        "sensor_id": "28FF123456789ABC",
        "temperature": 25.6,
        "th_value": 30.0,
        "tl_value": 20.0,
        "measurement_interval": 1000,
        "user_id": 1
    }

@pytest.fixture
def mock_serial_port():
    """Mock 시리얼 포트 (통합 테스트용)"""
    from unittest.mock import Mock
    return Mock()
'''
            try:
                conftest_path.write_text(enhanced_conftest, encoding="utf-8")
                self.fixed_issues.append("✅ conftest.py 개선")
                print("   ✅ conftest.py 개선 완료")
                return True
            except Exception as e:
                print(f"   ❌ conftest.py 개선 실패: {e}")

        return False

    def _generate_fix_report(self) -> None:
        """수정 결과 리포트 생성"""
        print(f"\n{'=' * 60}")
        print("🎯 DS18B20 프로젝트 자동 수정 완료 리포트")
        print(f"{'=' * 60}")

        print(f"\n✅ 자동 수정 완료 ({len(self.fixed_issues)}개):")
        for issue in self.fixed_issues:
            print(f"   {issue}")

        if self.manual_fixes_needed:
            print(f"\n🔧 수동 수정 필요 ({len(self.manual_fixes_needed)}개):")
            for issue in self.manual_fixes_needed:
                print(f"   {issue}")

        print("\n📋 다음 단계:")
        print("   1. uv run ruff check src/ --statistics  # 남은 에러 확인")
        print("   2. uv run mypy src/python/ --ignore-missing-imports  # 타입 체크")
        print("   3. uv run pytest tests/ -v  # 테스트 실행")

        # JSON 리포트도 생성
        report = {
            "timestamp": str(Path(__file__).stat().st_mtime),
            "fixed_issues": self.fixed_issues,
            "manual_fixes_needed": self.manual_fixes_needed,
            "total_fixed": len(self.fixed_issues),
            "total_manual": len(self.manual_fixes_needed),
        }

        report_path = (
            self.project_root / "tools" / "quality" / "reports" / "auto_fix_report.json"
        )
        report_path.parent.mkdir(parents=True, exist_ok=True)

        try:
            with open(report_path, "w", encoding="utf-8") as f:
                json.dump(report, f, ensure_ascii=False, indent=2)
            print(f"   📄 상세 리포트: {report_path}")
        except Exception:
            pass


def main():
    """메인 실행 함수"""
    project_root = Path.cwd()

    if not (project_root / "pyproject.toml").exists():
        print("❌ pyproject.toml을 찾을 수 없습니다. 프로젝트 루트에서 실행하세요.")
        sys.exit(1)

    fixer = DS18B20ProjectAutoFixer(project_root)
    success = fixer.run_all_fixes()

    if success:
        print("\n🎉 자동 수정 완료! 품질 검사를 실행하여 결과를 확인하세요.")
        sys.exit(0)
    else:
        print("\n⚠️ 일부 이슈는 수동 수정이 필요합니다.")
        sys.exit(1)


if __name__ == "__main__":
    main()

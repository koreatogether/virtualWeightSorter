#!/usr/bin/env python3
"""DS18B20 모니터링 시스템 환경 테스트 스크립트

이 스크립트는 다양한 터미널 환경에서 가상환경과 패키지들이
제대로 작동하는지 종합적으로 테스트합니다.

사용법:
    python test_environment.py
    uv run python test_environment.py
"""

import os
import platform
import subprocess
import sys
import traceback
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


class EnvironmentTester:
    def __init__(self):
        self.test_results = []
        self.failed_tests = []

    def log_test(self, test_name: str, success: bool, message: str = ""):
        """테스트 결과 로깅"""
        status = "✅ PASS" if success else "❌ FAIL"
        result = f"{status} {test_name}"
        if message:
            result += f" - {message}"

        print(result)
        self.test_results.append((test_name, success, message))

        if not success:
            self.failed_tests.append(test_name)

    def test_python_environment(self):
        """Python 환경 테스트"""
        print("\n" + "=" * 60)
        print("🐍 Python 환경 테스트")
        print("=" * 60)

        # Python 버전 확인
        try:
            version = sys.version_info
            version_str = f"{version.major}.{version.minor}.{version.micro}"

            if version.major == 3 and version.minor >= 10:
                self.log_test("Python 버전", True, f"Python {version_str}")
            else:
                self.log_test(
                    "Python 버전", False, f"Python {version_str} (3.10+ 필요)"
                )
        except Exception as e:
            self.log_test("Python 버전", False, str(e))

        # 플랫폼 정보
        try:
            platform_info = f"{platform.system()} {platform.release()}"
            self.log_test("플랫폼", True, platform_info)
        except Exception as e:
            self.log_test("플랫폼", False, str(e))

        # 가상환경 확인
        try:
            venv_path = os.environ.get("VIRTUAL_ENV")
            if venv_path:
                self.log_test("가상환경", True, f"활성화됨: {venv_path}")
            else:
                # uv 가상환경 확인
                if Path(".venv").exists():
                    self.log_test("가상환경", True, "uv 가상환경 감지됨")
                else:
                    self.log_test("가상환경", False, "가상환경이 활성화되지 않음")
        except Exception as e:
            self.log_test("가상환경", False, str(e))

    def test_required_packages(self):
        """필수 패키지 import 테스트"""
        print("\n" + "=" * 60)
        print("📦 필수 패키지 테스트")
        print("=" * 60)

        required_packages = {
            "dash": "Dash 웹 프레임워크",
            "plotly": "Plotly 그래프 라이브러리",
            "pandas": "Pandas 데이터 처리",
            "serial": "PySerial 시리얼 통신",
            "dash_bootstrap_components": "Dash Bootstrap Components",
        }

        for package, description in required_packages.items():
            try:
                module = __import__(package)
                version = getattr(module, "__version__", "Unknown")
                self.log_test(f"{package} import", True, f"{description} v{version}")
            except ImportError as e:
                self.log_test(f"{package} import", False, f"{description} - {e!s}")
            except Exception as e:
                self.log_test(f"{package} import", False, f"{description} - {e!s}")

    def test_development_tools(self):
        """개발 도구 테스트"""
        print("\n" + "=" * 60)
        print("🛠️ 개발 도구 테스트")
        print("=" * 60)

        dev_tools = {
            "pytest": "pytest --version",
            "ruff": "ruff --version",
            "mypy": "mypy --version",
            "radon": "radon --version",
            "safety": "safety --version",
        }

        for tool, command in dev_tools.items():
            try:
                # uv run으로 실행 시도
                result = subprocess.run(
                    ["uv", "run", *command.split()],
                    capture_output=True,
                    text=True,
                    timeout=10,
                )

                if result.returncode == 0:
                    version_info = result.stdout.strip().split("\n")[0]
                    self.log_test(f"{tool} 실행", True, version_info)
                else:
                    self.log_test(f"{tool} 실행", False, result.stderr.strip())

            except subprocess.TimeoutExpired:
                self.log_test(f"{tool} 실행", False, "실행 시간 초과")
            except FileNotFoundError:
                # uv가 없는 경우 직접 실행 시도
                try:
                    result = subprocess.run(
                        command.split(), capture_output=True, text=True, timeout=10
                    )

                    if result.returncode == 0:
                        version_info = result.stdout.strip().split("\n")[0]
                        self.log_test(
                            f"{tool} 실행", True, f"{version_info} (직접 실행)"
                        )
                    else:
                        self.log_test(f"{tool} 실행", False, result.stderr.strip())

                except Exception as e:
                    self.log_test(f"{tool} 실행", False, str(e))
            except Exception as e:
                self.log_test(f"{tool} 실행", False, str(e))

    def test_dash_functionality(self):
        """Dash 기본 기능 테스트"""
        print("\n" + "=" * 60)
        print("🌐 Dash 기능 테스트")
        print("=" * 60)

        try:
            import dash
            import dash_bootstrap_components as dbc
            import plotly.graph_objects as go
            from dash import dcc, html

            # 기본 Dash 앱 생성 테스트
            app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP])

            # 레이아웃 생성 테스트
            app.layout = html.Div(
                [
                    html.H1("테스트 앱"),
                    dcc.Graph(
                        figure=go.Figure(data=go.Scatter(x=[1, 2, 3], y=[4, 5, 6]))
                    ),
                    dbc.Button("테스트 버튼", color="primary"),
                ]
            )

            self.log_test("Dash 앱 생성", True, "기본 레이아웃 생성 성공")

        except Exception as e:
            self.log_test("Dash 앱 생성", False, str(e))

    def test_serial_functionality(self):
        """시리얼 통신 기능 테스트"""
        print("\n" + "=" * 60)
        print("🔌 시리얼 통신 테스트")
        print("=" * 60)

        try:
            import serial
            import serial.tools.list_ports

            # 사용 가능한 시리얼 포트 확인
            ports = list(serial.tools.list_ports.comports())

            if ports:
                port_info = ", ".join([f"{port.device}" for port in ports[:3]])
                self.log_test("시리얼 포트 감지", True, f"포트: {port_info}")
            else:
                self.log_test("시리얼 포트 감지", True, "사용 가능한 포트 없음 (정상)")

            # 시리얼 객체 생성 테스트 (실제 연결 없이)
            try:
                ser = serial.Serial()
                ser.port = "COM999"  # 존재하지 않는 포트
                ser.baudrate = 9600
                ser.timeout = 1
                # 실제 연결은 하지 않음
                self.log_test("시리얼 객체 생성", True, "Serial 객체 생성 성공")
            except Exception as e:
                self.log_test("시리얼 객체 생성", False, str(e))

        except Exception as e:
            self.log_test("시리얼 통신 모듈", False, str(e))

    def test_data_processing(self):
        """데이터 처리 기능 테스트"""
        print("\n" + "=" * 60)
        print("📊 데이터 처리 테스트")
        print("=" * 60)

        try:
            import json
            from datetime import datetime

            import pandas as pd

            # JSON 데이터 처리 테스트
            test_data = {
                "sensor_id": "28FF123456789ABC",
                "temperature": 25.5,
                "th_value": 30.0,
                "tl_value": 20.0,
                "measurement_interval": 1000,
                "timestamp": datetime.now().isoformat(),
            }

            json_str = json.dumps(test_data)
            json.loads(json_str)

            self.log_test("JSON 처리", True, "JSON 직렬화/역직렬화 성공")

            # Pandas DataFrame 테스트
            df = pd.DataFrame([test_data])

            # 온도값 소수 1자리 처리 테스트
            df["temperature"] = df["temperature"].round(1)

            self.log_test("Pandas 처리", True, f"DataFrame 생성 성공 ({len(df)} 행)")

        except Exception as e:
            self.log_test("데이터 처리", False, str(e))

    def test_file_system_access(self):
        """파일 시스템 접근 테스트"""
        print("\n" + "=" * 60)
        print("📁 파일 시스템 테스트")
        print("=" * 60)

        try:
            # 프로젝트 구조 확인
            required_dirs = ["docs", "tools", "src", ".kiro"]

            for dir_name in required_dirs:
                if Path(dir_name).exists():
                    self.log_test(f"{dir_name} 폴더", True, "존재함")
                else:
                    self.log_test(f"{dir_name} 폴더", False, "존재하지 않음")

            # 임시 파일 생성/삭제 테스트
            test_file = Path("temp_test_file.txt")
            test_file.write_text("테스트 내용")

            if test_file.exists():
                test_file.read_text()
                test_file.unlink()  # 파일 삭제
                self.log_test("파일 읽기/쓰기", True, "임시 파일 생성/삭제 성공")
            else:
                self.log_test("파일 읽기/쓰기", False, "파일 생성 실패")

        except Exception as e:
            self.log_test("파일 시스템", False, str(e))

    def test_terminal_environment(self):
        """터미널 환경 테스트"""
        print("\n" + "=" * 60)
        print("💻 터미널 환경 테스트")
        print("=" * 60)

        try:
            # 환경 변수 확인
            shell = os.environ.get("SHELL", os.environ.get("COMSPEC", "Unknown"))
            self.log_test("쉘 환경", True, f"쉘: {shell}")

            # PATH 환경 변수 확인
            path_env = os.environ.get("PATH", "")
            python_in_path = any(
                "python" in p.lower() for p in path_env.split(os.pathsep)
            )
            self.log_test(
                "Python PATH",
                python_in_path,
                "Python이 PATH에 있음" if python_in_path else "Python PATH 확인 필요",
            )

            # 현재 작업 디렉토리
            cwd = os.getcwd()
            self.log_test("작업 디렉토리", True, f"CWD: {cwd}")

            # 인코딩 확인
            encoding = sys.stdout.encoding
            self.log_test("출력 인코딩", True, f"인코딩: {encoding}")

        except Exception as e:
            self.log_test("터미널 환경", False, str(e))

    def run_all_tests(self):
        """모든 테스트 실행"""
        print("🚀 DS18B20 모니터링 시스템 환경 테스트 시작")
        print(f"📅 테스트 시간: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

        # 각 테스트 실행
        self.test_python_environment()
        self.test_required_packages()
        self.test_development_tools()
        self.test_dash_functionality()
        self.test_serial_functionality()
        self.test_data_processing()
        self.test_file_system_access()
        self.test_terminal_environment()

        # 결과 요약
        self.print_summary()

    def print_summary(self):
        """테스트 결과 요약 출력"""
        print("\n" + "=" * 60)
        print("📋 테스트 결과 요약")
        print("=" * 60)

        total_tests = len(self.test_results)
        passed_tests = sum(1 for _, success, _ in self.test_results if success)
        failed_tests = total_tests - passed_tests

        print(f"총 테스트: {total_tests}")
        print(f"✅ 성공: {passed_tests}")
        print(f"❌ 실패: {failed_tests}")
        print(f"📊 성공률: {(passed_tests/total_tests*100):.1f}%")

        if self.failed_tests:
            print("\n⚠️ 실패한 테스트:")
            for test_name in self.failed_tests:
                print(f"   - {test_name}")

            print("\n💡 해결 방법:")
            print("   1. 가상환경 활성화: uv run python test_environment.py")
            print("   2. 패키지 재설치: uv sync")
            print("   3. 개발 도구 설치: uv add --dev pytest ruff mypy")
        else:
            print("\n🎉 모든 테스트가 성공했습니다!")
            print("   Phase 1 개발을 시작할 준비가 완료되었습니다.")

        # 터미널별 실행 가이드
        print("\n🖥️ 터미널별 실행 방법:")
        print("   PowerShell: uv run python test_environment.py")
        print("   CMD:        uv run python test_environment.py")
        print("   Git Bash:   uv run python test_environment.py")
        print("   WSL/Linux:  uv run python test_environment.py")


def main():
    """메인 함수"""
    try:
        tester = EnvironmentTester()
        tester.run_all_tests()

        # 실패한 테스트가 있으면 종료 코드 1
        if tester.failed_tests:
            sys.exit(1)
        else:
            sys.exit(0)

    except KeyboardInterrupt:
        print("\n\n⚠️ 테스트가 사용자에 의해 중단되었습니다.")
        sys.exit(1)
    except Exception as e:
        print("\n\n💥 예상치 못한 오류가 발생했습니다:")
        print(f"   {e!s}")
        print("\n스택 트레이스:")
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()

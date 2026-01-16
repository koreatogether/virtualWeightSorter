#!/usr/bin/env python3
"""Arduino 라이브러리 설치 확인 스크립트

이 스크립트는 Arduino IDE에 OneWire와 DallasTemperature 라이브러리가
설치되어 있는지 확인합니다.
"""

import platform
from pathlib import Path


def find_arduino_libraries_path():
    """Arduino 라이브러리 경로 찾기"""
    system = platform.system()

    if system == "Windows":
        # Windows 경로들
        possible_paths = [
            Path.home() / "Documents" / "Arduino" / "libraries",
            Path.home() / "OneDrive" / "Documents" / "Arduino" / "libraries",
            Path("C:/Program Files (x86)/Arduino/libraries"),
            Path("C:/Program Files/Arduino/libraries"),
        ]
    elif system == "Darwin":  # macOS
        possible_paths = [
            Path.home() / "Documents" / "Arduino" / "libraries",
            Path("/Applications/Arduino.app/Contents/Java/libraries"),
        ]
    else:  # Linux
        possible_paths = [
            Path.home() / "Arduino" / "libraries",
            Path.home() / "Documents" / "Arduino" / "libraries",
            Path("/usr/share/arduino/libraries"),
        ]

    for path in possible_paths:
        if path.exists():
            return path

    return None


def check_library(libraries_path, library_name):
    """특정 라이브러리 설치 확인"""
    if not libraries_path:
        return False, "Arduino 라이브러리 경로를 찾을 수 없습니다"

    library_path = libraries_path / library_name

    if not library_path.exists():
        return False, f"{library_name} 라이브러리가 설치되지 않았습니다"

    # library.properties 파일 확인
    properties_file = library_path / "library.properties"
    if properties_file.exists():
        try:
            with open(properties_file, encoding="utf-8") as f:
                content = f.read()

            # 버전 정보 추출
            version = "Unknown"
            for line in content.split("\n"):
                if line.startswith("version="):
                    version = line.split("=")[1].strip()
                    break

            return True, f"{library_name} v{version} 설치됨"
        except Exception as e:
            return True, f"{library_name} 설치됨 (버전 정보 읽기 실패: {e})"

    return True, f"{library_name} 설치됨 (버전 정보 없음)"


def check_arduino_ide():
    """Arduino IDE 설치 확인"""
    system = platform.system()

    if system == "Windows":
        possible_paths = [
            Path("C:/Program Files (x86)/Arduino/arduino.exe"),
            Path("C:/Program Files/Arduino/arduino.exe"),
        ]

        # Windows Store 버전 확인
        store_path = Path.home() / "AppData" / "Local" / "Microsoft" / "WindowsApps"
        if store_path.exists():
            arduino_store = list(store_path.glob("*Arduino*"))
            if arduino_store:
                return True, "Arduino IDE (Windows Store 버전) 설치됨"

    elif system == "Darwin":  # macOS
        possible_paths = [
            Path("/Applications/Arduino.app"),
        ]

    else:  # Linux
        possible_paths = [
            Path("/usr/bin/arduino"),
            Path("/usr/local/bin/arduino"),
        ]

    for path in possible_paths:
        if path.exists():
            return True, f"Arduino IDE 설치됨: {path}"

    return False, "Arduino IDE를 찾을 수 없습니다"


def main():
    """메인 함수"""
    print("🔧 Arduino 환경 확인 스크립트")
    print("=" * 50)

    # Arduino IDE 확인
    ide_installed, ide_message = check_arduino_ide()
    status_icon = "✅" if ide_installed else "❌"
    print(f"{status_icon} Arduino IDE: {ide_message}")

    # 라이브러리 경로 찾기
    libraries_path = find_arduino_libraries_path()

    if libraries_path:
        print(f"✅ Arduino 라이브러리 경로: {libraries_path}")

        # 필수 라이브러리 확인
        required_libraries = ["OneWire", "DallasTemperature"]

        print("\n📦 필수 라이브러리 확인:")
        all_installed = True

        for library in required_libraries:
            installed, message = check_library(libraries_path, library)
            status_icon = "✅" if installed else "❌"
            print(f"  {status_icon} {message}")

            if not installed:
                all_installed = False

        # 설치된 모든 라이브러리 목록
        print("\n📋 설치된 라이브러리 목록:")
        try:
            library_dirs = [d for d in libraries_path.iterdir() if d.is_dir()]
            library_dirs.sort()

            for lib_dir in library_dirs[:10]:  # 처음 10개만 표시
                print(f"  - {lib_dir.name}")

            if len(library_dirs) > 10:
                print(f"  ... 및 {len(library_dirs) - 10}개 더")

        except Exception as e:
            print(f"  라이브러리 목록 읽기 실패: {e}")

    else:
        print("❌ Arduino 라이브러리 경로를 찾을 수 없습니다")
        all_installed = False

    # 결과 요약
    print("\n" + "=" * 50)
    print("📊 확인 결과 요약")
    print("=" * 50)

    if ide_installed and libraries_path and all_installed:
        print("🎉 Arduino 환경이 완전히 설정되었습니다!")
        print("   실제 Arduino 하드웨어에서 테스트할 수 있습니다.")
    else:
        print("⚠️  Arduino 환경 설정이 필요합니다.")
        print("\n💡 해결 방법:")

        if not ide_installed:
            print("  1. Arduino IDE 설치: https://www.arduino.cc/en/software")

        if not libraries_path:
            print("  2. Arduino IDE를 한 번 실행하여 라이브러리 폴더 생성")

        if libraries_path and not all_installed:
            print("  3. Arduino IDE에서 라이브러리 매니저로 다음 라이브러리 설치:")
            print("     - OneWire (by Paul Stoffregen)")
            print("     - DallasTemperature (by Miles Burton)")

    print("\n📝 참고: 현재 프로젝트는 Arduino Mock 시뮬레이터를 사용하므로")
    print("   실제 Arduino 환경은 선택사항입니다.")
    print("   자세한 설정 방법: docs/arduino_setup.md")


if __name__ == "__main__":
    main()

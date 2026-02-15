#!/usr/bin/env python3
"""
환경변수 로더 유틸리티

.env 파일에서 환경변수를 로드하고 기본값을 제공합니다.
"""

import os
from pathlib import Path
from typing import Optional


class EnvLoader:
    """환경변수 로더 클래스"""

    def __init__(self, env_file: Optional[str] = None) -> None:
        """
        환경변수 로더 초기화

        Args:
            env_file: .env 파일 경로 (기본값: 프로젝트 루트의 .env)
        """
        self.project_root = Path(__file__).parent.parent.parent.parent
        self.env_file = Path(env_file) if env_file else self.project_root / ".env"
        self._load_env_file()

    def _load_env_file(self) -> None:
        """
        .env 파일에서 환경변수 로드
        """
        if not self.env_file.exists():
            print(f"⚠️  .env 파일을 찾을 수 없습니다: {self.env_file}")
            print("💡 .env.example을 참고하여 .env 파일을 생성하세요")
            return

        try:
            with open(self.env_file, encoding="utf-8") as f:
                for line_num, line in enumerate(f, 1):
                    line = line.strip()

                    # 빈 줄이나 주석 건너뛰기
                    if not line or line.startswith("#"):
                        continue

                    # KEY=VALUE 형식 파싱
                    if "=" in line:
                        key, value = line.split("=", 1)
                        key = key.strip()
                        value = value.strip()

                        # 따옴표 제거
                        if value.startswith('"') and value.endswith('"'):
                            value = value[1:-1]
                        elif value.startswith("'") and value.endswith("'"):
                            value = value[1:-1]

                        # 환경변수 설정 (기존 값이 없는 경우만)
                        if key not in os.environ:
                            os.environ[key] = value
                    else:
                        print(f"⚠️  잘못된 .env 형식 (줄 {line_num}): {line}")

        except Exception as e:
            print(f"❌ .env 파일 로드 실패: {e}")

    @staticmethod
    def get_str(key: str, default: str = "") -> str:
        """
        문자열 환경변수 가져오기

        Args:
            key: 환경변수 키
            default: 기본값

        Returns:
            환경변수 값 또는 기본값
        """
        return os.getenv(key, default)

    @staticmethod
    def get_int(key: str, default: int = 0) -> int:
        """
        정수 환경변수 가져오기

        Args:
            key: 환경변수 키
            default: 기본값

        Returns:
            환경변수 값 또는 기본값
        """
        try:
            return int(os.getenv(key, str(default)))
        except ValueError:
            return default

    @staticmethod
    def get_float(key: str, default: float = 0.0) -> float:
        """
        실수 환경변수 가져오기

        Args:
            key: 환경변수 키
            default: 기본값

        Returns:
            환경변수 값 또는 기본값
        """
        try:
            return float(os.getenv(key, str(default)))
        except ValueError:
            return default

    @staticmethod
    def get_bool(key: str, default: bool = False) -> bool:
        """
        불린 환경변수 가져오기

        Args:
            key: 환경변수 키
            default: 기본값

        Returns:
            환경변수 값 또는 기본값
        """
        value = os.getenv(key, "").lower()
        if value in ("true", "1", "yes", "on"):
            return True
        elif value in ("false", "0", "no", "off"):
            return False
        else:
            return default

    @staticmethod
    def get_list(
        key: str, separator: str = ",", default: Optional[list] = None
    ) -> list:
        """
        리스트 환경변수 가져오기

        Args:
            key: 환경변수 키
            separator: 구분자
            default: 기본값

        Returns:
            환경변수 값을 분할한 리스트 또는 기본값
        """
        if default is None:
            default = []

        value = os.getenv(key, "")
        if not value:
            return default

        return [item.strip() for item in value.split(separator) if item.strip()]


# 전역 환경변수 로더 인스턴스
env_loader = EnvLoader()

# 편의 함수들
get_str = EnvLoader.get_str
get_int = EnvLoader.get_int
get_float = EnvLoader.get_float
get_bool = EnvLoader.get_bool
get_list = EnvLoader.get_list


def load_database_config() -> dict:
    """데이터베이스 설정 로드"""
    return {
        "host": get_str("DB_HOST", "localhost"),
        "port": get_int("DB_PORT", 5432),
        "database": get_str("DB_NAME", "dht22_monitoring"),
        "user": get_str("DB_USER", "postgres"),
        "password": get_str("DB_PASSWORD", ""),
    }


def load_server_config() -> dict:
    """서버 설정 로드"""
    return {
        "host": get_str("HOST", "localhost"),
        "port": get_int("PORT", 8000),
        "debug": get_bool("DEBUG", False),
        "ws_host": get_str("WS_HOST", "localhost"),
        "ws_port": get_int("WS_PORT", 8001),
    }


def load_sensor_config() -> dict:
    """센서 설정 로드"""
    return {
        "pin": get_int("SENSOR_PIN", 2),
        "type": get_str("SENSOR_TYPE", "DHT22"),
        "serial_port": get_str("SERIAL_PORT", "COM3"),
        "baud_rate": get_int("BAUD_RATE", 9600),
    }


def load_logging_config() -> dict:
    """로깅 설정 로드"""
    return {
        "level": get_str("LOG_LEVEL", "INFO"),
        "file_path": get_str("LOG_FILE_PATH", "logs/dht22.log"),
    }


if __name__ == "__main__":
    # 테스트 코드
    print("🔧 환경변수 로더 테스트")
    print("=" * 40)

    print(f"데이터베이스 설정: {load_database_config()}")
    print(f"서버 설정: {load_server_config()}")
    print(f"센서 설정: {load_sensor_config()}")
    print(f"로깅 설정: {load_logging_config()}")

"""
Boiler Temp Monitoring - Main (placeholder)

- 간단한 오케스트레이션 스크립트(테스트/시뮬레이션용)
- 주석 및 로깅은 한국어로 작성
- 실제 펌웨어는 Arduino(.ino)로 구현 예정
"""

import time
import logging

logger = logging.getLogger("boiler_monitor")
logging.basicConfig(level=logging.INFO)

SAMPLE_INTERVAL_SEC = 10


def readSensors():
    """센서(또는 모의 데이터)를 읽어 dict로 반환합니다."""
    # TODO: 하드웨어 드라이버 연동 또는 시리얼/네트워크로부터 데이터 수신
    sample = {
        "boiler_out_C": 65.2,
        "boiler_return_C": 45.1,
        "zone_1_C": 42.0,
        "zone_2_C": 41.8,
    }
    return sample


def processData(data):
    """데이터 전처리/검증을 수행합니다."""
    # 예: 결측치 체크, 단위 변환 등
    return data


def main():
    logger.info("보일러 온도 모니터링 시작 (placeholder)")
    try:
        while True:
            start = time.time()
            data = readSensors()
            data = processData(data)
            logger.info("Sensor data: %s", data)
            # TODO: DB 전송 / 알림 조건 검사 (추후 구현)

            elapsed = time.time() - start
            sleepTime = max(0, SAMPLE_INTERVAL_SEC - elapsed)
            time.sleep(sleepTime)
    except KeyboardInterrupt:
        logger.info("중지합니다")


if __name__ == "__main__":
    main()
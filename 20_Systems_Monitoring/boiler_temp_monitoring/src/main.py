"""Boiler Temp Monitoring - Main (placeholder)
"""

import time
import logging

logger = logging.getLogger("boiler_monitor")
logging.basicConfig(level=logging.INFO)

def read_sensors():
    # TODO: 센서 드라이버 연동 (Mock/Hardware)
    sample = {
        "boiler_out_C": 65.2,
        "boiler_return_C": 45.1,
        "zone_1_C": 42.0,
        "zone_2_C": 41.8,
    }
    return sample


def main():
    logger.info("Starting boiler temp monitoring (placeholder)")
    try:
        while True:
            data = read_sensors()
            logger.info("Sensor data: %s", data)
            # TODO: DB 전송 / 알림 조건 검사
            time.sleep(10)
    except KeyboardInterrupt:
        logger.info("Stopping")

if __name__ == "__main__":
    main()
"""
Serial communication module for reading DHT22 sensor data from Arduino
"""

import json
import logging
import time
from typing import Optional

import serial

logger = logging.getLogger(__name__)


class DHT22SerialReader:
    """Handles serial communication with Arduino DHT22 sensor"""

    def __init__(self, port: str = "COM3", baudrate: int = 9600, timeout: float = 1.0):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.connection: Optional[serial.Serial] = None
        self.is_connected = False

    def connect(self) -> bool:
        """Establish serial connection"""
        try:
            self.connection = serial.Serial(
                port=self.port, baudrate=self.baudrate, timeout=self.timeout
            )
            time.sleep(2)  # Allow Arduino to reset
            self.is_connected = True
            logger.info(f"Connected to {self.port} at {self.baudrate} baud")
            return True
        except serial.SerialException as e:
            logger.error(f"Failed to connect to {self.port}: {e}")
            self.is_connected = False
            return False

    def disconnect(self) -> None:
        """Close serial connection"""
        if self.connection and self.connection.is_open:
            self.connection.close()
            self.is_connected = False
            logger.info("Serial connection closed")

    def read_sensor_data(self) -> Optional[dict]:
        """Read and parse sensor data from Arduino"""
        if not self.is_connected or not self.connection:
            return None

        try:
            # Read line from serial
            line = self.connection.readline().decode("utf-8").strip()

            if not line:
                return None

            # Parse JSON data
            data = json.loads(line)

            # Add Python timestamp
            data["python_timestamp"] = time.time()

            return data

        except (json.JSONDecodeError, UnicodeDecodeError, serial.SerialException) as e:
            logger.warning(f"Error reading sensor data: {e}")
            return None

    def get_available_ports(self) -> list:
        """Get list of available serial ports"""
        import serial.tools.list_ports

        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]


class DHT22Simulator:
    """Simulates DHT22 sensor data for testing without hardware"""

    def __init__(self):
        self.start_time = time.time()

    def read_sensor_data(self) -> dict:
        """Generate simulated sensor data"""
        current_time = time.time()
        elapsed = current_time - self.start_time

        # Generate realistic variations
        base_temp = 23.0
        base_humidity = 50.0

        # Add some sine wave variation
        temp_variation = 3.0 * (0.5 + 0.5 * (elapsed / 60))  # Slow temperature drift
        humidity_variation = 10.0 * (
            0.5 + 0.5 * (elapsed / 120)
        )  # Slower humidity drift

        temperature = base_temp + temp_variation
        humidity = base_humidity + humidity_variation

        # Simple heat index calculation
        heat_index = temperature + (humidity / 100 * 5)

        return {
            "timestamp": int(elapsed * 1000),  # Milliseconds since start
            "temperature": round(temperature, 2),
            "humidity": round(humidity, 2),
            "heat_index": round(heat_index, 2),
            "sensor": "DHT22_SIM",
            "status": "OK",
            "python_timestamp": current_time,
        }

"""
Data processing utilities for DHT22 environmental monitoring
"""

import math
from datetime import datetime

import pandas as pd


def calculate_dew_point(temperature: float, humidity: float) -> float:
    """
    Calculate dew point using Magnus formula

    Args:
        temperature: Temperature in Celsius
        humidity: Relative humidity as percentage (0-100)

    Returns:
        Dew point in Celsius
    """
    a = 17.27
    b = 237.7

    alpha = ((a * temperature) / (b + temperature)) + math.log(humidity / 100.0)
    dew_point = (b * alpha) / (a - alpha)

    return round(dew_point, 2)


def calculate_heat_index(temperature: float, humidity: float) -> float:
    """
    Calculate heat index (apparent temperature)

    Args:
        temperature: Temperature in Celsius
        humidity: Relative humidity as percentage (0-100)

    Returns:
        Heat index in Celsius
    """
    # Convert to Fahrenheit for calculation
    temp_f = temperature * 9 / 5 + 32

    if temp_f < 80:
        return temperature  # Heat index not meaningful below 80°F

    # Heat Index formula (Rothfusz regression)
    hi = (
        -42.379
        + 2.04901523 * temp_f
        + 10.14333127 * humidity
        - 0.22475541 * temp_f * humidity
        - 6.83783e-3 * temp_f**2
        - 5.481717e-2 * humidity**2
        + 1.22874e-3 * temp_f**2 * humidity
        + 8.5282e-4 * temp_f * humidity**2
        - 1.99e-6 * temp_f**2 * humidity**2
    )

    # Convert back to Celsius
    heat_index_c = (hi - 32) * 5 / 9
    return round(heat_index_c, 2)


def calculate_discomfort_index(temperature: float, humidity: float) -> float:
    """
    Calculate discomfort index (Thom's formula)

    Args:
        temperature: Temperature in Celsius
        humidity: Relative humidity as percentage (0-100)

    Returns:
        Discomfort index
    """
    di = temperature - (0.55 - 0.0055 * humidity) * (temperature - 14.5)
    return round(di, 2)


def get_comfort_level(discomfort_index: float) -> str:
    """
    Get comfort level description based on discomfort index

    Args:
        discomfort_index: Calculated discomfort index

    Returns:
        Comfort level description
    """
    if discomfort_index < 21:
        return "매우 쾌적"
    elif discomfort_index < 24:
        return "쾌적"
    elif discomfort_index < 27:
        return "보통"
    elif discomfort_index < 29:
        return "약간 불쾌"
    elif discomfort_index < 32:
        return "불쾌"
    else:
        return "매우 불쾌"


def process_sensor_data(raw_data: dict) -> dict:
    """
    Process raw sensor data and add calculated values

    Args:
        raw_data: Raw sensor data from Arduino

    Returns:
        Processed data with additional calculated values
    """
    temperature = raw_data.get("temperature", 0)
    humidity = raw_data.get("humidity", 0)

    processed = raw_data.copy()

    # Add calculated values
    processed["dew_point"] = calculate_dew_point(temperature, humidity)
    processed["discomfort_index"] = calculate_discomfort_index(temperature, humidity)
    processed["comfort_level"] = get_comfort_level(processed["discomfort_index"])

    # Add timestamp if not present
    if "python_timestamp" not in processed:
        processed["python_timestamp"] = datetime.now().timestamp()

    # Add formatted datetime
    processed["datetime"] = datetime.fromtimestamp(
        processed["python_timestamp"]
    ).strftime("%Y-%m-%d %H:%M:%S")

    return processed


class DataBuffer:
    """Ring buffer for storing recent sensor readings"""

    def __init__(self, max_size: int = 100):
        self.max_size = max_size
        self.data: list[dict] = []

    def add(self, data_point: dict) -> None:
        """Add a data point to the buffer"""
        self.data.append(data_point)
        if len(self.data) > self.max_size:
            self.data.pop(0)

    def get_recent(self, count: int = None) -> list[dict]:
        """Get recent data points"""
        if count is None:
            return self.data.copy()
        return self.data[-count:] if count <= len(self.data) else self.data.copy()

    def to_dataframe(self) -> pd.DataFrame:
        """Convert buffer to pandas DataFrame"""
        if not self.data:
            return pd.DataFrame()
        return pd.DataFrame(self.data)

    def clear(self) -> None:
        """Clear all data from buffer"""
        self.data.clear()

    def get_stats(self) -> dict:
        """Get basic statistics from buffered data"""
        if not self.data:
            return {}

        df = self.to_dataframe()
        numeric_columns = ["temperature", "humidity", "dew_point", "discomfort_index"]

        stats = {}
        for col in numeric_columns:
            if col in df.columns:
                stats[col] = {
                    "min": df[col].min(),
                    "max": df[col].max(),
                    "mean": df[col].mean(),
                    "current": df[col].iloc[-1] if len(df) > 0 else None,
                }

        return stats

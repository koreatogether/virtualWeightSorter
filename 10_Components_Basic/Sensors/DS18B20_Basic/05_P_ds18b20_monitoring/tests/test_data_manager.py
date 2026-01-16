from __future__ import annotations

import csv
import json
import threading
import time
from datetime import datetime, timedelta

# Import DataManager directly from its file to avoid package-relative imports
from importlib import util
from pathlib import Path
from unittest.mock import patch

import pytest

REPO_ROOT = Path(__file__).resolve().parents[1]
DATA_MANAGER_PATH = REPO_ROOT / "src" / "python" / "dashboard" / "data_manager.py"
spec = util.spec_from_file_location("dashboard.data_manager", str(DATA_MANAGER_PATH))
module = util.module_from_spec(spec)  # type: ignore
assert spec and spec.loader
spec.loader.exec_module(module)  # type: ignore
DataManager = module.DataManager


def make_data(sensor_id: str, temperature: float, ts: datetime | None = None):
    if ts is None:
        ts = datetime.now()
    return {
        "sensor_id": sensor_id,
        "temperature": temperature,
        "th_value": 30.0,
        "tl_value": 20.0,
        "measurement_interval": 1000,
        "timestamp": ts.isoformat(),
    }


def test_add_and_get_latest_and_clear():
    dm = DataManager(max_data_points=5)
    d1 = make_data("s1", 25.456)
    dm.add_sensor_data(d1)

    latest = dm.get_latest_data()
    assert latest is not None
    # temperature should be rounded to 1 decimal
    assert latest["temperature"] == round(25.456, 1)
    assert latest["sensor_id"] == "s1"

    # clear and ensure data cleared
    dm.clear_data()
    assert dm.get_latest_data() is None
    stats = dm.get_statistics()
    assert stats["total_data_count"] == 0


def test_graph_data_and_statistics():
    dm = DataManager(max_data_points=10)
    temps = [20.12, 21.34, 22.56]
    for t in temps:
        dm.add_sensor_data(make_data("s2", t))

    graph = dm.get_graph_data(limit=10)
    assert len(graph["temperatures"]) == 3

    stats = dm.get_statistics()
    # avg_temperature should be rounded to 1 decimal
    expected_avg = round(sum(round(t, 1) for t in temps) / 3, 1)
    assert stats["avg_temperature"] == expected_avg
    assert stats["min_temperature"] == min(round(t, 1) for t in temps)
    assert stats["max_temperature"] == max(round(t, 1) for t in temps)


def test_get_data_range():
    dm = DataManager(max_data_points=50)
    now = datetime.now()
    older = now - timedelta(minutes=10)
    mid = now - timedelta(minutes=5)
    dm.add_sensor_data(make_data("s3", 19.0, ts=older))
    dm.add_sensor_data(make_data("s3", 21.0, ts=mid))
    dm.add_sensor_data(make_data("s3", 22.0, ts=now))

    # range starting 6 minutes ago should include mid and now
    start = now - timedelta(minutes=6)
    result = dm.get_data_range(start_time=start, end_time=None)
    assert len(result) >= 2


def test_temperature_alerts_and_export(tmp_path):
    dm = DataManager(max_data_points=10)

    # Add a high temperature to trigger HIGH_TEMPERATURE
    dm.add_sensor_data(make_data("s4", 35.0))
    alerts = dm.check_temperature_alerts()
    assert isinstance(alerts, list)
    assert alerts and alerts[0]["type"] == "HIGH_TEMPERATURE"

    # export json
    json_path = tmp_path / "export.json"
    ok = dm.export_data(str(json_path), format_type="json")
    assert ok
    assert json_path.exists()

    # export csv
    csv_path = tmp_path / "export.csv"
    ok2 = dm.export_data(str(csv_path), format_type="csv")
    assert ok2
    assert csv_path.exists()


# Additional comprehensive tests for DataManager

def test_threading_safety():
    """Test thread-safe operations on DataManager"""
    dm = DataManager(max_data_points=100)
    results = []
    errors = []

    def add_data_worker(worker_id: int) -> None:
        try:
            for i in range(10):
                data = make_data(f"sensor_{worker_id}", 20.0 + i)
                dm.add_sensor_data(data)
                time.sleep(0.001)  # Small delay to increase chance of race conditions
        except Exception as e:
            errors.append(e)

    def read_data_worker() -> None:
        try:
            for _ in range(20):
                latest = dm.get_latest_data()
                stats = dm.get_statistics()
                results.append((latest, stats))
                time.sleep(0.001)
        except Exception as e:
            errors.append(e)

    # Create multiple threads
    threads = []
    for i in range(3):
        t = threading.Thread(target=add_data_worker, args=(i,))
        threads.append(t)

    read_thread = threading.Thread(target=read_data_worker)
    threads.append(read_thread)

    # Start all threads
    for t in threads:
        t.start()

    # Wait for completion
    for t in threads:
        t.join()

    # Verify no errors occurred
    assert len(errors) == 0, f"Threading errors: {errors}"

    # Verify data integrity
    stats = dm.get_statistics()
    assert stats["total_data_count"] == 30  # 3 workers * 10 items each
    assert len(results) > 0


def test_edge_cases_and_error_handling():
    """Test edge cases and error handling scenarios"""
    dm = DataManager(max_data_points=5)

    # Test with invalid data types
    invalid_data = {
        "sensor_id": "test",
        "temperature": "not_a_number",
        "th_value": 30.0,
        "tl_value": 20.0,
        "measurement_interval": 1000,
        "timestamp": datetime.now().isoformat(),
    }

    # Should handle invalid data gracefully (not crash)
    dm.add_sensor_data(invalid_data)
    stats = dm.get_statistics()
    assert stats["total_data_count"] == 0  # Invalid data should not be added

    # Test with missing fields
    incomplete_data = {
        "sensor_id": "test",
        "temperature": 25.0,
        # Missing other required fields
    }
    dm.add_sensor_data(incomplete_data)
    assert dm.get_latest_data() is None

    # Test with extreme temperature values
    extreme_data = make_data("extreme", 999.9)
    dm.add_sensor_data(extreme_data)
    latest = dm.get_latest_data()
    assert latest is not None
    assert latest["temperature"] == 999.9  # Should accept extreme values


def test_capacity_limits():
    """Test DataManager capacity limits"""
    dm = DataManager(max_data_points=3)

    # Add more data than capacity
    for i in range(5):
        dm.add_sensor_data(make_data("test", 20.0 + i))

    # Should only keep the most recent 3 entries
    assert len(dm.sensor_data) == 3
    latest = dm.get_latest_data()
    assert latest["temperature"] == 24.0  # Last added temperature

    # Statistics should reflect all processed data
    stats = dm.get_statistics()
    assert stats["total_data_count"] == 5  # All 5 were processed


def test_temperature_alerts_comprehensive():
    """Test comprehensive temperature alert scenarios"""
    dm = DataManager(max_data_points=10)

    # Test no alerts when within range
    dm.add_sensor_data(make_data("normal", 25.0))  # th=30, tl=20
    alerts = dm.check_temperature_alerts()
    assert len(alerts) == 0

    # Test high temperature alert
    dm.add_sensor_data(make_data("hot", 35.0))  # Above th=30
    alerts = dm.check_temperature_alerts()
    assert len(alerts) == 1
    assert alerts[0]["type"] == "HIGH_TEMPERATURE"
    assert alerts[0]["temperature"] == 35.0
    assert alerts[0]["threshold"] == 30.0
    assert "상한값을 초과" in alerts[0]["message"]

    # Test low temperature alert
    dm.add_sensor_data(make_data("cold", 15.0))  # Below tl=20
    alerts = dm.check_temperature_alerts()
    assert len(alerts) == 1
    assert alerts[0]["type"] == "LOW_TEMPERATURE"
    assert alerts[0]["temperature"] == 15.0
    assert alerts[0]["threshold"] == 20.0
    assert "하한값 미만" in alerts[0]["message"]

    # Test when no data available
    dm.clear_data()
    alerts = dm.check_temperature_alerts()
    assert len(alerts) == 0


def test_graph_data_formatting():
    """Test graph data preparation with various scenarios"""
    dm = DataManager(max_data_points=10)

    # Test empty data
    graph_data = dm.get_graph_data()
    assert len(graph_data["timestamps"]) == 0
    assert len(graph_data["temperatures"]) == 0

    # Add test data with different timestamps
    now = datetime.now()
    timestamps = [
        now - timedelta(minutes=5),
        now - timedelta(minutes=3),
        now - timedelta(minutes=1),
        now
    ]

    for i, ts in enumerate(timestamps):
        data = make_data(f"sensor_{i}", 20.0 + i, ts)
        dm.add_sensor_data(data)

    # Test full graph data
    graph_data = dm.get_graph_data()
    assert len(graph_data["timestamps"]) == 4
    assert len(graph_data["temperatures"]) == 4
    assert len(graph_data["th_values"]) == 4
    assert len(graph_data["tl_values"]) == 4

    # Verify temperature progression
    assert graph_data["temperatures"] == [20.0, 21.0, 22.0, 23.0]

    # Test with limit
    limited_graph = dm.get_graph_data(limit=2)
    assert len(limited_graph["timestamps"]) == 2
    assert limited_graph["temperatures"] == [22.0, 23.0]  # Most recent 2


def test_data_range_filtering():
    """Test time-based data filtering"""
    dm = DataManager(max_data_points=20)

    base_time = datetime.now()

    # Add data spanning 10 minutes
    for i in range(10):
        ts = base_time - timedelta(minutes=9-i)
        data = make_data("sensor", 20.0 + i, ts)
        dm.add_sensor_data(data)

    # Test range filtering - last 5 minutes
    start_time = base_time - timedelta(minutes=5)
    recent_data = dm.get_data_range(start_time=start_time)
    assert len(recent_data) == 6  # Including data at exactly 5 minutes ago

    # Test with both start and end time
    end_time = base_time - timedelta(minutes=2)
    middle_data = dm.get_data_range(start_time=start_time, end_time=end_time)
    assert len(middle_data) == 4  # 5min, 4min, 3min, 2min ago

    # Test with limit
    limited_data = dm.get_data_range(start_time=start_time, limit=3)
    assert len(limited_data) == 3

    # Test convenience method
    recent_60min = dm.get_recent_data(minutes=60)
    assert len(recent_60min) == 10  # All data is within 60 minutes


def test_data_export_comprehensive(tmp_path):
    """Test comprehensive data export functionality"""
    dm = DataManager(max_data_points=10)

    # Add test data
    for i in range(3):
        dm.add_sensor_data(make_data(f"sensor_{i}", 20.0 + i))

    # Test JSON export
    json_path = tmp_path / "test_export.json"
    success = dm.export_data(str(json_path), "json")
    assert success
    assert json_path.exists()

    # Verify JSON content
    with open(json_path, encoding="utf-8") as f:
        exported_data = json.load(f)

    assert "metadata" in exported_data
    assert "data" in exported_data
    assert exported_data["metadata"]["total_records"] == 3
    assert len(exported_data["data"]) == 3
    assert "statistics" in exported_data["metadata"]

    # Test CSV export
    csv_path = tmp_path / "test_export.csv"
    success = dm.export_data(str(csv_path), "csv")
    assert success
    assert csv_path.exists()

    # Verify CSV content
    with open(csv_path, encoding="utf-8") as f:
        reader = csv.DictReader(f)
        rows = list(reader)

    assert len(rows) == 3
    assert "sensor_id" in rows[0]
    assert "temperature" in rows[0]

    # Test empty data export
    dm.clear_data()
    empty_json_path = tmp_path / "empty_export.json"
    success = dm.export_data(str(empty_json_path), "json")
    assert success

    # Test unsupported format
    unsupported_path = tmp_path / "test.xml"
    success = dm.export_data(str(unsupported_path), "xml")
    assert not success

    # Test invalid file path
    invalid_path = "/invalid/path/test.json"
    success = dm.export_data(invalid_path, "json")
    assert not success


def test_data_summary():
    """Test comprehensive data summary functionality"""
    dm = DataManager(max_data_points=100)

    # Test empty summary
    summary = dm.get_data_summary()
    assert summary["total_data_points"] == 0
    assert summary["recent_data_points"] == 0
    assert summary["current_sensor_id"] is None
    assert summary["data_rate"] == 0

    # Add recent data (within last hour)
    now = datetime.now()
    for i in range(5):
        ts = now - timedelta(minutes=i * 10)  # 0, 10, 20, 30, 40 minutes ago
        data = make_data("sensor_test", 25.0 + i, ts)
        dm.add_sensor_data(data)

    # Add older data (beyond 1 hour)
    old_data = make_data("sensor_test", 30.0, now - timedelta(hours=2))
    dm.add_sensor_data(old_data)

    summary = dm.get_data_summary()
    assert summary["total_data_points"] == 6
    assert summary["recent_data_points"] == 5  # Only recent data
    assert summary["current_sensor_id"] == "sensor_test"
    assert summary["data_rate"] == 5/60  # 5 points in 60 minutes

    # Verify config is included
    assert "current_config" in summary
    assert summary["current_config"]["th_value"] == 30.0


def test_invalid_timestamp_handling():
    """Test handling of invalid timestamps"""
    dm = DataManager(max_data_points=10)

    # Add data with invalid timestamp
    invalid_data = {
        "sensor_id": "test",
        "temperature": 25.0,
        "th_value": 30.0,
        "tl_value": 20.0,
        "measurement_interval": 1000,
        "timestamp": "invalid_timestamp",
    }

    dm.add_sensor_data(invalid_data)

    # Should still add data but skip invalid timestamp in range queries
    assert dm.get_latest_data() is not None

    # Graph data should handle invalid timestamps gracefully
    graph_data = dm.get_graph_data()
    # Should not include the invalid timestamp entry
    assert len(graph_data["timestamps"]) == 0


def test_configuration_tracking():
    """Test sensor configuration tracking"""
    dm = DataManager(max_data_points=10)

    # Initial config should be defaults
    config = dm.get_current_config()
    assert config["sensor_id"] is None
    assert config["th_value"] == 30.0
    assert config["tl_value"] == 20.0
    assert config["measurement_interval"] == 1000

    # Add data with different config
    custom_data = {
        "sensor_id": "custom_sensor",
        "temperature": 25.0,
        "th_value": 35.0,
        "tl_value": 15.0,
        "measurement_interval": 2000,
        "timestamp": datetime.now().isoformat(),
    }

    dm.add_sensor_data(custom_data)

    # Config should be updated
    updated_config = dm.get_current_config()
    assert updated_config["sensor_id"] == "custom_sensor"
    assert updated_config["th_value"] == 35.0
    assert updated_config["tl_value"] == 15.0
    assert updated_config["measurement_interval"] == 2000


def test_statistics_accuracy():
    """Test statistical calculations accuracy"""
    dm = DataManager(max_data_points=10)

    # Add known temperature values
    temperatures = [20.0, 22.5, 25.0, 27.5, 30.0]
    for temp in temperatures:
        dm.add_sensor_data(make_data("test", temp))

    stats = dm.get_statistics()

    # Verify statistics
    assert stats["total_data_count"] == 5
    assert stats["min_temperature"] == 20.0
    assert stats["max_temperature"] == 30.0
    assert stats["avg_temperature"] == 25.0  # (20+22.5+25+27.5+30)/5 = 25
    assert stats["last_update_time"] is not None

    # Test with single data point
    dm.clear_data()
    dm.add_sensor_data(make_data("single", 23.7))

    single_stats = dm.get_statistics()
    assert single_stats["min_temperature"] == 23.7
    assert single_stats["max_temperature"] == 23.7
    assert single_stats["avg_temperature"] == 23.7


def test_main_function_coverage():
    """Test the main function for coverage completeness"""
    # Import and test the main function
    with patch('sys.stdout'):
        # Should not raise any exceptions
        module.main()


@pytest.fixture()
def sample_data_manager():
    """Fixture providing a DataManager with sample data"""
    dm = DataManager(max_data_points=20)

    # Add some sample data
    base_time = datetime.now()
    for i in range(5):
        ts = base_time - timedelta(minutes=i)
        data = make_data(f"sensor_{i}", 20.0 + i * 2, ts)
        dm.add_sensor_data(data)

    return dm


def test_data_manager_fixture(sample_data_manager):
    """Test using the fixture"""
    assert sample_data_manager.get_statistics()["total_data_count"] == 5
    assert len(sample_data_manager.sensor_data) == 5

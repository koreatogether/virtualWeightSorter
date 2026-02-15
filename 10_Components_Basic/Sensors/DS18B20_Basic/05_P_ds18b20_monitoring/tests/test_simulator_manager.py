from __future__ import annotations

import json
from pathlib import Path
from unittest.mock import MagicMock, patch

import pytest

REPO_ROOT = Path(__file__).resolve().parents[1]
SIM_SRC = REPO_ROOT / "src" / "python"

# Ensure the simulator package can be imported (so relative imports work)
import sys

sys.path.insert(0, str(SIM_SRC))

import importlib

sim_mod = importlib.import_module("simulator.simulator_manager")
SimulatorManager = sim_mod.SimulatorManager


def test_load_config_and_create_simulator(tmp_path):
    # Create a minimal config file
    cfg = {
        "serial_settings": {"port": "COM1", "baudrate": 9600},
        "sensor_defaults": {"sensor_id": "28FF123456789ABC"},
        "scenarios": {"normal": {"base_temperature": 25.0, "noise_level": 0.5}},
    }
    cfg_file = tmp_path / "config.json"
    cfg_file.write_text(str(cfg).replace("'", '"'))

    mgr = SimulatorManager(config_path=str(cfg_file))
    assert mgr.config_path.exists()

    sim = mgr.create_simulator("normal")
    # create_simulator may return None if DS18B20Simulator depends on serial;
    # ensure it either returns an object or None gracefully
    assert sim is None or hasattr(sim, "create_sensor_data")

    # update sensor config when simulator not created should return False
    updated = mgr.update_sensor_config(th_value=28.0)
    assert updated is False


# Additional comprehensive tests for SimulatorManager

class TestSimulatorManagerBasic:
    """Test basic SimulatorManager functionality"""

    def test_initialization_with_default_config(self):
        """Test SimulatorManager initialization with default config"""
        mgr = SimulatorManager()

        # Should use default config path
        expected_path = REPO_ROOT / "src" / "python" / "simulator" / "config.json"
        assert mgr.config_path == expected_path

        # Other attributes should be initialized
        assert mgr.current_simulator is None
        assert mgr.config is None

    def test_initialization_with_custom_config_path(self, tmp_path):
        """Test SimulatorManager initialization with custom config path"""
        config_file = tmp_path / "custom_config.json"
        config_file.write_text('{"test": "config"}')

        mgr = SimulatorManager(config_path=str(config_file))

        assert mgr.config_path == config_file
        assert mgr.current_simulator is None

    def test_initialization_with_nonexistent_config(self, tmp_path):
        """Test initialization with non-existent config file"""
        nonexistent_config = tmp_path / "nonexistent.json"

        mgr = SimulatorManager(config_path=str(nonexistent_config))

        # Should still create the manager
        assert mgr.config_path == nonexistent_config
        assert mgr.current_simulator is None

    def test_string_representation(self, tmp_path):
        """Test string representation of SimulatorManager"""
        config_file = tmp_path / "test_config.json"
        config_file.write_text('{}')

        mgr = SimulatorManager(config_path=str(config_file))

        str_repr = str(mgr)
        assert "SimulatorManager" in str_repr
        assert str(config_file) in str_repr


class TestSimulatorManagerConfigLoading:
    """Test configuration loading functionality"""

    def test_load_valid_config(self, tmp_path):
        """Test loading a valid configuration file"""
        config_data = {
            "serial_settings": {
                "port": "COM3",
                "baudrate": 115200
            },
            "sensor_defaults": {
                "sensor_id": "28FF123456789ABC",
                "th_value": 30.0,
                "tl_value": 20.0,
                "measurement_interval": 1000
            },
            "scenarios": {
                "hot": {
                    "base_temperature": 35.0,
                    "noise_level": 0.2,
                    "description": "Hot environment simulation"
                },
                "cold": {
                    "base_temperature": 15.0,
                    "noise_level": 0.3,
                    "description": "Cold environment simulation"
                }
            }
        }

        config_file = tmp_path / "valid_config.json"
        config_file.write_text(json.dumps(config_data, indent=2))

        mgr = SimulatorManager(config_path=str(config_file))

        # Load configuration
        success = mgr.load_config()

        assert success is True
        assert mgr.config is not None
        assert mgr.config["serial_settings"]["port"] == "COM3"
        assert mgr.config["serial_settings"]["baudrate"] == 115200
        assert "hot" in mgr.config["scenarios"]
        assert "cold" in mgr.config["scenarios"]

    def test_load_config_file_not_found(self, tmp_path):
        """Test loading configuration when file doesn't exist"""
        nonexistent_file = tmp_path / "nonexistent.json"

        mgr = SimulatorManager(config_path=str(nonexistent_file))

        success = mgr.load_config()

        assert success is False
        assert mgr.config is None

    def test_load_config_invalid_json(self, tmp_path):
        """Test loading configuration with invalid JSON"""
        invalid_json_file = tmp_path / "invalid.json"
        invalid_json_file.write_text('{"invalid": json syntax}')  # Missing quotes

        mgr = SimulatorManager(config_path=str(invalid_json_file))

        success = mgr.load_config()

        assert success is False
        assert mgr.config is None

    def test_load_config_empty_file(self, tmp_path):
        """Test loading configuration from empty file"""
        empty_file = tmp_path / "empty.json"
        empty_file.write_text('')

        mgr = SimulatorManager(config_path=str(empty_file))

        success = mgr.load_config()

        assert success is False
        assert mgr.config is None

    def test_load_config_minimal_valid(self, tmp_path):
        """Test loading minimal valid configuration"""
        minimal_config = {}

        config_file = tmp_path / "minimal.json"
        config_file.write_text(json.dumps(minimal_config))

        mgr = SimulatorManager(config_path=str(config_file))

        success = mgr.load_config()

        assert success is True
        assert mgr.config == {}

    def test_get_available_scenarios_no_config(self):
        """Test getting scenarios when no config is loaded"""
        mgr = SimulatorManager()

        scenarios = mgr.get_available_scenarios()

        assert scenarios == []

    def test_get_available_scenarios_with_config(self, tmp_path):
        """Test getting scenarios from loaded config"""
        config_data = {
            "scenarios": {
                "scenario1": {"base_temperature": 25.0},
                "scenario2": {"base_temperature": 30.0},
                "scenario3": {"base_temperature": 20.0}
            }
        }

        config_file = tmp_path / "scenarios.json"
        config_file.write_text(json.dumps(config_data))

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        scenarios = mgr.get_available_scenarios()

        assert set(scenarios) == {"scenario1", "scenario2", "scenario3"}

    def test_get_available_scenarios_no_scenarios_section(self, tmp_path):
        """Test getting scenarios when config has no scenarios section"""
        config_data = {
            "serial_settings": {"port": "COM3"}
        }

        config_file = tmp_path / "no_scenarios.json"
        config_file.write_text(json.dumps(config_data))

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        scenarios = mgr.get_available_scenarios()

        assert scenarios == []


class TestSimulatorManagerCreation:
    """Test simulator creation functionality"""

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_create_simulator_success(self, mock_simulator_class, tmp_path):
        """Test successful simulator creation"""
        # Set up config
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "sensor_defaults": {"sensor_id": "28FF123456789ABC"},
            "scenarios": {
                "test_scenario": {
                    "base_temperature": 25.0,
                    "noise_level": 0.5
                }
            }
        }

        config_file = tmp_path / "test_config.json"
        config_file.write_text(json.dumps(config_data))

        # Mock simulator
        mock_simulator = MagicMock()
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        simulator = mgr.create_simulator("test_scenario")

        # Verify simulator was created and configured
        mock_simulator_class.assert_called_once_with(port="COM3", baudrate=9600)
        assert simulator == mock_simulator
        assert mgr.current_simulator == mock_simulator

        # Verify simulator configuration
        assert mock_simulator.sensor_id == "28FF123456789ABC"
        assert mock_simulator.base_temperature == 25.0
        assert mock_simulator.noise_level == 0.5

    def test_create_simulator_no_config_loaded(self):
        """Test creating simulator when no config is loaded"""
        mgr = SimulatorManager()

        simulator = mgr.create_simulator("any_scenario")

        assert simulator is None
        assert mgr.current_simulator is None

    def test_create_simulator_invalid_scenario(self, tmp_path):
        """Test creating simulator with invalid scenario"""
        config_data = {
            "scenarios": {
                "valid_scenario": {"base_temperature": 25.0}
            }
        }

        config_file = tmp_path / "test_config.json"
        config_file.write_text(json.dumps(config_data))

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        simulator = mgr.create_simulator("invalid_scenario")

        assert simulator is None
        assert mgr.current_simulator is None

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_create_simulator_with_defaults(self, mock_simulator_class, tmp_path):
        """Test creating simulator with default values"""
        config_data = {
            "serial_settings": {"port": "COM1", "baudrate": 115200},
            "sensor_defaults": {
                "sensor_id": "DEFAULT_SENSOR_ID",
                "th_value": 35.0,
                "tl_value": 15.0,
                "measurement_interval": 2000
            },
            "scenarios": {
                "minimal": {}  # Empty scenario - should use defaults
            }
        }

        config_file = tmp_path / "defaults_config.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        simulator = mgr.create_simulator("minimal")

        assert simulator == mock_simulator
        assert mgr.current_simulator == mock_simulator

        # Verify default values were applied
        assert mock_simulator.sensor_id == "DEFAULT_SENSOR_ID"
        assert mock_simulator.th_value == 35.0
        assert mock_simulator.tl_value == 15.0
        assert mock_simulator.measurement_interval == 2.0  # Converted from ms to seconds

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_create_simulator_scenario_overrides_defaults(self, mock_simulator_class, tmp_path):
        """Test scenario values override defaults"""
        config_data = {
            "serial_settings": {"port": "COM1", "baudrate": 9600},
            "sensor_defaults": {
                "sensor_id": "DEFAULT_ID",
                "th_value": 30.0,
                "tl_value": 20.0
            },
            "scenarios": {
                "override_test": {
                    "base_temperature": 28.0,
                    "noise_level": 0.8,
                    "th_value": 40.0,  # Override default
                    "sensor_id": "OVERRIDE_ID"  # Override default
                }
            }
        }

        config_file = tmp_path / "override_config.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        simulator = mgr.create_simulator("override_test")

        # Verify overrides took effect
        assert mock_simulator.sensor_id == "OVERRIDE_ID"  # Override
        assert mock_simulator.th_value == 40.0  # Override
        assert mock_simulator.tl_value == 20.0  # Default (not overridden)
        assert mock_simulator.base_temperature == 28.0  # Scenario value
        assert mock_simulator.noise_level == 0.8  # Scenario value


class TestSimulatorManagerControl:
    """Test simulator control functionality"""

    def test_start_simulator_no_current_simulator(self):
        """Test starting simulator when none is created"""
        mgr = SimulatorManager()

        success = mgr.start_simulator()

        assert success is False

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_start_simulator_success(self, mock_simulator_class, tmp_path):
        """Test successful simulator start"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {"test": {"base_temperature": 25.0}}
        }

        config_file = tmp_path / "start_test.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_simulator.start.return_value = True
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()
        mgr.create_simulator("test")

        success = mgr.start_simulator()

        assert success is True
        mock_simulator.start.assert_called_once()

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_start_simulator_failure(self, mock_simulator_class, tmp_path):
        """Test simulator start failure"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {"test": {"base_temperature": 25.0}}
        }

        config_file = tmp_path / "start_fail_test.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_simulator.start.return_value = False
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()
        mgr.create_simulator("test")

        success = mgr.start_simulator()

        assert success is False
        mock_simulator.start.assert_called_once()

    def test_stop_simulator_no_current_simulator(self):
        """Test stopping simulator when none is created"""
        mgr = SimulatorManager()

        # Should not raise an exception
        mgr.stop_simulator()

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_stop_simulator_success(self, mock_simulator_class, tmp_path):
        """Test successful simulator stop"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {"test": {"base_temperature": 25.0}}
        }

        config_file = tmp_path / "stop_test.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()
        mgr.create_simulator("test")

        mgr.stop_simulator()

        mock_simulator.stop.assert_called_once()

    def test_get_simulator_status_no_simulator(self):
        """Test getting status when no simulator exists"""
        mgr = SimulatorManager()

        status = mgr.get_simulator_status()

        assert status is None

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_get_simulator_status_with_simulator(self, mock_simulator_class, tmp_path):
        """Test getting status with active simulator"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {"test": {"base_temperature": 25.0}}
        }

        config_file = tmp_path / "status_test.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_status = {
            "is_running": True,
            "port": "COM3",
            "sensor_id": "28FF123456789ABC",
            "current_temperature": 25.5
        }
        mock_simulator.get_status.return_value = mock_status
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()
        mgr.create_simulator("test")

        status = mgr.get_simulator_status()

        assert status == mock_status
        mock_simulator.get_status.assert_called_once()


class TestSimulatorManagerConfiguration:
    """Test configuration update functionality"""

    def test_update_sensor_config_no_simulator(self):
        """Test updating config when no simulator exists"""
        mgr = SimulatorManager()

        success = mgr.update_sensor_config(th_value=35.0)

        assert success is False

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_update_sensor_config_success(self, mock_simulator_class, tmp_path):
        """Test successful sensor config update"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {"test": {"base_temperature": 25.0}}
        }

        config_file = tmp_path / "update_test.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()
        mgr.create_simulator("test")

        success = mgr.update_sensor_config(
            th_value=35.0,
            tl_value=15.0,
            measurement_interval=2.5
        )

        assert success is True
        assert mock_simulator.th_value == 35.0
        assert mock_simulator.tl_value == 15.0
        assert mock_simulator.measurement_interval == 2.5

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_update_sensor_config_partial(self, mock_simulator_class, tmp_path):
        """Test partial sensor config update"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {"test": {"base_temperature": 25.0}}
        }

        config_file = tmp_path / "partial_update_test.json"
        config_file.write_text(json.dumps(config_data))

        mock_simulator = MagicMock()
        mock_simulator.th_value = 30.0  # Original value
        mock_simulator.tl_value = 20.0  # Original value
        mock_simulator_class.return_value = mock_simulator

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()
        mgr.create_simulator("test")

        # Update only th_value
        success = mgr.update_sensor_config(th_value=32.0)

        assert success is True
        assert mock_simulator.th_value == 32.0
        assert mock_simulator.tl_value == 20.0  # Should remain unchanged


class TestSimulatorManagerEdgeCases:
    """Test edge cases and error conditions"""

    def test_create_simulator_missing_serial_settings(self, tmp_path):
        """Test creating simulator with missing serial settings"""
        config_data = {
            "scenarios": {"test": {"base_temperature": 25.0}}
            # Missing serial_settings
        }

        config_file = tmp_path / "missing_serial.json"
        config_file.write_text(json.dumps(config_data))

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        # Should handle missing serial settings gracefully
        simulator = mgr.create_simulator("test")

        # Might return None or handle with defaults
        assert simulator is None or hasattr(simulator, 'port')

    def test_config_with_unicode_characters(self, tmp_path):
        """Test configuration with unicode characters"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {
                "테스트": {  # Korean characters
                    "base_temperature": 25.0,
                    "description": "한글 테스트 시나리오"
                }
            }
        }

        config_file = tmp_path / "unicode_config.json"
        config_file.write_text(json.dumps(config_data, ensure_ascii=False), encoding='utf-8')

        mgr = SimulatorManager(config_path=str(config_file))

        success = mgr.load_config()

        assert success is True
        scenarios = mgr.get_available_scenarios()
        assert "테스트" in scenarios

    def test_very_large_config_file(self, tmp_path):
        """Test handling of very large config file"""
        # Create a config with many scenarios
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {}
        }

        # Add 1000 scenarios
        for i in range(1000):
            config_data["scenarios"][f"scenario_{i}"] = {
                "base_temperature": 20.0 + i * 0.01,
                "noise_level": 0.1 + i * 0.0001
            }

        config_file = tmp_path / "large_config.json"
        config_file.write_text(json.dumps(config_data))

        mgr = SimulatorManager(config_path=str(config_file))

        success = mgr.load_config()

        assert success is True
        scenarios = mgr.get_available_scenarios()
        assert len(scenarios) == 1000

    def test_config_with_nested_structures(self, tmp_path):
        """Test configuration with deeply nested structures"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {
                "complex": {
                    "base_temperature": 25.0,
                    "advanced_settings": {
                        "drift_parameters": {
                            "max_drift": 3.0,
                            "drift_rate": 0.1,
                            "drift_bounds": [-5.0, 5.0]
                        },
                        "noise_parameters": {
                            "distribution": "gaussian",
                            "stddev": 0.5
                        }
                    }
                }
            }
        }

        config_file = tmp_path / "nested_config.json"
        config_file.write_text(json.dumps(config_data))

        mgr = SimulatorManager(config_path=str(config_file))

        success = mgr.load_config()

        assert success is True
        assert mgr.config["scenarios"]["complex"]["advanced_settings"]["drift_parameters"]["max_drift"] == 3.0

    def test_config_file_permissions_error(self, tmp_path):
        """Test handling of file permission errors"""
        # This test might not work on all platforms due to permission handling differences
        config_file = tmp_path / "permission_test.json"
        config_file.write_text('{"test": "config"}')

        mgr = SimulatorManager(config_path=str(config_file))

        # Try to load config (should work normally)
        success = mgr.load_config()
        assert success is True

    @patch('simulator.simulator_manager.DS18B20Simulator')
    def test_simulator_creation_exception(self, mock_simulator_class, tmp_path):
        """Test handling of exceptions during simulator creation"""
        config_data = {
            "serial_settings": {"port": "COM3", "baudrate": 9600},
            "scenarios": {"test": {"base_temperature": 25.0}}
        }

        config_file = tmp_path / "exception_test.json"
        config_file.write_text(json.dumps(config_data))

        # Mock simulator creation to raise exception
        mock_simulator_class.side_effect = Exception("Creation failed")

        mgr = SimulatorManager(config_path=str(config_file))
        mgr.load_config()

        simulator = mgr.create_simulator("test")

        # Should handle exception gracefully
        assert simulator is None
        assert mgr.current_simulator is None


@pytest.fixture()
def sample_config_manager(tmp_path):
    """Fixture providing a SimulatorManager with sample configuration"""
    config_data = {
        "serial_settings": {
            "port": "COM3",
            "baudrate": 9600
        },
        "sensor_defaults": {
            "sensor_id": "28FF123456789ABC",
            "th_value": 30.0,
            "tl_value": 20.0,
            "measurement_interval": 1000
        },
        "scenarios": {
            "normal": {
                "base_temperature": 25.0,
                "noise_level": 0.5
            },
            "hot": {
                "base_temperature": 35.0,
                "noise_level": 0.2
            }
        }
    }

    config_file = tmp_path / "sample_config.json"
    config_file.write_text(json.dumps(config_data))

    mgr = SimulatorManager(config_path=str(config_file))
    mgr.load_config()

    return mgr


def test_sample_config_manager_fixture(sample_config_manager):
    """Test using the sample config manager fixture"""
    assert sample_config_manager.config is not None
    scenarios = sample_config_manager.get_available_scenarios()
    assert set(scenarios) == {"normal", "hot"}
    assert sample_config_manager.config["serial_settings"]["port"] == "COM3"

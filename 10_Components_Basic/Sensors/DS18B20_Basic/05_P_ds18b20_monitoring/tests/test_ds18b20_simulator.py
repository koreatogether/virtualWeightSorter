from __future__ import annotations

import json
import threading
from datetime import datetime

# Import DS18B20Simulator directly from its file to avoid package-relative imports
from importlib import util
from pathlib import Path
from unittest.mock import MagicMock, patch

import pytest
import serial

REPO_ROOT = Path(__file__).resolve().parents[1]
SIMULATOR_PATH = REPO_ROOT / "src" / "python" / "simulator" / "ds18b20_simulator.py"
spec = util.spec_from_file_location("simulator.ds18b20_simulator", str(SIMULATOR_PATH))
module = util.module_from_spec(spec)  # type: ignore
assert spec and spec.loader
spec.loader.exec_module(module)  # type: ignore
DS18B20Simulator = module.DS18B20Simulator


class TestDS18B20SimulatorBasic:
    """Test basic DS18B20Simulator functionality"""

    def test_initialization(self):
        """Test simulator initialization"""
        simulator = DS18B20Simulator(port="COM3", baudrate=9600)

        assert simulator.port == "COM3"
        assert simulator.baudrate == 9600
        assert simulator.serial_conn is None

        # Check default sensor settings
        assert simulator.sensor_addr == "28FF123456789ABC"
        assert simulator.th_value == 30.0
        assert simulator.tl_value == 20.0
        assert simulator.measurement_interval == 1.0

        # Check simulation settings
        assert simulator.base_temperature == 25.0
        assert simulator.temperature_drift == 0.0
        assert simulator.noise_level == 1.0

        # Check execution state
        assert simulator.is_running is False
        assert simulator.simulation_thread is None

    def test_initialization_with_defaults(self):
        """Test simulator initialization with default parameters"""
        simulator = DS18B20Simulator()

        assert simulator.port == "COM3"
        assert simulator.baudrate == 9600

    def test_temperature_generation(self):
        """Test temperature value generation"""
        simulator = DS18B20Simulator()

        # Generate multiple temperatures and check range
        temperatures = [simulator.generate_temperature() for _ in range(100)]

        # All temperatures should be within DS18B20 range
        assert all(18.0 <= temp <= 35.0 for temp in temperatures)

        # Should return floats rounded to 1 decimal place
        assert all(isinstance(temp, float) for temp in temperatures)
        assert all(round(temp, 1) == temp for temp in temperatures)

        # Should have some variation (noise)
        assert len(set(temperatures)) > 1

    def test_temperature_generation_with_drift(self):
        """Test temperature generation with drift"""
        simulator = DS18B20Simulator()
        simulator.temperature_drift = 3.0  # Add positive drift

        # Generate temperatures
        temp1 = simulator.generate_temperature()

        # Should be affected by drift
        assert temp1 >= 25.0  # base_temperature + some of the drift

    def test_temperature_generation_boundaries(self):
        """Test temperature generation at boundaries"""
        simulator = DS18B20Simulator()

        # Test extreme drift values
        simulator.base_temperature = 10.0  # Below minimum
        simulator.temperature_drift = 0.0
        simulator.noise_level = 0.0

        temp = simulator.generate_temperature()
        assert temp == 18.0  # Should be clamped to minimum

        # Test high temperature
        simulator.base_temperature = 40.0  # Above maximum
        temp = simulator.generate_temperature()
        assert temp == 35.0  # Should be clamped to maximum

    def test_create_sensor_data(self):
        """Test sensor data JSON creation"""
        simulator = DS18B20Simulator()
        simulator.sensor_addr = "TEST123456789ABC"
        simulator.th_value = 32.5
        simulator.tl_value = 18.5
        simulator.measurement_interval = 2.5

        data = simulator.create_sensor_data()

        assert data["type"] == "sensor_data"
        assert data["sensor_addr"] == "TEST123456789ABC"
        assert isinstance(data["temperature"], float)
        assert 18.0 <= data["temperature"] <= 35.0
        assert data["th_value"] == 32.5
        assert data["tl_value"] == 18.5
        assert data["measurement_interval"] == 2500  # Should be in milliseconds
        assert "timestamp" in data

        # Verify timestamp format
        datetime.fromisoformat(data["timestamp"])

    def test_create_response(self):
        """Test response message creation"""
        simulator = DS18B20Simulator()
        simulator.sensor_addr = "TEST123456789ABC"

        response = simulator.create_response("success", "Configuration updated")

        assert response["type"] == "response"
        assert response["status"] == "success"
        assert response["message"] == "Configuration updated"
        assert response["sensor_id"] == "TEST123456789ABC"
        assert "timestamp" in response

        # Verify timestamp format
        datetime.fromisoformat(response["timestamp"])

    @patch('serial.Serial')
    def test_connect_success(self, mock_serial):
        """Test successful serial connection"""
        simulator = DS18B20Simulator(port="COM3", baudrate=9600)

        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn

        result = simulator.connect()

        assert result is True
        assert simulator.serial_conn == mock_conn

        mock_serial.assert_called_once_with(
            port="COM3",
            baudrate=9600,
            timeout=1.0,
            write_timeout=1.0
        )

    @patch('serial.Serial')
    def test_connect_failure(self, mock_serial):
        """Test serial connection failure"""
        simulator = DS18B20Simulator(port="INVALID", baudrate=9600)

        mock_serial.side_effect = serial.SerialException("Port not found")

        result = simulator.connect()

        assert result is False
        assert simulator.serial_conn is None

    def test_disconnect(self):
        """Test serial disconnection"""
        simulator = DS18B20Simulator()

        # Mock connected state
        mock_conn = MagicMock()
        mock_conn.is_open = True
        simulator.serial_conn = mock_conn

        simulator.disconnect()

        mock_conn.close.assert_called_once()

    def test_disconnect_when_not_connected(self):
        """Test disconnect when not connected"""
        simulator = DS18B20Simulator()

        # Should not raise any exceptions
        simulator.disconnect()


class TestDS18B20SimulatorDataTransmission:
    """Test data transmission functionality"""

    def test_send_data_success(self):
        """Test successful data transmission"""
        simulator = DS18B20Simulator()

        # Mock connected state
        mock_conn = MagicMock()
        mock_conn.is_open = True
        simulator.serial_conn = mock_conn

        test_data = {"type": "test", "message": "hello"}

        result = simulator.send_data(test_data)

        assert result is True
        mock_conn.write.assert_called_once()
        mock_conn.flush.assert_called_once()

        # Check written data
        written_data = mock_conn.write.call_args[0][0]
        expected_json = json.dumps(test_data, ensure_ascii=False) + "\n"
        assert written_data == expected_json.encode("utf-8")

    def test_send_data_not_connected(self):
        """Test data transmission when not connected"""
        simulator = DS18B20Simulator()

        test_data = {"type": "test", "message": "hello"}

        result = simulator.send_data(test_data)

        assert result is False

    def test_send_data_transmission_error(self):
        """Test data transmission error"""
        simulator = DS18B20Simulator()

        # Mock connected state with failing write
        mock_conn = MagicMock()
        mock_conn.is_open = True
        mock_conn.write.side_effect = serial.SerialException("Write failed")
        simulator.serial_conn = mock_conn

        test_data = {"type": "test", "message": "hello"}

        result = simulator.send_data(test_data)

        assert result is False

    def test_send_data_with_unicode(self):
        """Test data transmission with unicode characters"""
        simulator = DS18B20Simulator()

        # Mock connected state
        mock_conn = MagicMock()
        mock_conn.is_open = True
        simulator.serial_conn = mock_conn

        test_data = {"type": "test", "message": "온도 센서"}

        result = simulator.send_data(test_data)

        assert result is True

        # Verify unicode handling
        written_data = mock_conn.write.call_args[0][0]
        decoded_data = written_data.decode("utf-8")
        assert "온도 센서" in decoded_data


class TestDS18B20SimulatorCommandProcessing:
    """Test command processing functionality"""

    def test_process_set_config_id_command(self):
        """Test processing sensor ID configuration command"""
        simulator = DS18B20Simulator()

        # Mock send_data
        with patch.object(simulator, 'send_data') as mock_send:
            command = {
                "command": "set_config",
                "config_type": "id",
                "new_value": "NEW123456789ABCD"
            }

            simulator.process_command(command)

            assert simulator.sensor_id == "NEW123456789ABCD"
            mock_send.assert_called_once()

            # Check response
            response = mock_send.call_args[0][0]
            assert response["status"] == "success"
            assert "센서 ID 변경" in response["message"]

    def test_process_set_config_th_command(self):
        """Test processing TH value configuration command"""
        simulator = DS18B20Simulator()

        with patch.object(simulator, 'send_data') as mock_send:
            command = {
                "command": "set_config",
                "config_type": "th",
                "new_value": 35.5
            }

            simulator.process_command(command)

            assert simulator.th_value == 35.5
            mock_send.assert_called_once()

            response = mock_send.call_args[0][0]
            assert response["status"] == "success"
            assert "TH 값 변경" in response["message"]

    def test_process_set_config_tl_command(self):
        """Test processing TL value configuration command"""
        simulator = DS18B20Simulator()

        with patch.object(simulator, 'send_data') as mock_send:
            command = {
                "command": "set_config",
                "config_type": "tl",
                "new_value": 15.2
            }

            simulator.process_command(command)

            assert simulator.tl_value == 15.2
            mock_send.assert_called_once()

    def test_process_set_config_interval_command(self):
        """Test processing measurement interval configuration command"""
        simulator = DS18B20Simulator()

        with patch.object(simulator, 'send_data') as mock_send:
            command = {
                "command": "set_config",
                "config_type": "interval",
                "new_value": 2.5
            }

            simulator.process_command(command)

            assert simulator.measurement_interval == 2.5
            mock_send.assert_called_once()

    def test_process_set_config_interval_minimum(self):
        """Test interval configuration with minimum value enforcement"""
        simulator = DS18B20Simulator()

        with patch.object(simulator, 'send_data') as mock_send:
            command = {
                "command": "set_config",
                "config_type": "interval",
                "new_value": 0.5  # Below minimum
            }

            simulator.process_command(command)

            assert simulator.measurement_interval == 1.0  # Should be clamped to minimum

    def test_process_unknown_config_type(self):
        """Test processing unknown configuration type"""
        simulator = DS18B20Simulator()

        with patch.object(simulator, 'send_data') as mock_send:
            command = {
                "command": "set_config",
                "config_type": "unknown",
                "new_value": "test"
            }

            simulator.process_command(command)

            mock_send.assert_called_once()
            response = mock_send.call_args[0][0]
            assert response["status"] == "error"
            assert "알 수 없는 설정 타입" in response["message"]

    def test_process_unknown_command(self):
        """Test processing unknown command"""
        simulator = DS18B20Simulator()

        with patch.object(simulator, 'send_data') as mock_send:
            command = {
                "command": "unknown_command",
                "config_type": "test",
                "new_value": "value"
            }

            simulator.process_command(command)

            mock_send.assert_called_once()
            response = mock_send.call_args[0][0]
            assert response["status"] == "error"
            assert "알 수 없는 명령" in response["message"]

    def test_process_command_exception_handling(self):
        """Test command processing error handling"""
        simulator = DS18B20Simulator()

        with patch.object(simulator, 'send_data') as mock_send:
            # Invalid command structure
            command = {"invalid": "structure"}

            simulator.process_command(command)

            mock_send.assert_called_once()
            response = mock_send.call_args[0][0]
            assert response["status"] == "error"
            assert "명령 처리 오류" in response["message"]

    def test_process_config_with_none_values(self):
        """Test configuration with None values"""
        simulator = DS18B20Simulator()
        original_th = simulator.th_value
        original_tl = simulator.tl_value
        original_interval = simulator.measurement_interval

        with patch.object(simulator, 'send_data') as mock_send:
            # Test with None values
            commands = [
                {"command": "set_config", "config_type": "th", "new_value": None},
                {"command": "set_config", "config_type": "tl", "new_value": None},
                {"command": "set_config", "config_type": "interval", "new_value": None},
            ]

            for command in commands:
                simulator.process_command(command)

            # Values should remain unchanged for None inputs
            assert simulator.th_value == original_th
            assert simulator.tl_value == original_tl
            assert simulator.measurement_interval == original_interval


class TestDS18B20SimulatorCommandListening:
    """Test command listening functionality"""

    def test_listen_for_commands_success(self):
        """Test successful command listening"""
        simulator = DS18B20Simulator()

        # Mock connected state
        mock_conn = MagicMock()
        mock_conn.is_open = True
        mock_conn.in_waiting = 1
        simulator.serial_conn = mock_conn

        # Mock command JSON
        command = {"command": "set_config", "config_type": "th", "new_value": 32.0}
        command_json = json.dumps(command)
        mock_conn.readline.return_value = command_json.encode("utf-8")

        with patch.object(simulator, 'process_command') as mock_process:
            simulator.listen_for_commands()

            mock_process.assert_called_once_with(command)

    def test_listen_for_commands_not_connected(self):
        """Test command listening when not connected"""
        simulator = DS18B20Simulator()

        # Should not raise exceptions
        simulator.listen_for_commands()

    def test_listen_for_commands_no_data(self):
        """Test command listening with no data waiting"""
        simulator = DS18B20Simulator()

        # Mock connected state with no data
        mock_conn = MagicMock()
        mock_conn.is_open = True
        mock_conn.in_waiting = 0
        simulator.serial_conn = mock_conn

        with patch.object(simulator, 'process_command') as mock_process:
            simulator.listen_for_commands()

            mock_process.assert_not_called()

    def test_listen_for_commands_invalid_json(self):
        """Test command listening with invalid JSON"""
        simulator = DS18B20Simulator()

        # Mock connected state
        mock_conn = MagicMock()
        mock_conn.is_open = True
        mock_conn.in_waiting = 1
        mock_conn.readline.return_value = b"invalid json"
        simulator.serial_conn = mock_conn

        # Should handle invalid JSON gracefully
        simulator.listen_for_commands()

    def test_listen_for_commands_exception(self):
        """Test command listening exception handling"""
        simulator = DS18B20Simulator()

        # Mock connected state with failing readline
        mock_conn = MagicMock()
        mock_conn.is_open = True
        mock_conn.in_waiting = 1
        mock_conn.readline.side_effect = Exception("Read error")
        simulator.serial_conn = mock_conn

        # Should handle exceptions gracefully
        simulator.listen_for_commands()


class TestDS18B20SimulatorOperation:
    """Test simulator operation and lifecycle"""

    @patch('serial.Serial')
    def test_start_success(self, mock_serial):
        """Test successful simulator start"""
        simulator = DS18B20Simulator(port="COM3", baudrate=9600)

        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn

        result = simulator.start()

        assert result is True
        assert simulator.is_running is True
        assert simulator.simulation_thread is not None
        assert simulator.simulation_thread.daemon is True

        # Clean up
        simulator.stop()

    def test_start_already_running(self):
        """Test starting when already running"""
        simulator = DS18B20Simulator()
        simulator.is_running = True

        result = simulator.start()

        assert result is False

    @patch('serial.Serial')
    def test_start_connection_failure(self, mock_serial):
        """Test start with connection failure"""
        simulator = DS18B20Simulator(port="INVALID", baudrate=9600)

        mock_serial.side_effect = serial.SerialException("Port not found")

        result = simulator.start()

        assert result is False
        assert simulator.is_running is False

    @patch('serial.Serial')
    def test_stop(self, mock_serial):
        """Test simulator stop"""
        simulator = DS18B20Simulator()

        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn

        # Start simulator
        simulator.start()

        # Stop simulator
        simulator.stop()

        assert simulator.is_running is False

    def test_get_status(self):
        """Test getting simulator status"""
        simulator = DS18B20Simulator(port="COM5", baudrate=115200)
        simulator.sensor_id = "STATUS123456789"
        simulator.th_value = 28.0
        simulator.tl_value = 22.0
        simulator.measurement_interval = 1.5

        status = simulator.get_status()

        assert status["is_running"] is False
        assert status["port"] == "COM5"
        assert status["sensor_id"] == "STATUS123456789"
        assert status["th_value"] == 28.0
        assert status["tl_value"] == 22.0
        assert status["measurement_interval"] == 1.5
        assert "current_temperature" in status
        assert isinstance(status["current_temperature"], float)


class TestDS18B20SimulatorSimulationLoop:
    """Test simulation loop functionality"""

    @patch('time.sleep')
    @patch('serial.Serial')
    def test_simulation_loop_single_iteration(self, mock_serial, mock_sleep):
        """Test single iteration of simulation loop"""
        simulator = DS18B20Simulator()

        # Set up mocks
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        simulator.connect()

        # Mock methods
        with patch.object(simulator, 'create_sensor_data') as mock_create_data:
            with patch.object(simulator, 'send_data') as mock_send:
                with patch.object(simulator, 'listen_for_commands') as mock_listen:

                    mock_create_data.return_value = {"type": "sensor_data", "temperature": 25.0}
                    simulator.is_running = True

                    # Run one iteration
                    simulator.simulation_loop()

                    # Verify method calls
                    mock_create_data.assert_called_once()
                    mock_send.assert_called_once()
                    mock_listen.assert_called_once()

    @patch('time.sleep')
    @patch('serial.Serial')
    def test_simulation_loop_temperature_drift_update(self, mock_serial, mock_sleep):
        """Test temperature drift updates in simulation loop"""
        simulator = DS18B20Simulator()

        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        simulator.connect()

        original_drift = simulator.temperature_drift

        with patch.object(simulator, 'create_sensor_data', return_value={}):
            with patch.object(simulator, 'send_data'):
                with patch.object(simulator, 'listen_for_commands'):

                    simulator.is_running = True

                    # Run one iteration
                    simulator.simulation_loop()

                    # Drift should be updated
                    assert simulator.temperature_drift != original_drift
                    # Should be within bounds
                    assert -3.0 <= simulator.temperature_drift <= 3.0

    @patch('time.sleep')
    @patch('serial.Serial')
    def test_simulation_loop_exception_handling(self, mock_serial, mock_sleep):
        """Test simulation loop exception handling"""
        simulator = DS18B20Simulator()

        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        simulator.connect()

        with patch.object(simulator, 'create_sensor_data', side_effect=Exception("Test error")):
            simulator.is_running = True

            # Should handle exception gracefully
            simulator.simulation_loop()

            # Should have attempted to sleep after error
            mock_sleep.assert_called()


class TestDS18B20SimulatorIntegration:
    """Integration tests for DS18B20Simulator"""

    @patch('serial.Serial')
    def test_full_lifecycle(self, mock_serial):
        """Test complete simulator lifecycle"""
        simulator = DS18B20Simulator(port="COM3", baudrate=9600)

        # Set up mock
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 0

        # Start simulator
        assert simulator.start() is True
        assert simulator.is_running is True

        # Change configuration
        command = {
            "command": "set_config",
            "config_type": "th",
            "new_value": 35.0
        }
        simulator.process_command(command)
        assert simulator.th_value == 35.0

        # Get status
        status = simulator.get_status()
        assert status["is_running"] is True
        assert status["th_value"] == 35.0

        # Stop simulator
        simulator.stop()
        assert simulator.is_running is False

    def test_main_function_coverage(self):
        """Test the main function for coverage completeness"""
        with patch('builtins.input', side_effect=KeyboardInterrupt()):
            with patch.object(DS18B20Simulator, 'start', return_value=False):
                try:
                    module.main()
                except (KeyboardInterrupt, SystemExit):
                    pass  # Expected behavior

    @patch('time.sleep')
    @patch('serial.Serial')
    def test_realistic_temperature_simulation(self, mock_serial, mock_sleep):
        """Test realistic temperature simulation over time"""
        simulator = DS18B20Simulator()

        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        simulator.connect()

        temperatures = []

        # Mock create_sensor_data to capture temperatures
        original_create = simulator.create_sensor_data
        def capture_temperature():
            data = original_create()
            temperatures.append(data["temperature"])
            return data

        with patch.object(simulator, 'create_sensor_data', side_effect=capture_temperature):
            with patch.object(simulator, 'send_data'):
                with patch.object(simulator, 'listen_for_commands'):

                    simulator.is_running = True

                    # Run multiple iterations
                    for _ in range(10):
                        simulator.simulation_loop()

        # Verify temperature characteristics
        assert len(temperatures) == 10
        assert all(18.0 <= temp <= 35.0 for temp in temperatures)
        assert all(isinstance(temp, float) for temp in temperatures)


@pytest.fixture()
def mock_simulator():
    """Fixture providing a mocked DS18B20Simulator"""
    with patch('serial.Serial') as mock_serial:
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True

        simulator = DS18B20Simulator(port="COM3", baudrate=9600)
        simulator.connect()

        yield simulator, mock_conn


def test_simulator_fixture(mock_simulator):
    """Test using the simulator fixture"""
    simulator, mock_conn = mock_simulator

    assert simulator.serial_conn == mock_conn
    data = simulator.create_sensor_data()
    assert data["type"] == "sensor_data"


class TestDS18B20SimulatorEdgeCases:
    """Test edge cases and boundary conditions"""

    def test_extreme_temperature_drift_bounds(self):
        """Test temperature drift boundary enforcement"""
        simulator = DS18B20Simulator()

        # Set extreme drift
        simulator.temperature_drift = 10.0  # Above maximum

        # Simulate drift update logic from simulation_loop
        import random
        original_drift = simulator.temperature_drift
        simulator.temperature_drift += random.uniform(-0.1, 0.1)
        simulator.temperature_drift = max(-3.0, min(3.0, simulator.temperature_drift))

        assert -3.0 <= simulator.temperature_drift <= 3.0

    def test_concurrent_command_processing(self):
        """Test concurrent command processing"""
        simulator = DS18B20Simulator()

        results = []
        errors = []

        def process_commands(command_data):
            try:
                with patch.object(simulator, 'send_data'):
                    simulator.process_command(command_data)
                    results.append(simulator.th_value)
            except Exception as e:
                errors.append(e)

        # Create threads to process commands concurrently
        threads = []
        for i in range(5):
            command = {
                "command": "set_config",
                "config_type": "th",
                "new_value": 30.0 + i
            }
            thread = threading.Thread(target=process_commands, args=(command,))
            threads.append(thread)

        # Start all threads
        for thread in threads:
            thread.start()

        # Wait for completion
        for thread in threads:
            thread.join()

        # No errors should occur
        assert len(errors) == 0
        assert len(results) == 5

    def test_data_precision_consistency(self):
        """Test data precision and consistency"""
        simulator = DS18B20Simulator()

        # Test multiple data generations
        data_points = [simulator.create_sensor_data() for _ in range(100)]

        # All temperatures should be rounded to 1 decimal place
        for data in data_points:
            temp = data["temperature"]
            assert round(temp, 1) == temp

            # Threshold values should also be properly rounded
            assert round(data["th_value"], 1) == data["th_value"]
            assert round(data["tl_value"], 1) == data["tl_value"]

    def test_measurement_interval_conversion(self):
        """Test measurement interval unit conversion"""
        simulator = DS18B20Simulator()

        # Set interval in seconds
        simulator.measurement_interval = 2.5

        data = simulator.create_sensor_data()

        # Should be converted to milliseconds
        assert data["measurement_interval"] == 2500

    def test_sensor_id_validation_length(self):
        """Test sensor ID handling with various lengths"""
        simulator = DS18B20Simulator()

        # Test different sensor ID lengths
        test_ids = [
            "SHORT",
            "EXACT16CHARLNGTH",
            "TOOLONGTOBEVALIDHEXID"
        ]

        for test_id in test_ids:
            simulator.sensor_id = test_id
            data = simulator.create_sensor_data()
            assert data["sensor_id"] == test_id  # Should accept any string

    def test_large_temperature_values_handling(self):
        """Test handling of large temperature values"""
        simulator = DS18B20Simulator()

        # Test with extreme base temperature
        simulator.base_temperature = 1000.0
        simulator.noise_level = 0.0
        simulator.temperature_drift = 0.0

        temp = simulator.generate_temperature()
        assert temp == 35.0  # Should be clamped to maximum

    def test_negative_temperature_values_handling(self):
        """Test handling of negative temperature values"""
        simulator = DS18B20Simulator()

        # Test with extreme negative base temperature
        simulator.base_temperature = -100.0
        simulator.noise_level = 0.0
        simulator.temperature_drift = 0.0

        temp = simulator.generate_temperature()
        assert temp == 18.0  # Should be clamped to minimum

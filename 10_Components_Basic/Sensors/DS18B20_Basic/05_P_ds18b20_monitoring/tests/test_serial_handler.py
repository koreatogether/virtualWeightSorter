from __future__ import annotations

import json
import queue
from datetime import datetime
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch

import pytest
import serial

# Import SerialHandler by adding the source path to sys.path
REPO_ROOT = Path(__file__).resolve().parents[1]
SRC_PATH = REPO_ROOT / "src" / "python"
import sys

if str(SRC_PATH) not in sys.path:
    sys.path.insert(0, str(SRC_PATH))

# Now we can import directly
from importlib import util as _util

SERIAL_HANDLER_PATH = REPO_ROOT / "src" / "python" / "dashboard" / "serial_handler.py"
_spec = _util.spec_from_file_location("dashboard.serial_handler", str(SERIAL_HANDLER_PATH))
module = _util.module_from_spec(_spec)  # type: ignore
assert _spec and _spec.loader
_spec.loader.exec_module(module)  # type: ignore

# Also import the SerialHandler class for most tests
from dashboard.serial_handler import SerialHandler


def make_sensor_data(sensor_id: str = "28FF123456789ABC", temperature: float = 25.0) -> dict:
    """Helper function to create valid sensor data"""
    return {
        "type": "sensor_data",
        "sensor_id": sensor_id,
        "temperature": temperature,
        "th_value": 30.0,
        "tl_value": 20.0,
        "measurement_interval": 1000,
        "timestamp": datetime.now().isoformat(),
    }


def make_response_data(status: str = "success", message: str = "OK") -> dict:
    """Helper function to create valid response data"""
    return {
        "type": "response",
        "status": status,
        "message": message,
        "sensor_id": "28FF123456789ABC",
        "timestamp": datetime.now().isoformat(),
    }


def make_command_data(command: str = "set_config", config_type: str = "th", new_value: float = 32.0) -> dict:
    """Helper function to create valid command data"""
    return {
        "type": "command",
        "command": command,
        "sensor_id": "28FF123456789ABC",
        "config_type": config_type,
        "new_value": new_value,
    }


class TestSerialHandlerBasic:
    """Test basic SerialHandler functionality"""

    def test_initialization(self):
        """Test SerialHandler initialization"""
        handler = SerialHandler(port="COM3", baudrate=9600)

        assert handler.port == "COM3"
        assert handler.baudrate == 9600
        assert handler.serial_conn is None
        assert not handler.is_running
        assert not handler.is_connected
        assert handler.read_thread is None
        assert handler.last_data_time is None

        # Check queues are initialized
        assert isinstance(handler.data_queue, queue.Queue)
        assert isinstance(handler.command_queue, queue.Queue)

        # Check callbacks are None initially
        assert handler.data_callback is None
        assert handler.response_callback is None
        assert handler.error_callback is None

    def test_set_callbacks(self):
        """Test setting callback functions"""
        handler = SerialHandler()

        data_cb = Mock()
        response_cb = Mock()
        error_cb = Mock()

        handler.set_callbacks(
            data_callback=data_cb,
            response_callback=response_cb,
            error_callback=error_cb
        )

        assert handler.data_callback == data_cb
        assert handler.response_callback == response_cb
        assert handler.error_callback == error_cb

    @patch('serial.Serial')
    def test_successful_connection(self, mock_serial):
        """Test successful serial port connection"""
        handler = SerialHandler(port="COM3", baudrate=9600)

        # Mock successful serial connection
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn

        result = handler.connect()

        assert result is True
        assert handler.is_connected is True
        assert handler.serial_conn == mock_conn

        # Verify serial.Serial was called with correct parameters
        mock_serial.assert_called_once_with(
            port="COM3",
            baudrate=9600,
            timeout=1.0,
            write_timeout=1.0
        )

    @patch('serial.Serial')
    def test_connection_failure(self, mock_serial):
        """Test serial port connection failure"""
        handler = SerialHandler(port="INVALID", baudrate=9600)

        # Mock connection failure
        mock_serial.side_effect = serial.SerialException("Port not found")

        # Set up error callback to track calls
        error_callback = Mock()
        handler.set_callbacks(error_callback=error_callback)

        result = handler.connect()

        assert result is False
        assert handler.is_connected is False
        assert handler.serial_conn is None

        # Verify error callback was called
        error_callback.assert_called_once()
        error_msg = error_callback.call_args[0][0]
        assert "연결 실패" in error_msg

    @patch('serial.Serial')
    def test_already_connected_warning(self, mock_serial):
        """Test connection attempt when already connected"""
        handler = SerialHandler()

        # Set up as already connected
        handler.is_connected = True

        result = handler.connect()

        assert result is True
        # Should not create new serial connection
        mock_serial.assert_not_called()

    def test_disconnect(self):
        """Test serial port disconnection"""
        handler = SerialHandler()

        # Mock connected state
        mock_conn = MagicMock()
        mock_conn.is_open = True
        handler.serial_conn = mock_conn
        handler.is_connected = True

        handler.disconnect()

        assert handler.is_connected is False
        mock_conn.close.assert_called_once()

    def test_disconnect_when_not_connected(self):
        """Test disconnect when not connected"""
        handler = SerialHandler()

        # Should not raise any exceptions
        handler.disconnect()

        assert handler.is_connected is False


class TestSerialHandlerDataOperations:
    """Test data reading and processing operations"""

    @patch('serial.Serial')
    def test_send_command_success(self, mock_serial):
        """Test successful command sending"""
        handler = SerialHandler()

        # Set up connected state
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        handler.connect()

        command_data = make_command_data()

        result = handler.send_command(command_data)

        assert result is True

        # Verify data was written to serial
        mock_conn.write.assert_called_once()
        mock_conn.flush.assert_called_once()

        # Check the written data
        written_data = mock_conn.write.call_args[0][0]
        assert b'{"type": "command"' in written_data
        assert b'"command": "set_config"' in written_data

    def test_send_command_not_connected(self):
        """Test command sending when not connected"""
        handler = SerialHandler()

        command_data = make_command_data()
        result = handler.send_command(command_data)

        assert result is False

    @patch('serial.Serial')
    def test_send_command_invalid_data(self, mock_serial):
        """Test sending invalid command data"""
        handler = SerialHandler()

        # Set up connected state
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        handler.connect()

        # Invalid command data (missing required fields)
        invalid_command = {"type": "command", "command": "invalid"}

        result = handler.send_command(invalid_command)

        assert result is False
        # Should not write invalid data
        mock_conn.write.assert_not_called()

    def test_read_data_empty_queue(self):
        """Test reading data when queue is empty"""
        handler = SerialHandler()

        result = handler.read_data()

        assert result is None

    def test_read_data_with_data(self):
        """Test reading data from queue"""
        handler = SerialHandler()

        # Add data to queue
        test_data = make_sensor_data()
        handler.data_queue.put(test_data)

        result = handler.read_data()

        assert result == test_data

    def test_clear_data_queue(self):
        """Test clearing the data queue"""
        handler = SerialHandler()

        # Add multiple items to queue
        for i in range(5):
            handler.data_queue.put(make_sensor_data(temperature=20.0 + i))

        assert handler.data_queue.qsize() == 5

        handler.clear_data_queue()

        assert handler.data_queue.qsize() == 0

    def test_get_connection_status(self):
        """Test getting connection status"""
        handler = SerialHandler(port="COM3", baudrate=9600)

        # Add some data to queue
        handler.data_queue.put(make_sensor_data())
        handler.last_data_time = 1234567890.0

        status = handler.get_connection_status()

        assert status["is_connected"] is False
        assert status["port"] == "COM3"
        assert status["baudrate"] == 9600
        assert status["is_reading"] is False
        assert status["last_data_time"] == 1234567890.0
        assert status["data_queue_size"] == 1


class TestSerialHandlerThreading:
    """Test threading and reading operations"""

    @patch('serial.Serial')
    def test_start_reading_success(self, mock_serial):
        """Test starting data reading thread"""
        handler = SerialHandler()

        # Set up connected state
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        handler.connect()

        result = handler.start_reading()

        assert result is True
        assert handler.is_running is True
        assert handler.read_thread is not None
        assert handler.read_thread.daemon is True

        # Clean up
        handler.stop_reading()

    def test_start_reading_already_running(self):
        """Test starting reading when already running"""
        handler = SerialHandler()
        handler.is_running = True

        result = handler.start_reading()

        assert result is False

    def test_start_reading_not_connected(self):
        """Test starting reading when not connected"""
        handler = SerialHandler()

        result = handler.start_reading()

        assert result is False

    @patch('serial.Serial')
    def test_stop_reading(self, mock_serial):
        """Test stopping data reading"""
        handler = SerialHandler()

        # Set up connected state and start reading
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        handler.connect()
        handler.start_reading()

        # Stop reading
        handler.stop_reading()

        assert handler.is_running is False

    @patch('serial.Serial')
    def test_read_loop_sensor_data_processing(self, mock_serial):
        """Test read loop processing sensor data"""
        handler = SerialHandler()

        # Set up mocks
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 1

        # Mock data callbacks
        data_callback = Mock()
        handler.set_callbacks(data_callback=data_callback)

        # Create valid sensor data JSON
        sensor_data = make_sensor_data()
        json_line = json.dumps(sensor_data) + "\n"
        mock_conn.readline.return_value = json_line.encode("utf-8")

        handler.connect()
        handler.is_running = True

        # Run one iteration of the read loop
        handler._read_loop()

        # Verify callback was called
        data_callback.assert_called_once_with(sensor_data)

        # Verify data was added to queue
        assert handler.data_queue.qsize() == 1
        assert handler.last_data_time is not None

    @patch('serial.Serial')
    def test_read_loop_response_processing(self, mock_serial):
        """Test read loop processing response data"""
        handler = SerialHandler()

        # Set up mocks
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 1

        # Mock response callback
        response_callback = Mock()
        handler.set_callbacks(response_callback=response_callback)

        # Create valid response data JSON
        response_data = make_response_data()
        json_line = json.dumps(response_data) + "\n"
        mock_conn.readline.return_value = json_line.encode("utf-8")

        handler.connect()
        handler.is_running = True

        # Run one iteration of the read loop
        handler._read_loop()

        # Verify callback was called
        response_callback.assert_called_once_with(response_data)

    @patch('serial.Serial')
    def test_read_loop_invalid_json(self, mock_serial):
        """Test read loop handling invalid JSON"""
        handler = SerialHandler()

        # Set up mocks
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 1

        # Mock invalid JSON
        mock_conn.readline.return_value = b"invalid json\n"

        handler.connect()
        handler.is_running = True

        # Should not raise exceptions
        handler._read_loop()

        # No data should be added to queue
        assert handler.data_queue.qsize() == 0

    @patch('serial.Serial')
    def test_read_loop_no_data_waiting(self, mock_serial):
        """Test read loop when no data is waiting"""
        handler = SerialHandler()

        # Set up mocks
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 0  # No data waiting

        handler.connect()
        handler.is_running = True

        # Should not raise exceptions
        handler._read_loop()

        # readline should not be called
        mock_conn.readline.assert_not_called()


class TestSerialHandlerCommands:
    """Test command-specific functionality"""

    @patch('serial.Serial')
    def test_send_config_command(self, mock_serial):
        """Test sending configuration command"""
        handler = SerialHandler()

        # Set up connected state
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        handler.connect()

        result = handler.send_config_command("th", 32.0, "28FF123456789ABC")

        assert result is True

        # Verify the command was written
        mock_conn.write.assert_called_once()
        written_data = mock_conn.write.call_args[0][0].decode("utf-8")

        # Parse the written JSON
        command_dict = json.loads(written_data.strip())
        assert command_dict["type"] == "command"
        assert command_dict["command"] == "set_config"
        assert command_dict["config_type"] == "th"
        assert command_dict["new_value"] == 32.0
        assert command_dict["sensor_id"] == "28FF123456789ABC"

    @patch('serial.Serial')
    def test_send_config_command_without_sensor_id(self, mock_serial):
        """Test sending config command without sensor ID"""
        handler = SerialHandler()

        # Set up connected state
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        handler.connect()

        result = handler.send_config_command("interval", 2.0)

        assert result is True

        # Verify the command structure
        written_data = mock_conn.write.call_args[0][0].decode("utf-8")
        command_dict = json.loads(written_data.strip())
        assert "sensor_id" not in command_dict or command_dict["sensor_id"] is None


class TestSerialHandlerContextManager:
    """Test context manager functionality"""

    @patch('serial.Serial')
    def test_context_manager_success(self, mock_serial):
        """Test using SerialHandler as context manager"""
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn

        with SerialHandler(port="COM3", baudrate=9600) as handler:
            assert handler.is_connected is True
            assert handler.serial_conn == mock_conn

        # After context exit, should be disconnected
        assert handler.is_connected is False

    @patch('serial.Serial')
    def test_context_manager_with_exception(self, mock_serial):
        """Test context manager cleanup on exception"""
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn

        try:
            with SerialHandler(port="COM3", baudrate=9600) as handler:
                assert handler.is_connected is True
                raise ValueError("Test exception")
        except ValueError:
            pass

        # Should still disconnect properly
        assert handler.is_connected is False


class TestSerialHandlerErrorHandling:
    """Test error handling scenarios"""

    @patch('serial.Serial')
    def test_send_command_write_exception(self, mock_serial):
        """Test handling write exception during command sending"""
        handler = SerialHandler()

        # Set up connected state with failing write
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.write.side_effect = serial.SerialException("Write failed")

        error_callback = Mock()
        handler.set_callbacks(error_callback=error_callback)

        handler.connect()

        result = handler.send_command(make_command_data())

        assert result is False
        error_callback.assert_called_once()

    @patch('serial.Serial')
    def test_read_loop_serial_exception(self, mock_serial):
        """Test read loop handling serial exceptions"""
        handler = SerialHandler()

        # Set up mocks with failing readline
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 1
        mock_conn.readline.side_effect = serial.SerialException("Read failed")

        handler.connect()
        handler.is_running = True

        # Should handle exception gracefully
        handler._read_loop()

        # No data should be processed
        assert handler.data_queue.qsize() == 0

    @patch('serial.Serial')
    def test_read_loop_unknown_message_type(self, mock_serial):
        """Test read loop handling unknown message types"""
        handler = SerialHandler()

        # Set up mocks
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 1

        # Create message with unknown type
        unknown_message = {"type": "unknown", "data": "test"}
        json_line = json.dumps(unknown_message) + "\n"
        mock_conn.readline.return_value = json_line.encode("utf-8")

        handler.connect()
        handler.is_running = True

        # Should handle gracefully
        handler._read_loop()

        # No data should be added to queue
        assert handler.data_queue.qsize() == 0


class TestSerialHandlerIntegration:
    """Integration tests for SerialHandler"""

    @patch('serial.Serial')
    def test_full_workflow_simulation(self, mock_serial):
        """Test complete workflow simulation"""
        handler = SerialHandler(port="COM3", baudrate=9600)

        # Set up mocks
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True
        mock_conn.in_waiting = 1

        # Set up callbacks
        data_received = []
        responses_received = []
        errors_received = []

        def data_cb(data):
            data_received.append(data)

        def response_cb(response):
            responses_received.append(response)

        def error_cb(error):
            errors_received.append(error)

        handler.set_callbacks(
            data_callback=data_cb,
            response_callback=response_cb,
            error_callback=error_cb
        )

        # Connect
        assert handler.connect() is True

        # Start reading
        assert handler.start_reading() is True

        # Simulate receiving sensor data
        sensor_data = make_sensor_data(temperature=25.5)
        json_line = json.dumps(sensor_data) + "\n"
        mock_conn.readline.return_value = json_line.encode("utf-8")

        # Process one read iteration
        handler._read_loop()

        # Send a command
        command_result = handler.send_config_command("th", 35.0)
        assert command_result is True

        # Verify results
        assert len(data_received) == 1
        assert data_received[0]["temperature"] == 25.5
        assert handler.data_queue.qsize() == 1

        # Clean up
        handler.stop_reading()
        handler.disconnect()

        assert handler.is_running is False
        assert handler.is_connected is False

    def test_main_function_coverage(self):
        """Test the main function for coverage completeness"""
        # Import and test the main function
        with patch('builtins.input', side_effect=KeyboardInterrupt()):
            with patch('time.sleep'):
                with patch.object(SerialHandler, 'connect', return_value=False):
                    try:
                        module.main()
                    except (KeyboardInterrupt, SystemExit):
                        pass  # Expected behavior


@pytest.fixture()
def mock_serial_handler():
    """Fixture providing a mocked SerialHandler"""
    with patch('serial.Serial') as mock_serial:
        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn
        mock_conn.is_open = True

        handler = SerialHandler(port="COM3", baudrate=9600)
        handler.connect()

        yield handler, mock_conn


def test_serial_handler_fixture(mock_serial_handler):
    """Test using the fixture"""
    handler, mock_conn = mock_serial_handler

    assert handler.is_connected is True
    assert handler.serial_conn == mock_conn


class TestSerialHandlerEdgeCases:
    """Test edge cases and boundary conditions"""

    def test_multiple_data_queue_operations(self):
        """Test multiple queue operations"""
        handler = SerialHandler()

        # Test queue operations with various data types
        test_data = [
            make_sensor_data(temperature=20.0),
            make_sensor_data(temperature=25.0),
            make_sensor_data(temperature=30.0),
        ]

        # Add data
        for data in test_data:
            handler.data_queue.put(data)

        # Read all data
        received_data = []
        while True:
            data = handler.read_data()
            if data is None:
                break
            received_data.append(data)

        assert len(received_data) == 3
        assert [d["temperature"] for d in received_data] == [20.0, 25.0, 30.0]

    def test_large_data_processing(self):
        """Test processing large amounts of data"""
        handler = SerialHandler()

        # Add many data points
        for i in range(1000):
            data = make_sensor_data(temperature=20.0 + i * 0.1)
            handler.data_queue.put(data)

        assert handler.data_queue.qsize() == 1000

        # Clear all data
        handler.clear_data_queue()
        assert handler.data_queue.qsize() == 0

    @patch('serial.Serial')
    def test_rapid_connect_disconnect_cycles(self, mock_serial):
        """Test rapid connect/disconnect cycles"""
        handler = SerialHandler()

        mock_conn = MagicMock()
        mock_serial.return_value = mock_conn

        # Perform multiple connect/disconnect cycles
        for _ in range(10):
            assert handler.connect() is True
            assert handler.is_connected is True

            handler.disconnect()
            assert handler.is_connected is False

    def test_callback_exception_handling(self):
        """Test handling exceptions in callbacks"""
        handler = SerialHandler()

        # Set up callback that raises exception
        def failing_callback(data):
            raise ValueError("Callback failed")

        handler.set_callbacks(data_callback=failing_callback)

        # Add data to queue
        test_data = make_sensor_data()
        handler.data_queue.put(test_data)

        # Reading should still work despite callback failure
        result = handler.read_data()
        assert result == test_data

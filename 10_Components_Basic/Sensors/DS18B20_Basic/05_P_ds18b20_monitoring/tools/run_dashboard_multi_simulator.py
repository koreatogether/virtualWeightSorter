#!/usr/bin/env python3
"""Run the Dash dashboard with the multi-sensor offline simulator enabled.

This script enables 5 DS18B20 sensors simulation and runs the dashboard
to test the dynamic UI generation and sensor callback functionality.
"""

import contextlib
import sys
from pathlib import Path

# Add src/python to path so imports like `dashboard` and `simulator` work
ROOT = Path(__file__).resolve().parents[1]
SRC_PY = ROOT / "src" / "python"
if str(SRC_PY) not in sys.path:
    sys.path.insert(0, str(SRC_PY))

# Import dashboard app and multi-sensor simulator
try:
    from dashboard.app import app, dashboard_state, data_manager, log_manager
    from simulator.multi_sensor_simulator import MultiSensorDS18B20Simulator
except Exception as e:
    print("Failed to import dashboard or simulator modules:", e)
    raise

# Create and configure multi-sensor simulator
multi_sim = MultiSensorDS18B20Simulator()


# Set up data callback to feed data into dashboard
def on_sensor_data_received(data):
    """다중 센서 데이터를 data_manager에 전달"""
    try:
        data_manager.add_sensor_data(data)
        print(f"Sensor data: {data['sensor_addr'][-4:]} - {data['temperature']:.1f}°C")
    except Exception as e:
        print(f"Error processing sensor data: {e}")


multi_sim.set_data_callback(on_sensor_data_received)

# Enable simulator mode
dashboard_state.simulator_mode = True
dashboard_state.simulator_instance = multi_sim

# Start the multi-sensor simulation
multi_sim.start_simulation()

# Add a startup log entry
with contextlib.suppress(Exception):
    log_manager.add_log(
        f"다중 센서 시뮬레이터 시작 ({multi_sim.get_sensor_count()}개 센서)", "info"
    )

print(f"Starting Dash app with {multi_sim.get_sensor_count()} sensor simulation...")
print("Dashboard available at: http://127.0.0.1:8050/")
print("Expected features:")
print("- 5개 센서가 자동으로 감지됨")
print("- 각 센서별 개별 UI 섹션 생성")
print("- 각 센서별 버튼 콜백 함수 작동")
print("- 통합 그래프에서 5개 센서 비교")
print("- 동적 센서 수 변화 대응")

# Run the Dash app (blocking)
try:
    app.run(debug=False, host="127.0.0.1", port=8050)
finally:
    multi_sim.stop_simulation()
    print("Multi-sensor simulation stopped.")

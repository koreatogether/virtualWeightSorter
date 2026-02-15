#!/usr/bin/env python3
"""Run the Dash dashboard with the built-in offline simulator enabled.

This script adjusts sys.path so `dashboard` and `simulator` packages under
`src/python` are importable, enables the OfflineDS18B20Simulator, and then
runs the Dash app.
"""

import contextlib
import sys
from pathlib import Path

# Add src/python to path so imports like `dashboard` and `simulator` work
ROOT = Path(__file__).resolve().parents[1]
SRC_PY = ROOT / "src" / "python"
if str(SRC_PY) not in sys.path:
    sys.path.insert(0, str(SRC_PY))

# Import dashboard app and simulator
try:
    from dashboard.app import app, dashboard_state, log_manager
    from simulator.offline_simulator import OfflineDS18B20Simulator
except Exception as e:
    print("Failed to import dashboard or simulator modules:", e)
    raise

# Enable simulator mode and attach an instance
dashboard_state.simulator_mode = True
sim = OfflineDS18B20Simulator()
sim.is_running = True
dashboard_state.simulator_instance = sim

# Add a startup log entry
with contextlib.suppress(Exception):
    log_manager.add_log("자동 시뮬레이터 시작", "info")

print("Starting Dash app with offline simulator enabled...")
# Run the Dash app (blocking)
app.run(debug=False, host="127.0.0.1", port=8050)
